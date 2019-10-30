#pragma once

#include "CodeDef.h"
#include "CVector3.h"

/************************************************/
/*    Define                                */
/************************************************/
#define	DEF_QPI  0.7854			//45deg
#define	DEF_HPI  1.5708			//90deg
#define	DEF_PI  3.1415265
#define DEF_2PI 6.2831853
#define DEF_001DEG	0.00017		//0.01deg
#define DEF_G	9.80665
#define COF_RAD2DEG	57.296

#define ALLOWABLE_DEF_SLEW	0.003 //旋回指令とFBの許容誤差　rad/s 1ノッチの10％	
#define ALLOWABLE_DEF_BH	0.005 //引込指令とFBの許容誤差　m/s 1ノッチの5％
#define ALLOWABLE_DEF_HOIST	0.005 //引込指令とFBの許容誤差　m/s 1ノッチの10％
#define MY_ABS(a)	((a)<0.0 ? -(a) : (a))

#define MOTION_NUM		6
#define MOTION_ID_MH	1
#define MOTION_ID_BH	2
#define MOTION_ID_SLEW	3

#define NO_ERR_EXIST	0

inline double rad2deg(double rad) { return COF_RAD2DEG * rad; }


/************************************************/
/*    # タスクオブジェクトインデックス構造体    */
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
#define CTR_TYPE_CONST_V_TIME				0x0100  //Keep specified speed ref for specified time
#define CTR_TYPE_ACC_TIME					0x0200  //Specified time acceleration
#define CTR_TYPE_ACC_V						0x0201  //Toward specified speed acceleration
#define CTR_TYPE_ACC_TIME_OR_V				0x0202  //Specified time acceleration or reach specified speed
#define CTR_TYPE_ACC_AS				0x0203 //Toward specified speed acceleration for inching antisway
#define CTR_TYPE_DEC_TIME					0x0300  //Specified time deceleration
#define CTR_TYPE_DEC_V						0x0301  //Toward specified speed deceleration
#define CTR_TYPE_DEC_TIME_OR_V				0x0302  //Specified time acceleration or reach specified speed

typedef struct _stMotion_Element {	//運動要素
	int type;				//制御種別
	int status;				//制御種状態
	int time_count;			//予定継続時間のカウンタ返還値
	int act_counter;		//実行回数
	double _a;				//目標加減速度
	double _v;				//目標速度
	double _p;				//目標位置
	double _t;				//継続時間
	double vh_lim;			//速度制限High
	double vl_lim;			//速度制限Low
	double phase1;			//起動位相１
	double phase2;			//起動位相 2
	double opt_d1;			//オプションdouble
	double opt_d2;			//オプションdouble
	int opt_i1;				//オプションint
	int opt_i2;				//オプションint
}ST_MOTION_ELEMENT, *LPST_MOTION_ELEMENT;

#define M_ELEMENT_MAX	32
#define M_AXIS			8	//動作軸
#define MH_AXIS			1	//主巻動作
#define TT_AXIS			2	//横行動作
#define GT_AXIS			3	//走行動作
#define BH_AXIS			4	//起伏動作
#define SLW_AXIS		5	//旋回動作
#define SKW_AXIS		6	//スキュー動作
#define LFT_AXIS		7	//吊具操作

#define PTN_UNIT_FIN	-1	//Completed
#define PTN_NOTHING		0	//No Motion
#define PTN_STANDBY		1	//Waiting Triggr
#define PTN_ACTIVE		2	//On Going
#define PTN_PAUSE		3	//Paused

typedef struct _stMOTION_UNIT {	//動作パターン
	int axis_type;				//動作軸種別　MH_AXIS,TT_AXIS,GT_AXIS,BH_AXIS..... 
	int n_step;					//動作パターン構成要素数
	int ptn_status;				//動作パターン実行状況
	int iAct;					//実行中index -1で完了
	int motion_type;			//オプション指定
	ST_MOTION_ELEMENT motions[M_ELEMENT_MAX];
}ST_MOTION_UNIT, *LPST_MOTION_UNIT;


#define JOBTYPE_PICK	1
#define JOBTYPE_GRND	2
#define JOBTYPE_PARK	3
#define JOBTYPE_AS_POS	4
#define JOBTYPE_AS_SWAY	5

typedef struct _stJOB_UNIT {	//作業要素（PICK、GROUND、PARK）
	int type;					//JOB種別
	int job_status;				//JOB実行状況
	int mAct[M_AXIS];			//実行対象　配列[0]から動作コードセット-1セットで終わり
	int iAct[M_AXIS];			//実行中index -1で完了
	ST_MOTION_UNIT motions[M_AXIS];
}ST_JOB_UNIT, *LPST_JOB_UNIT;

typedef struct _stJOB_Report {	//JOB完了報告フォーマット
	WORD status;
}ST_JOB_REPRORT, *LPST_JOB_REPRORT;

/// MODE Order
typedef struct _stJOB_ORDER {	//JOB　ORDER構造体
	WORD type;					//JOBタイプ
	WORD property;				//実行条件
	WORD status;				//実行ステータス　　-1：無効
	WORD result;				//実行結果
	ST_JOB_UNIT		from_recipe;//from運転パターン
	ST_JOB_UNIT		to_recipe;	//to運転パターン
	ST_JOB_REPRORT repo1;		//from完了報告
	ST_JOB_REPRORT repo2;		//to完了報告
}ST_JOB_ORDER, *LPST_JOB_ORDER;

/// MODE Order
typedef struct _stMODE_ORDER {	//MODE Order 構造体
	WORD type;				//MODEタイプ
	WORD property;			//実行条件
	WORD status;			//実行ステータス　-1：無効
	WORD result;			//実行結果
}ST_MODE_ORDER, *LPST_MODE_ORDER;

/// E-Stop Order
typedef struct _stESTOP_ORDER {
	WORD type;				//E-STOPタイプ
	WORD property;			//実行条件
	WORD status;			//実行ステータス
	WORD result;			//実行結果
}ST_ESTOP_ORDER, *LPST_ESTOP_ORDER;

/// 手動操作 Order
typedef struct _stMANUAL_ORDER {
	int type;				//手動操作タイプ
	int status;				//実行ステータス　-1：無効
	int anti_sway_mode;		//振止めモード設定PB
	int ope_mode;			//操作モード設定PB
	int remote_mode;		//遠隔運転モード設定PB
	int anti_sway_trigger;	//振れ止め起動PB
	int auto_start;			//自動起動PB
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

/// ユーザインターフェイス Order
typedef struct _stUI_ORDER {
	DWORD type;				//UI処理タイプ
	DWORD status;			//実行ステータス　-1：無効

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
	int auto_mode;
	int remote_mode;		//遠隔運転モード設定PB
	int anti_sway_trigger;	//振れ止め起動PB
	int auto_start;			//自動起動PB

}ST_UI_ORDER, *LPST_UI_ORDER;

/// ステータス報告 Order
typedef struct _stSTAT_ORDER {
	WORD type;				//UI処理タイプ
	WORD status;			//実行ステータス　-1：無効
}ST_STAT_ORDER, *LPST_STAT_ORDER;


/************************************************/
/*   IO TABLE                                   */
/************************************************/

typedef struct _stIO_Physic {
	double M_load;//吊荷質量

	//吊点　　座標原点　xy：旋回軸　z：地面(上が+）
	Vector3 cp;		//吊点xyz
	double R;		//軸長さ
	double th;		//旋回角度
	double ph;		//起伏角度

	Vector3 cv;		//吊点vx vy vz
	double vR;		//軸長さ変化速度
	double wth;		//旋回角速度
	double wph;		//起伏角速度

	//吊荷　　座標原点　xy：旋回軸　z：地面(上が+）
	Vector3 lp;		//吊点xyz
	Vector3 lv;		//吊点vx vy vz


	//吊荷吊点間相対位置
	double L;		//ロープ長
	double lph;		//Z軸との角度
	double lth;		//XY平面角度

	double vL;		//巻速度
	double vlph;	//Z軸との角速度
	double vlth;	//XY平面角速度

	double T;		//振れ周期
	double w0;		//振れ角周波数(2PI()/T）

	Vector3 PhPlane_r;	//Z軸角度の位相平面 x:OmegaTheata y:TheataDot z:angle
	Vector3 PhPlane_n;	//回転座標半径方向の位相平面 x:OmegaTheata y:TheataDot z:angle
	Vector3 PhPlane_t;	//回転座標接線方向の位相平面 x:OmegaTheata y:TheataDot z:angle

	double sway_amp_r_ph2;	//位相平面半径2乗
	double sway_amp_n_ph2;	//位相平面半径2乗　法線方向
	double sway_amp_t_ph2;	//位相平面半径2乗　接線方向

	double sway_amp_r_ph;	//位相平面半径
	double sway_amp_n_ph;	//位相平面半径　法線方向
	double sway_amp_t_ph;	//位相平面半径　接線方向
	
}ST_IO_PHYSIC, *LPST_IO_PHYSIC;

typedef struct _stIO_Ref {

	double	slew_w;				//旋回角速度
	double	hoist_v;			//巻速度
	double	bh_v;				//引込速度
	bool	b_bh_manual_ctrl;	//手動操作中
	bool	b_slew_manual_ctrl;	//手動操作中
	bool	b_mh_manual_ctrl;	//手動操作中

}ST_IO_REF, *LPST_IO_REF;

#define NUM_OF_AS	3
#define AS_SLEW_ID  0
#define AS_BH_ID	1
#define AS_MH_ID	2

typedef struct _stAS_CTRL {
	double tgpos_h;							//巻目標位置
	double tgpos_gt;						//走行目標位置
	double tgpos_slew;						//旋回目標位置
	double tgpos_bh;						//引込目標位置

	double tgspd_h;							//巻目標速度
	double tgspd_gt;						//走行目標速度
	double tgspd_slew;						//旋回目標速度
	double tgspd_bh;						//引込目標速度

	double as_gain_pos[NUM_OF_AS];		//振止ゲイン　位置合わせ用　接線方向  加速時間sec
	double as_gain_damp[NUM_OF_AS];		//振止ゲイン　振れ止め用	接線方向　加速時間sec

	double phase_acc_offset[NUM_OF_AS];		//Offset of center of phase plane on acceleration
	double phase_dec_offset[NUM_OF_AS];		//Offset of center of phase plane on deceleration
	
	double phase_chk_range[NUM_OF_AS];		//振れ止め位相確認許容誤差	
	int as_out_dir[NUM_OF_AS];				//振れ止め出力の方向

	double tgD[NUM_OF_AS];					//振止目標-現在角度
	double tgD_abs[NUM_OF_AS];				//振止目標-現在角度 絶対値

	double allowable_pos_overshoot_plus[NUM_OF_AS];		//振止目標位置オーバー許容値　進行方向
	double allowable_pos_overshoot_ninus[NUM_OF_AS];	//振止目標位置オーバー許容値　進行逆方向

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

#define I_AS_LV_COMPLE		0	//完了判定値
#define I_AS_LV_TRIGGER		1	//振止起動判定
#define I_AS_LV_DAMPING		2	//ダンピングモード判定


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
	double as_compl_nposLv[3];		// m    0:complete 1:trigger 2:spare
	double as_compl_tposLv[3];		// rad  0:complete 1:trigger 2:spare

}ST_SPEC, *LPST_SPEC;

