#include "strpg.h"
#include "../lib/HandmadeMath.h"
#include "drw.h"
#include "view.h"

View view;

void
updateview(void)
{
	HMM_Mat4 vw, proj;

	view.Δeye = HMM_SubV3(view.eye, view.center);
	view.zoom = HMM_Len(view.Δeye);
	view.ar = (float)view.w / view.h;
	proj = HMM_Perspective_RH_NO(view.fov, view.ar, 0.01f, 10000.0f);
	vw = HMM_LookAt_RH(view.eye, view.center, view.up);
	view.mvp = HMM_MulM4(proj, vw);
	drawstate |= DSstalepick | DSmoving;
	reqdraw(Reqrefresh);
}

void
worldpandraw(float x, float y)
{
	updateview();
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
		updateview();
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
		updateview();
	}
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
	}else{
		view.center = v;
		d = HMM_MulV3F(view.front, 10.0f);
		v = HMM_SubV3(v, d);
		view.eye = v;
		view.Δeye = d;
		zoomdraw(-0.1f, 0.0f, 0.0f);
	}
	updateview();
}

void
moveview(float Δx, float Δy)
{
	HMM_Vec3 v, d;

	Δx *= HMM_DegToRad;
	Δy *= HMM_DegToRad;
	v = HMM_AddV3(HMM_MulV3F(view.front, Δy), HMM_MulV3F(view.right, Δx));
	v = HMM_AddV3(view.center, v);

	view.center = v;
	d = HMM_MulV3F(view.front, 10.0f);
	v = HMM_SubV3(v, d);
	view.eye = v;
	view.Δeye = d;
	zoomdraw(-0.1f, 0.0f, 0.0f);
	updateview();
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
initview(void)
{
	if(drawing.flags & DFnope)
		return;
	view.fov = 45.0f;
	view.tfov = tanf(view.fov / 2);
	resetview();
}
