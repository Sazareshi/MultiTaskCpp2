#pragma once

#include "CVector3.h"
//OpenGL
#include <glut.h>
#include <gl_screenshot.h> 
#include <freeglut.h> 

#define CAM_DIST_SIDE1	15.0	//カメラの吊点からの水平設置距離

typedef struct _gl_basic {
	int WinPosX;			//生成するウィンドウ位置のX座標
	int WinPosY;			//生成するウィンドウ位置のY座標
	int WinWidth;			//生成するウィンドウの幅
	int WinHeight;			//生成するウィンドウの高さ
	char WindowTitle[128];	//ウィンドウタイトル

	Vector3 ViewPoint;		// 視点の位置	
	Vector3 ViewCenter;		// 視野の中心位置
	Vector3 ViewUpside;		// 視野の上方向

	GLdouble fovy, aspect, zNear, zFar;

	bool _Bitmap;

	gl_screenshot gs;		//bmpファイルの出力

	bool bGLactive;

}ST_GL_BASIC, *P_ST_GL_BASIC;

//----------------------------------------------------
// 物質質感の定義
//----------------------------------------------------
struct MaterialStruct {
	GLfloat ambient[4];
	GLfloat diffuse[4];
	GLfloat specular[4];
	GLfloat shininess;
};

