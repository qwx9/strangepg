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
};
extern View view;

#define	ZV	HMM_V3(0.0f, 0.0f, 0.0f)

void	zoomdraw(float, float, float);
void	pandraw(float, float);
void	worldview(HMM_Vec3);
void	rotzview(float, float, float, float);
void	moveview(float, float);
