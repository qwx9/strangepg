/* fucking hell; can't be macros because of function pointer use */

#ifdef isalnum
static int
xisalnum(int c)
{
	return isalnum(c);
}
#else
#define	xisalnum(c)	isalnum(c)
#endif

#ifdef isalpha
static int
xisalpha(int c)
{
	return isalpha(c);
}
#else
#define	xisalpha(c)	isalpha(c)
#endif

#ifdef iscntrl
static int
xiscntrl(int c)
{
	return iscntrl(c);
}
#else
#define	xiscntrl(c)	iscntrl(c)
#endif

#ifdef isdigit
static int
xisdigit(int c)
{
	return isdigit(c);
}
#else
#define	xisdigit(c)	isdigit(c)
#endif

#ifdef isgraph
static int
xisgraph(int c)
{
	return isgraph(c);
}
#else
#define	xisgraph(c)	isgraph(c)
#endif

#ifdef islower
static int
xislower(int c)
{
	return islower(c);
}
#else
#define	xislower(c)	islower(c)
#endif

#ifdef isprint
static int
xisprint(int c)
{
	return isprint(c);
}
#else
#define	xisprint(c)	isprint(c)
#endif

#ifdef ispunct
static int
xispunct(int c)
{
	return ispunct(c);
}
#else
#define	xispunct(c)	ispunct(c)
#endif

#ifdef isspace
static int
xisspace(int c)
{
	return isspace(c);
}
#else
#define	xisspace(c)	isspace(c)
#endif

#ifdef isupper
static int
xisupper(int c)
{
	return isupper(c);
}
#else
#define	xisupper(c)	isupper(c)
#endif

#ifdef isxdigit
static int
xisxdigit(int c)
{
	return isxdigit(c);
}
#else
#define	xisxdigit(c)	isxdigit(c)
#endif
