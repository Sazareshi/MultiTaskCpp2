#pragma once

#include "CommonFormat.h"
#include "CVector3.h"

class CORDER_Table
{
public:
	CORDER_Table();
	~CORDER_Table();

	ST_JOB_ORDER job_A;
	ST_JOB_ORDER job_B;
	ST_MODE_ORDER mode;
	ST_ESTOP_ORDER e_stop;
	ST_MANUAL_ORDER manualA;
	ST_MANUAL_ORDER manualB;
	ST_UI_ORDER ui;
};

#define ENV_MODE_REAL	0
#define ENV_MODE_SIM1	1
#define ENV_MODE_SIM2	2

class CMODE_Table
{
public:
	CMODE_Table() {};
	~CMODE_Table() {};
	
	DWORD operation;
	DWORD auto_control;
	DWORD environment;
};

#define NUM_OF_TASK_FAULT	100
#define NUM_OF_TASK			7

class CFAULT_Table
{
public:
	CFAULT_Table() {};
	~CFAULT_Table() {};

	BYTE fault[NUM_OF_TASK][NUM_OF_TASK_FAULT];
};


typedef struct _stIO_Physic {
	double M_load;//�݉׎���

//�ݓ_�@�@���W���_�@xy�F���񎲁@z�F�N�����_(�オ+�j
	Vector3 cp;		//�ݓ_xyz
	double R;		//������
	double th;		//����p�x
	double ph;		//�N���p�x

	Vector3 cv;		//�ݓ_vx vy vz
	double vR;		//�������ω����x
	double wth;		//����p���x
	double wph;		//�N���p���x

//�݉ד_�@�@���W���_�@�ݓ_
	Vector3 pos;		//�ݓ_xyz
	double L;		//���[�v��
	double Le;		//�L�����[�v���i�U������j
	double _th;		//�����ʊp�x
	double _ph;		//Z���p�x

	Vector3 vel;		//�ݓ_vx vy vz
	double vL;		// ���[�v���ω����x
	double _wth;	//�����ʊp���x
	double _wph;	//Z���p���x
}ST_IO_PHYSIC, *LPST_IO_PHYSIC;

class CIO_Table
{
public:
	CIO_Table() {};
	~CIO_Table() {};

	ST_IO_PHYSIC physics;

};
