#include "strpg.h"
#include "../lib/HandmadeMath.h"
#include "drw.h"
#include "view.h"

View view = {
	.fov = 45.0f,
	.ar = 1920.0f / 1080.0f,	/* dummy */
};

void
expandbb(AABB *a, AABB *b)
{
	int i;

	for(i=0; i<3; i++){
		if(a->tl.p[i] > b->tl.p[i])
			a->tl.p[i] = b->tl.p[i];
		if(a->br.p[i] < b->br.p[i])
			a->br.p[i] = b->br.p[i];
	}
}

void
intersectbb(AABB *a, AABB *b)
{
	int i;

	for(i=0; i<3; i++){
		if(a->tl.p[i] < b->tl.p[i])
			a->tl.p[i] = b->tl.p[i];
		if(a->br.p[i] > b->br.p[i])
			a->br.p[i] = b->br.p[i];
	}
}

static inline void
updateviewbb(void)
{
	AABB b;
	HAABB hb;
	HMM_Vec3 pp[] = {
		HMM_V3(-1.0f, +1.0f, +1.0f),
		HMM_V3(+1.0f, +1.0f, +1.0f),
		HMM_V3(+1.0f, -1.0f, -1.0f),
		HMM_V3(-1.0f, -1.0f, -1.0f),
		/*
		HMM_V3(-1.0f, -1.0f, -1.0f),
		HMM_V3(-1.0f, -1.0f, +1.0f),
		HMM_V3(-1.0f, +1.0f, -1.0f),
		HMM_V3(-1.0f, +1.0f, +1.0f),
		HMM_V3(+1.0f, -1.0f, -1.0f),
		HMM_V3(+1.0f, -1.0f, +1.0f),
		HMM_V3(+1.0f, +1.0f, -1.0f),
		HMM_V3(+1.0f, +1.0f, +1.0f),
		*/
	}, *p;

	/* FIXME: easier way? two points only? */
	for(p=pp; p<pp+nelem(pp); p++){
		hb.TL.X = p->X * view.zoom * view.ar * view.tfov;
		hb.TL.Y = p->Y * view.zoom * view.tfov;
		hb.TL.Z = p->Z * view.zoom;
		hb.TL = HMM_RotateV3Q(hb.TL, view.rot);
		hb.TL = HMM_AddV3(hb.TL, view.center);
		hb.BR = hb.TL;
		if(p != pp)
			expandbb(&b, &hb.b);
		else
			b = hb.b;
	}
	DPRINT(Debugbih, "vwbb %.2f,%.2f,%.2f %.2f,%.2f,%.2f",
		b.tl.x, b.tl.y, b.tl.z, b.br.x, b.br.y, b.br.z);
	view.bb = b;
}

void
updateview(void)
{
	HMM_Mat4 vw, proj;

	view.Δeye = HMM_SubV3(view.eye, view.center);
	view.zoom = HMM_LenV3(view.Δeye);
	proj = HMM_Perspective_RH_NO(view.fov, view.ar, Near, Far);
	vw = HMM_LookAt_RH(view.eye, view.center, view.up);
	view.mvp = HMM_MulM4(proj, vw);
	drawstate |= DSstalepick | DSmoving;
	updateviewbb();
}

void
pandraw(float Δx, float Δy)
{
	HMM_Quat q;
	HMM_Vec3 o;

	if((drawing.flags & DF3d) == 0){
		/* FIXME: redundancy with scr2world and ui code */
		Δx /= view.w;
		Δy /= view.h;
		Δx *= 2.0f * view.Δeye.Z * view.ar * view.tfov;
		Δy *= 2.0f * view.Δeye.Z * view.tfov;
		o = HMM_RotateV3Q(HMM_V3(Δx, -Δy, 0.0f), view.rot);
		view.center = HMM_AddV3(view.center, o);
		view.eye = HMM_AddV3(view.eye, o);
	}else{
		Δx *= HMM_DegToRad * 0.4f;
		Δy *= HMM_DegToRad * 0.4f;
		o = view.center;
		q = HMM_QFromAxisAngle_RH(view.up, Δx);
		q = HMM_MulQ(HMM_QFromAxisAngle_RH(view.right, Δy), q);
		view.rot = HMM_MulQ(q, view.rot);
		view.eye = HMM_AddV3(HMM_RotateV3Q(HMM_SubV3(view.eye, o), q), o);
		view.right = HMM_RotateV3Q(view.right, q);
		view.up = HMM_RotateV3Q(view.up, q);
		view.front = HMM_RotateV3Q(view.front, q);
	}
	updateview();
	reqdraw(Reqrefresh);
}

void
worldview(HMM_Vec3 v)
{
	HMM_Vec3 d;

	if((drawing.flags & DF3d) == 0){
		v.Z += 10.0f;
		view.eye = v;
		view.center.X = v.X;
		view.center.Y = v.Y;
		updateview();
		reqdraw(Reqrefresh);
	}else{
		view.center = v;
		d = HMM_MulV3F(view.front, 10.0f);
		view.eye = HMM_AddV3(HMM_RotateV3Q(v, view.rot), d);
		view.Δeye = HMM_SubV3(view.eye, view.center);
		zoomdraw(-0.1f, 0.0f, 0.0f);
	}
}

void
moveview(float Δx, float Δy)
{
	float d;
	HMM_Vec3 v;

	d = HMM_LenV3(view.Δeye);
	Δx /= view.w;
	Δy /= view.h;
	Δx *= 2.0f * d * view.ar * view.tfov;
	Δy *= 2.0f * d * view.tfov;
	v = HMM_AddV3(HMM_MulV3F(view.right, -Δx), HMM_MulV3F(view.front, -Δy));
	view.eye = HMM_AddV3(view.eye, v);
	view.center = HMM_AddV3(view.center, v);
	updateview();
	reqdraw(Reqrefresh);
}

/* FIXME: angle only a hack, not really correct */
void
rotzview(float x, float y, float Δx, float Δy)
{
	float Δ;
	HMM_Vec3 o;
	HMM_Quat q;

	if(x < 0.5f * view.w)
		Δy = -Δy;
	if(y < 0.5f * view.h)
		Δx = -Δx;
	Δ = -0.5f * (Δx - Δy);
	Δ *= HMM_DegToRad;
	q = HMM_QFromAxisAngle_RH(view.front, Δ);
	view.rot = HMM_MulQ(q, view.rot);
	o = view.Δeye;
	view.eye = HMM_AddV3(HMM_RotateV3Q(HMM_SubV3(view.eye, o), q), o);
	view.center = HMM_AddV3(HMM_RotateV3Q(HMM_SubV3(view.center, o), q), o);
	view.right = HMM_RotateV3Q(view.right, q);
	view.up = HMM_RotateV3Q(view.up, q);
	updateview();
	reqdraw(Reqrefresh);
}

void
zoomdraw(float Δ, float Δx, float Δy)
{
	HMM_Vec3 v;

	if((drawing.flags & DF3d) == 0){
		Δ *= view.eye.Z;
		view.eye.Z -= Δ;
		pandraw(Δx, Δy);
	}else{
		v = HMM_MulV3F(view.Δeye, Δ);
		view.eye = HMM_SubV3(view.eye, v);
		updateview();
		reqdraw(Reqrefresh);
	}
}

void
resetview(void)
{
	view.center = ZV;
	view.zoom = 100.0f;
	view.eye = HMM_V3(0.0f, 0.0f, view.zoom);
	view.Δeye = HMM_SubV3(view.eye, view.center);
	view.up = HMM_V3(0.0f, 1.0f, 0.0f);
	view.right = HMM_V3(1.0f, 0.0f, 0.0f);
	view.front = HMM_V3(0.0f, 0.0f, 1.0f);
	view.rot = HMM_Q(0.0f, 0.0f, 0.0f, 1.0f);
}

void
initscreen(int w, int h)
{
	view.w = w;
	view.h = h;
	view.ar = (float)w / h;
	updateview();
}

void
initview(void)
{
	if(drawing.flags & DFnope)
		return;
	view.tfov = tanf(view.fov / 2);
	resetview();
}
