#include "stdafx.h"
#include "CPublicRelation.h"
#include "helper.h"

extern CIO_Table*		pIO_Table;
extern CORDER_Table*	pOrder;				//共有メモリOrderクラスポインタ

//----------------------------------------------------
// 物質質感の定義
//----------------------------------------------------
//jade(翡翠)
MaterialStruct ms_jade = {
	{ 0.135,     0.2225,   0.1575,   1.0 },
	{ 0.54,      0.89,     0.63,     1.0 },
	{ 0.316228,  0.316228, 0.316228, 1.0 },
	12.8 };
//ruby(ルビー)
MaterialStruct ms_ruby = {
	{ 0.1745,   0.01175,  0.01175,   1.0 },
	{ 0.61424,  0.04136,  0.04136,   1.0 },
	{ 0.727811, 0.626959, 0.626959,  1.0 },
	76.8 };
//----------------------------------------------------
// 色の定義の定義
//----------------------------------------------------
GLfloat red[] = { 0.8, 0.2, 0.2, 1.0 }; //赤色
GLfloat green[] = { 0.2, 0.8, 0.2, 1.0 };//緑色
GLfloat blue[] = { 0.2, 0.2, 0.8, 1.0 };//青色
GLfloat yellow[] = { 0.8, 0.8, 0.2, 1.0 };//黄色
GLfloat white[] = { 1.0, 1.0, 1.0, 1.0 };//白色
GLfloat shininess = 30.0;//光沢の強さ
						 //-----------------------------------------


//#################### OPEN GL ################################################
int CPublicRelation::GL_WindowID[OpenGL_MAX_WND];
int CPublicRelation::list;
GLint CPublicRelation::VP_mode; // Veiw Point 切り替え用

void CPublicRelation::ActOpenGL() {

	GL_Initialize();			//このアプリケーションでの初期化
	
	glutInit(&__argc, __argv);//環境の初期化
	glutInitWindowPosition(st_gl_basic.WinPosX, st_gl_basic.WinPosY);//ウィンドウの位置の指定
	glutInitWindowSize(st_gl_basic.WinWidth, st_gl_basic.WinHeight); //ウィンドウサイズの指定
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);//ディスプレイモードの指定
	GL_WindowID[0]=glutCreateWindow(st_gl_basic.WindowTitle);		//ウィンドウの作成
	glutDisplayFunc(GL_Display);			//描画時に呼び出される関数を指定する（関数名：GL_Display）
	glutKeyboardFunc(GL_Keyboard);			//キーボード入力時に呼び出される関数を指定する（関数名：GL_Keyboard）
	
#if 0
	//2画面
	glutInitWindowPosition(0, 0);//ウィンドウの位置の指定
	GL_WindowID[1] = glutCreateWindow("Second Window");
	glutDisplayFunc(GL_Display);
#endif
	
	glutReshapeFunc(GL_resize);

	glutMouseFunc(GL_mouse_on);
	glutMouseWheelFunc(GL_mouse_wheel);
#if 0
	glutIdleFunc(Idle);					//プログラムアイドル状態時に呼び出される関数

	glutMouseWheelFunc(mouse_wheel);	//マウスホイール操作時に呼び出される関数
	Initialize();						//初期設定の関数を呼び出す

#endif
	st_gl_basic.bGLactive = TRUE;
	glutMainLoop();
	return;

}

void  CPublicRelation::GL_Initialize(void) {
	st_gl_basic.bGLactive = FALSE;

	st_gl_basic.WinPosX = 600;
	st_gl_basic.WinPosY = 100;
	st_gl_basic.WinWidth = 800;
	st_gl_basic.WinHeight = 600;

	snprintf(st_gl_basic.WindowTitle, 12, "%s", "Crame Model");

	st_gl_basic.ViewPoint.x = 10.0;
	st_gl_basic.ViewPoint.y = 50.0;
	st_gl_basic.ViewPoint.z = 100.0;

	st_gl_basic.VP_Offset.x = 0.0;
	st_gl_basic.VP_Offset.y = 0.0;
	st_gl_basic.VP_Offset.z = 0.0;


	st_gl_basic.ViewCenter.x = 0.0;
	st_gl_basic.ViewCenter.y = 0.0;
	st_gl_basic.ViewCenter.z = 20.0;

	st_gl_basic.ViewUpside.x = 0.0;
	st_gl_basic.ViewUpside.y = 0.0;
	st_gl_basic.ViewUpside.z = 1.0;

	st_gl_basic.fovy = 60.0;
	st_gl_basic.aspect = (double)st_gl_basic.WinWidth / (double)st_gl_basic.WinHeight;
	st_gl_basic.zNear = 0.1;
	st_gl_basic.zFar = 900.0;

	
	glClearColor(0.3, 0.3, 0.3, 1.0);	//ウィンドウを塗りつぶす　GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha
	glEnable(GL_DEPTH_TEST);			//デプスバッファを使用：glutInitDisplayMode() で GLUT_DEPTH を指定する
	//陰影ON-----------------------------
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);//光源0を利用

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);// (GLenum sfactor, GLenum dfactor)

}

void  CPublicRelation::GL_resize(int w, int h) {
	glViewport(0,0,w,h);	//GLint x, GLint y, GLsizei width, GLsizei height
	
	/*変換行列初期化*/
	glLoadIdentity();

	/*スクリーン上の表示領域をビューポートの大きさに比例させる*/
	glOrtho(-w/ st_gl_basic.WinWidth,w/st_gl_basic.WinWidth,-h/st_gl_basic.WinHeight,h/st_gl_basic.WinHeight,-1.0,1.0);// GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar
};


void CPublicRelation::GL_Display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //バッファの消去
	glClearColor(0.3, 0.3, 0.3, 1.0);

//透視変換行列の設定------------------------------
	glMatrixMode(GL_PROJECTION);	//GLenum mode 行列モードの設定（GL_PROJECTION : 透視変換行列の設定、GL_MODELVIEW：モデルビュー変換行列）
	glLoadIdentity();				//行列の初期化
	gluPerspective(														//透視投影法の視体積gluPerspactive(th, w/h, near, far);
		st_gl_basic.fovy,	//  GLdouble fovy, 
		st_gl_basic.aspect,	//	GLdouble aspect,
		st_gl_basic.zNear,	//	GLdouble zNear,
		st_gl_basic.zFar	//	GLdouble zFar
	);	
		
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

//視点（カメラ）の設定------------------------------
	if (VP_mode == 2) {
	 st_gl_basic.ViewPoint.x = pIO_Table->physics.cp.x;
	 st_gl_basic.ViewPoint.y = pIO_Table->physics.cp.y;
	 //st_gl_basic.ViewPoint.z += (10.0 + st_gl_basic.VP_Offset.z);
	 st_gl_basic.ViewCenter = pIO_Table->physics.cp;
	 st_gl_basic.ViewCenter.x += 0.1;
	 st_gl_basic.ViewCenter.z = 0.0;
	 st_gl_basic.ViewUpside.x = sin(pIO_Table->physics.th);
	 st_gl_basic.ViewUpside.y = cos(pIO_Table->physics.th);
	 st_gl_basic.ViewUpside.z = 0.0;
	}
	else if (VP_mode == 1) {
	 st_gl_basic.ViewPoint.x = (pIO_Table->physics.R - CAM_DIST_SIDE1)*sin(pIO_Table->physics.th);
	 st_gl_basic.ViewPoint.y = (pIO_Table->physics.R - CAM_DIST_SIDE1)*cos(pIO_Table->physics.th);
	 st_gl_basic.ViewPoint.z = 10.0;

	 st_gl_basic.ViewCenter.x = pIO_Table->physics.R*sin(pIO_Table->physics.th);
	 st_gl_basic.ViewCenter.y = pIO_Table->physics.R*cos(pIO_Table->physics.th);
	 st_gl_basic.ViewCenter.z = 10.0 + st_gl_basic.VP_Offset.z;

	 st_gl_basic.ViewUpside.x = 0.0;
	 st_gl_basic.ViewUpside.y = 0.0;
	 st_gl_basic.ViewUpside.z = 1.0;
	}
	else;

	gluLookAt(
		st_gl_basic.ViewPoint.x, st_gl_basic.ViewPoint.y, st_gl_basic.ViewPoint.z,//GLdouble eyex, eyey, eyez 視点の位置x,y,z;
		st_gl_basic.ViewCenter.x, st_gl_basic.ViewCenter.y, st_gl_basic.ViewCenter.z,  //視界の中心位置の参照点座標x,y,z
		st_gl_basic.ViewUpside.x, st_gl_basic.ViewUpside.y, st_gl_basic.ViewUpside.z
	); 		//GLdouble upx,upy, upz 視界の上方向のベクトルx,y,z


								
								//モデルビュー変換行列の設定--------------------------
//	glMatrixMode(GL_MODELVIEW);//GLenum mode 行列モードの設定（GL_PROJECTION : 透視変換行列の設定、GL_MODELVIEW：モデルビュー変換行列）
//	glLoadIdentity();//行列の初期化
//	glViewport(0, 0, st_gl_basic.WinWidth, st_gl_basic.WinHeight);





	//陰影OFF-----------------------------
	glDisable(GL_LIGHTING);
	//-----------------------------------
#if 0
#endif
	GL_Ground();

	//軸 描画
	glColor3d(0.0, 0.0, 1.0);		//色の設定
	glLineWidth(8.0);				//線幅
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 25.0);
	glEnd();

	//直方体
	drowCuboid(2.0, 2.0, 16.0, //x軸方向の幅, y軸方向の幅, z軸方向の幅,
		0.0, 0.0, 8.0,   //中心のx座標, 中心のy座標, 中心のz座標,
		ms_ruby,          //色,
		-pIO_Table->physics.th * COF_RAD2DEG,   //回転角度,
		0.0, 0.0, 1.0);   //回転軸x座標, 回転軸y座標, 回転軸z標


	//ロープ 描画
	glColor3d(1.0, 1.0, 1.0);		//色の設定
	glLineWidth(2.0);				//線幅
	glBegin(GL_LINES);
	glVertex3f(pIO_Table->physics.cp.x, pIO_Table->physics.cp.y, pIO_Table->physics.cp.z);
	glVertex3f(pIO_Table->physics.lp.x, pIO_Table->physics.lp.y, pIO_Table->physics.lp.z);
	glEnd();

	//球 描画
	glPushMatrix();
	glColor3d(1.0, 0.0, 0.0);				//色の設定
	glTranslated(pIO_Table->physics.lp.x, pIO_Table->physics.lp.y, pIO_Table->physics.lp.z);		//平行移動値の設定
	glutSolidSphere(1.0, 20, 20);//引数：(半径, Z軸まわりの分割数, Z軸に沿った分割数)
	glPopMatrix();

	//立方体

	glPushMatrix();
	GLfloat yellow[] = { 1.0, 1.0, 1.0, 1.0 };
	glTranslated(pIO_Table->physics.cp.x, pIO_Table->physics.cp.y, pIO_Table->physics.cp.z);//平行移動値の設定
	glRotatef(-pIO_Table->physics.th * COF_RAD2DEG, 0.0, 0.0, 1.0);
	glColor4d(0.0, 0.0, 1.0, 1.0);//色の設定
	glutSolidCube(0.1);//引数：(一辺の長さ)
	glPopMatrix();


	//Boom 描画
	glColor3d(0.0, 0.0, 1.0);		//色の設定
	glLineWidth(8.0);				//線幅
	glBegin(GL_LINES);
	glVertex3f(0.0, 0.0, 25.0);
	glVertex3f(pIO_Table->physics.cp.x, pIO_Table->physics.cp.y, pIO_Table->physics.cp.z);
	glEnd();


	//文字の描画
	char t_char[20];
	char t_char2[20];

	
	strcpy_s(t_char2, "L = ");
	sprintf_s(t_char, "%f", pIO_Table->physics.L);
	strcat_s(t_char2, t_char);
	GL_DISPLAY_TEXT(5, 93, t_char2);

	strcpy_s(t_char2, "th = ");
	sprintf_s(t_char, "%f", COF_RAD2DEG * pIO_Table->physics.th);
	strcat_s(t_char2, t_char);
	GL_DISPLAY_TEXT(5, 88, t_char2);

	strcpy_s(t_char2, "as_tg = ");
	sprintf_s(t_char, "%f", COF_RAD2DEG * pIO_Table->as_ctrl.tgpos_slew);
	strcat_s(t_char2, t_char);
	GL_DISPLAY_TEXT(28, 88, t_char2);

	strcpy_s(t_char2, "as_dir = ");
	sprintf_s(t_char, "%d", pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID]);
	strcat_s(t_char2, t_char);
	GL_DISPLAY_TEXT(52, 88, t_char2);


	strcpy_s(t_char2, "bm = ");
	sprintf_s(t_char, "%f", pIO_Table->physics.R);
	strcat_s(t_char2, t_char);
	GL_DISPLAY_TEXT(5, 83, t_char2);

	strcpy_s(t_char2, "as_tg = ");
	sprintf_s(t_char, "%f", pIO_Table->as_ctrl.tgpos_bh);
	strcat_s(t_char2, t_char);
	GL_DISPLAY_TEXT(28, 83, t_char2);

	strcpy_s(t_char2, "as_dir = ");
	sprintf_s(t_char, "%d", pIO_Table->as_ctrl.as_out_dir[AS_BH_ID]);
	strcat_s(t_char2, t_char);
	GL_DISPLAY_TEXT(52, 83, t_char2);



	glutSwapBuffers(); //glutInitDisplayMode(GLUT_DOUBLE)でダブルバッファリングを利用可
}

void CPublicRelation::GL_mouse_on(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_LEFT_BUTTON:
		break;
	case GLUT_MIDDLE_BUTTON:
		if(state == GLUT_UP)
			if (VP_mode++ > 2)
				VP_mode = 0;

		if (VP_mode == 1) {
/*
			st_gl_basic.fovy = 60.0;
			st_gl_basic.ViewPoint.x = 0.0;
			st_gl_basic.ViewPoint.y = 50.0;
			st_gl_basic.ViewPoint.z = 5.0;

			st_gl_basic.ViewCenter.x = 0.0;
			st_gl_basic.ViewCenter.y = 0.0;
			st_gl_basic.ViewCenter.z = 20.0;

			st_gl_basic.ViewUpside.x = 0.0;
			st_gl_basic.ViewUpside.y = 0.0;
			st_gl_basic.ViewUpside.z = 1.0;
*/
			double temp_R = pIO_Table->physics.R - CAM_DIST_SIDE1;
			if (temp_R < 1.0)temp_R = 1.0;
			st_gl_basic.ViewPoint.x = temp_R*sin(pIO_Table->physics.th);
			st_gl_basic.ViewPoint.y = temp_R*cos(pIO_Table->physics.th);
			st_gl_basic.ViewPoint.z += 10.0;

			st_gl_basic.ViewCenter.x = pIO_Table->physics.R*sin(pIO_Table->physics.th);
			st_gl_basic.ViewCenter.y = pIO_Table->physics.R*cos(pIO_Table->physics.th);
			st_gl_basic.ViewCenter.z = 10.0;

			st_gl_basic.ViewUpside.x = 0.0;
			st_gl_basic.ViewUpside.y = 0.0;
			st_gl_basic.ViewUpside.z = 1.0;

			//st_gl_basic.fovy = 120.0;

		}
		else if (VP_mode == 2) {
			st_gl_basic.ViewPoint = pIO_Table->physics.cp;
			st_gl_basic.ViewPoint.z += 10.0;

			st_gl_basic.ViewCenter = pIO_Table->physics.cp;
			st_gl_basic.ViewCenter.x += 0.1;
			st_gl_basic.ViewCenter.z = 0.0;

			st_gl_basic.ViewUpside.x = sin(pIO_Table->physics.th);
			st_gl_basic.ViewUpside.y = cos(pIO_Table->physics.th);
			st_gl_basic.ViewUpside.z = 0.0;

			st_gl_basic.fovy = 30.0;
		}
		else {
			st_gl_basic.ViewPoint.x = 0.0;
			st_gl_basic.ViewPoint.y = 1.0;
			st_gl_basic.ViewPoint.z = 100.0;

			st_gl_basic.ViewCenter.x = 0.0;
			st_gl_basic.ViewCenter.y = 0.1;
			st_gl_basic.ViewCenter.z = 0.0;

			st_gl_basic.ViewUpside.x = 0.0;
			st_gl_basic.ViewUpside.y = 0.0;
			st_gl_basic.ViewUpside.z = 1.0;

			st_gl_basic.fovy = 60.0;

		}
		break;
	case GLUT_RIGHT_BUTTON:
		break;
	default:
		break;
	}

	switch (state) {
	case GLUT_UP:
		break;
	case GLUT_DOWN:
		break;
	default:
		break;
	}

}

void CPublicRelation::GL_mouse_wheel(int button, int dir, int x, int y) {
	if (dir > 0) st_gl_basic.VP_Offset.z += 0.5;
	else st_gl_basic.VP_Offset.z -= 0.5;
	return;
}


void  CPublicRelation::GL_Ground(void) {
	double ground_max_x = 100.0; // 1.0 = 1.0m
	double ground_max_y = 100.0;// 1.0 = 1.0m
//	double ground_height = 0.0;
	double ground_height = pIO_Table->physics.lp.z;

	glColor3d(0.8, 0.8, 0.8);  // 大地の色
	glLineWidth(1.0);
	glBegin(GL_LINES);
	
	for (double ly = -ground_max_y; ly <= ground_max_y; ly += 2.0) {
		glVertex3f(-ground_max_x, ly, ground_height);
		glVertex3f(ground_max_x, ly, ground_height);
	}
	for (double lx = -ground_max_x; lx <= ground_max_x; lx += 2.0) {
		glVertex3f(lx, ground_max_y, ground_height);
		glVertex3f(lx, -ground_max_y, ground_height);
	}
	glEnd();
} //大地の描画Keyboard


  // 直方体の描画
void CPublicRelation::drowCuboid(double a, double b, double c,
	double x, double y, double z,
	MaterialStruct color) {
	GLdouble vertex[][3] = {
		{ -a / 2.0, -b / 2.0, -c / 2.0 },
	{ a / 2.0, -b / 2.0, -c / 2.0 },
	{ a / 2.0,  b / 2.0, -c / 2.0 },
	{ -a / 2.0,  b / 2.0, -c / 2.0 },
	{ -a / 2.0, -b / 2.0,  c / 2.0 },
	{ a / 2.0, -b / 2.0,  c / 2.0 },
	{ a / 2.0,  b / 2.0,  c / 2.0 },
	{ -a / 2.0,  b / 2.0,  c / 2.0 }
	};
	int face[][4] = {//面の定義
		{ 3, 2, 1, 0 },
	{ 1, 2, 6, 5 },
	{ 4, 5, 6, 7 },
	{ 0, 4, 7, 3 },
	{ 0, 1, 5, 4 },
	{ 2, 3, 7, 6 }
	};
	GLdouble normal[][3] = {//面の法線ベクトル
		{ 0.0, 0.0, -1.0 },
	{ 1.0, 0.0, 0.0 },
	{ 0.0, 0.0, 1.0 },
	{ -1.0, 0.0, 0.0 },
	{ 0.0,-1.0, 0.0 },
	{ 0.0, 1.0, 0.0 }
	};
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, color.ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, color.diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, color.specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, &color.shininess);
	glTranslated(x, y, z);//平行移動値の設定
	glBegin(GL_QUADS);
	for (int j = 0; j < 6; ++j) {
		glNormal3dv(normal[j]); //法線ベクトルの指定
		for (int i = 0; i < 4; ++i) {
			glVertex3dv(vertex[face[j][i]]);
		}
	}
	glEnd();
	glPopMatrix();
}

//----------------------------------------------------
//回転を考慮した立方体の描画
//----------------------------------------------------
void CPublicRelation::drowCuboid(double a, double b, double c,
	double x, double y, double z,
	MaterialStruct color,
	double theta,
	double nx, double ny, double nz) {
	double nn = sqrt(pow(nx, 2) + pow(ny, 2) + pow(nz, 2));
	if (nn>0.0) {
		nx = nx / nn;
		ny = ny / nn;
		nz = nz / nn;
	}
	glPushMatrix();
	glTranslated(x, y, z);//平行移動値の設定
	glPushMatrix();
	if (theta != 0 && nn>0.0) glRotated(theta, nx, ny, nz);
	drowCuboid(a, b, c, 0, 0, 0, color);
	glPopMatrix();
	glPopMatrix();
}

//----------------------------------------------------
// キーボード入力時に呼び出される関数
//----------------------------------------------------
void CPublicRelation::GL_Keyboard(unsigned char key, int x, int y) {
	switch (key)
	{
	case 'a':
		pOrder->ui.notch_mh = 5; pOrder->ui.notch_mh_dir = -1;
		break;
	case 'd':
		pOrder->ui.notch_mh = 5; pOrder->ui.notch_mh_dir = 1;
		break;
	case 's':
		pOrder->ui.notch_mh = 0; pOrder->ui.notch_mh_dir = 0;
		break;

	case 'q':
		exit(0);
		break;

	default:
		unsigned char key_ = key;
		break;
	}
}

void CPublicRelation::GL_DRAW_STRING(int x, int y, char *string, void *font = GLUT_BITMAP_TIMES_ROMAN_24) {
	int len, i;
	glRasterPos2f((GLfloat)x, (GLfloat)y);
	len = (int)strlen(string);
	for (i = 0; i < len; i++) {
		glutBitmapCharacter(font, string[i]);
	}
}

void CPublicRelation::GL_DISPLAY_TEXT(int x, int y, char *string) {
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	glPushAttrib(GL_ENABLE_BIT);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, 100, 0, 100);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glColor3f(0.0, 1.0, 0.0);
	glCallList(list);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
	glMatrixMode(GL_MODELVIEW);
	list = glGenLists(1);
	glNewList(list, GL_COMPILE);

	GL_DRAW_STRING(x, y, string, GLUT_BITMAP_TIMES_ROMAN_24);
	glEndList();

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}
 
