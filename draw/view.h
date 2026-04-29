typedef union HAABB HAABB;

/* glue */
union HAABB{
	AABB b;
	struct{
		HMM_Vec3 TL;
		HMM_Vec3 BR;
	};
};

typedef struct View View;
struct View{
	int w;
	int h;
	float ar;
	float fov;
	float tfov;
	HMM_Vec3 eye;
	HMM_Vec3 center;
	HMM_Vec3 up;
	HMM_Vec3 right;
	HMM_Vec3 front;
	HMM_Vec3 Δeye;
	double zoom;
	HMM_Mat4 mvp;
	HMM_Quat rot;
	AABB bb;
};
extern View view;

/* FIXME: typical values are from 0.1 to 100; must keep them as
 * close together as possible to avoid imprecision, frustrum is
 * mapped into a clipping volume 2 units deep */
#define	Near	0.01f
#define	Far		50000.0f

#define	ZV	HMM_V3(0.0f, 0.0f, 0.0f)

void	expandbb(AABB*, AABB*);
void	intersectbb(AABB*, AABB*);

void	updateview(void);
void	zoomdraw(float, float, float);
void	pandraw(float, float);
void	worldview(HMM_Vec3);
void	rotzview(float, float, float, float);
void	moveview(float, float);

static inline HMM_Vec3
s2w(HMM_Vec2 s)
{
	s.X = 2.0f * (s.X + 0.5f) / view.w - 1.0f;
	s.Y = 2.0f * (s.Y + 0.5f) / view.h - 1.0f;
	s.X *= view.zoom * view.ar * view.tfov;
	s.Y *= view.zoom * view.tfov;
	return HMM_AddV3(HMM_RotateV3Q(HMM_V3(s.X, -s.Y, 0.0f), view.rot), view.center);
}

static inline HMM_Vec3
d2w(HMM_Vec2 s)
{
	return HMM_NormV3(HMM_SubV3(s2w(s), view.eye));
}
