#include "stdafx.h"
#include "CPublicRelation.h"

//#################### OPEN GL ################################################
void CPublicRelation::ActOpenGL() {

	GL_Initialize();			//このアプリケーションでの初期化
	
	glutInit(&__argc, __argv);//環境の初期化
	glutInitWindowPosition(st_gl_basic.WinPosX, st_gl_basic.WinPosY);//ウィンドウの位置の指定
	glutInitWindowSize(st_gl_basic.WinWidth, st_gl_basic.WinHeight); //ウィンドウサイズの指定
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);//ディスプレイモードの指定
	glutCreateWindow(st_gl_basic.WindowTitle);		//ウィンドウの作成
	glutDisplayFunc(GL_Display);			//描画時に呼び出される関数を指定する（関数名：Display）

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

	st_gl_basic.ViewPoint.x = 0.0;
	st_gl_basic.ViewPoint.y = 500.0;
	st_gl_basic.ViewPoint.z = 500.0;

}


void CPublicRelation::GL_Display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //バッファの消去

//透視変換行列の設定------------------------------
	glMatrixMode(GL_PROJECTION);	//GLenum mode 行列モードの設定（GL_PROJECTION : 透視変換行列の設定、GL_MODELVIEW：モデルビュー変換行列）
	glLoadIdentity();				//行列の初期化
	gluPerspective(														//透視投影法の視体積gluPerspactive(th, w/h, near, far);
		30.0,															//  GLdouble fovy, 
		(double)st_gl_basic.WinWidth / (double)st_gl_basic.WinHeight,	//	GLdouble aspect,
		0.1,															//	GLdouble zNear,
		1000.0);														//	GLdouble zFar);
		
	gluLookAt(
		st_gl_basic.ViewPoint.x, st_gl_basic.ViewPoint.y, st_gl_basic.ViewPoint.z,//GLdouble eyex, eyey, eyez 視点の位置x,y,z;
		0.0, 0.0, 0.0,  //視界の中心位置の参照点座標x,y,z
		0.0, 0.0, 1.0); 		//GLdouble upx,upy, upz 視界の上方向のベクトルx,y,z


//モデルビュー変換行列の設定--------------------------
	glMatrixMode(GL_MODELVIEW);//GLenum mode 行列モードの設定（GL_PROJECTION : 透視変換行列の設定、GL_MODELVIEW：モデルビュー変換行列）
	glLoadIdentity();//行列の初期化
	glViewport(0, 0, st_gl_basic.WinWidth, st_gl_basic.WinHeight);

#if 0
//陰影ON-----------------------------
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);//光源0を利用


//球 描画
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, ms_ruby.ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, ms_ruby.diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, ms_ruby.specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, &ms_ruby.shininess);
	glTranslated(pLoad->r.x, pLoad->r.y, pLoad->r.z);//平行移動値の設定
	glutSolidSphere(1.0, 20, 20);//引数：(半径, Z軸まわりの分割数, Z軸に沿った分割数)
	glPopMatrix();

//立方体　描画
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
	glTranslated(pHanging->r.x, pHanging->r.y, pHanging->r.z);//平行移動値の設定
	glutSolidCube(1.0);//引数：(一辺の長さ)
	glPopMatrix();


//ロープ描画
	glColor3d(0.0, 0.0, 1.0);
	glLineWidth(2.0);

	glBegin(GL_LINES);
	glVertex3d(pLoad->r.x, pLoad->r.y, pLoad->r.z);
	glVertex3d(pHanging->r.x, pHanging->r.y, pHanging->r.z);
	glEnd();



//文字の描画
	char t_char[20];
	char t_char2[20];

	strcpy_s(t_char2, "t = ");
	sprintf_s(t_char, "%f", t);
	strcat_s(t_char2, t_char);
	DISPLAY_TEXT(5, 93, t_char2);

	strcpy_s(t_char2, "ax = ");
	sprintf_s(t_char, "%f", pHanging->a.x);
	strcat_s(t_char2, t_char);
	DISPLAY_TEXT(5, 88, t_char2);

	strcpy_s(t_char2, "vx = ");
	sprintf_s(t_char, "%f", pHanging->v.x);
	strcat_s(t_char2, t_char);
	DISPLAY_TEXT(5, 83, t_char2);

	strcpy_s(t_char2, "L = ");
	Vector3 tempL;
	tempL = tempL.subVectors(pHanging->r, pLoad->r);

	sprintf_s(t_char, "%f", tempL.length());
	strcat_s(t_char2, t_char);
	DISPLAY_TEXT(5, 78, t_char2);


	//陰影OFF-----------------------------
	glDisable(GL_LIGHTING);
	//-----------------------------------
#endif
	GL_Ground();

	glutSwapBuffers(); //glutInitDisplayMode(GLUT_DOUBLE)でダブルバッファリングを利用可
}


void  CPublicRelation::GL_Ground(void) {
	double ground_max_x = 100.0; // 1.0 = 1.0m
	double ground_max_y = 100.0;// 1.0 = 1.0m
	glColor3d(0.8, 0.8, 0.8);  // 大地の色

	glBegin(GL_LINES);
	for (double ly = -ground_max_y; ly <= ground_max_y; ly += 2.0) {
		glVertex3d(-ground_max_x, ly, 0);
		glVertex3d(ground_max_x, ly, 0);
	}
	for (double lx = -ground_max_x; lx <= ground_max_x; lx += 2.0) {
		glVertex3d(lx, ground_max_y, 0);
		glVertex3d(lx, -ground_max_y, 0);
	}
	glEnd();
} //大地の描画Keyboard
  /*

*/
