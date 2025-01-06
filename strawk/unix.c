#include <stdio.h>
#include <stddef.h>
#include <signal.h>
#include "awk.h"

static noreturn void fpecatch(int n
#ifdef SA_SIGINFO
	, siginfo_t *si, void *uc
#endif
)
{
#ifdef SA_SIGINFO
	const char *mesg = NULL;

	switch (si->si_code) {
	case FPE_INTDIV:
		mesg = "Integer divide by zero";
		break;
	case FPE_INTOVF:
		mesg = "Integer overflow";
		break;
	case FPE_FLTDIV:
		mesg = "Floating point divide by zero";
		break;
	case FPE_FLTOVF:
		mesg = "Floating point overflow";
		break;
	case FPE_FLTUND:
		mesg = "Floating point underflow";
		break;
	case FPE_FLTRES:
		mesg = "Floating point inexact result";
		break;
	case FPE_FLTINV:
		mesg = "Invalid Floating point operation";
		break;
	case FPE_FLTSUB:
		mesg = "Subscript out of range";
		break;
	case 0:
	default:
		mesg = "Unknown error";
		break;
	}
#endif
	FATAL("floating point exception"
#ifdef SA_SIGINFO
		": %s", mesg
#endif
	    );
}

void
catchfpe(void)
{
#ifdef SA_SIGINFO
	{
		struct sigaction sa;
		sa.sa_sigaction = fpecatch;
		sa.sa_flags = SA_SIGINFO;
		sigemptyset(&sa.sa_mask);
		(void)sigaction(SIGFPE, &sa, NULL);
	}
#else
	(void)signal(SIGFPE, fpecatch);
#endif
}
