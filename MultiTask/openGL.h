#pragma once

//OpenGL
#include <glut.h>
#include <gl_screenshot.h> 

typedef struct _gl_basic {
	int WinPosX;			//��������E�B���h�E�ʒu��X���W
	int WinPosY;			//��������E�B���h�E�ʒu��Y���W
	int WinWidth;			//��������E�B���h�E�̕�
	int WinHeight;			//��������E�B���h�E�̍���
	char WindowTitle[128];	// = "���E�̎n�܂�"; //�E�B���h�E�̃^�C�g��

	bool _Bitmap;
	int tn;
	double t;
	double dt;
	double omega;
	gl_screenshot gs;		//bmp�t�@�C���̏o��
	double PI;

	bool bGLactive;

	//----------------------------------------------------
	// ���_�̒�`
	//----------------------------------------------------
	Vector3 ViewPoint;
}ST_GL_BASIC, *P_ST_GL_BASIC;

