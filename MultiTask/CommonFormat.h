#pragma once

#include "CodeDef.h"

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

/// JOB Order
typedef struct _stMotion_Element {	//運動要素
	int type;		//制御種別
	int status;		//制御種状態
	double _a;		//目標加減速度
	double _v;		//目標速度
	double _p;		//目標位置
	double _t;		//継続時間
	double vh_lim;	//速度制限High
	double vl_lim;	//速度制限Low
	double opt_d1;	//オプションdouble
	double opt_d2;	//オプションdouble
	int opt_i1;		//オプションint
	int opt_i2;		//オプションint
}ST_MOTION_ELEMENT, *LPST_MOTION_ELEMENT;

#define M_ELEMENT_MAX	32
#define M_AXIS			8	//動作軸
#define MH_AXIS			0	//主巻動作
#define TT_AXIS			1	//横行動作
#define GT_AXIS			2	//走行動作
#define BH_AXIS			3	//起伏動作
#define SLW_AXIS		4	//旋回動作
#define SKW_AXIS		5	//スキュー動作
#define LFT_AXIS		6	//吊具操作
typedef struct _stJOB_UNIT {	//作業要素（PICK、GROUND、PARK）
	int type;		//動作種別
	int mAct[M_AXIS];//実行対象　配列[0]から動作コードセット-1セットで終わり
	int iAct[M_AXIS];//実行中index -1で完了
	ST_MOTION_ELEMENT motions[M_AXIS][M_ELEMENT_MAX];
}ST_JOB_UNIT, *LPST_JOB_UNIT;

typedef struct _stJOB_Report {	//JOB完了報告フォーマット
	WORD status;
}ST_JOB_REPRORT, *LPST_JOB_REPRORT;

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
	WORD type;				//手動操作タイプ
	WORD status;			//実行ステータス　-1：無効
	double mh;
	double tt;
	double gt;
	double bh;
	double slew;
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

	DWORD env_mode;
	DWORD ope_mode;
	DWORD as_mode;
	DWORD auto_mode;

}ST_UI_ORDER, *LPST_UI_ORDER;

/// ステータス報告 Order
typedef struct _stSTAT_ORDER {
	WORD type;				//UI処理タイプ
	WORD status;			//実行ステータス　-1：無効
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

#define ALLOWABLE_DEF_SLEW	0.003 //旋回指令とFBの許容誤差　rad/s 1ノッチの10％	
#define ALLOWABLE_DEF_BH	0.005 //引込指令とFBの許容誤差　m/s 1ノッチの5％
#define ALLOWABLE_DEF_HOIST	0.005 //引込指令とFBの許容誤差　m/s 1ノッチの10％
#define MY_ABS(a)	((a)<0.0 ? -(a) : (a))

inline double rad2deg(double rad) { return COF_RAD2DEG * rad; }
