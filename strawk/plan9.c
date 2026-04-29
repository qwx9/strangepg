#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <wctype.h>
#include "awk.h"

wint_t
towupper(wint_t r)
{
	return toupperrune(r);
}

wint_t
towlower(wint_t r)
{
	return tolowerrune(r);
}

int
wctomb(char *s, wchar_t wc)
{
	return runetochar(s, &wc);
}

void
catchfpe(void)
{
}
