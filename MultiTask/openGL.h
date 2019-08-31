#pragma once

#include "CVector3.h"
//OpenGL
#include <glut.h>
#include <gl_screenshot.h> 
#include <freeglut.h> 

#define CAM_DIST_SIDE1	15.0	//�J�����̒ݓ_����̐����ݒu����

typedef struct _gl_basic {
	int WinPosX;			//��������E�B���h�E�ʒu��X���W
	int WinPosY;			//��������E�B���h�E�ʒu��Y���W
	int WinWidth;			//��������E�B���h�E�̕�
	int WinHeight;			//��������E�B���h�E�̍���
	char WindowTitle[128];	//�E�B���h�E�^�C�g��

	Vector3 ViewPoint;		// ���_�̈ʒu	
	Vector3 ViewCenter;		// ����̒��S�ʒu
	Vector3 ViewUpside;		// ����̏����

	GLdouble fovy, aspect, zNear, zFar;

	bool _Bitmap;

	gl_screenshot gs;		//bmp�t�@�C���̏o��

	bool bGLactive;

}ST_GL_BASIC, *P_ST_GL_BASIC;

//----------------------------------------------------
// ���������̒�`
//----------------------------------------------------
struct MaterialStruct {
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat shininess;
};

