#include "strpg.h"
#include "../lib/HandmadeMath.h"
#include "drw.h"
#include "view.h"

/* FIXME: consolidate with rest of ui view and draw.c code */

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

	if((drawing.flags & DF3d) == 0){
		/* FIXME: redundancy with scr2world and ui code */
		Δx /= view.w;
		Δy /= view.h;
		Δx *= 2.0f * view.Δeye.Z * view.ar * view.tfov;
		Δy *= 2.0f * view.Δeye.Z * view.tfov;
		view.center.X += Δx;
		view.center.Y -= Δy;
		view.eye.X += Δx;
		view.eye.Y -= Δy;
		updateview();
	}else{
		Δx *= HMM_DegToRad * 0.4f;
		Δy *= HMM_DegToRad * 0.4f;
		q = HMM_QFromAxisAngle_RH(view.up, Δx);
		q = HMM_MulQ(HMM_QFromAxisAngle_RH(view.right, Δy), q);
		view.rot = HMM_MulQ(q, view.rot);
		view.eye = HMM_RotateV3Q(view.eye, q);
		view.right = HMM_RotateV3Q(view.right, q);
		view.up = HMM_RotateV3Q(view.up, q);
		view.front = HMM_RotateV3Q(view.front, q);
		updateview();
	}
}

/* FIXME: pan/zoom in world coordinates; standardize this, make mvp global */
void
worldview(HMM_Vec3 v)
{
	view.eye = v;
	if((drawing.flags & DF3d) == 0){
		view.center.X = v.X;
		view.center.Y = v.Y;
	}
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
		v = HMM_MulV3F(view.eye, Δ);
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
