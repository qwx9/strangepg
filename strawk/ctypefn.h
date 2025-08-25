/* fucking hell; can't be macros because of function pointer use */

static int
xisalnum(int c)
{
	return isalnum(c);
}

static int
xisalpha(int c)
{
	return isalpha(c);
}

static int
xiscntrl(int c)
{
	return iscntrl(c);
}

static int
xisdigit(int c)
{
	return isdigit(c);
}

static int
xisgraph(int c)
{
	return isgraph(c);
}

static int
xislower(int c)
{
	return islower(c);
}

static int
xisprint(int c)
{
	return isprint(c);
}

static int
xispunct(int c)
{
	return ispunct(c);
}

static int
xisspace(int c)
{
	return isspace(c);
}

static int
xisupper(int c)
{
	return isupper(c);
}

static int
xisxdigit(int c)
{
	return isxdigit(c);
}
