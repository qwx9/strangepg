#include <stdio.h>
#include <stddef.h>
#include <signal.h>
#ifdef VERSION
#include "strpg.h"
#include "threads.h"
#endif
#include "awk.h"

enum
{
	Runeerror = 128, /* from somewhere else */
	Runemax = 0x10FFFF,

	Bit1    = 7,
	Bitx    = 6,
	Bit2    = 5,
	Bit3    = 4,
	Bit4    = 3,
	Bit5    = 2,

	T1      = ((1<<(Bit1+1))-1) ^ 0xFF,     /* 0000 0000 */
	Tx      = ((1<<(Bitx+1))-1) ^ 0xFF,     /* 1000 0000 */
	T2      = ((1<<(Bit2+1))-1) ^ 0xFF,     /* 1100 0000 */
	T3      = ((1<<(Bit3+1))-1) ^ 0xFF,     /* 1110 0000 */
	T4      = ((1<<(Bit4+1))-1) ^ 0xFF,     /* 1111 0000 */
	T5      = ((1<<(Bit5+1))-1) ^ 0xFF,     /* 1111 1000 */

	Rune1   = (1<<(Bit1+0*Bitx))-1,	 	/* 0000 0000 0000 0000 0111 1111 */
	Rune2   = (1<<(Bit2+1*Bitx))-1,	 	/* 0000 0000 0000 0111 1111 1111 */
	Rune3   = (1<<(Bit3+2*Bitx))-1,	 	/* 0000 0000 1111 1111 1111 1111 */
	Rune4   = (1<<(Bit4+3*Bitx))-1,	 	/* 0011 1111 1111 1111 1111 1111 */

	Maskx   = (1<<Bitx)-1,		  	/* 0011 1111 */
	Testx   = Maskx ^ 0xFF,		 	/* 1100 0000 */

};

int runetochar(char *str, unsigned int *cp)
{
	int c;

	c = *cp;
	/* one character sequence 00000-0007F => 00-7F */     
	if (c <= Rune1) {
		str[0] = c;
		return 1;
	}
	
	/* two character sequence 00080-007FF => T2 Tx */
	if (c <= Rune2) {
		str[0] = T2 | (c >> 1*Bitx);
		str[1] = Tx | (c & Maskx);
		return 2;
	}

	/* three character sequence 00800-0FFFF => T3 Tx Tx */
	if (c > Runemax)
		c = Runeerror;
	if (c <= Rune3) {
		str[0] = T3 |  (c >> 2*Bitx);
		str[1] = Tx | ((c >> 1*Bitx) & Maskx);
		str[2] = Tx |  (c & Maskx);
		return 3;
	}
	
	/* four character sequence 010000-1FFFFF => T4 Tx Tx Tx */
	str[0] = T4 |  (c >> 3*Bitx);
	str[1] = Tx | ((c >> 2*Bitx) & Maskx);
	str[2] = Tx | ((c >> 1*Bitx) & Maskx);
	str[3] = Tx |  (c & Maskx);
	return 4;
}               

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
