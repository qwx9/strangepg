#include "strpg.h"

static Vec *
checksize(Vec *v)
{
	assert(v->len <= v->bufsz);
	if(v->len + 1 < v->bufsz)
		return v;
	v->buf = erealloc(v->buf, v->bufsz * v->elsz * 2, v->bufsz * v->elsz);
	v->bufsz *= 2;
	return v;
}

void
vecnuke(Vec *v)
{
	if(v == nil)
		return;
	free(v->buf);
}

usize
vecindexof(Vec *v, void *p)
{
	uintptr x;

	assert(v != nil && p >= v->buf && p < (uchar *)v->buf + v->elsz * v->len);
	x = (uchar *)p - (uchar *)v->buf;
	if(x % v->elsz != 0)
		panic("vecindexof: invalid pointer");
	return x / v->elsz;
}

void *
vecget(Vec *v, usize i)
{
	void *n;

	if(i >= v->len || v->len == 0){
		werrstr("vecget %#p[%zd]: out of bounds", v, i);
		return nil;
	}
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
vecp(Vec *v, usize i)
{
	void *p;

	p = (uchar *)v->buf + v->elsz * i;
	assert(p >= v->buf && p < (uchar *)v->buf + v->elsz * v->len);
	return p;
}

void *
vecpush(Vec *v, void *p, usize *ip)
{
	usize i;
	uchar *t;

	assert((uintptr)p > v->elsz);
	checksize(v);
	i = v->elsz * v->len;
	t = (uchar *)v->buf + i;
	memcpy(t, p, v->elsz);
	v->len++;
	if(ip != nil)
		*ip = i / v->elsz;
	return (void *)t;
}

Vec
vec(usize elsz)
{
	Vec v;

	assert(elsz > 0);
	memset(&v, 0, sizeof v);
	v.elsz = elsz;
	v.len = 0;
	v.bufsz = 8;
	v.buf = emalloc(elsz * v.bufsz);
	return v;
}
