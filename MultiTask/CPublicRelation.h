#pragma once
#include "CTaskObj.h"
#include <time.h>
#include "CVector3.h"
#include "CRK4.h"
#include "openGL.h"

extern vector<void*>	VectpCTaskObj;	//�^�X�N�I�u�W�F�N�g�̃|�C���^
extern ST_iTask g_itask;

class CPublicRelation :
	public CTaskObj
{
public:
	CPublicRelation();
	~CPublicRelation();

	ST_GL_BASIC st_gl_basic; // OpenGL ��{�\����


	LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
	void set_panel_tip_txt();//�^�u�p�l����Static�e�L�X�g��ݒ�

	void ActOpenGL();
};

