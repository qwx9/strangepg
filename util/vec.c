#include "strpg.h"

/* slower, defensive implementation */
enum{
	Shardsz = 4096,
};

static Vec *
checksize(Vec *v)
{
	assert(v->len <= v->bufsz);
	if(v->len + 1 < v->bufsz)
		return v;
	v = erealloc(v->buf,
		(v->bufsz + Shardsz) * v->elsz,
		v->bufsz * v->elsz);
	v->bufsz += Shardsz;
	return v;
}

void
vecnuke(Vec *v)
{
	if(v == nil)
		return;
	free(v->buf);
	free(v);
}

void *
vecget(Vec *v, usize i)
{
	void *n;

	assert(i < v->len && v->len > 0);
	n = (uchar*)v->buf + i * v->elsz;
	return n;
}

void *
vecpoptail(Vec *v)
{
	void *n;

	assert(v->len > 0);
	n = (uchar*)v->buf + (v->len - 1) * v->elsz;
	v->len--;
	return n;
}

void *
vecpush(Vec *v, void *p)
{
	uchar *t;

	assert((uintptr)p > v->elsz);
	v = checksize(v);
	t = (uchar *)v->buf + v->elsz * v->len;
	memcpy(t, p, v->elsz);
	v->len++;
	return (void *)t;
}

Vec
vec(usize len, usize elsz)
{
	Vec v = {0};

	assert(elsz > 0);
	v.elsz = elsz;
	v.len = 0;
	v.bufsz = len > 0 ? len : Shardsz;
	v.buf = emalloc(elsz * v.bufsz);
	return v;
}
