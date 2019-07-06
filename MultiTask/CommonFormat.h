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

typedef struct _stCLIENT_ORDER{
	WORD status;
	WORD order;
}ST_CLIENT_ORDER, *LPST_CLIENT_ORDER;

typedef struct _stCLIENT_ORDER_RECEIPT{
	WORD status;
	WORD receipt;
}ST_CLIENT_ORDER_RECEIPT, *LPST_CLIENT_ORDER_RECEIPT;

typedef struct _stCLIENT_ORDER_REPORT {
	WORD status;
	WORD report;
}ST_CLIENT_ORDER_REPORT, *LPST_CLIENT_ORDER_REPORT;

typedef struct _stCLIENT_ORDER_PACK {
	WORD status;
	ST_CLIENT_ORDER order;
	ST_CLIENT_ORDER_RECEIPT receipt;
	ST_CLIENT_ORDER_REPORT	report;
}ST_CLIENT_ORDER_PACK, *LPST_CLIENT_ORDER_PACK;

typedef struct _stORDER_FILE {
	WORD ihot;
	ST_CLIENT_ORDER_PACK order_pack[ORDER_MAX];
}ST_ORDER_FILE, *LPST_ORDER_FILE;


//### JOB LIST ###

typedef struct _stJOB_RECIPE {
	WORD preproc;
	WORD from;
	WORD midproc;
	WORD to;
	WORD postproc;
}ST_JOB_RECIPE, *LPST_JOB_RECIPE;

typedef struct _stJOB {
	WORD jobID;
	ST_JOB_RECIPE recipe;
}ST_JOB, *LPST_JOB;

typedef struct _stJOB_REPORT {
	WORD report;
}ST_JOB_REPORT, *LPST_JOB_REPORT;


typedef struct _stJOB_PACK {

	LPST_CLIENT_ORDER_PACK pOrder[ORDER_MAX];
	ST_CLIENT_ORDER_RECEIPT receipt[ORDER_MAX];
	ST_CLIENT_ORDER_REPORT	report[ORDER_MAX];

}ST_JOB_PACK, *LPST_JOB_PACK;

typedef struct _stJOB_FILE {
	WORD ihot;
	ST_JOB_PACK job_pack[ORDER_MAX];

}ST_JOB_FILE, *LPST_JOB_FILE;