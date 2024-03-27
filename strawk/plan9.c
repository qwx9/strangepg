#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "awk.h"

// FIXME: fix or remove

wint_t
towupper(wint_t r)
{
	//return toupperrune(r);
	return toupper(r);
}

wint_t
towlower(wint_t r)
{
	//return tolowerrune(r);
	return tolower(r);
}
