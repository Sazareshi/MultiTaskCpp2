#pragma once
#include "CTaskObj.h"
#include <time.h>
#include "CVector3.h"
#include "CRK4.h"
#include "openGL.h"
#include "CComDevice.h"
#include "CManager.h"


extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
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
	void set_panel_tip_txt();//タブパネルのStaticテキストを設定
	void routine_work(void *param);

	void check_console();

	void init_task(void *pobj);

						 
// Open GL
	static int GL_WindowID[OpenGL_MAX_WND];
	static int list;
	
	static ST_GL_BASIC st_gl_basic; // OpenGL 基本構造体
	static GLint VP_mode; // Veiw Point 切り替え用
	 
	void ActOpenGL();
	static void GL_Initialize(void);
	static void GL_Display(void);
	static void GL_Ground(void); //大地の描画Keyboard
	static void GL_resize(int w, int h);//Window size変更時処理　int view port w, int view port h


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


	static void drowCuboid(double a, double b, double c,     // x軸方向の幅, y軸方向の幅, z軸方向の幅,
		double x, double y, double z,     // 中心のx座標, 中心のy座標, 中心のz座標, 
		MaterialStruct color);            // 色
	static void drowCuboid(double a, double b, double c,     // x軸方向の幅, y軸方向の幅, z軸方向の幅,
		double x, double y, double z,     // 中心のx座標, 中心のy座標, 中心のz座標, 
		MaterialStruct color,             // 色,
		double theta,                     // 回転角度, 
		double nx, double ny, double nz); // 回転軸x座標, 回転軸y座標, 回転軸z標

};

