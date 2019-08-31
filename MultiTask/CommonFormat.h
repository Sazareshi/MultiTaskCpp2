#pragma once

#include "CodeDef.h"

/************************************************/
/*    # �^�X�N�I�u�W�F�N�g�C���f�b�N�X�\����    */
/************************************************/

typedef struct _st_iTask {
	int ana;
	int clerk;
	int comd;
	int comp;
	int ply;
	int pr;
	int mng;
}ST_iTask, *P_ST_iTask;

/************************************************/
/*    ORDER LIST                                */
/************************************************/

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
	double mh;
	double tt;
	double gt;
	double bh;
	double slew;
}ST_MANUAL_ORDER, *LPST_MANUAL_ORDER;

/// ���[�U�C���^�[�t�F�C�X Order
typedef struct _stUI_ORDER {
	DWORD type;				//UI�����^�C�v
	DWORD status;			//���s�X�e�[�^�X�@-1�F����

	DWORD notch_mh;
	int notch_mh_dir;
	DWORD notch_tt;
	int notch_tt_dir;
	DWORD notch_gt;
	int notch_gt_dir;
	DWORD notch_bh;
	int notch_bh_dir;
	DWORD notch_slew;
	int notch_slew_dir;
	DWORD notch_operm;
	int notch_operm_dir;
	DWORD notch_hook;
	int notch_hook_dir;

	DWORD env_mode;
	DWORD ope_mode;
	DWORD as_mode;
	DWORD auto_mode;

}ST_UI_ORDER, *LPST_UI_ORDER;

/// �X�e�[�^�X�� Order
typedef struct _stSTAT_ORDER {
	WORD type;				//UI�����^�C�v
	WORD status;			//���s�X�e�[�^�X�@-1�F����
}ST_STAT_ORDER, *LPST_STAT_ORDER;

/************************************************/
/*    SPEC LIST                                */
/************************************************/

#define NOTCH_MAX 6
#define ACCELERATION_MAX 4
#define FWD_ACC 0
#define FWD_DEC 1
#define REV_ACC 2
#define REV_DEC 3


typedef struct _stSpec {
	double slew_notch_spd[NOTCH_MAX];
	double hoist_notch_spd[NOTCH_MAX];
	double bh_notch_spd[NOTCH_MAX];
	double gantry_notch_spd[NOTCH_MAX];
	double hook_notch_spd[NOTCH_MAX];
	double operm_notch_spd[NOTCH_MAX];

	double slew_acc[ACCELERATION_MAX];
	double hoist_acc[ACCELERATION_MAX];
	double bh_acc[ACCELERATION_MAX];
	double gantry_acc[ACCELERATION_MAX];
	double hook_acc[ACCELERATION_MAX];
	double operm_acc[ACCELERATION_MAX];

	double boom_height;


}ST_SPEC, *LPST_SPEC;

/************************************************/
/*    Define                                */
/************************************************/
#define	DEF_HARF_PI  1.57079632
#define	DEF_PI  3.1415265
#define DEF_2PI 6.2831853
#define DEF_G	9.80665
#define COF_RAD2DEG	57.296

#define ALLOWABLE_DEF_SLEW	0.003 //����w�߂�FB�̋��e�덷�@rad/s 1�m�b�`��10��	
#define ALLOWABLE_DEF_BH	0.005 //�����w�߂�FB�̋��e�덷�@m/s 1�m�b�`��5��
#define ALLOWABLE_DEF_HOIST	0.005 //�����w�߂�FB�̋��e�덷�@m/s 1�m�b�`��10��
#define MY_ABS(a)	((a)<0.0 ? -(a) : (a))

inline double rad2deg(double rad) { return COF_RAD2DEG * rad; }
