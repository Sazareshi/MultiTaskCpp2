#pragma once

#include "CodeDef.h"
#include "CVector3.h"

/************************************************/
/*    Define                                */
/************************************************/
#define	DEF_QPI  0.7854			//45deg
#define DEF_TPI  1.0472         //60deg
#define	DEF_HPI  1.5708			//90deg
#define	DEF_PI  3.1415265
#define DEF_2PI 6.2831853
#define DEF_001DEG	0.00017		//0.01deg
#define DEF_G	9.80665
#define COF_RAD2DEG	57.296

#define ALLOWABLE_DEF_SLEW	0.003 //����w�߂�FB�̋��e�덷�@rad/s 1�m�b�`��10��	
#define ALLOWABLE_DEF_BH	0.005 //�����w�߂�FB�̋��e�덷�@m/s 1�m�b�`��5��
#define ALLOWABLE_DEF_HOIST	0.005 //�����w�߂�FB�̋��e�덷�@m/s 1�m�b�`��10��
#define MY_ABS(a)	((a)<0.0 ? -(a) : (a))

#define MOTION_NUM		6
#define MOTION_ID_MH	1
#define MOTION_ID_BH	2
#define MOTION_ID_SLEW	3

#define NO_ERR_EXIST	0
#define ERR_NO_CASE_EXIST	1
#define CAL_RESULT_POSITIVE 1
#define CAL_RESULT_NEGATIVE 0

#define ERROR_ANA_CREAPE_AUTO_PATERN 0x7001

inline double rad2deg(double rad) { return COF_RAD2DEG * rad; }


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

// Control Type
#define CTR_TYPE_TIME_WAIT					0x0000  //Keep condition for specified time
#define CTR_TYPE_SINGLE_PHASE_WAIT			0x0001  //Keep condition awaiting one sway phase
#define CTR_TYPE_DOUBLE_PHASE_WAIT			0x0002  //Keep condition awaiting two sway phase
#define CTR_TYPE_BH_WAIT					0x0003	//Keep condition awaiting BH position
#define CTR_TYPE_SLEW_WAIT					0x0004	//Keep condition awaiting SLEW position
#define CTR_TYPE_MH_WAIT					0x0005	//Keep condition awaiting HOIST position
#define CTR_TYPE_TIME_WAIT_2PN				0x0006  //Keep condition for specified time
#define CTR_TYPE_TIME_WAIT_2PP				0x0007  //Keep condition for specified time
#define CTR_TYPE_CONST_V_TIME				0x0100  //Keep specified speed ref for specified time
#define CTR_TYPE_ACC_TIME					0x0200  //Specified time acceleration
#define CTR_TYPE_ACC_V						0x0201  //Toward specified speed acceleration
#define CTR_TYPE_ACC_TIME_OR_V				0x0202  //Specified time acceleration or reach specified speed
#define CTR_TYPE_ACC_AS						0x0203 //Toward specified speed acceleration for inching antisway
#define CTR_TYPE_ACC_AS_2PN					0x0204 //Toward specified speed acceleration for inching antisway
#define CTR_TYPE_DEC_TIME					0x0300  //Specified time deceleration
#define CTR_TYPE_DEC_V						0x0301  //Toward specified speed deceleration
#define CTR_TYPE_DEC_TIME_OR_V				0x0302  //Specified time acceleration or reach specified speed
#define CTR_TYPE_DEC_AS_2PN					0x0303  //Toward specified speed deceleration

#define AS_INIT_SWAY_0						0		//�����U��0
#define AS_INIT_SWAY_SMALL					1		//�����U�ꏬ
#define AS_INIT_SWAY_LARGE					2		//�����U���



typedef struct _stMotion_Element {	//�^���v�f
	int type;				//������
	int status;				//�������
	int time_count;			//�\��p�����Ԃ̃J�E���^�ԊҒl
	int act_counter;		//���s��
	double _a;				//�ڕW�������x
	double _v;				//�ڕW���x
	double _p;				//�ڕW�ʒu
	double _t;				//�p������
	double vh_lim;			//���x����High
	double vl_lim;			//���x����Low
	double phase1;			//�N���ʑ��P
	double phase2;			//�N���ʑ� 2
	double opt_d1;			//�I�v�V����double
	double opt_d2;			//�I�v�V����double
	int opt_i1;				//�I�v�V����int
	int opt_i2;				//�I�v�V����int
}ST_MOTION_ELEMENT, *LPST_MOTION_ELEMENT;

#define M_ELEMENT_MAX	32
#define M_AXIS			8	//���쎲
#define MH_AXIS			1	//�努����
#define TT_AXIS			2	//���s����
#define GT_AXIS			3	//���s����
#define BH_AXIS			4	//�N������
#define SLW_AXIS		5	//���񓮍�
#define SKW_AXIS		6	//�X�L���[����
#define LFT_AXIS		7	//�݋��

#define PTN_UNIT_FIN	-1	//Completed
#define PTN_NOTHING		0	//No Motion
#define PTN_STANDBY		1	//Waiting Triggr
#define PTN_ACTIVE		2	//On Going
#define PTN_PAUSE		3	//Paused

typedef struct _stMOTION_UNIT {	//����p�^�[��
	int axis_type;				//���쎲��ʁ@MH_AXIS,TT_AXIS,GT_AXIS,BH_AXIS..... 
	int n_step;					//����p�^�[���\���v�f��
	int ptn_status;				//����p�^�[�����s��
	int iAct;					//���s��index -1�Ŋ���
	int motion_type;			//�I�v�V�����w��
	ST_MOTION_ELEMENT motions[M_ELEMENT_MAX];
}ST_MOTION_UNIT, *LPST_MOTION_UNIT;


#define JOBTYPE_PICK	1
#define JOBTYPE_GRND	2
#define JOBTYPE_PARK	3
#define JOBTYPE_AS_POS	4
#define JOBTYPE_AS_SWAY	5

typedef struct _stCOMMAND_UNIT {	//��Ɨv�f�iPICK�AGROUND�APARK�j
	int type;					//JOB���
	int job_status;				//JOB���s��
	int mAct[M_AXIS];			//���s�Ώہ@�z��[0]���瓮��R�[�h�Z�b�g-1�Z�b�g�ŏI���
	ST_MOTION_UNIT motions[M_AXIS];
}ST_COMMAND_UNIT, *LPST_COMMAND_UNIT;

typedef struct _stCOMMAND_Report {	//JOB�����񍐃t�H�[�}�b�g
	WORD status;
}ST_COMMAND_REPRORT, *LPST_COMMAND_REPRORT;


typedef struct _stCOMMAND_Target {	//JOB�@ORDER�\����
	double bh_pos;
	double slew_pos;
	double mh_pos;
}ST_COMMAND_TARGET, *LPST_COMMAND_TARGET;


//####### JOB Status  ################
#define JOB_ORDER_COMPLETED			0x0000
#define JOB_ORDER_STANDBY			0x0001
#define JOB_ORDER_STEP_STANDBY		0x0002
#define JOB_ORDER_STEP_ON_GOING		0x0003
#define JOB_ORDER_STEP_COMPLETED	0x0004
#define JOB_ORDER_STEP_IDLE			0x0005
#define JOB_ORDER_STEP_ERROR		0x1000

//####### ORDER TYPE  ################
#define TEST_ORDER					0xffff
#define NO_ORDER					0x0000

#define JOB_ORDER_MAX_STEP	3

/// MODE Order
typedef struct _stJOB_ORDER {	//JOB�@ORDER�\����
	WORD type;										//JOB�^�C�v
	WORD property;									//���s����
	WORD status;									//���s�X�e�[�^�X�@�@-1�F����
	int n_job_step;									//���s�X�e�b�v��
	int job_step_now;								//���s���X�e�b�v
	ST_COMMAND_TARGET	command_target[JOB_ORDER_MAX_STEP];
	ST_COMMAND_UNIT		command_recipe[JOB_ORDER_MAX_STEP];	//�^�]�p�^�[��
	ST_COMMAND_REPRORT	job_report[JOB_ORDER_MAX_STEP];	//������
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
	int type;				//�蓮����^�C�v
	int status;				//���s�X�e�[�^�X�@-1�F����
	int anti_sway_mode;		//�U�~�߃��[�h�ݒ�PB
	int ope_mode;			//���샂�[�h�ݒ�PB
	int remote_mode;		//���u�^�]���[�h�ݒ�PB
	int anti_sway_trigger;	//�U��~�ߋN��PB
	int auto_start;			//�����N��PB
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

	int env_mode;
	int ope_mode;
	int as_mode;
	int as_mode_hoist;		//���U��~�ߐݒ�PB
	int auto_mode;
	int remote_mode;		//���u�^�]���[�h�ݒ�PB
	int anti_sway_trigger;	//�U��~�ߋN��PB
	int auto_start;			//�����N��PB

}ST_UI_ORDER, *LPST_UI_ORDER;

/// �X�e�[�^�X�� Order
typedef struct _stSTAT_ORDER {
	WORD type;				//UI�����^�C�v
	WORD status;			//���s�X�e�[�^�X�@-1�F����
}ST_STAT_ORDER, *LPST_STAT_ORDER;


/************************************************/
/*   IO TABLE                                   */
/************************************************/

typedef struct _stIO_Physic {
	double M_load;//�݉׎���

	//�ݓ_�@�@���W���_�@xy�F���񎲁@z�F�n��(�オ+�j
	Vector3 cp;		//�ݓ_xyz
	double R;		//������
	double th;		//����p�x
	double ph;		//�N���p�x

	Vector3 cv;		//�ݓ_vx vy vz
	double vR;		//�������ω����x
	double wth;		//����p���x
	double wph;		//�N���p���x

	//�ׁ݉@�@���W���_�@xy�F���񎲁@z�F�n��(�オ+�j
	Vector3 lp;		//�ݓ_xyz
	Vector3 lv;		//�ݓ_vx vy vz


	//�݉גݓ_�ԑ��Έʒu
	double L;		//���[�v��
	double lph;		//Z���Ƃ̊p�x
	double lth;		//XY���ʊp�x

	double vL;		//�����x
	double vlph;	//Z���Ƃ̊p���x
	double vlth;	//XY���ʊp���x

	double T;		//�U�����
	double w0;		//�U��p���g��(2PI()/T�j

	Vector3 PhPlane_r;	//Z���p�x�̈ʑ����� x:Theata y:TheataDot/Omega z:phase angle
	double wPhPlane_r; //Z���p�x�̈ʑ��̊p���g��
	Vector3 PhPlane_n;	//��]���W���a�����̈ʑ����� x:Theata y:TheataDot/Omega z:phase angle
	Vector3 PhPlane_t;	//��]���W�ڐ������̈ʑ����� x:Theata y:TheataDot/Omega z:phase angle

	double sway_amp_r_ph2;	//�ʑ����ʔ��a2��
	double sway_amp_n_ph2;	//�ʑ����ʔ��a2��@�@������
	double sway_amp_t_ph2;	//�ʑ����ʔ��a2��@�ڐ�����

	double sway_amp_r_ph;	//�ʑ����ʔ��a
	double sway_amp_n_ph;	//�ʑ����ʔ��a�@�@������
	double sway_amp_t_ph;	//�ʑ����ʔ��a�@�ڐ�����
	
}ST_IO_PHYSIC, *LPST_IO_PHYSIC;

typedef struct _stIO_Ref {

	double	slew_w;				//����p���x
	double	hoist_v;			//�����x
	double	bh_v;				//�������x
	bool	b_bh_manual_ctrl;	//�蓮���쒆
	bool	b_slew_manual_ctrl;	//�蓮���쒆
	bool	b_mh_manual_ctrl;	//�蓮���쒆

}ST_IO_REF, *LPST_IO_REF;

#define NUM_OF_AS			3
#define AS_SLEW_ID			0
#define AS_BH_ID			1
#define AS_MH_ID			2
#define AS_START_LOW_PHASE	 1
#define AS_START_HIGH_PHASE	 -1
#define AS_DIR_PLUS			 1
#define AS_DIR_MINUS		-1


typedef struct _stAS_CTRL {
	double tgpos_h;							//���ڕW�ʒu
	double tgpos_gt;						//���s�ڕW�ʒu
	double tgpos_slew;						//����ڕW�ʒu
	double tgpos_bh;						//�����ڕW�ʒu

	double tgspd_h;							//���ڕW���x
	double tgspd_gt;						//���s�ڕW���x
	double tgspd_slew;						//����ڕW���x
	double tgspd_bh;						//�����ڕW���x

	double as_gain_ph[NUM_OF_AS];			//�U�~�Q�C��  �����ʑ�rad
	double as_gain_time[NUM_OF_AS];			//�U�~�Q�C���@��������sec

	double phase_acc_offset[NUM_OF_AS];		//�������ʑ��ʐU�ꒆ�S
	double phase_dec_offset[NUM_OF_AS];		//�������ʑ��ʐU�ꒆ�S
	
	double phase_chk_range[NUM_OF_AS];		//�U��~�߈ʑ��m�F���e�덷	-
	int as_out_dir[NUM_OF_AS];				//�U��~�ߏo�͂̕���
	int as_start_ph[NUM_OF_AS];			//�U��~�߉����J�n���̈ʑ�����

	double tgD[NUM_OF_AS];					//�U�~�ڕW-���݊p�x
	double tgD_abs[NUM_OF_AS];				//�U�~�ڕW-���݊p�x ��Βl
	double Dmax_2step[NUM_OF_AS];			//2Step�����ړ��ő�ړ�����

	double allowable_pos_overshoot_plus[NUM_OF_AS];		//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s����
	double allowable_pos_overshoot_minus[NUM_OF_AS];	//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s�t����

}ST_AS_CTRL, *LPST_AS_CTRL;

/************************************************/
/*    SPEC LIST                                 */
/************************************************/

#define NOTCH_MAX 6
#define ACCELERATION_MAX 4
#define FWD_ACC 0
#define FWD_DEC 1
#define REV_ACC 2
#define REV_DEC 3

#define I_AS_LV_COMPLE		0	//��������l
#define I_AS_LV_TRIGGER		1	//�U�~�N������
#define I_AS_LV_DAMPING		2	//�_���s���O���[�h����
#define I_AS_LV_POSITION	2	//�ʒu���߃��[�h����


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

	double as_compl_swayLv[3];		// rad  0:complete 1:trigger 2:antisway
	double as_compl_swayLv_sq[3];	// rad2 0:complete 1:trigger 2:antisway 
	double as_compl_nposLv[3];		// m    0:complete 1:trigger 2:positioning
	double as_compl_tposLv[3];		// rad  0:complete 1:trigger 2:positioning

}ST_SPEC, *LPST_SPEC;

