#pragma once

#include "CodeDef.h"
///# �^�X�N�I�u�W�F�N�g�C���f�b�N�X�\����
typedef struct _st_iTask {
	int ana;
	int clerk;
	int comc;
	int comp;
	int ply;
	int pr;
	int mng;
}ST_iTask, *P_ST_iTask;

//### ORDER LIST ###
#define ORDER_MAX		8

/// JOB Order
typedef struct _stMotion_Element {	//�^���v�f
	int type;		//������
	int status;		//�������
	double _a;		//�ڕW�������x
	double _v;		//�ڕW���x
	double _p;		//�ڕW�ʒu
	double _t;		//�p������
	double vh_lim;	//���x����High
	double vl_lim;	//���x����Low
	double opt_d1;	//�I�v�V����double
	double opt_d2;	//�I�v�V����double
	int opt_i1;		//�I�v�V����int
	int opt_i2;		//�I�v�V����int
}ST_MOTION_ELEMENT, *LPST_MOTION_ELEMENT;

#define M_ELEMENT_MAX	32
#define M_AXIS			8	//���쎲
#define MH_AXIS			0	//�努����
#define TT_AXIS			1	//���s����
#define GT_AXIS			2	//���s����
#define BH_AXIS			3	//�N������
#define SLW_AXIS		4	//���񓮍�
#define SKW_AXIS		5	//�X�L���[����
#define LFT_AXIS		6	//�݋��
typedef struct _stJOB_UNIT {	//��Ɨv�f�iPICK�AGROUND�APARK�j
	int type;		//������
	int mAct[M_AXIS];//���s�Ώہ@�z��[0]���瓮��R�[�h�Z�b�g-1�Z�b�g�ŏI���
	int iAct[M_AXIS];//���s��index -1�Ŋ���
	ST_MOTION_ELEMENT motions[M_AXIS][M_ELEMENT_MAX];
}ST_JOB_UNIT, *LPST_JOB_UNIT;

typedef struct _stJOB_Report {	//JOB�����񍐃t�H�[�}�b�g
	WORD status;
}ST_JOB_REPRORT, *LPST_JOB_REPRORT;

typedef struct _stJOB_ORDER {	//JOB�@ORDER�\����
	WORD type;					//JOB�^�C�v
	WORD property;				//���s����
	WORD status;				//���s�X�e�[�^�X�@�@-1�F����
	WORD result;				//���s����
	ST_JOB_UNIT		from_recipe;//from�^�]�p�^�[��
	ST_JOB_UNIT		to_recipe;	//to�^�]�p�^�[��
	ST_JOB_REPRORT repo1;		//from������
	ST_JOB_REPRORT repo2;		//to������
}ST_JOB_ORDER, *LPST_JOB_ORDER;

/// MODE Order
typedef struct _stMODE_ORDER {	//MODE Order �\����
	WORD type;				//MODE�^�C�v
	WORD property;			//���s����
	WORD status;			//���s�X�e�[�^�X�@-1�F����
	WORD result;			//���s����
}ST_MODE_ORDER, *LPST_MODE_ORDER;

/// E-Stop Order
typedef struct _stESTOP_ORDER {
	WORD type;				//E-STOP�^�C�v
	WORD property;			//���s����
	WORD status;			//���s�X�e�[�^�X
	WORD result;			//���s����
}ST_ESTOP_ORDER, *LPST_ESTOP_ORDER;

/// �蓮���� Order
typedef struct _stMANUAL_ORDER {
	WORD type;				//�蓮����^�C�v
	WORD status;			//���s�X�e�[�^�X�@-1�F����
	WORD mh;
	WORD tt;
	WORD gt;
	WORD bh;
	WORD slew;
}ST_MANUAL_ORDER, *LPST_MANUAL_ORDER;

/// ���[�U�C���^�[�t�F�C�X Order
typedef struct _stUI_ORDER {
	WORD type;				//UI�����^�C�v
	WORD status;			//���s�X�e�[�^�X�@-1�F����
	WORD mh;
	WORD tt;
	WORD gt;
	WORD bh;
	WORD slew;
}ST_UI_ORDER, *LPST_UI_ORDER;

/// �X�e�[�^�X�� Order
typedef struct _stSTAT_ORDER {
	WORD type;				//UI�����^�C�v
	WORD status;			//���s�X�e�[�^�X�@-1�F����
}ST_STAT_ORDER, *LPST_STAT_ORDER;


#define ORDER_MAX_ESTP	3
#define ORDER_MAX_MODE	3
#define ORDER_MAX_MANU	3
#define ORDER_MAX_JOB	3
#define ORDER_MAX_UI	3
#define ORDER_MAX_STAT	3

typedef struct _stORDERs {
	ST_ESTOP_ORDER	_ESTP[ORDER_MAX_ESTP];
	ST_MODE_ORDER	_MODE[ORDER_MAX_MODE];
	ST_MANUAL_ORDER _MANUAL[ORDER_MAX_JOB];
	ST_JOB_ORDER	_JOB[ORDER_MAX_JOB];
	ST_UI_ORDER		_UI[ORDER_MAX_UI];
	ST_STAT_ORDER	_STATUS[ORDER_MAX_STAT];
}ST_ORDERs, *LPST_ORDERs;
