#include "stdafx.h"
#include "CAnalyst.h"
#include "SharedObjects.h"

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
	if (MY_ABS(def_w) <= ALLOWABLE_DEF_BH) hp.a_bm_ref = 0.0;
	else if (def_w < 0.0) hp.a_bm_ref = g_spec.bh_acc[FWD_ACC];
	else if (def_w > 0.0) hp.a_bm_ref = g_spec.bh_acc[FWD_DEC];
	else hp.a_bm_ref = 0.0;

	def_w = pIO_Table->physics.wth - pIO_Table->ref.slew_w;
	if (MY_ABS(def_w) <= ALLOWABLE_DEF_SLEW) {
		hp.dw_sl_ref = 0.0;
	}
	else if (def_w < 0.0)
		hp.dw_sl_ref = g_spec.slew_acc[FWD_ACC];
	else if (def_w > 0.0)
		hp.dw_sl_ref = g_spec.slew_acc[FWD_DEC];
	else hp.dw_sl_ref = 0.0;

	def_w = pIO_Table->physics.vL - pIO_Table->ref.hoist_v;
	if (MY_ABS(def_w) <= ALLOWABLE_DEF_HOIST*2) 
		hp.a_h_ref = 0.0;
	else if (def_w < 0.0) 
		hp.a_h_ref = g_spec.hoist_acc[FWD_ACC];
	else if (def_w > 0.0)  
		hp.a_h_ref = g_spec.hoist_acc[FWD_DEC];
	else  hp.a_h_ref = 0.0;

	//MOB計算
	//吊点の動作
	hp.timeEvolution(0.0);  //double t  吊点の計算には経過時間は使わないので0.0

	//吊荷の動作
	hl.timeEvolution(0.0); //double t  吊点の計算には経過時間は使わないので0.0
	hl.r.add(hl.dr);
	hl.v.add(hl.dv);


	//IO TABLE セット

	pIO_Table->physics.cp = hp.r; 		//吊点xyz

	pIO_Table->physics.R = hp.r_bm;		//軸長さ
	pIO_Table->physics.th = hp.th_sl;	//旋回角度
	pIO_Table->physics.ph = 0.0;		//起伏角度

	pIO_Table->physics.cv = hp.v; 		//吊点vx vy vz
	pIO_Table->physics.vR = hp.v_bm;	//軸長さ変化速度
	pIO_Table->physics.wth = hp.w_sl;	//旋回角速度
	pIO_Table->physics.wph = 0.0;		//起伏角速度

	pIO_Table->physics.lp = hl.r;		//吊荷xyz
	pIO_Table->physics.lv = hl.v;		//吊荷速度vx vy vz
		
	Vector3 rel_lp =  hl.r - hp.r;		//吊荷相対xyz
	Vector3 rel_lvp = hl.v - hp.v;		//吊荷相対速度vx vy vz

	
	double last_L = pIO_Table->physics.L;
	//ロープ長
	pIO_Table->physics.L = hp.l_h;	
	pIO_Table->physics.vL = hp.v_h;	//巻速度
	
	//　Z軸との角度
	
	double temp_f = sqrt(rel_lp.x * rel_lp.x + rel_lp.y * rel_lp.y);//XY平面半径
	pIO_Table->physics.lph = asin(temp_f/ pIO_Table->physics.L);
	
//Z軸角度の位相平面  x:OmegaTheata y:TheataDot
pIO_Table->physics.PhPlane_r.x = temp_f / pIO_Table->physics.L * pIO_Table->physics.w0;
pIO_Table->physics.PhPlane_r.y = sqrt(rel_lvp.x * rel_lvp.x + rel_lvp.y * rel_lvp.y) / pIO_Table->physics.L;
if (pIO_Table->physics.PhPlane_r.x < 0.0) {
	if (pIO_Table->physics.PhPlane_r.y < 0.0) pIO_Table->physics.PhPlane_r.z -= DEF_PI;
	else pIO_Table->physics.PhPlane_r.z += DEF_PI;
}
pIO_Table->physics.sway_amp_r = pIO_Table->physics.PhPlane_r.x * pIO_Table->physics.PhPlane_r.x + pIO_Table->physics.PhPlane_r.y * pIO_Table->physics.PhPlane_r.y;

//  XY平面角度
double radious = pIO_Table->physics.L * sin(pIO_Table->physics.lph);

if (radious < 0.0001) radious = 0.0001;

double last_th = pIO_Table->physics.lth;
pIO_Table->physics.lth = acos(rel_lp.x / radious);


if (pIO_Table->physics.L > 1.0)
pIO_Table->physics.w0 = sqrt(DEF_G / pIO_Table->physics.L);//振れ角周波数
pIO_Table->physics.T = DEF_2PI / pIO_Table->physics.w0;			//振れ周期

//xy平面半径方向の位相平面  x:OmegaTheata y:TheataDot　z:Phi
pIO_Table->physics.PhPlane_n.x = (rel_lp.x * sin(pIO_Table->physics.th) + rel_lp.y *cos(pIO_Table->physics.th)) / pIO_Table->physics.L * pIO_Table->physics.w0;
pIO_Table->physics.PhPlane_n.y = (rel_lvp.x * sin(pIO_Table->physics.th) + rel_lvp.y *cos(pIO_Table->physics.th)) / pIO_Table->physics.L;
pIO_Table->physics.PhPlane_n.z = atan(pIO_Table->physics.PhPlane_n.y / pIO_Table->physics.PhPlane_n.x);
if (pIO_Table->physics.PhPlane_n.x < 0.0) {
	if (pIO_Table->physics.PhPlane_n.y < 0.0) pIO_Table->physics.PhPlane_n.z -= DEF_PI;
	else pIO_Table->physics.PhPlane_n.z += DEF_PI;
}
pIO_Table->physics.sway_amp_n = pIO_Table->physics.PhPlane_n.x * pIO_Table->physics.PhPlane_n.x + pIO_Table->physics.PhPlane_n.y * pIO_Table->physics.PhPlane_n.y;

//xy平面接線方向の位相平面  x:OmegaTheata y:TheataDot　z:Phi
pIO_Table->physics.PhPlane_t.x = (rel_lp.x * cos(pIO_Table->physics.th) - rel_lp.y *sin(pIO_Table->physics.th)) / pIO_Table->physics.L* pIO_Table->physics.w0;
pIO_Table->physics.PhPlane_t.y = (rel_lvp.x * cos(pIO_Table->physics.th) - rel_lvp.y *sin(pIO_Table->physics.th)) / pIO_Table->physics.L;
pIO_Table->physics.PhPlane_t.z = atan(pIO_Table->physics.PhPlane_t.y / pIO_Table->physics.PhPlane_t.x);
if (pIO_Table->physics.PhPlane_t.x < 0.0) {
	if (pIO_Table->physics.PhPlane_t.y < 0.0) pIO_Table->physics.PhPlane_t.z -= DEF_PI;
	else pIO_Table->physics.PhPlane_t.z += DEF_PI;
}

pIO_Table->physics.sway_amp_t = pIO_Table->physics.PhPlane_t.x * pIO_Table->physics.PhPlane_t.x + pIO_Table->physics.PhPlane_t.y * pIO_Table->physics.PhPlane_t.y;

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
	Vector3 rel_lp = hl.r - hp.r;		//吊荷相対xyz

	ws << L" working!" << *(inf.psys_counter) % 100 << "  Amp t:n ;"  << pIO_Table->physics.sway_amp_t << ":" << pIO_Table->physics.sway_amp_n << " Mode  t:n ;  " << pMode->antisway_control_t << ":" << pMode->antisway_control_n;
	tweet2owner(ws.str()); ws.str(L""); ws.clear();

	//シミュレータ計算
	if (pMode->auto_control != ENV_MODE_SIM2) cal_simulation();

	update_as_ctrl();
	cal_as_gain();
	cal_as_target();


};

LPST_JOB_ORDER CAnalyst::cal_job_recipe(int job_type) {
	return &(pOrder->job_A);
};

void CAnalyst::cal_as_target() {
	if (pMode->antisway != OPE_MODE_AS_ON) {
		pIO_Table->as_ctrl.tgpos_bh = pIO_Table->physics.R;
		pIO_Table->as_ctrl.tgpos_slew = pIO_Table->physics.th;
	}
	return;
};

void CAnalyst::update_as_ctrl() {
	if (pMode->antisway != OPE_MODE_AS_ON) {
		pMode->antisway_control_h = pMode->antisway_control_t = pMode->antisway_control_n = AS_MODE_DEACTIVATE;
		return;
	}
	else {
		pMode->antisway_control_h = pMode->antisway_control_t = pMode->antisway_control_n = AS_MODE_STANDBY;
	}

	if (pMode->auto_control != AUTO_MODE_ACTIVE) {
		if (pIO_Table->physics.sway_amp_n > g_spec.as_compl_swayLv_sq[I_AS_LV_TRIGGER]) {
			if (abs(pIO_Table->physics.R - pIO_Table->as_ctrl.tgpos_bh) < g_spec.as_compl_nposLv[I_AS_LV_TRIGGER]) 
				pMode->antisway_control_n = AS_MODE_ACTIVE_INCH_SWAY;
			else if(pIO_Table->physics.sway_amp_n > g_spec.as_compl_swayLv_sq[I_AS_LV_ANTISWAY])
				pMode->antisway_control_n = AS_MODE_ACTIVE_INCH_SWAY;
			else
				pMode->antisway_control_n = AS_MODE_ACTIVE_INCH_POS;
		}
		if (pIO_Table->physics.sway_amp_t > g_spec.as_compl_swayLv_sq[I_AS_LV_TRIGGER]) {
			if (abs(pIO_Table->physics.th - pIO_Table->as_ctrl.tgpos_slew) < g_spec.as_compl_tposLv[I_AS_LV_TRIGGER])
				pMode->antisway_control_t = AS_MODE_ACTIVE_INCH_SWAY;
			else if (pIO_Table->physics.sway_amp_t > g_spec.as_compl_swayLv_sq[I_AS_LV_ANTISWAY])
				pMode->antisway_control_t = AS_MODE_ACTIVE_INCH_SWAY;
			else
				pMode->antisway_control_t = AS_MODE_ACTIVE_INCH_POS;
		}
	}
	else {
		;
	}

	return;
};
void CAnalyst::cal_as_gain() {

	return;
};


