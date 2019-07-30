#include "stdafx.h"
#include "CPublicRelation.h"

//#################### OPEN GL ################################################
void CPublicRelation::ActOpenGL() {

	GL_Initialize();			//���̃A�v���P�[�V�����ł̏�����
	
	glutInit(&__argc, __argv);//���̏�����
	glutInitWindowPosition(st_gl_basic.WinPosX, st_gl_basic.WinPosY);//�E�B���h�E�̈ʒu�̎w��
	glutInitWindowSize(st_gl_basic.WinWidth, st_gl_basic.WinHeight); //�E�B���h�E�T�C�Y�̎w��
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);//�f�B�X�v���C���[�h�̎w��
	glutCreateWindow(st_gl_basic.WindowTitle);		//�E�B���h�E�̍쐬
	glutDisplayFunc(GL_Display);			//�`�掞�ɌĂяo�����֐����w�肷��i�֐����FDisplay�j

#if 0
	glutIdleFunc(Idle);					//�v���O�����A�C�h����Ԏ��ɌĂяo�����֐�

	glutMouseWheelFunc(mouse_wheel);	//�}�E�X�z�C�[�����쎞�ɌĂяo�����֐�
	Initialize();						//�����ݒ�̊֐����Ăяo��

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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //�o�b�t�@�̏���

//�����ϊ��s��̐ݒ�------------------------------
	glMatrixMode(GL_PROJECTION);	//GLenum mode �s�񃂁[�h�̐ݒ�iGL_PROJECTION : �����ϊ��s��̐ݒ�AGL_MODELVIEW�F���f���r���[�ϊ��s��j
	glLoadIdentity();				//�s��̏�����
	gluPerspective(														//�������e�@�̎��̐�gluPerspactive(th, w/h, near, far);
		30.0,															//  GLdouble fovy, 
		(double)st_gl_basic.WinWidth / (double)st_gl_basic.WinHeight,	//	GLdouble aspect,
		0.1,															//	GLdouble zNear,
		1000.0);														//	GLdouble zFar);
		
	gluLookAt(
		st_gl_basic.ViewPoint.x, st_gl_basic.ViewPoint.y, st_gl_basic.ViewPoint.z,//GLdouble eyex, eyey, eyez ���_�̈ʒux,y,z;
		0.0, 0.0, 0.0,  //���E�̒��S�ʒu�̎Q�Ɠ_���Wx,y,z
		0.0, 0.0, 1.0); 		//GLdouble upx,upy, upz ���E�̏�����̃x�N�g��x,y,z


//���f���r���[�ϊ��s��̐ݒ�--------------------------
	glMatrixMode(GL_MODELVIEW);//GLenum mode �s�񃂁[�h�̐ݒ�iGL_PROJECTION : �����ϊ��s��̐ݒ�AGL_MODELVIEW�F���f���r���[�ϊ��s��j
	glLoadIdentity();//�s��̏�����
	glViewport(0, 0, st_gl_basic.WinWidth, st_gl_basic.WinHeight);

#if 0
//�A�eON-----------------------------
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);//����0�𗘗p


//�� �`��
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT, ms_ruby.ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, ms_ruby.diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, ms_ruby.specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, &ms_ruby.shininess);
	glTranslated(pLoad->r.x, pLoad->r.y, pLoad->r.z);//���s�ړ��l�̐ݒ�
	glutSolidSphere(1.0, 20, 20);//�����F(���a, Z���܂��̕�����, Z���ɉ�����������)
	glPopMatrix();

//�����́@�`��
	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
	glTranslated(pHanging->r.x, pHanging->r.y, pHanging->r.z);//���s�ړ��l�̐ݒ�
	glutSolidCube(1.0);//�����F(��ӂ̒���)
	glPopMatrix();


//���[�v�`��
	glColor3d(0.0, 0.0, 1.0);
	glLineWidth(2.0);

	glBegin(GL_LINES);
	glVertex3d(pLoad->r.x, pLoad->r.y, pLoad->r.z);
	glVertex3d(pHanging->r.x, pHanging->r.y, pHanging->r.z);
	glEnd();



//�����̕`��
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


	//�A�eOFF-----------------------------
	glDisable(GL_LIGHTING);
	//-----------------------------------
#endif
	GL_Ground();

	glutSwapBuffers(); //glutInitDisplayMode(GLUT_DOUBLE)�Ń_�u���o�b�t�@�����O�𗘗p��
}


void  CPublicRelation::GL_Ground(void) {
	double ground_max_x = 100.0; // 1.0 = 1.0m
	double ground_max_y = 100.0;// 1.0 = 1.0m
	glColor3d(0.8, 0.8, 0.8);  // ��n�̐F

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
} //��n�̕`��Keyboard
  /*

*/
