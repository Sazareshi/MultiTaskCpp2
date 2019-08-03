#pragma once
#include "CTaskObj.h"
#include <time.h>
#include "CVector3.h"
#include "CRK4.h"
#include "openGL.h"
#include "Mob.h"
#include "CComDevice.h"
#include "CManager.h"


extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

typedef struct st_PR_UIio{
	int notch_slew;
	int notch_bh;
	int env_mode;
}ST_PR_UIio, *P_ST_PR_UIio;

class CPublicRelation :
	public CTaskObj
{
public:
	CPublicRelation();
	~CPublicRelation();


	LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
	void set_panel_tip_txt();//タブパネルのStaticテキストを設定

	ST_PR_UIio ui_table;

// Mob
	Mob_HoistPoint  hp;
	Mob_HungLoad	hl;
							 
// Open GL
		
	static ST_GL_BASIC st_gl_basic; // OpenGL 基本構造体

	void ActOpenGL();
	static void GL_Initialize(void);
	static void GL_Display(void);
	static void GL_Ground(void); //大地の描画Keyboard


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

};

