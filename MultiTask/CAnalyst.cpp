#include "stdafx.h"
#include "CAnalyst.h"

extern CORDER_Table*	pOrder;				//共有メモリOrderクラスポインタ
extern CMODE_Table*		pMode;				//共有メモリModeクラスポインタ
extern ST_SPEC			g_spec;				//クレーン仕様
extern CIO_Table*		pIO_Table;


CAnalyst::CAnalyst(){
}

CAnalyst::~CAnalyst(){
}


void CAnalyst::cal_simulation() {

	//加速度指令値計算  acc_cyl x:r y:th z:z

	double def_w = pIO_Table->physics.vR - pIO_Table->ref.bh_v;
	if (MY_ABS(def_w) <= ALLOWABLE_DEF_BH) hp.acc_cyl_ref.x = 0.0;
	else if (def_w < 0.0) hp.acc_cyl_ref.x = g_spec.bh_acc[FWD_ACC];
	else if (def_w > 0.0) hp.acc_cyl_ref.x = g_spec.bh_acc[FWD_DEC];
	else hp.acc_cyl_ref.x = 0.0;

	def_w = pIO_Table->physics.wth - pIO_Table->ref.slew_w;
	if (MY_ABS(def_w) <= ALLOWABLE_DEF_SLEW) {
		hp.acc_cyl_ref.y = 0.0;
	}
	else if (def_w < 0.0)
		hp.acc_cyl_ref.y = g_spec.slew_acc[FWD_ACC];
	else if (def_w > 0.0)
		hp.acc_cyl_ref.y = g_spec.slew_acc[FWD_DEC];
	else hp.acc_cyl_ref.x = 0.0;

	def_w = pIO_Table->physics.cv.z - pIO_Table->ref.hoist_v;
	if (MY_ABS(def_w) <= ALLOWABLE_DEF_HOIST) hp.acc_cyl_ref.z = 0.0;
	else if (def_w < 0.0) hp.acc_cyl_ref.z = g_spec.hoist_acc[FWD_ACC];
	else if (def_w > 0.0) hp.acc_cyl_ref.z = g_spec.hoist_acc[FWD_DEC];
	else hp.acc_cyl_ref.z = 0.0;

	//MOB計算
	//吊点の動作
	hp.timeEvolution(0.0);  //double t  吊点の計算には経過時間は使わないので0.0

	//吊荷の動作
	hl.timeEvolution(0.0);
	hl.r.add(hl.dr);
	hl.v.add(hl.dv);


	//IO TABLE セット
	pIO_Table->physics.cp = hp.r;
	pIO_Table->physics.cv = hp.v;
	pIO_Table->physics.th = hp.th_sl;	//旋回角度
	pIO_Table->physics.wth = hp.w_sl;	//旋回角速度
	pIO_Table->physics.R = hp.r_bm;		//引込位置
	pIO_Table->physics.vR = hp.v_bm;	//引込速度

	pIO_Table->physics.lp = hl.r;
	pIO_Table->physics.lv = hl.v;

};

void CAnalyst::init_task(void *pobj) {
	set_panel_tip_txt();

	Vector3 _r(0.0, DEFAULT_BH, DEFAULT_HP_Z);
	Vector3 _v(0.0, 0.0, 0.0);

	hp.init_mob((double)inf.cycle_ms / 1000.0, _r, _v);

	_r.z -= DEFAULT_ROPE_L;
	hl.init_mob((double)inf.cycle_ms / 1000.0, _r, _v);

	hl.pHP = &hp; //吊点と紐付け

	return;
};

void CAnalyst::routine_work(void *param) {

	ws << L" working!" << *(inf.psys_counter) % 100 << " x:y ; " << hp.r.x << ":" << hp.r.y;
	tweet2owner(ws.str()); ws.str(L""); ws.clear();

	//シミュレータ計算
	if (pMode->auto_control != ENV_MODE_SIM2) cal_simulation();

};



