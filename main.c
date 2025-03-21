#include <stdio.h>

#include "libzetc.h"

typedef long (*syscall_fn_t)(long, long, long, long, long, long, long);

static syscall_fn_t next_sys_call = NULL;

static long hook_function(long a1, long a2, long a3,
			  long a4, long a5, long a6,
			  long a7)
{
	//printf("output from hook_function: syscall number %ld\n", a1);
	switch (a1) {
	case 2: // OPEN
		a2 = (long) translate ((char *)a2);
		break;
	case 257: // OPENAT
	case 262: // NEWFSTATAT
		a3 = (long) translate ((char *)a3);
		break;
	default:
		//printf("UNKNOWN %ld", a1);
		break;
	}
	return next_sys_call(a1, a2, a3, a4, a5, a6, a7);
}

int __hook_init(long placeholder __attribute__((unused)),
		void *sys_call_hook_ptr)
{
	start_up();

	next_sys_call = *((syscall_fn_t *) sys_call_hook_ptr);
	*((syscall_fn_t *) sys_call_hook_ptr) = hook_function;

	return 0;
}
