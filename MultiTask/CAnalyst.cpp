#include "stdafx.h"
#include "CAnalyst.h"
#include "SharedObjects.h"
#include "CPlayer.h"

extern CORDER_Table*	pOrder;				//共有メモリOrderクラスポインタ
extern CMODE_Table*		pMode;				//共有メモリModeクラスポインタ
extern ST_SPEC			g_spec;				//クレーン仕様
extern CIO_Table*		pIO_Table;


CAnalyst::CAnalyst(){
	for (int i = 0; i < NUM_OF_AS; i++)	pIO_Table->auto_ctrl.phase_chk_range[i] = DEF_PI / 50.0;
}

CAnalyst::~CAnalyst(){
}



void CAnalyst::cal_simulation() {

	//##加速度指令値計算  acc_cyl x:r y:th z:z

	double def_w = pIO_Table->physics.vR - pIO_Table->ref.bh_v;
	if (MY_ABS(def_w) <= ALLOWABLE_DEF_BH) hp.a_bm_ref = 0.0;	//0速指令判定
	else if (def_w < 0.0) hp.a_bm_ref = g_spec.bh_acc[FWD_ACC];
	else if (def_w > 0.0) hp.a_bm_ref = g_spec.bh_acc[FWD_DEC];
	else hp.a_bm_ref = 0.0;

	def_w = pIO_Table->physics.wth - pIO_Table->ref.slew_w;
	if (MY_ABS(def_w) <= ALLOWABLE_DEF_SLEW) hp.dw_sl_ref = 0.0;
	else if (def_w < 0.0)	hp.dw_sl_ref = g_spec.slew_acc[FWD_ACC];
	else if (def_w > 0.0)	hp.dw_sl_ref = g_spec.slew_acc[FWD_DEC];
	else hp.dw_sl_ref = 0.0;

	def_w = pIO_Table->physics.vL - pIO_Table->ref.hoist_v;
	if (MY_ABS(def_w) <= ALLOWABLE_DEF_HOIST*2) hp.a_h_ref = 0.0;
	else if (def_w < 0.0) hp.a_h_ref = g_spec.hoist_acc[FWD_ACC];
	else if (def_w > 0.0) hp.a_h_ref = g_spec.hoist_acc[FWD_DEC];
	else  hp.a_h_ref = 0.0;

	//##MOB計算
	//吊点の動作
	hp.timeEvolution(0.0);  //double t  吊点の計算には経過時間は使わないので0.0

	//吊荷の動作
	hl.timeEvolution(0.0); //double t  吊荷の計算には経過時間は使わないので0.0
	
	hl.r.add(hl.dr);
	hl.v.add(hl.dv);


	//##IO TABLE セット

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
	//###ロープ長
	pIO_Table->physics.L = hp.l_h;	
	pIO_Table->physics.vL = hp.v_h;	//巻速度
	
	//###Z軸との角度

	double temp_r = sqrt(rel_lp.x * rel_lp.x + rel_lp.y * rel_lp.y);//XY平面半径
	double temp_v = sqrt(rel_lvp.x * rel_lvp.x + rel_lvp.y * rel_lvp.y);//XY平面半径速度
	pIO_Table->physics.lph = asin(temp_r / pIO_Table->physics.L);//ｚ軸との角度

	//###Z軸角度の位相平面  x:Theata y:TheataDot/Omega
	pIO_Table->physics.PhPlane_r.x = temp_r / pIO_Table->physics.L;//r/L = theata
	if (pIO_Table->physics.PhPlane_r.x < DEF_001DEG)pIO_Table->physics.PhPlane_r.x = DEF_001DEG;//0割り防止用下限

	pIO_Table->physics.PhPlane_r.y = temp_v / (pIO_Table->physics.L * pIO_Table->physics.w0);//vr/L/w = thata dot/w

	double last_rz = pIO_Table->physics.PhPlane_r.z; //r位相前回値

	pIO_Table->physics.PhPlane_r.z = atan(pIO_Table->physics.PhPlane_r.y / pIO_Table->physics.PhPlane_r.x);
	buf_average_phase_rdz[i_buf_rdz] = pIO_Table->physics.PhPlane_r.z - last_rz; i_buf_rdz++;
	if (i_buf_rdz > MAX_SAMPLE_AVERAGE_RZ - 1) i_buf_rdz = 0;

	double temp_double=0.0;
	for (int i = 0; i < MAX_SAMPLE_AVERAGE_RZ; i++) temp_double += buf_average_phase_rdz[i];
	pIO_Table->physics.wPhPlane_r = temp_double/ MAX_SAMPLE_AVERAGE_RZ;


	pIO_Table->physics.sway_amp_r_ph2 = pIO_Table->physics.PhPlane_r.x * pIO_Table->physics.PhPlane_r.x + pIO_Table->physics.PhPlane_r.y * pIO_Table->physics.PhPlane_r.y;
	pIO_Table->physics.sway_amp_r_ph = sqrt(pIO_Table->physics.sway_amp_r_ph2);



	//###XY平面角度
	double radious = pIO_Table->physics.L * sin(pIO_Table->physics.lph);
	if (radious < DEF_001DEG) radious = DEF_001DEG;
	double last_th = pIO_Table->physics.lth;
	pIO_Table->physics.lth = acos(rel_lp.x / radious);

	//振れ角周波数
	if (pIO_Table->physics.L > 1.0)	pIO_Table->physics.w0 = sqrt(DEF_G / pIO_Table->physics.L);
	else pIO_Table->physics.w0 = sqrt(DEF_G);
	//振れ周期
	pIO_Table->physics.T = DEF_2PI / pIO_Table->physics.w0;

	//###xy平面半径方向(引込）の位相平面  x:Theata y:TheataDot/Omega　z:Phi 
	pIO_Table->physics.PhPlane_n.x = (rel_lp.x * sin(pIO_Table->physics.th) + rel_lp.y *cos(pIO_Table->physics.th)) / pIO_Table->physics.L;
	pIO_Table->physics.PhPlane_n.y = (rel_lvp.x * sin(pIO_Table->physics.th) + rel_lvp.y *cos(pIO_Table->physics.th)) / (pIO_Table->physics.L * pIO_Table->physics.w0);

	if (abs(pIO_Table->physics.PhPlane_n.x) < 0.000001) {
		pIO_Table->physics.PhPlane_n.z = 0.0;
	}
	else {
		pIO_Table->physics.PhPlane_n.z = atan(pIO_Table->physics.PhPlane_n.y / pIO_Table->physics.PhPlane_n.x);
	}
	if (pIO_Table->physics.PhPlane_n.x < 0.0) {//位相は-π～πで表現する
		if (pIO_Table->physics.PhPlane_n.y < 0.0) pIO_Table->physics.PhPlane_n.z -= DEF_PI;
		else pIO_Table->physics.PhPlane_n.z += DEF_PI;
	}
	pIO_Table->physics.sway_amp_n_ph2 = pIO_Table->physics.PhPlane_n.x * pIO_Table->physics.PhPlane_n.x + pIO_Table->physics.PhPlane_n.y * pIO_Table->physics.PhPlane_n.y;
	pIO_Table->physics.sway_amp_n_ph = sqrt(pIO_Table->physics.sway_amp_n_ph2);

	//###xy平面接線方向の位相平面  x:Theata y:TheataDot/Omega　z:Phi
	pIO_Table->physics.PhPlane_t.x = (rel_lp.x * cos(pIO_Table->physics.th) - rel_lp.y *sin(pIO_Table->physics.th)) / pIO_Table->physics.L;
	pIO_Table->physics.PhPlane_t.y = (rel_lvp.x * cos(pIO_Table->physics.th) - rel_lvp.y *sin(pIO_Table->physics.th)) / (pIO_Table->physics.L* pIO_Table->physics.w0);
	
	if (abs(pIO_Table->physics.PhPlane_t.x) < 0.000001) {
		pIO_Table->physics.PhPlane_t.z = 0.0;
	}
	else {
		pIO_Table->physics.PhPlane_t.z = atan(pIO_Table->physics.PhPlane_t.y / pIO_Table->physics.PhPlane_t.x);
	}


	if (pIO_Table->physics.PhPlane_t.x < 0.0) {//位相は-π～πで表現する
		if (pIO_Table->physics.PhPlane_t.y < 0.0) pIO_Table->physics.PhPlane_t.z -= DEF_PI;
		else pIO_Table->physics.PhPlane_t.z += DEF_PI;
	}
	pIO_Table->physics.sway_amp_t_ph2 = pIO_Table->physics.PhPlane_t.x * pIO_Table->physics.PhPlane_t.x + pIO_Table->physics.PhPlane_t.y * pIO_Table->physics.PhPlane_t.y;
	pIO_Table->physics.sway_amp_t_ph = sqrt(pIO_Table->physics.sway_amp_t_ph2);


	//###各位相平面の加減速時の回転中心OFFSET値　r0 rad

	pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID] = g_spec.slew_acc[FWD_ACC] * pIO_Table->physics.R / DEF_G;	//Offset of center of phase plane on acceleration
	pIO_Table->auto_ctrl.phase_dec_offset[AS_SLEW_ID] = g_spec.slew_acc[FWD_DEC] * pIO_Table->physics.R / DEF_G;	//Offset of center of phase plane on deceleration
	pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID] = g_spec.bh_acc[FWD_ACC] / DEF_G;								//Offset of center of phase plane on acceleration
	pIO_Table->auto_ctrl.phase_dec_offset[AS_BH_ID] = g_spec.bh_acc[FWD_DEC] / DEF_G;								//Offset of center of phase plane on deceleration

};

void CAnalyst::init_task(void *pobj) {
	set_panel_tip_txt();

	Vector3 _r(0.0, DEFAULT_BH, DEFAULT_HP_Z);
	Vector3 _v(0.0, 0.0, 0.0);

	hp.init_mob((double)inf.cycle_ms / 1000.0, _r, _v);

	_r.z -= DEFAULT_ROPE_L;
	hl.init_mob((double)inf.cycle_ms / 1000.0, _r, _v);

	hl.pHP = &hp; //吊点と紐付け

	pIO_Table->auto_ctrl.allowable_pos_overshoot_plus[AS_BH_ID]= 1.0;		//振止目標位置オーバー許容値　進行方向
	pIO_Table->auto_ctrl.allowable_pos_overshoot_plus[AS_SLEW_ID] = 1.0;	//振止目標位置オーバー許容値　進行方向
	pIO_Table->auto_ctrl.allowable_pos_overshoot_plus[AS_MH_ID] = 1.0;		//振止目標位置オーバー許容値　進行方向

	pIO_Table->auto_ctrl.allowable_pos_overshoot_minus[AS_BH_ID] = 1.0;		//振止目標位置オーバー許容値　進行逆方向
	pIO_Table->auto_ctrl.allowable_pos_overshoot_minus[AS_SLEW_ID] = 1.0;	//振止目標位置オーバー許容値　進行逆方向
	pIO_Table->auto_ctrl.allowable_pos_overshoot_minus[AS_MH_ID] = 1.0;		//振止目標位置オーバー許容値　進行逆方向

	//位相角速度平均処理用バッファ初期化
	for (int i = 0; i < MAX_SAMPLE_AVERAGE_RZ; i++) buf_average_phase_rdz[i] = 0.0;
	i_buf_rdz = 0;
	
	//2STEP 小移動最大移動距離
	pIO_Table->auto_ctrl.Dmax_2step[AS_BH_ID] = 2.0 * g_spec.bh_notch_spd[NOTCH_MAX - 1] * g_spec.bh_notch_spd[NOTCH_MAX - 1] / g_spec.bh_acc[FWD_ACC];
	pIO_Table->auto_ctrl.Dmax_2step[AS_SLEW_ID] = 2.0 * g_spec.slew_notch_spd[NOTCH_MAX - 1] * g_spec.slew_notch_spd[NOTCH_MAX - 1] / g_spec.slew_acc[FWD_ACC];

	return;
};

void CAnalyst::routine_work(void *param) {
	Vector3 rel_lp = hl.r - hp.r;		//吊荷相対xyz
//	ws << L" working!" << *(inf.psys_counter) % 100 << "  AS ptn n ;" << pMode->antisway_ptn_n << ": AS ctr n" << pMode->antisway_control_n << "__   AS ptn t ;" << pMode->antisway_ptn_t << ": AS ctr t" << pMode->antisway_control_t;
	
	ws << L"BH AS:" << pMode->antisway_control_n << L"SLEW AS:" << pMode->antisway_control_t;
	tweet2owner(ws.str()); ws.str(L""); ws.clear();

	//シミュレータ計算
	if (pMode->environment != ENV_MODE_SIM2) cal_simulation();

	cal_auto_target(CAL_FOR_MANUAL);
	update_auto_ctrl();
};

//@@@
//##############################################
void CAnalyst::cal_auto_target(int mode) {
	if (mode == CAL_FOR_AUTO_JOB_A) {
		pIO_Table->auto_ctrl.tgpos_bh = pOrder->job_A.command_target[pOrder->job_A.job_step_now].bh_pos;
		pIO_Table->auto_ctrl.tgpos_slew = pOrder->job_A.command_target[pOrder->job_A.job_step_now].slew_pos;
	}
	else {
		if (pMode->antisway != OPE_MODE_AS_ON) {
			pIO_Table->auto_ctrl.tgpos_bh = pIO_Table->physics.R;
			pIO_Table->auto_ctrl.tgpos_slew = pIO_Table->physics.th;	//旋回目標位置は旋回角度
		}
		else {
			if (pMode->antisway_control_n == AS_MOVE_INTERRUPT)	pIO_Table->auto_ctrl.tgpos_bh = pIO_Table->physics.R;
			if (pMode->antisway_control_t == AS_MOVE_INTERRUPT)	pIO_Table->auto_ctrl.tgpos_slew = pIO_Table->physics.th;
		}
	}

	if (pMode->antisway_hoist != OPE_MODE_AS_ON) {
		pIO_Table->auto_ctrl.tgpos_h = pIO_Table->physics.L;
	}
	else {
		;
	}

	pIO_Table->auto_ctrl.tgD[AS_BH_ID] = pIO_Table->auto_ctrl.tgpos_bh - pIO_Table->physics.R;
	pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] = abs(pIO_Table->auto_ctrl.tgD[AS_BH_ID]);
	pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] = pIO_Table->auto_ctrl.tgpos_slew - pIO_Table->physics.th;
	pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] = abs(pIO_Table->auto_ctrl.tgD[AS_SLEW_ID]);
	pIO_Table->auto_ctrl.tgD[AS_MH_ID] = pIO_Table->auto_ctrl.tgpos_h - pIO_Table->physics.L;
	pIO_Table->auto_ctrl.tgD_abs[AS_MH_ID] = abs(pIO_Table->auto_ctrl.tgD[AS_MH_ID]);

	return;
};

//### Update Anti-sway Control Mode     ##########################################
void CAnalyst::update_auto_ctrl() {

	// Update Anti-Sway Control mode
	if (pMode->antisway != OPE_MODE_AS_ON) {
		pMode->antisway_control_h = pMode->antisway_control_t = pMode->antisway_control_n = AS_MOVE_DEACTIVATE;
		pMode->antisway_ptn_h = pMode->antisway_ptn_t = pMode->antisway_ptn_n = AS_PTN_0;
		return;
	}

	// ###Normal direction

	if (pIO_Table->ref.b_bh_manual_ctrl) {	//手動操作中
		pMode->antisway_ptn_n = AS_PTN_0;
		pMode->antisway_control_n = AS_MOVE_INTERRUPT;
	}
	else if ((pIO_Table->physics.vR != 0.0) || (pMode->antisway_hoist == OPE_MODE_AS_ON) ){	//振止動作中
		pMode->antisway_ptn_n = AS_PTN_0;
		pMode->antisway_control_n = AS_MOVE_STANDBY;
	}
	else if ((pIO_Table->physics.sway_amp_n_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) &&			//振れが完了レベル
				(pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] < g_spec.as_compl_nposLv[I_AS_LV_COMPLE])){	//位置が完了レベル
		pMode->antisway_ptn_n = AS_PTN_0;
		pMode->antisway_control_n = AS_MOVE_COMPLETE;
	}
#if 1
	else {
		pMode->antisway_control_n = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_n = AS_PTN_2ACCDEC;
	}
#else
	else if (pIO_Table->physics.sway_amp_n_ph > pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID]) {	//振れが加速振れより大
			pMode->antisway_control_n = AS_MOVE_ANTISWAY;
			pMode->antisway_ptn_n = AS_PTN_1STEP;
	}
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] < g_spec.as_compl_nposLv[I_AS_LV_TRIGGER]) {	//目標が位置決め起動判定距離内
		if (pIO_Table->physics.sway_amp_n_ph > g_spec.as_compl_swayLv[I_AS_LV_TRIGGER]){			//振れがトリガ判定値以上
			pMode->antisway_control_n = AS_MOVE_ANTISWAY;
			pMode->antisway_ptn_n = AS_PTN_2STEP_PN;
		}
		else {
			if (pMode->antisway_control_n != AS_MOVE_COMPLETE) {
				pMode->antisway_control_n = AS_MOVE_ANTISWAY;
				pMode->antisway_ptn_n = AS_PTN_2STEP_PN;
			}
			else {
				pMode->antisway_ptn_n = AS_PTN_0;
			}
		}
	}
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] <= pIO_Table->auto_ctrl.Dmax_2step[AS_BH_ID]) { //2回インチング最大移動距離以下
		pMode->antisway_control_n = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_n = AS_PTN_2STEP_PP;
	}
	else{
		pMode->antisway_control_n = AS_MOVE_ANTISWAY;

		//2段加減速最小移動距離
		double Dmin = g_spec.bh_notch_spd[NOTCH_MAX - 1]*(pIO_Table->physics.T + g_spec.bh_notch_spd[NOTCH_MAX - 1] / g_spec.bh_acc[FWD_ACC]);
		//2段加減速
		if (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] <= Dmin) pMode->antisway_ptn_n = AS_PTN_3STEP;
		//3段
		else pMode->antisway_ptn_n = AS_PTN_2ACCDEC;
	}
#endif
	//##### Tangent direction

	if (pIO_Table->ref.b_slew_manual_ctrl) {
		pMode->antisway_ptn_t = AS_PTN_0;
		pMode->antisway_control_t = AS_MOVE_INTERRUPT;
	}
	else if ((pIO_Table->physics.wth != 0.0)||(pMode->antisway_hoist == OPE_MODE_AS_ON)) {	//振止動作中
		pMode->antisway_ptn_t = AS_PTN_0;
		pMode->antisway_control_t = AS_MOVE_STANDBY;
	}
	else if ((pIO_Table->physics.sway_amp_t_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) &&		//目標が位置決め完了判定距離内
		(pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] < g_spec.as_compl_tposLv[I_AS_LV_COMPLE])) {	//振れが完了判定内
		pMode->antisway_ptn_t = AS_PTN_0;
		pMode->antisway_control_t = AS_MOVE_COMPLETE;
	}
#if 1
	else{	//振れが加速振れ以上
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_t = AS_PTN_2ACCDEC;
	}
#else
	else if (pIO_Table->physics.sway_amp_t_ph > pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID]) {	//振れが加速振れ以上
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_t = AS_PTN_1STEP;
	}
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] < g_spec.as_compl_tposLv[I_AS_LV_TRIGGER]) {	//目標が位置決め起動判定距離内
		if (pIO_Table->physics.sway_amp_t_ph > g_spec.as_compl_swayLv[I_AS_LV_TRIGGER]) {	//振れがトリガ判定値以上
			pMode->antisway_control_t = AS_MOVE_ANTISWAY;
			pMode->antisway_ptn_t = AS_PTN_2STEP_PN;
		}
		else {
			if (pMode->antisway_control_t != AS_MOVE_COMPLETE) {
				pMode->antisway_control_t = AS_MOVE_ANTISWAY;
				pMode->antisway_ptn_t = AS_PTN_2STEP_PN;
			}
			else 
				pMode->antisway_ptn_t = AS_PTN_0;
		}
	}
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] <= pIO_Table->auto_ctrl.Dmax_2step[AS_SLEW_ID]) { //2回インチング最大移動距離以下
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_t = AS_PTN_2STEP_PP;
	}
	else {
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;

		//2段加減速最小移動距離
		double Dmin = g_spec.slew_notch_spd[NOTCH_MAX - 1] * (pIO_Table->physics.T + g_spec.slew_notch_spd[NOTCH_MAX - 1] / g_spec.bh_acc[FWD_ACC]);
		//2段加減速
		if (pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] <= Dmin) pMode->antisway_ptn_t = AS_PTN_3STEP;
		else pMode->antisway_ptn_t = AS_PTN_2ACCDEC;
	}
#endif
	//##### Hoist direction

	if (pMode->antisway_hoist != OPE_MODE_AS_ON) {
		pMode->antisway_ptn_h = AS_PTN_0;
		pMode->antisway_control_h = AS_MOVE_DEACTIVATE;
	}
	else if (pIO_Table->ref.b_mh_manual_ctrl){
		pMode->antisway_ptn_h = AS_PTN_0;
		pMode->antisway_control_h = AS_MOVE_INTERRUPT;
	}
	else if (pIO_Table->physics.vL != 0.0) {
		pMode->antisway_ptn_h = AS_PTN_0;
		pMode->antisway_control_h = AS_MOVE_STANDBY;
	}
	else if (pIO_Table->physics.sway_amp_r_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) {	//振れが完了判定内
		pMode->antisway_ptn_h = AS_PTN_0;
		pMode->antisway_control_h = AS_MOVE_COMPLETE;
	}
	else if (pMode->antisway_control_h == AS_MOVE_COMPLETE) {	//振止完了中
		if (pIO_Table->physics.sway_amp_r_ph > g_spec.as_compl_swayLv[I_AS_LV_TRIGGER]) {	//振れがトリガ判定値以上
			pMode->antisway_control_h = AS_MOVE_ANTISWAY;
		}
		else {
				pMode->antisway_ptn_h = AS_PTN_0;
		}
	}
	else if (pIO_Table->physics.sway_amp_r_ph > g_spec.as_compl_swayLv[I_AS_LV_TRIGGER]) {	//振れがトリガ判定値以上
		pMode->antisway_control_h = AS_MOVE_ANTISWAY;
	}
	else;

	return;
};

//### 振れ止めゲイン計算（1Step　2Step）##########################################
void CAnalyst::cal_as_gain(int motion_id, int type) {

	double R0,gain_limit1, gain_limit2;

	if (motion_id == AS_BH_ID) {

		gain_limit2 = g_spec.bh_notch_spd[NOTCH_MAX - 1] / g_spec.bh_acc[FWD_ACC];	//最大速度による加速時間制限
		if (type == AS_PTN_1STEP) {
			R0 = pIO_Table->physics.sway_amp_n_ph;
			gain_limit1 = 4 * pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID];			//1回のインチングで抑制可能な最大振幅は、4r0
			if (R0 > gain_limit1) R0 = gain_limit1;
			if (R0 > gain_limit2 * pIO_Table->physics.w0) R0 = gain_limit2 * pIO_Table->physics.w0;

			pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID] = acos(1 - 0.5 * R0 / pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID]);
			pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID] / pIO_Table->physics.w0;
		}
		else if(type == AS_PTN_2STEP_PN){
			//初期振れ量でまず、ゲインを設定
			if (pIO_Table->physics.sway_amp_n_ph < pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID]) {//振れ振幅が加速振れ内側
				//2回のインチングで振れ止め用
				double temp_d = acos(1 - pIO_Table->physics.sway_amp_n_ph /4.0/ pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID]);
				pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = temp_d/ pIO_Table->physics.w0;
			}
			else {
				//2回目のインチングで振れ止め（待ち時間調整タイプ）用
				pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = sqrt(pIO_Table->physics.sway_amp_n_ph * pIO_Table->physics.L / g_spec.bh_acc[FWD_ACC]);
			}
			//最大速度による加速時間制限
			if (pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] > gain_limit2) { 
				pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = gain_limit2;
			}
			//位相π/2以下にする制限
			if (pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] > DEF_HPI/ pIO_Table->physics.w0) {
				pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = DEF_HPI / pIO_Table->physics.w0;
			}
			pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID] = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] * pIO_Table->physics.w0;
		}
		else if (type == AS_PTN_2STEP_PP) {
			pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = sqrt(0.5 * pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] / g_spec.bh_acc[FWD_ACC]);
			//最大速度による加速時間制限
			if (pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] > gain_limit2) {
				pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = gain_limit2;
			}
			pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID] = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] * pIO_Table->physics.w0;
		}
		else {
			pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID] = 0.0;
			pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = 0.0;
		}
	}
	else if (motion_id == AS_SLEW_ID) {
		gain_limit2 = g_spec.slew_notch_spd[NOTCH_MAX - 1] / g_spec.slew_acc[FWD_ACC];	//最大速度による加速時間制限

		if (type == AS_PTN_1STEP) {
			R0 = pIO_Table->physics.sway_amp_t_ph;
			gain_limit1 = 4 * pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID]; //1回のインチングで抑制可能な最大振幅は、4r0
			if (R0 > gain_limit1) R0 = gain_limit1;
			if (R0 > gain_limit2 * pIO_Table->physics.w0) R0 = gain_limit2 * pIO_Table->physics.w0;

			pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID] = acos(1 - 0.5 * R0 / pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID]);
			pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID] / pIO_Table->physics.w0;
		}
		else if (type == AS_PTN_2STEP_PN) {
			//初期振れ量でまず、ゲインを設定
			if (pIO_Table->physics.sway_amp_t_ph < pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID]) {//振れ振幅が加速振れ内側
				//2回のインチングで振れ止め用
//				double temp_ph = acos(1 - pIO_Table->physics.sway_amp_t_ph / 4.0 / pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID] );
				double temp_ph = acos(1 - pIO_Table->physics.sway_amp_t_ph / 1.8 / pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID]);
				pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = temp_ph  /  pIO_Table->physics.w0;
			}
			else {
				//2回目のインチングで振れ止め（待ち時間調整）用
				pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = sqrt(pIO_Table->physics.sway_amp_t_ph * pIO_Table->physics.L / g_spec.slew_acc[FWD_ACC]);
			}

			//最大速度による加速時間制限
			if (pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] > gain_limit2) {
				pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = gain_limit2;
			}
			//位相π/2以下にする制限
			if (pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] > DEF_HPI / pIO_Table->physics.w0) {
				pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = DEF_HPI / pIO_Table->physics.w0;
			}
			pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID] = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] * pIO_Table->physics.w0;
		}
		else if (type == AS_PTN_2STEP_PP) {
			pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = sqrt(0.5 * pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] / g_spec.slew_acc[FWD_ACC]);
			//最大速度による加速時間制限
			if (pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] > gain_limit2) {
				pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = gain_limit2;
			}
			//位相π/2以下にする制限
			if (pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] > DEF_HPI / pIO_Table->physics.w0) {
				pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = DEF_HPI / pIO_Table->physics.w0;
			}
			pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID] = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] * pIO_Table->physics.w0;
		}
		else {
			pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID] = 0.0;
			pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = 0.0;
		}
	}
	else {
		pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID] = 0.0;
		pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = 0.0;
		pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID] = 0.0;
		pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = 0.0;
	}
	return;
};

//################################################################################
int CAnalyst::cal_job_recipe(int job_id, int mode) {
#if 0
	cal_as_gain();
	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;


	LPST_MOTION_UNIT p_bh_target, p_slew_target;
	double check_d, check_d2;

	switch (job_id) {
	case ORDER_ID_JOB_A: {
		p_bh_target = &(pOrder->job_A.command_recipe[pOrder->job_A.job_step_now].motions[BH_AXIS]);
		p_slew_target = &(pOrder->job_A.command_recipe[pOrder->job_A.job_step_now].motions[SLW_AXIS]);

		check_d = pIO_Table->physics.T * g_spec.bh_notch_spd[1] //1ノッチ一周期　＋　振れ止め1回の移動距離
			+ g_spec.bh_acc[FWD_ACC] * pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID] * pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID];
		check_d2 = pIO_Table->physics.T * g_spec.bh_notch_spd[1] / 3.0 //1ノッチ 1/3周期　＋　1ノッチインチング距離
			+ g_spec.bh_notch_spd[1] * g_spec.bh_notch_spd[1] / g_spec.bh_acc[FWD_ACC];

		if (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] > check_d) {
			if (cal_move_trapezoid(MOTION_ID_BH, p_bh_target, AUTO_PTN_MODE_AUTOMOVE) == NO_ERR_EXIST) {
				p_bh_target->ptn_status = PTN_STANDBY;
			}
		}
		else if ((pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] > check_d2) && (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] > 0.5)) {
			if (cal_move_3Step(MOTION_ID_BH, p_bh_target, AUTO_PTN_MODE_AUTOMOVE) == NO_ERR_EXIST) {
				p_bh_target->ptn_status = PTN_STANDBY;
			}
		}
		else {
			if (pMode->antisway_control_n == AS_MOVE_ANTISWAY) {
				if (cal_move_1Step(MOTION_ID_BH, p_bh_target, AUTO_PTN_MODE_AUTOMOVE) == NO_ERR_EXIST) {
					p_bh_target->ptn_status = PTN_STANDBY;
				}
			}
		}

		if (p_bh_target->ptn_status = PTN_STANDBY) {
			if (cal_move_trapezoid(MOTION_ID_SLEW, p_slew_target, AUTO_PTN_MODE_AUTOMOVE) == NO_ERR_EXIST) {
				p_slew_target->ptn_status = PTN_STANDBY;
			}
		}

		if (p_slew_target->ptn_status != PTN_STANDBY) return ERROR_ANA_CREAPE_AUTO_PATERN;
	}break;
	default:break;
	}
#endif
	return NO_ERR_EXIST;
};

//### 1Stepパターン 　   #########################################################
int CAnalyst::cal_move_1Step(int motion_id, LPST_MOTION_UNIT target, int mode) {

	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	target->n_step = 0;//ステップ数初期化

	switch (motion_id) {
	case MOTION_ID_BH: {
		//振れ止め移動方向セット
		pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 0;//移動方向不定

		cal_as_gain(AS_BH_ID,AS_PTN_1STEP);//振れ止めゲイン計算

		if (pMode->antisway_control_n & AS_MOVE_ANTISWAY) {//振れ止め有効
			target->n_step = 4;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = pMode->antisway_ptn_n;

			//Step 1　位相待ち
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[0]._t = pIO_Table->physics.T*2.0;// _t　タイムオーバー2周期
				//phase1 マイナス方向用位相
				target->motions[0].phase1 = pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID];
				//phase1 プラス方向用位相
				target->motions[0].phase2 = -DEF_PI + pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID];// high phase
				target->motions[0]._v = 0.0;
				target->motions[0].opt_i1 = AS_PTN_1STEP;//パターン出力時判定用
			}
			//Step 2　加速
			{
				target->motions[1].type = CTR_TYPE_ACC_AS;
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[1]._v = g_spec.bh_acc[FWD_ACC] * target->motions[1]._t;			// _v

			}
			//Step 3　減速
			{
				target->motions[2].type = CTR_TYPE_DEC_V;			
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p			
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[2]._v = 0.0;													// _v
			}
			//Step 4　動作停止待機
			{
				target->motions[3].type = CTR_TYPE_TIME_WAIT;
				target->motions[3]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[3]._t = PTN_CONFIRMATION_TIME;									// _t
				target->motions[3]._v = 0.0;													// _v
			}
			//time_count
			for (int i = 0; i < target->n_step ; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}
		else {
			target->n_step = 1;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern

			 //Step 1
				target->motions[0].type = CTR_TYPE_TIME_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;	// _p
				target->motions[0]._t = PTN_CONFIRMATION_TIME;			// _t
				target->motions[0]._v = 0.0;
				target->motions[0].act_counter = 0;
				target->motions[0].time_count = (int)(target->motions[0]._t * 1000) / (int)play_scan_ms;
		}

		//自動モードでは、旋回が終わるまで次のパターンに入らない様にする　後日要検討
		if (mode == AUTO_PTN_MODE_AUTOMOVE) {
			target->motions[target->n_step].type = CTR_TYPE_SLEW_WAIT;
			target->motions[target->n_step]._p = pIO_Table->auto_ctrl.tgpos_bh;
			target->motions[target->n_step]._t = PTN_ERROR_CHECK_TIME1;
			target->motions[target->n_step].time_count = (int)(target->motions[target->n_step]._t * 1000) / (int)play_scan_ms;
			target->motions[target->n_step]._v = 0.0;
			target->n_step += 1;
		}
	}break;
	case MOTION_ID_SLEW: {

		//振れ止め移動方向セット
		pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 0;//移動方向不定
		cal_as_gain(AS_SLEW_ID, AS_PTN_1STEP);//振れ止めゲイン計算

		if (pMode->antisway_control_t &  AS_MOVE_ANTISWAY) {
			target->n_step = 4;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = pMode->antisway_ptn_t;

			//Step 1　位相待ち
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[0]._t = pIO_Table->physics.T*2.0;								// _t　タイムオーバー2周期
				target->motions[0].phase1 = pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID];		// low phase
				target->motions[0].phase2 = -DEF_PI+pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID];// high phase
				target->motions[0]._v = 0.0;
				target->motions[0].opt_i1 = AS_PTN_1STEP;//パターン出力時判定用
			}
			//Step 2　加速
			{
				target->motions[1].type = CTR_TYPE_ACC_AS;
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[1]._v = g_spec.slew_acc[FWD_ACC] * target->motions[1]._t;
			}
			//Step 3　減速
			{
				target->motions[2].type = CTR_TYPE_DEC_V;
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p			
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[2]._v = 0.0;													// _v
			}
			//Step 4　動作停止待機
			{
				target->motions[3].type = CTR_TYPE_TIME_WAIT;
				target->motions[3]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[3]._t = PTN_CONFIRMATION_TIME;									// _t
				target->motions[3]._v = 0.0;													// _v
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}
		else {
			target->n_step = 1;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			  //Step 1
				target->motions[0].type = CTR_TYPE_TIME_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_slew;	// _p
				target->motions[0]._t = PTN_CONFIRMATION_TIME;				// _t
				target->motions[0]._v = 0.0;
				target->motions[0].act_counter = 0;
				target->motions[0].time_count = (int)(target->motions[0]._t * 1000) / (int)play_scan_ms;
		}
	}break;
	case MOTION_ID_MH: {
		//処理無し
	}break;
	default: return  ERR_NO_CASE_EXIST;
	}

	return NO_ERR_EXIST;
};

//### 2Step+-パターン　  #########################################################
int CAnalyst::cal_move_2Step_pn(int motion_id, LPST_MOTION_UNIT target, int mode) {

	double adjust_t_pos, adjust_t_sway;
	int adjust_count_pos, adjust_count_sway, adjust_dir;
	
	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	target->n_step = 0;//ステップ数初期化

	switch (motion_id) {
	case MOTION_ID_BH: {
		int large_sway_flag;
		if (pIO_Table->physics.sway_amp_n_ph < pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID]) {
			large_sway_flag = OFF;
		}
		else {
			large_sway_flag = ON;
		}

		//振れ止め移動方向セット
		if (large_sway_flag == ON) {
			if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] > 0) pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_PLUS;
			else pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_MINUS;
		}
		else {
			pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 0;
		}
		pIO_Table->auto_ctrl.as_start_ph[AS_BH_ID] = 0;	//振れ止め開始位相方向フラグクリア
		cal_as_gain(AS_BH_ID, AS_PTN_2STEP_PN);//振れ止めゲイン計算

	//目標位置までの位置合わせ補正時間,　目標位置までの距離に応じていずれかのステップの加速時間を増やす
		adjust_t_pos = sqrt(pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] * pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID]
			  + pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] / g_spec.bh_acc[FWD_ACC]); 
		adjust_t_pos -= pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];
		adjust_count_pos = (int)(adjust_t_pos * 1000) / (int)play_scan_ms;

	//補正時間を適用する移動方向を判定する
		if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] > 0.0) {
			adjust_dir = AS_DIR_PLUS;//マイナス方向を減らす
		}
		else {
			adjust_dir = AS_DIR_MINUS;
		}

	//初期振れ振幅による停止補正時間,　振れ振幅に応じて停止時間の増減をする
		double R = 2.0 * pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID] * (1.0 - cos(pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID]));
		double dph = pIO_Table->physics.sway_amp_n_ph / R;
		adjust_t_sway = dph / pIO_Table->physics.w0;
		adjust_count_sway = (int)(adjust_t_sway * 1000) / (int)play_scan_ms;

		if (pMode->antisway_control_n & AS_MOVE_ANTISWAY) {//振れ止め有効
			target->n_step = 7;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0;								//Initialize activated pattern
			target->motion_type = AS_PTN_2STEP_PN;

			//Step 1　位相待ち
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[0]._t = pIO_Table->physics.T*2.0;								// _t　タイムオーバー2周期
				target->motions[0].phase1 = 0;													// low phase
				target->motions[0].phase2 = DEF_PI;												// high phase
				target->motions[0]._v = 0.0;
				target->motions[0].opt_i1 = AS_PTN_2STEP_PN;//パターン出力時判定用
			}
			//Step 2　加速
			{
				target->motions[1].type = CTR_TYPE_ACC_AS_2PN;
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[1]._v = g_spec.bh_acc[FWD_ACC] * target->motions[1]._t;			// _v
				target->motions[1].opt_i1 = adjust_count_pos;	//位置合わせ用補正タイマーカウント
				target->motions[1].opt_i2 = adjust_dir;			//補正タイマー適用移動方向
			}
			//Step 3　減速
			{
				target->motions[2].type = CTR_TYPE_DEC_AS_2PN;
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p			
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[2]._v = 0.0;													// _v
				target->motions[2].opt_i1 = adjust_count_pos;	//位置合わせ用補正タイマーカウント
				target->motions[2].opt_i2 = adjust_dir;			//補正タイマー適用移動方向
			}
			//Step 4　位相待ち待機
			{
				target->motions[3].type = CTR_TYPE_TIME_WAIT_2PN;
				target->motions[3]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				if (large_sway_flag) {
					target->motions[3]._t = 2.0*(DEF_PI / pIO_Table->physics.w0 - pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID]);	// _t
					target->motions[3].opt_i1 = adjust_count_sway;	//振れ止め用補正タイマーカウント
					target->motions[3].opt_i2 = AS_INIT_SWAY_LARGE;
				}
				else {

					double R1, cos_ph, r, r0, r0_p_r, ph1,phx;
					cos_ph = cos(pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID]);
					r = pIO_Table->physics.sway_amp_n_ph;
					r0 = pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID];
					r0_p_r = r + r0;
					
					R1 = sqrt(r0_p_r * r0_p_r + 4.0 * r0 - 4.0 * r0_p_r*r0*cos_ph);

					ph1 = acos((2.0*r0 - r0_p_r * cos_ph) / R1) - pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID];
					phx = atan(sin(ph1) / (r0 / R1 - cos(ph1)));

					if (phx < 0.0) { //マイナス符号は、ｙ軸を超えているので変換必要
						phx = DEF_PI + phx;
					}
					target->motions[3]._t = DEF_PI / pIO_Table->physics.w0;	// _t
					target->motions[3].opt_i1 = (int)(phx / pIO_Table->physics.w0 * 1000) / (int)play_scan_ms;	//振れ止め用補正タイマーカウント
					target->motions[3].opt_i2 = AS_INIT_SWAY_SMALL;
				}
				target->motions[3]._v = 0.0;													// _v
			}
			//Step 5　加速
			{
				target->motions[4].type = CTR_TYPE_ACC_AS_2PN;
				target->motions[4]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[4]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[4]._v = g_spec.bh_acc[FWD_ACC] * target->motions[1]._t;			// _v
				target->motions[4].opt_i1 = adjust_count_pos;	//位置合わせ用補正タイマーカウント
				target->motions[4].opt_i2 = adjust_dir;			//補正タイマー適用移動方向
			}
			//Step 6　減速
			{
				target->motions[5].type = CTR_TYPE_DEC_AS_2PN;
				target->motions[5]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p			
				target->motions[5]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[5]._v = 0.0;													// _v
				target->motions[5].opt_i1 = adjust_count_pos;	//位置合わせ用補正タイマーカウント
				target->motions[5].opt_i2 = adjust_dir;			//補正タイマー適用移動方向
			}
			//Step 7　動作停止待機
			{
				target->motions[6].type = CTR_TYPE_TIME_WAIT;
				target->motions[6]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[6]._t = PTN_CONFIRMATION_TIME*10;									// _t
				target->motions[6]._v = 0.0;													// _v
			}
			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}
		else {
			target->n_step = 1;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern

			 //Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;
			target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;	// _p
			target->motions[0]._t = PTN_CONFIRMATION_TIME;			// _t
			target->motions[0]._v = 0.0;
			target->motions[0].act_counter = 0;
			target->motions[0].time_count = (int)(target->motions[0]._t * 1000) / (int)play_scan_ms;
		}
	}break;
	case MOTION_ID_SLEW: {
		int large_sway_flag;
		if (pIO_Table->physics.sway_amp_t_ph < pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID]) {
			large_sway_flag = OFF;
		}
		else {
			large_sway_flag = ON;
		}

		//振れ止め移動方向セット
		if (large_sway_flag == ON) {
			if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] > 0) pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_PLUS;
			else pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_MINUS;
		}
		else {
			pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 0;
		}
		//振れ止め移動方向セット
		pIO_Table->auto_ctrl.as_start_ph[AS_SLEW_ID] = 0;//振れ止め開始位相方向フラグクリア
		cal_as_gain(AS_SLEW_ID, AS_PTN_2STEP_PN);//振れ止めゲイン計算

		//目標位置までの位置合わせ補正時間,　目標位置までの距離に応じていずれかのステップの加速時間を減らす
		adjust_t_pos = sqrt(pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] * pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID]
			+ pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] / g_spec.slew_acc[FWD_ACC]);
		adjust_t_pos -= pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];

		adjust_count_pos = (int)(adjust_t_pos * 1000) / (int)play_scan_ms;

		//補正時間を適用する移動方向を判定する
		if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] > 0.0) {
			adjust_dir = AS_DIR_PLUS;//プラス方向を増やす
		}
		else {
			adjust_dir = AS_DIR_MINUS;
		}

		//初期振れ振幅による停止補正時間,　振れ振幅に応じて停止時間の増減をする
		double R = 2.0 * pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID] * (1.0 - cos(pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID]));
		double dph = pIO_Table->physics.sway_amp_t_ph / R;
		adjust_t_sway = dph / pIO_Table->physics.w0;
		adjust_count_sway = (int)(adjust_t_sway * 1000) / (int)play_scan_ms;

		if (pMode->antisway_control_t &  AS_MOVE_ANTISWAY) {
			target->n_step = 7;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = pMode->antisway_ptn_t;

			//Step 1　位相待ち
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_slew;// _p
				target->motions[0]._t = pIO_Table->physics.T*2.0;		// _t　タイムオーバー2周期
				target->motions[0].phase1 = 0;							// low phase
				target->motions[0].phase2 = DEF_PI;						// high phase
				target->motions[0]._v = 0.0;
				target->motions[0].opt_i1 = AS_PTN_2STEP_PN;//パターン出力時判定用
			}
			//Step 2　加速
			{
				target->motions[1].type = CTR_TYPE_ACC_AS_2PN;
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[1]._v = g_spec.slew_acc[FWD_ACC] * target->motions[1]._t;			// _v
				target->motions[1].opt_i1 = adjust_count_pos;	//位置合わせ用補正タイマーカウント
				target->motions[1].opt_i2 = adjust_dir;			//補正タイマー適用移動方向
			}
			//Step 3　減速
			{
				target->motions[2].type = CTR_TYPE_DEC_AS_2PN;
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p			
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[2]._v = 0.0;													// _v
				target->motions[2].opt_i1 = adjust_count_pos;	//位置合わせ用補正タイマーカウント
				target->motions[2].opt_i2 = adjust_dir;			//補正タイマー適用移動方向
			}
			//Step 4　動作停止待機
			{
				target->motions[3].type = CTR_TYPE_TIME_WAIT_2PN;
				target->motions[3]._p = pIO_Table->auto_ctrl.tgpos_bh;		// _p
				if (large_sway_flag) {
					target->motions[3]._t = 2.0*(DEF_PI / pIO_Table->physics.w0 - pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID]);									// _t
					target->motions[3].opt_i1 = adjust_count_sway;	//振れ止め用補正タイマーカウント
					target->motions[3].opt_i2 = AS_INIT_SWAY_LARGE;
				}
				else {

					double R1, cos_ph, r, r0, r0_p_r, ph1, phx;
					cos_ph = cos(pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID]);
					r = pIO_Table->physics.sway_amp_t_ph;
					r0 = pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID];
					r0_p_r = r + r0;

					R1 = sqrt(r0_p_r * r0_p_r + 4.0 * r0 - 4.0 * r0_p_r*r0*cos_ph);
					ph1 = acos((2.0*r0 - r0_p_r * cos_ph) / R1) - pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID];
					phx = atan(sin(ph1) / (r0 / R1 - cos(ph1)));

					if (phx < 0.0) { //マイナス符号は、ｙ軸を超えているので変換必要
						phx = DEF_PI + phx;
					}
					target->motions[3]._t = DEF_PI / pIO_Table->physics.w0;	// _t
					target->motions[3].opt_i1 = (int)(phx / pIO_Table->physics.w0 * 1000) / (int)play_scan_ms;	//振れ止め用補正タイマーカウント
					target->motions[3].opt_i2 = AS_INIT_SWAY_SMALL;
				}
				target->motions[3]._v = 0.0;													// _v
			}
			//Step 5　加速
			{
				target->motions[4].type = CTR_TYPE_ACC_AS_2PN;
				target->motions[4]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[4]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[4]._v = g_spec.slew_acc[FWD_ACC] * target->motions[4]._t;			// _v
				target->motions[4].opt_i1 = adjust_count_pos;	//位置合わせ用補正タイマーカウント
				target->motions[4].opt_i2 = adjust_dir;			//補正タイマー適用移動方向
			}
			//Step 6　減速
			{
				target->motions[5].type = CTR_TYPE_DEC_AS_2PN;
				target->motions[5]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p			
				target->motions[5]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[5]._v = 0.0;													// _v
				target->motions[5].opt_i1 = adjust_count_pos;	//位置合わせ用補正タイマーカウント
				target->motions[5].opt_i2 = adjust_dir;			//補正タイマー適用移動方向
			}
			//Step 7　動作停止待機
			{
				target->motions[6].type = CTR_TYPE_TIME_WAIT;
				target->motions[6]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[6]._t = PTN_CONFIRMATION_TIME*10;									// _t
				target->motions[6]._v = 0.0;													// _v
				target->motions[6].opt_i1 = 0;	//振れ止め用補正タイマーカウント
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}
		else {
			target->n_step = 1;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			  //Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;
			target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_slew;	// _p
			target->motions[0]._t = PTN_CONFIRMATION_TIME;				// _t
			target->motions[0]._v = 0.0;
			target->motions[0].act_counter = 0;
			target->motions[0].time_count = (int)(target->motions[0]._t * 1000) / (int)play_scan_ms;
		}
	}break;
	case MOTION_ID_MH: {
		int step_count;
		target->axis_type = MH_AXIS;
		target->ptn_status = PTN_STANDBY;
		target->iAct = 0; //Initialize activated pattern
		target->motion_type = AS_PTN_2STEP_PN;

		if (pMode->antisway_control_h == AS_MOVE_ANTISWAY) {

			double t0, t1, tx, v1;
			t0 = pIO_Table->physics.T * 2.0 / 16.0;

			t1 = pIO_Table->physics.T / 4.0 - t0;
			if (t1 > pIO_Table->physics.T / 8.0) t1 = pIO_Table->physics.T / 8.0;

			tx = pIO_Table->physics.T / 4.0 - t1;
			v1 = t1 * g_spec.hoist_acc[FWD_ACC];

			//Step 1 位相待ち
			{
				target->n_step = 1;
				step_count = 0;
				target->motions[step_count].type = CTR_TYPE_SINGLE_PHASE_WAIT;
				target->motions[step_count]._p = pIO_Table->auto_ctrl.tgpos_h;	// _p
				target->motions[step_count]._t = pIO_Table->physics.T*2.0;		// _t タイムオーバーを2周期とする
				target->motions[step_count].phase1 = t0 * pIO_Table->physics.w0;		// low phase
				target->motions[step_count].phase2 = target->motions[step_count].phase1;// high phase
				target->motions[step_count]._v = 0.0;
			}
			//Step 2 巻上
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = tx;
				target->motions[step_count]._v = -v1;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count]._t * target->motions[step_count]._v
					- target->motions[step_count]._v*target->motions[step_count]._v / g_spec.hoist_acc[FWD_ACC] / 2.0;
			}
			//Step 3 停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 4.0 - t0;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count - 1]._v*target->motions[step_count - 1]._v / g_spec.hoist_acc[FWD_ACC] / 2.0;
			}
			//Step 4 巻下
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = tx;
				target->motions[step_count]._v = v1;
				target->motions[step_count]._p = pIO_Table->auto_ctrl.tgpos_h
					+ target->motions[step_count]._v*target->motions[step_count]._v / g_spec.hoist_acc[FWD_ACC] / 2.0;
			}
			//Step 5 停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = t1;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = pIO_Table->auto_ctrl.tgpos_h;
			}
			//Step end 部
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_TIME_WAIT;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._t = PTN_CONFIRMATION_TIME;
				target->motions[step_count]._p = target->motions[step_count - 1]._p;
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}
		else {
			target->n_step = 1;
			target->axis_type = MH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern

			 //Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;

			target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_h;// _p
			target->motions[0]._t = PTN_CONFIRMATION_TIME;			// _t
			target->motions[0]._v = 0.0;
			for (int i = 0; i < 1; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}
	}break;
	default: return  ERR_NO_CASE_EXIST;
	}

	return NO_ERR_EXIST;
};

//### 2Step++パターン　  #########################################################
int CAnalyst::cal_move_2Step_pp(int motion_id, LPST_MOTION_UNIT target, int mode) {

	double adjust_t_sway;
	int adjust_count_sway;

	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;
	target->n_step = 0;

	switch (motion_id) {
	case MOTION_ID_BH: {

		//振れ止め移動方向セット
		if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] > 0) {
			pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_PLUS;
		}
		else {
			pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_MINUS;
		}

		pIO_Table->auto_ctrl.as_start_ph[AS_BH_ID] = 0;	//振れ止め開始位相方向フラグクリア
		cal_as_gain(AS_BH_ID, AS_PTN_2STEP_PP);//振れ止めゲイン計算

		//初期振れ振幅による停止補正時間,　振れ振幅に応じて停止時間の増減をする
		double R = 2.0 * pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID] * (1.0 - cos(pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID]));
		double dph;
		if (R < 0.0001) { //0割り防止
			dph = 0.0;
		}
		else {
			dph = pIO_Table->physics.sway_amp_n_ph / R;
			if (dph > DEF_QPI) {//上限設定（大きい振れは対応不可）
				dph = DEF_QPI;
			}
		}
		adjust_t_sway = dph / pIO_Table->physics.w0;
		adjust_count_sway = (int)(adjust_t_sway * 1000) / (int)play_scan_ms;

		if (pMode->antisway_control_n & AS_MOVE_ANTISWAY) {//振れ止め有効
			target->n_step = 7;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0;								//Initialize activated pattern
			target->motion_type = AS_PTN_2STEP_PP;

			//Step 1　位相待ち
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[0]._t = pIO_Table->physics.T*2.0;								// _t　タイムオーバー2周期
				target->motions[0].phase1 = 0;													// low phase
				target->motions[0].phase2 = DEF_PI;												// high phase
				target->motions[0]._v = 0.0;
				target->motions[0].opt_i1 = AS_PTN_2STEP_PP;//パターン出力時判定用
			}
			//Step 2　加速
			{
				target->motions[1].type = CTR_TYPE_ACC_AS;
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[1]._v = g_spec.bh_acc[FWD_ACC] * target->motions[1]._t;			// _v
			}
			//Step 3　減速
			{
				target->motions[2].type = CTR_TYPE_DEC_V;
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p			
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[2]._v = 0.0;													// _v
			}
			//Step 4　位相待ち待機
			{
				target->motions[3].type = CTR_TYPE_TIME_WAIT_2PP;
				target->motions[3]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[3]._t = DEF_PI/pIO_Table->physics.w0 - 2.0* pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];									// _t
					target->motions[3]._v = 0.0;													// _v
				target->motions[3].opt_i1 = adjust_count_sway;	//振れ止め用補正タイマーカウント
		//		target->motions[3].opt_i1 = 0;	//振れ止め用補正タイマーカウント
			}
			//Step 5　加速
			{
				target->motions[4].type = CTR_TYPE_ACC_AS;
				target->motions[4]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[4]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[4]._v = g_spec.bh_acc[FWD_ACC] * target->motions[1]._t;			// _v
			}
			//Step 6　減速
			{
				target->motions[5].type = CTR_TYPE_DEC_V;
				target->motions[5]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p			
				target->motions[5]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[5]._v = 0.0;													// _v
			}
			//Step 7　動作停止待機
			{
				target->motions[6].type = CTR_TYPE_TIME_WAIT;
				target->motions[6]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[6]._t = PTN_CONFIRMATION_TIME;									// _t
				target->motions[6]._v = 0.0;													// _v
			}
			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}
		else {
			target->n_step = 1;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern

			 //Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;
			target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;	// _p
			target->motions[0]._t = PTN_CONFIRMATION_TIME;			// _t
			target->motions[0]._v = 0.0;
			target->motions[0].act_counter = 0;
			target->motions[0].time_count = (int)(target->motions[0]._t * 1000) / (int)play_scan_ms;
		}
	}break;
	case MOTION_ID_SLEW: {
		//振れ止め移動方向セット
		if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] > 0) pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_PLUS;
		else pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_MINUS;

		pIO_Table->auto_ctrl.as_start_ph[AS_SLEW_ID] = 0;//振れ止め開始位相方向フラグクリア

		cal_as_gain(AS_SLEW_ID, AS_PTN_2STEP_PP);//振れ止めゲイン計算

		//初期振れ振幅による停止補正時間,　振れ振幅に応じて停止時間の増減をする
		double R = 2.0 * pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID] * (1.0 - cos(pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID]));
		double dph;
		if (R < 0.0001) { //0割り防止
			dph = 0.0;
		}
		else {
			dph = pIO_Table->physics.sway_amp_t_ph / R;
			if (dph > DEF_QPI) {//上限設定（大きい振れは対応不可）
				dph = DEF_QPI;
			}
		}
		adjust_t_sway = dph / pIO_Table->physics.w0;
		adjust_count_sway = (int)(adjust_t_sway * 1000) / (int)play_scan_ms;

		if (pMode->antisway_control_t &  AS_MOVE_ANTISWAY) {
			target->n_step = 7;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = pMode->antisway_ptn_t;

			//Step 1　位相待ち
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_slew;// _p
				target->motions[0]._t = pIO_Table->physics.T*2.0;		// _t　タイムオーバー2周期
				target->motions[0].phase1 = 0;							// low phase
				target->motions[0].phase2 = DEF_PI;						// high phase
				target->motions[0]._v = 0.0;
				target->motions[0].opt_i1 = AS_PTN_2STEP_PP;//パターン出力時判定用
			}
			//Step 2　加速
			{
				target->motions[1].type = CTR_TYPE_ACC_AS;
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[1]._v = g_spec.slew_acc[FWD_ACC] * target->motions[1]._t;			// _v
			}
			//Step 3　減速
			{
				target->motions[2].type = CTR_TYPE_DEC_V;
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p			
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[2]._v = 0.0;													// _v
			}
			//Step 4　動作停止待機
			{
				target->motions[3].type = CTR_TYPE_TIME_WAIT_2PP;
				target->motions[3]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[3]._t = DEF_PI / pIO_Table->physics.w0 - 2.0* pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];									// _t
				target->motions[3]._v = 0.0;													// _v
				target->motions[3].opt_i1 = adjust_count_sway;	//振れ止め用補正タイマーカウント
			}
			//Step 5　加速
			{
				target->motions[4].type = CTR_TYPE_ACC_AS;
				target->motions[4]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[4]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[4]._v = g_spec.slew_acc[FWD_ACC] * target->motions[1]._t;			// _v
			}
			//Step 6　減速
			{
				target->motions[5].type = CTR_TYPE_DEC_V;
				target->motions[5]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p			
				target->motions[5]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[5]._v = 0.0;													// _v
			}
			//Step 7　動作停止待機
			{
				target->motions[6].type = CTR_TYPE_TIME_WAIT;
				target->motions[6]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[6]._t = PTN_CONFIRMATION_TIME;									// _t
				target->motions[6]._v = 0.0;													// _v
				target->motions[6].opt_i1 = 0;	//振れ止め用補正タイマーカウント
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}
		else {
			target->n_step = 1;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			  //Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;
			target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_slew;	// _p
			target->motions[0]._t = PTN_CONFIRMATION_TIME;				// _t
			target->motions[0]._v = 0.0;
			target->motions[0].act_counter = 0;
			target->motions[0].time_count = (int)(target->motions[0]._t * 1000) / (int)play_scan_ms;
		}
	}break;
	case MOTION_ID_MH: {
		//処理無し
	}break;
	}
	return NO_ERR_EXIST;
};

//### 台形パターン       #########################################################
int CAnalyst::cal_move_trapezoid(int motion_id, LPST_MOTION_UNIT target, int mode) {

	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 0;
	pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 0;
	pIO_Table->auto_ctrl.as_out_dir[AS_MH_ID] = 0;

	int n = 0;
	double dx, Da;

	target->n_step = 0;
#if 0
	switch (motion_id) {
	case MOTION_ID_BH: {

		Da = pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID];//目標移動距離
		acc = g_spec.bh_acc[FWD_ACC];

		dx = pIO_Table->physics.T * g_spec.bh_notch_spd[1] //1ノッチ一周期　＋　振れ止め1回の移動距離
			+ g_spec.bh_acc[FWD_ACC] * pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID] * pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID];

		if (Da < dx) return CAL_RESULT_NEGATIVE;//パターン作成不可

		if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] > 0.0) pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 1;
		else if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] < 0.0)pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = -1;
		else;

		if (mode == AUTO_PTN_MODE_AUTOMOVE) {
			double t = DEF_HPI / pIO_Table->physics.w0 / 4.0;
			Da = pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] - t * t*g_spec.bh_acc[FWD_ACC];
			if (Da >= 0.0) {
				for (int i = NOTCH_MAX - 1; i > 0; i--) {
					if (n == 0) {
						double Vn = g_spec.bh_notch_spd[i];
						double chk_dist = Vn * Vn / g_spec.bh_acc[FWD_ACC];
						if (Da < chk_dist) continue;
						else {
							n = i;
							break;
						}
					}
				}
			}
		}
		else {
			Da = pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID];
			if (Da >= 0.0) n = cal_notch_select_bh(ptn_notch_freq[AS_BH_ID], Da, AS_PTN_MOVE_LONG);
		}

		if ((n == 0) || (Da < 0.0)) {//パターン不要
			target->n_step = 1;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern

							  //Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;
			// _p
			target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;
			target->motions[0]._v = 0.0;
			// _t
			target->motions[0]._t = PTN_CONFIRMATION_TIME;
			target->motions[0].act_counter = 0;
			target->motions[0].time_count = (int)(target->motions[0]._t * 1000) / (int)play_scan_ms;
		}
		else if (mode == AUTO_PTN_MODE_AUTOMOVE) {
			int step_count = 0;

			target->n_step = 3;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_MOVE_LONG_AUTO;

			//Step 1 台形パターン部
			{
				target->motions[0].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = Da / g_spec.bh_notch_spd[n];
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[n];
				target->motions[step_count]._p = target->motions[0]._p = pIO_Table->physics.R +
					pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * (Da - target->motions[step_count]._v * target->motions[step_count]._v / g_spec.bh_acc[FWD_ACC] / 2.0);
			}
			//Step 2～ 台形減速パターン部
			{
				step_count += 1;
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = g_spec.bh_notch_spd[n] / g_spec.bh_acc[FWD_ACC];
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[0]._p = pIO_Table->physics.R + pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * Da;
			}
			//Step end 部
			{
				step_count += 1;
				target->motions[step_count].type = CTR_TYPE_TIME_WAIT;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._t = PTN_CONFIRMATION_TIME;
				target->motions[step_count]._p = target->motions[0]._p = pIO_Table->physics.R + pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * Da;
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}
		else {
			int step_count = 0;

			target->n_step = 1;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_MOVE_LONG;

			//Step 1 基本台形パターン部
			{
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[n];
				target->motions[step_count]._p = pIO_Table->physics.R
					+ target->motions[step_count]._v * target->motions[step_count]._t
					- pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count]._v * target->motions[step_count]._v / g_spec.bh_acc[FWD_ACC] / 2.0;
			}

			//Step 2～ 台形減速パターン部
			{
				for (; n > 0; n--) {

					if (ptn_notch_freq[AS_BH_ID][n] == 0) continue;

					step_count += 1;
					target->n_step += 1;

					target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
					target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[n];

					double  temp_d = (target->motions[step_count - 1]._v - target->motions[step_count]._v) / g_spec.bh_acc[FWD_ACC];
					if (temp_d < 0.0)temp_d *= -1.0;
					target->motions[step_count]._t = (double)ptn_notch_freq[AS_BH_ID][n] * pIO_Table->physics.T + temp_d;

					temp_d = 0.5*g_spec.bh_acc[FWD_ACC] * temp_d * temp_d;
					target->motions[step_count]._p = target->motions[step_count - 1]._p
						+ target->motions[step_count]._v * target->motions[step_count]._t
						- pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * temp_d;
				}
			}

			//Step end 部
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_TIME_WAIT;
				target->motions[step_count]._v = 0.0;
				double  temp_d = (target->motions[step_count - 1]._v - target->motions[step_count]._v) / g_spec.bh_acc[FWD_ACC];
				if (temp_d < 0.0)temp_d *= -1.0;
				target->motions[step_count]._t = temp_d;
				temp_d = 0.5*g_spec.bh_acc[FWD_ACC] * temp_d * temp_d;
				target->motions[step_count]._p = target->motions[step_count - 1]._p - pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * temp_d;
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}

		}

		if (mode == AUTO_PTN_MODE_AUTOMOVE) {
			target->motions[target->n_step].type = CTR_TYPE_SLEW_WAIT;
			target->motions[target->n_step]._p = pIO_Table->auto_ctrl.tgpos_bh;
			target->motions[target->n_step]._t = PTN_ERROR_CHECK_TIME1;
			target->motions[target->n_step].time_count = (int)(target->motions[target->n_step]._t * 1000) / (int)play_scan_ms;
			target->motions[target->n_step]._v = 0.0;
			target->n_step += 1;
		}

	}break;
	case MOTION_ID_SLEW: {

		Da = pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID];//目標移動距離
		acc = g_spec.slew_acc[FWD_ACC];

		dx = pIO_Table->physics.T * g_spec.slew_notch_spd[1]
			+ acc * pIO_Table->auto_ctrl.as_gain_damp[AS_SLEW_ID] * pIO_Table->auto_ctrl.as_gain_damp[AS_SLEW_ID];

		if (Da < dx) return CAL_RESULT_NEGATIVE;//パターン作成不可

		if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] > 0.0) pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 1;
		else if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] < 0.0)pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = -1;
		else;

		if (mode == AUTO_PTN_MODE_AUTOMOVE) {
			double t = DEF_HPI / pIO_Table->physics.w0 / 4.0;
			Da = pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] - t * t*g_spec.slew_acc[FWD_ACC];
			if (Da >= 0.0) {
				for (int i = NOTCH_MAX - 1; i > 0; i--) {
					if (n == 0) {
						double Vn = g_spec.slew_notch_spd[i];
						double chk_dist = Vn * Vn / g_spec.slew_acc[FWD_ACC];
						if (Da < chk_dist) continue;
						else {
							n = i;
							break;
						}
					}
				}
			}
		}
		else {
			Da = pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID];
			if (Da >= 0.0) n = cal_notch_select_slew(ptn_notch_freq[AS_SLEW_ID], Da, AS_PTN_MOVE_LONG);
		}


		if ((n == 0) || (Da <= 0.0)) {//パターン不要
			target->n_step = 1;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern

							  //Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;
			target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_slew;	// _p
			target->motions[0]._v = 0.0;
			target->motions[0]._t = PTN_CONFIRMATION_TIME;			// _t
			target->motions[0].act_counter = 0;
			target->motions[0].time_count = (int)(target->motions[0]._t * 1000) / (int)play_scan_ms;
		}
		else if (mode == AUTO_PTN_MODE_AUTOMOVE) {

			int step_count = 0;

			target->n_step = 3;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_MOVE_LONG_AUTO;

			//Step 1 台形パターン部
			{
				target->motions[0].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = Da / g_spec.slew_notch_spd[n];
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];
				target->motions[step_count]._p = target->motions[0]._p = pIO_Table->physics.th +
					pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * (Da - target->motions[step_count]._v * target->motions[step_count]._v / g_spec.slew_acc[FWD_ACC] / 2.0);
			}
			//Step 2～ 台形減速パターン部
			{
				step_count += 1;
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = g_spec.slew_notch_spd[n] / g_spec.slew_acc[FWD_ACC];
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[0]._p = target->motions[0]._p = pIO_Table->physics.th + pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * Da;
			}
			//Step end 部
			{
				step_count += 1;
				target->motions[step_count].type = CTR_TYPE_TIME_WAIT;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._t = PTN_CONFIRMATION_TIME;
				target->motions[step_count]._p = target->motions[0]._p = target->motions[0]._p = pIO_Table->physics.th + pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * Da;
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}
		else {
			int step_count = 0;

			target->n_step = 1;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_MOVE_LONG;

			//Step 1 基本台形パターン部
			{
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];
				target->motions[step_count]._p = pIO_Table->physics.th
					+ target->motions[step_count]._v * target->motions[step_count]._t
					- pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count]._v * target->motions[step_count]._v / g_spec.slew_acc[FWD_ACC] / 2.0;
			}
			//Step 2～ 台形減速パターン部
			{
				for (; n > 0; n--) {

					if (ptn_notch_freq[AS_SLEW_ID][n] == 0) continue;

					step_count += 1;
					target->n_step += 1;

					target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
					target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];

					double  temp_d = (target->motions[step_count - 1]._v - target->motions[step_count]._v) / g_spec.slew_acc[FWD_ACC];
					if (temp_d < 0.0)temp_d *= -1.0;//次ノッチ速度までの減速時間
					target->motions[step_count]._t = (double)ptn_notch_freq[AS_SLEW_ID][n] * pIO_Table->physics.T + temp_d;

					temp_d = 0.5*g_spec.slew_acc[FWD_ACC] * temp_d * temp_d;
					target->motions[step_count]._p = target->motions[step_count - 1]._p
						+ target->motions[step_count]._v * target->motions[step_count]._t
						- pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * temp_d;
				}
			}
			//Step end 部
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_TIME_WAIT;
				target->motions[step_count]._v = 0.0;
				double  temp_d = (target->motions[step_count - 1]._v - target->motions[step_count]._v) / g_spec.slew_acc[FWD_ACC];
				if (temp_d < 0.0)temp_d *= -1.0;
				target->motions[step_count]._t = temp_d;
				temp_d = 0.5 * g_spec.slew_acc[FWD_ACC] * temp_d * temp_d;
				target->motions[step_count]._p = target->motions[step_count - 1]._p - pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * temp_d;
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}

	}break;
	default: return 1;
	}
#endif
	return CAL_RESULT_POSITIVE;
};

//### 2段加減速パターン　#########################################################
int CAnalyst::cal_move_2accdec(int motion_id, LPST_MOTION_UNIT target, int mode) {

	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	target->n_step = 0;

	LPAS_NOTCH_SET p_as_notch_spd = &as_notch_spd;

	switch (motion_id) {
	case MOTION_ID_BH: {
	
		//進行方向セット（開始位相判別用）
		if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] > 0.0) pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_PLUS;
		else if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] < 0.0)pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_MINUS;
		else;

		double dir = (double)pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID];

		if (cal_notch_set(p_as_notch_spd, pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID], AS_PTN_2ACCDEC, AS_BH_ID)) {
			int step_count = 0;

			target->n_step = 1;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_2ACCDEC;

			//Step 1 定速1段目
			{
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_2nd[AS_BH_ID] + p_as_notch_spd->t_const_2nd[AS_BH_ID];
				target->motions[step_count]._v = dir * p_as_notch_spd ->v_2nd[AS_BH_ID];
				target->motions[step_count]._p = pIO_Table->physics.R
					+ target->motions[step_count]._v * p_as_notch_spd->t_acc_2nd[AS_BH_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_2nd[AS_BH_ID];
			}

			//Step 2 定速2段目
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_BH_ID] + p_as_notch_spd->t_const_d[AS_BH_ID];
				target->motions[step_count]._v = dir * p_as_notch_spd->v_top[AS_BH_ID];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ (target->motions[step_count]._v + target->motions[step_count - 1]._v) * p_as_notch_spd->t_acc_top[AS_BH_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_d[AS_BH_ID];
			}
			//Step 3 定速3段目
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_BH_ID] + p_as_notch_spd->t_const_2nd[AS_BH_ID];
				target->motions[step_count]._v = dir * p_as_notch_spd->v_2nd[AS_BH_ID];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ (target->motions[step_count - 1]._v + target->motions[step_count]._v) * p_as_notch_spd->t_acc_top[AS_BH_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_acc_top[AS_BH_ID];
			}
			//Step 4 定速4段目 = 停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_2nd[AS_BH_ID];
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count - 1]._v * target->motions[step_count]._t / 2.0;
			}
			//Step end 部
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_TIME_WAIT;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._t = PTN_CONFIRMATION_TIME;
				target->motions[step_count]._p = target->motions[step_count - 1]._p;
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}

			if (mode == AUTO_PTN_MODE_AUTOMOVE) {//自動では旋回位置待ちを入れる　後で要検討
				target->motions[target->n_step].type = CTR_TYPE_SLEW_WAIT;
				target->motions[target->n_step]._p = pIO_Table->auto_ctrl.tgpos_bh;
				target->motions[target->n_step]._t = PTN_ERROR_CHECK_TIME1;
				target->motions[target->n_step].time_count = (int)(target->motions[target->n_step]._t * 1000) / (int)play_scan_ms;
				target->motions[target->n_step]._v = 0.0;
				target->n_step += 1;
			}

		}
		else{//パターン不可
			target->n_step = 1;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern

	        //Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;
			// _p
			target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;
			target->motions[0]._v = 0.0;
			// _t
			target->motions[0]._t = PTN_CONFIRMATION_TIME;
			target->motions[0].act_counter = 0;
			target->motions[0].time_count = (int)(target->motions[0]._t * 1000) / (int)play_scan_ms;
			
			return CAL_RESULT_NEGATIVE;
		}
	}break;
	case MOTION_ID_SLEW: {

		//進行方向セット（開始位相判別用）
		if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] > 0.0) pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_PLUS;
		else if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] < 0.0)pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_MINUS;
		else;

		double dir = (double)pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID];

		if (cal_notch_set(p_as_notch_spd, pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID], AS_PTN_2ACCDEC, AS_SLEW_ID)) {
			int step_count = 0;

			target->n_step = 1;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_2ACCDEC;

			//Step 1 定速1段目
			{
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_2nd[AS_SLEW_ID] + p_as_notch_spd->t_const_2nd[AS_SLEW_ID];
				target->motions[step_count]._v = dir * p_as_notch_spd->v_2nd[AS_SLEW_ID];
				target->motions[step_count]._p = pIO_Table->physics.th
					+ target->motions[step_count]._v * p_as_notch_spd->t_acc_2nd[AS_SLEW_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_2nd[AS_SLEW_ID];
			}

			//Step 2 定速2段目
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_SLEW_ID] + p_as_notch_spd->t_const_d[AS_SLEW_ID];
				target->motions[step_count]._v = dir * p_as_notch_spd->v_top[AS_SLEW_ID];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ (target->motions[step_count]._v + target->motions[step_count - 1]._v) * p_as_notch_spd->t_acc_top[AS_SLEW_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_d[AS_SLEW_ID];
			}
			//Step 3 定速3段目
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_SLEW_ID] + p_as_notch_spd->t_const_2nd[AS_SLEW_ID];
				target->motions[step_count]._v = dir * p_as_notch_spd->v_2nd[AS_SLEW_ID];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ (target->motions[step_count - 1]._v + target->motions[step_count]._v) * p_as_notch_spd->t_acc_top[AS_SLEW_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_acc_top[AS_SLEW_ID];
			}
			//Step 4 定速4段目 = 停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_2nd[AS_SLEW_ID];
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count - 1]._v * target->motions[step_count]._t / 2.0;
			}
			//Step end 部
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_TIME_WAIT;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._t = PTN_CONFIRMATION_TIME;
				target->motions[step_count]._p = target->motions[step_count - 1]._p;
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}

	}break;
	default: return CAL_RESULT_NEGATIVE;
	}
	return CAL_RESULT_POSITIVE;
};

//### 3Step移動パターン　#########################################################
int CAnalyst::cal_move_3Step(int motion_id, LPST_MOTION_UNIT target, int mode) {

	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	LPAS_NOTCH_SET p_as_notch_spd = &as_notch_spd;
	target->n_step = 0;

	switch (motion_id) {
	case MOTION_ID_BH: {

		if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] > 0.0) pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_PLUS;
		else if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] < 0.0)pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_MINUS;
		else;

		double dir = (double)pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID];

		if (cal_notch_set(p_as_notch_spd, pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID], AS_PTN_3STEP, AS_BH_ID)){
			int step_count = 0;

			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_3STEP;

			//Step 1 加速
			{
				step_count = 0;
				target->n_step = 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_BH_ID] + p_as_notch_spd ->t_const_2nd[AS_BH_ID];
				target->motions[step_count]._v = dir * p_as_notch_spd ->v_top[AS_BH_ID];
				target->motions[step_count]._p = pIO_Table->physics.R
												+ target->motions[step_count]._v * p_as_notch_spd->t_acc_top[AS_BH_ID] / 2.0
												+ target->motions[step_count]._v * p_as_notch_spd->t_const_2nd[AS_BH_ID];
			}

			//Step 2　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_BH_ID] + p_as_notch_spd->t_const_2nd[AS_BH_ID];
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count-1]._p
					+ target->motions[step_count-1]._v * p_as_notch_spd->t_acc_top[AS_BH_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_2nd[AS_BH_ID];
			}

			//Step 3　定速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._v = dir * p_as_notch_spd->v_top[AS_BH_ID];
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_BH_ID] + p_as_notch_spd->t_const_d[AS_BH_ID];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
												+ target->motions[step_count]._v * p_as_notch_spd->t_acc_top[AS_BH_ID] / 2.0
												+ target->motions[step_count]._v * p_as_notch_spd->t_const_d[AS_BH_ID];
			}
			//Step 4　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_BH_ID] + p_as_notch_spd->t_const_2nd[AS_BH_ID];
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count-1]._v * p_as_notch_spd->t_acc_top[AS_BH_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_2nd[AS_BH_ID];
			}
			//Step 5 加速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_BH_ID] + p_as_notch_spd->t_const_2nd[AS_BH_ID];
				target->motions[step_count]._v = dir * p_as_notch_spd->v_top[AS_BH_ID];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count]._v * p_as_notch_spd->t_acc_top[AS_BH_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_2nd[AS_BH_ID];
			}

			//Step 6　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_BH_ID];
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count - 1]._v * p_as_notch_spd->t_acc_top[AS_BH_ID] / 2.0;
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}

		}
		else {
			target->n_step = 1;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_0;

			//Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;
			target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;// _p
			target->motions[0]._v = 0.0;						// _v
			target->motions[0]._t = PTN_CONFIRMATION_TIME;		// _t
			target->motions[0].act_counter = 0;
			target->motions[0].time_count = (int)(target->motions[0]._t * 1000) / (int)play_scan_ms;
		}

		if (mode == AUTO_PTN_MODE_AUTOMOVE) {//自動では旋回待ちを入れる　後で要検討
			target->motions[target->n_step].type = CTR_TYPE_SLEW_WAIT;
			target->motions[target->n_step]._p = pIO_Table->auto_ctrl.tgpos_bh;
			target->motions[target->n_step]._t = PTN_ERROR_CHECK_TIME1;
			target->motions[target->n_step].time_count = (int)(target->motions[target->n_step]._t * 1000) / (int)play_scan_ms;
			target->motions[target->n_step]._v = 0.0;
			target->n_step += 1;
		}
	}break;
	case MOTION_ID_SLEW: {

		if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] > 0.0) pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_PLUS;
		else if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] < 0.0)pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_MINUS;
		else;

		double dir = (double)pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID];

		if (cal_notch_set(p_as_notch_spd, pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID], AS_PTN_3STEP, AS_SLEW_ID)) {
			int step_count = 0;

			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_3STEP;

			//Step 1 加速
			{
				step_count = 0;
				target->n_step = 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_SLEW_ID] + p_as_notch_spd->t_const_2nd[AS_SLEW_ID];
				target->motions[step_count]._v = dir * p_as_notch_spd->v_top[AS_SLEW_ID];
				target->motions[step_count]._p = pIO_Table->physics.R
					+ target->motions[step_count]._v * p_as_notch_spd->t_acc_top[AS_SLEW_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_2nd[AS_SLEW_ID];
			}

			//Step 2　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_SLEW_ID] + p_as_notch_spd->t_const_2nd[AS_SLEW_ID];
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count - 1]._v * p_as_notch_spd->t_acc_top[AS_SLEW_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_2nd[AS_SLEW_ID];
			}

			//Step 3　定速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._v = dir * p_as_notch_spd->v_top[AS_SLEW_ID];
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_SLEW_ID] + p_as_notch_spd->t_const_d[AS_SLEW_ID];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count]._v * p_as_notch_spd->t_acc_top[AS_SLEW_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_d[AS_SLEW_ID];
			}
			//Step 4　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_SLEW_ID] + p_as_notch_spd->t_const_2nd[AS_SLEW_ID];;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count - 1]._v * p_as_notch_spd->t_acc_top[AS_SLEW_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_2nd[AS_SLEW_ID];
			}
			//Step 5 加速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_SLEW_ID] + p_as_notch_spd->t_const_2nd[AS_SLEW_ID];
				target->motions[step_count]._v = dir * p_as_notch_spd->v_top[AS_SLEW_ID];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count]._v * p_as_notch_spd->t_acc_top[AS_SLEW_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_2nd[AS_SLEW_ID];
			}

			//Step 6　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_top[AS_SLEW_ID];
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
												+ target->motions[step_count - 1]._v * p_as_notch_spd->t_acc_top[AS_SLEW_ID] / 2.0;
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}

		}
		else{
			target->n_step = 1;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_0;
			//Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;
			target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_slew;	// _p
			target->motions[0]._v = 0.0;
			target->motions[0]._t = PTN_CONFIRMATION_TIME;			// _t
			target->motions[0].act_counter = 0;
			target->motions[0].time_count = (int)(target->motions[0]._t * 1000) / (int)play_scan_ms;
		}

		}
	default:
		return 0;
	}

	return NO_ERR_EXIST;
};

//### 移動振れ止めパターンのノッチ組み合わせ計算　引込　###########################
// 戻り値：Topノッチ,　パターン無い時 0
int CAnalyst::cal_notch_set(LPAS_NOTCH_SET notch_set, double Da, int mode, int type) {

	notch_set->i_notch_top[type] = 0; notch_set->i_notch_2nd[type] = 0;
	notch_set->v_top[type] = 0; notch_set->v_2nd[type] = 0;

	if (mode == AS_PTN_2ACCDEC) { //2段加減速パターン 当面は１パターン固定
		int n = 0;
		double tn,Dmin,t_const_temp;

		switch (type) {
		case AS_BH_ID:
			notch_set->i_notch_top[type] = 5; notch_set->i_notch_2nd[type] = 3;				//ノッチの配列位置セット
			notch_set->v_top[type] = g_spec.bh_notch_spd[notch_set->i_notch_top[type]];		//トップスピード
			notch_set->v_2nd[type] = g_spec.bh_notch_spd[notch_set->i_notch_2nd[type]];		//中間スピード
			notch_set->t_acc_2nd[type] = notch_set->v_2nd[type] / g_spec.bh_acc[FWD_ACC];	//一段目までの加速時間
			tn = g_spec.bh_notch_spd[5] / g_spec.bh_acc[FWD_ACC];
			notch_set->t_acc_top[type] = tn - notch_set->t_acc_2nd[type];					//一段目から二段までの加速時間

			if (tn <= pIO_Table->physics.T) t_const_temp = pIO_Table->physics.T - tn;		//加速中の定速時間（加速時間T以下）
			else							t_const_temp = 3 * pIO_Table->physics.T - tn;	//加速中の定速時間（加速時間T以上）

			Dmin = notch_set->v_top[type] * notch_set->v_top[type] / g_spec.bh_acc[FWD_ACC]	//TOP速度0秒での移動距離
				+ t_const_temp * notch_set->v_2nd[type];
			notch_set->t_const_2nd[type] = t_const_temp / 2;
			notch_set->t_const_d[type] = (Da - Dmin) / notch_set->v_top[type];

			if (notch_set->t_const_d[type] > 0.0) {
				n = 5;
			}
			else {//TOPノッチ移動距離不足

				notch_set->i_notch_top[type] = 4; notch_set->i_notch_2nd[type] = 2;				//ノッチの配列位置セット
				notch_set->v_top[type] = g_spec.bh_notch_spd[notch_set->i_notch_top[type]];		//トップスピード
				notch_set->v_2nd[type] = g_spec.bh_notch_spd[notch_set->i_notch_2nd[type]];		//中間スピード
				notch_set->t_acc_2nd[type] = notch_set->v_2nd[type] / g_spec.bh_acc[FWD_ACC];	//一段目までの加速時間
				tn = g_spec.bh_notch_spd[4] / g_spec.bh_acc[FWD_ACC];
				notch_set->t_acc_top[type] = tn - notch_set->t_acc_2nd[type];					//一段目から二段までの加速時間

				if (tn <= pIO_Table->physics.T) t_const_temp = pIO_Table->physics.T - tn;		//加速中の定速時間（加速時間T以下）
				else							t_const_temp = 3 * pIO_Table->physics.T - tn;	//加速中の定速時間（加速時間T以上）

				Dmin = notch_set->v_top[type] * notch_set->v_top[type] / g_spec.bh_acc[FWD_ACC]	//TOP速度0秒での移動距離
					+ t_const_temp * notch_set->v_2nd[type];
				notch_set->t_const_2nd[type] = t_const_temp / 2;
				notch_set->t_const_d[type] = (Da - Dmin) / notch_set->v_top[type];
				if (notch_set->t_const_d[type] > 0.0) {
					n = 4;
				}
				else {
					n = 0;
				}
			}

			return n;
		case AS_SLEW_ID:
			notch_set->i_notch_top[type] = 5; notch_set->i_notch_2nd[type] = 3;
			notch_set->v_top[type] = g_spec.slew_notch_spd[notch_set->i_notch_top[type]];
			notch_set->v_2nd[type] = g_spec.slew_notch_spd[notch_set->i_notch_2nd[type]];
			notch_set->t_acc_2nd[type] = notch_set->v_2nd[type] / g_spec.slew_acc[FWD_ACC];
			tn = g_spec.slew_notch_spd[5] / g_spec.slew_acc[FWD_ACC];
			notch_set->t_acc_top[type] = tn - notch_set->t_acc_2nd[type];

			if (tn <= pIO_Table->physics.T) t_const_temp = pIO_Table->physics.T - tn;
			else							t_const_temp = 3 * pIO_Table->physics.T - tn;

			Dmin = notch_set->v_top[type] * notch_set->v_top[type] / g_spec.slew_acc[FWD_ACC]
				+ t_const_temp * notch_set->v_2nd[type];
			notch_set->t_const_2nd[type] = t_const_temp / 2;
			notch_set->t_const_d[type] = (Da - Dmin) / notch_set->v_top[type];

			if (notch_set->t_const_d[type] > 0.0) {
				n = 5;
			}
			else {
				notch_set->i_notch_top[type] = 4; notch_set->i_notch_2nd[type] = 2;
				notch_set->v_top[type] = g_spec.slew_notch_spd[notch_set->i_notch_top[type]];
				notch_set->v_2nd[type] = g_spec.slew_notch_spd[notch_set->i_notch_2nd[type]];
				notch_set->t_acc_2nd[type] = notch_set->v_2nd[type] / g_spec.slew_acc[FWD_ACC];
				tn = g_spec.slew_notch_spd[4] / g_spec.slew_acc[FWD_ACC];
				notch_set->t_acc_top[type] = tn - notch_set->t_acc_2nd[type];

				if (tn <= pIO_Table->physics.T) t_const_temp = pIO_Table->physics.T - tn;
				else							t_const_temp = 3 * pIO_Table->physics.T - tn;

				Dmin = notch_set->v_top[type] * notch_set->v_top[type] / g_spec.slew_acc[FWD_ACC]
					+ t_const_temp * notch_set->v_2nd[type];
				notch_set->t_const_2nd[type] = t_const_temp / 2;
				notch_set->t_const_d[type] = (Da - Dmin) / notch_set->v_top[type];

				if (notch_set->t_const_d[type] > 0.0) {
					n = 4;
				}
				else {
					notch_set->i_notch_top[type] = 3; notch_set->i_notch_2nd[type] = 1;
					notch_set->v_top[type] = g_spec.slew_notch_spd[notch_set->i_notch_top[type]];
					notch_set->v_2nd[type] = g_spec.slew_notch_spd[notch_set->i_notch_2nd[type]];
					notch_set->t_acc_2nd[type] = notch_set->v_2nd[type] / g_spec.slew_acc[FWD_ACC];
					tn = g_spec.slew_notch_spd[3] / g_spec.slew_acc[FWD_ACC];
					notch_set->t_acc_top[type] = tn - notch_set->t_acc_2nd[type];

					if (tn <= pIO_Table->physics.T) t_const_temp = pIO_Table->physics.T - tn;
					else							t_const_temp = 3 * pIO_Table->physics.T - tn;

					Dmin = notch_set->v_top[type] * notch_set->v_top[type] / g_spec.slew_acc[FWD_ACC]
						+ t_const_temp * notch_set->v_2nd[type];
					notch_set->t_const_2nd[type] = t_const_temp / 2;
					notch_set->t_const_d[type] = (Da - Dmin) / notch_set->v_top[type];

					if (notch_set->t_const_d[type] > 0.0) {
						n = 3;
					}
					else {
						n = 0;
					}
				}
			}

	//		if (notch_set->t_const_d[type] < 0.0) n = 0;//移動距離不足
	//		else n = 5;

			return n;
		default:
			return 0;
			break;
		}
	}
	else if (mode == AS_PTN_3STEP) { //3段パターン
		int n,k,i_notch_max,i_notch;
		double base_phi, base_t, tn ,temp_d, temp_t, acc_phi,temp_double;

		switch (type) {
		case AS_BH_ID: {
			//Step1 移動距離から最大ノッチを設定
			for (i_notch_max = NOTCH_MAX - 1; i_notch_max > 0; i_notch_max--) {
				//設定ノッチでの最小移動距離
				temp_double = 3.0 * g_spec.bh_notch_spd[i_notch_max] * g_spec.bh_notch_spd[i_notch_max] / g_spec.bh_acc[FWD_ACC];
				if (Da > temp_double)break;
			}
			if (i_notch_max < 1) return 0;//移動距離不足

			//Step2 基準の位相を設定(最小値を求める）
			temp_t = g_spec.bh_notch_spd[i_notch_max] / g_spec.bh_acc[FWD_ACC];
			acc_phi = temp_t * pIO_Table->physics.w0;
			base_phi = -DEF_TPI;
			for (n = 0; n < 5; n++) {
				for (k = 0; k < 2; k++) {
					if (k == 0) {
						temp_double = DEF_2PI * n - DEF_TPI;
					}
					else {
						temp_double = DEF_2PI * n + DEF_TPI;
					}
					if (temp_double > acc_phi) {//加速時間の位相変化が基準設定より小さい
						base_phi = temp_double;//基準値更新
						break;
					}
				}
				if (base_phi > acc_phi) {//加速時間の位相変化が基準設定より大きい
					break;
				}
			}
			//Step3 ノッチの決定
			base_t = base_phi / pIO_Table->physics.w0;//基準位相の時間換算
			for (i_notch = i_notch_max; i_notch > 0; i_notch--) {
				temp_d = 3.0 * g_spec.bh_notch_spd[i_notch] * g_spec.bh_notch_spd[i_notch] / g_spec.bh_acc[FWD_ACC];
				temp_d += 2.0 *  g_spec.bh_notch_spd[i_notch] * (base_t - (g_spec.bh_notch_spd[i_notch] / g_spec.bh_acc[FWD_ACC]));

				if (temp_d < Da) {
					break;
				}
			}
			if (i_notch < 1) {
				return 0;//適合ノッチ無し
			}
			else {
				tn = g_spec.bh_notch_spd[i_notch] / g_spec.bh_acc[FWD_ACC];
				notch_set->i_notch_top[type] = notch_set->i_notch_2nd[type] = i_notch;
				notch_set->v_top[type] = notch_set->v_2nd[type] = g_spec.bh_notch_spd[i_notch];
				notch_set->t_acc_top[type] = notch_set->t_acc_2nd[type] = tn;
				notch_set->t_const_2nd[type] = base_t - tn;
				notch_set->t_const_d[type] = (Da - temp_d) / notch_set->v_top[type];
				return i_notch; //ノッチ選択完了
			}
		}break;
		case AS_SLEW_ID: {
			//Step1 移動距離から最大ノッチを設定
			for (i_notch_max = NOTCH_MAX - 1; i_notch_max > 0; i_notch_max--) {
				//設定ノッチでの最小移動距離
				temp_double = 3.0 * g_spec.slew_notch_spd[i_notch_max] * g_spec.slew_notch_spd[i_notch_max] / g_spec.slew_acc[FWD_ACC];
				if (Da > temp_double)break;
			}
			if (i_notch_max < 1) return 0;//移動距離不足

			//Step2 基準の位相を設定(最小値を求める）
			temp_t = g_spec.slew_notch_spd[i_notch_max] / g_spec.slew_acc[FWD_ACC];
			acc_phi = temp_t * pIO_Table->physics.w0;
			base_phi = -DEF_TPI;
			for (n = 0; n < 5; n++) {
				for (k = 0; k < 2; k++) {
					if (k == 0) {
						temp_double = DEF_2PI * n - DEF_TPI;
					}
					else {
						temp_double = DEF_2PI * n + DEF_TPI;
					}
					if (temp_double > acc_phi) {//加速時間の位相変化が基準設定より小さい
						base_phi = temp_double;//基準値更新
						break;
					}
				}
				if (base_phi > acc_phi) {//加速時間の位相変化が基準設定より大きい
					break;
				}
			}
			//Step3 ノッチの決定
			base_t = base_phi / pIO_Table->physics.w0;//基準位相の時間換算
			for (i_notch = i_notch_max; i_notch > 0; i_notch--) {
				temp_d = 3.0 * g_spec.slew_notch_spd[i_notch] * g_spec.slew_notch_spd[i_notch] / g_spec.slew_acc[FWD_ACC];
				temp_d += 2.0 *  g_spec.slew_notch_spd[i_notch] * (base_t - (g_spec.slew_notch_spd[i_notch] / g_spec.slew_acc[FWD_ACC]));

				if (temp_d < Da) {
					break;
				}
			}
			if (i_notch < 1) {
				return 0;//適合ノッチ無し
			}
			else {
				tn = g_spec.slew_notch_spd[i_notch] / g_spec.slew_acc[FWD_ACC];
				notch_set->i_notch_top[type] = notch_set->i_notch_2nd[type] = i_notch;
				notch_set->v_top[type] = notch_set->v_2nd[type] = g_spec.slew_notch_spd[i_notch];
				notch_set->t_acc_top[type] = notch_set->t_acc_2nd[type] = tn;
				notch_set->t_const_2nd[type] = base_t - tn;
				notch_set->t_const_d[type] = (Da - temp_d) / notch_set->v_top[type];
				return i_notch; //ノッチ選択完了
			}
		}break;
		default:
			return 0;//該当パターンID無し
			break;
		}
	}
	else ;
	return 0xff;//該当パターンID無し
}