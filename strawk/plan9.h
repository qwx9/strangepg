//#define __DJGPP__
#include <bsd.h>

#define __attribute__(x)
typedef size_t wint_t;

wint_t	towupper(wint_t);
wint_t	towlower(wint_t);

#define signbit(x) (x == -0.0 || x < 0.0)
