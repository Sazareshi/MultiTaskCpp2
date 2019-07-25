#pragma once
#include "CTaskObj.h"
#include <time.h>
#include "CVector3.h"
#include "CRK4.h"
#include "openGL.h"

extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

class CPublicRelation :
	public CTaskObj
{
public:
	CPublicRelation();
	~CPublicRelation();

	ST_GL_BASIC st_gl_basic; // OpenGL 基本構造体


	LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
	void set_panel_tip_txt();//タブパネルのStaticテキストを設定

	void ActOpenGL();
};

