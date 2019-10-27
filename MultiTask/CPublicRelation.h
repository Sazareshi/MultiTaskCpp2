#pragma once
#include "CTaskObj.h"
#include <time.h>
#include "CVector3.h"
#include "CRK4.h"
#include "openGL.h"
#include "CComDevice.h"
#include "CManager.h"


extern vector<void*>	VectpCTaskObj;	//�^�X�N�I�u�W�F�N�g�̃|�C���^
extern ST_iTask g_itask;


#define OpenGL_MAX_WND	4


class CPublicRelation :
	public CTaskObj
{
public:
	CPublicRelation();
	~CPublicRelation();

	CPublicRelation* pPRObj;

	LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
	void set_panel_tip_txt();//�^�u�p�l����Static�e�L�X�g��ݒ�
	void routine_work(void *param);

	void check_console();

	void init_task(void *pobj);

						 
// Open GL
	static int GL_WindowID[OpenGL_MAX_WND];
	static int list;
	
	static ST_GL_BASIC st_gl_basic; // OpenGL ��{�\����
	static GLint VP_mode; // Veiw Point �؂�ւ��p
	 
	void ActOpenGL();
	static void GL_Initialize(void);
	static void GL_Display(void);
	static void GL_Ground(void); //��n�̕`��Keyboard
	static void GL_resize(int w, int h);//Window size�ύX�������@int view port w, int view port h


	static void GL_Idle();
	static void GL_Keyboard(unsigned char key, int x, int y);

	static void GL_findPlane(GLfloat plane[4], GLfloat v0[3], GLfloat v1[3], GLfloat v2[3]);
	static void GL_shadowMatrix(GLfloat *m, GLfloat plane[4], GLfloat light[4]);
	static void GL_DrawFloor(bool bTexture);
	static void GL_DrawShadow(void);
	static void GL_DrawStructure(bool);
	static void GL_DRAW_STRING(int x, int y, char *string, void *font);
	static void GL_DISPLAY_TEXT(int x, int y, char *string);

	static void GL_qmul(double r[], const double p[], const double q[]);
	static void GL_qrot(double r[], double q[]);
	static void GL_mouse_motion(int x, int y);
	static void GL_mouse_on(int button, int state, int x, int y);
	static void GL_mouse_wheel(int wheel_number, int direction, int x, int y);


	static void drowCuboid(double a, double b, double c,     // x�������̕�, y�������̕�, z�������̕�,
		double x, double y, double z,     // ���S��x���W, ���S��y���W, ���S��z���W, 
		MaterialStruct color);            // �F
	static void drowCuboid(double a, double b, double c,     // x�������̕�, y�������̕�, z�������̕�,
		double x, double y, double z,     // ���S��x���W, ���S��y���W, ���S��z���W, 
		MaterialStruct color,             // �F,
		double theta,                     // ��]�p�x, 
		double nx, double ny, double nz); // ��]��x���W, ��]��y���W, ��]��z�W

};

