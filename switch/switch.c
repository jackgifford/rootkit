#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>


static int
open_hook(struct thread *td, void *syscall_args)
{
	int error;
	struct open_args *uap = (struct open_args *)syscall_args;
	error = sys_open(td, syscall_args);

	char name[NAME_MAX];
	size_t size;

	if (copyinstr(uap->path, name, NAME_MAX, &size) == EFAULT)
		return EFAULT;

	printf("%s\n", name);
	return error;
}

static int
openat_hook(struct thread *td, void *syscall_args)
{
	int error;
	struct openat_args *uap;
	uap = (struct openat_args *)syscall_args;

	error = sys_openat(td, syscall_args);

	char name[NAME_MAX];
	size_t size;

	if (copyinstr(uap->path, name, NAME_MAX, &size) == EFAULT)
		return (EFAULT);
	
	printf("%s\n", name);

	return error;
}


static int load (struct module *module, int cmd, void *arg)
{
	int error = 0;
	switch(cmd)
	{
		case MOD_LOAD:
			sysent[SYS_openat].sy_call = (sy_call_t *)openat_hook;
			sysent[SYS_open].sy_call = (sy_call_t *)open_hook;
			break;
		case MOD_UNLOAD:
			sysent[SYS_openat].sy_call = (sy_call_t *)sys_openat;
			sysent[SYS_open].sy_call = (sy_call_t *)sys_open;
			break;
		default:
			error = EOPNOTSUPP;
			break;
	}	
	
	return error;
}

static moduledata_t read_hook_mod = {
	"switch",
	load,
	NULL
};

DECLARE_MODULE(switch, read_hook_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
