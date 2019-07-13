#pragma once

///# タスクオブジェクトインデックス構造体
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
typedef struct _stJOB_REPRORT {
	WORD code;
	WORD property;
	WORD status;
}ST_JOB_REPRORT, *LPST_JOB_REPRORT;

typedef struct _stJOB_ORDER {
	WORD code;
	WORD property;
	WORD status;
	WORD responce;
	ST_JOB_REPRORT repo1;
	ST_JOB_REPRORT repo2;
}ST_JOB_ORDER, *LPST_JOB_ORDER;

typedef struct _stMODE_ORDER {
	WORD code;
	WORD property;
	WORD status;
	WORD responce;
}ST_MODE_ORDER, *LPST_MODE_ORDER;

typedef struct _stESTOP_ORDER {
	WORD code;
	WORD property;
	WORD status;
	WORD responce;
}ST_ESTOP_ORDER, *LPST_ESTOP_ORDER;

typedef struct _stMANUAL_ORDER {
	WORD status;
	WORD hoist;
	WORD trolley;
	WORD gantry;
	WORD relief;
	WORD turn;
}ST_MANUAL_ORDER, *LPST_MANUAL_ORDER;

typedef struct _stUI_ORDER {
	WORD status;
	WORD hoist;
	WORD trolley;
	WORD gantry;
	WORD relief;
	WORD turn;
}ST_UI_ORDER, *LPST_UI_ORDER;





