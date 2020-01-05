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

	//##MOB計算
	//吊点の動作
	hp.timeEvolution(0.0);  //double t  吊点の計算には経過時間は使わないので0.0

	//## Calicurate the motion of hung load
	hl.timeEvolution(0.0); //double t  吊点の計算には経過時間は使わないので0.0
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
	pIO_Table->physics.lph = asin(temp_r / pIO_Table->physics.L);

	//###Z軸角度の位相平面  x:Theata y:TheataDot/Omega
	pIO_Table->physics.PhPlane_r.x = temp_r / pIO_Table->physics.L;
	if (pIO_Table->physics.PhPlane_r.x < DEF_001DEG)pIO_Table->physics.PhPlane_r.x = DEF_001DEG;

	pIO_Table->physics.PhPlane_r.y = temp_v / (pIO_Table->physics.L * pIO_Table->physics.w0);
	pIO_Table->physics.PhPlane_r.z = atan(pIO_Table->physics.PhPlane_r.y / pIO_Table->physics.PhPlane_r.x);

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

	//###xy平面半径方向の位相平面  x:Theata y:TheataDot/Omega　z:Phi
	pIO_Table->physics.PhPlane_n.x = (rel_lp.x * sin(pIO_Table->physics.th) + rel_lp.y *cos(pIO_Table->physics.th)) / pIO_Table->physics.L;
	pIO_Table->physics.PhPlane_n.y = (rel_lvp.x * sin(pIO_Table->physics.th) + rel_lvp.y *cos(pIO_Table->physics.th)) / (pIO_Table->physics.L * pIO_Table->physics.w0);

	if (abs(pIO_Table->physics.PhPlane_n.x) < 0.000001) {
		pIO_Table->physics.PhPlane_n.z = 0.0;
	}
	else {
		pIO_Table->physics.PhPlane_n.z = atan(pIO_Table->physics.PhPlane_n.y / pIO_Table->physics.PhPlane_n.x);
	}
	if (pIO_Table->physics.PhPlane_n.x < 0.0) {
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


	if (pIO_Table->physics.PhPlane_t.x < 0.0) {
		if (pIO_Table->physics.PhPlane_t.y < 0.0) pIO_Table->physics.PhPlane_t.z -= DEF_PI;
		else pIO_Table->physics.PhPlane_t.z += DEF_PI;
	}
	pIO_Table->physics.sway_amp_t_ph2 = pIO_Table->physics.PhPlane_t.x * pIO_Table->physics.PhPlane_t.x + pIO_Table->physics.PhPlane_t.y * pIO_Table->physics.PhPlane_t.y;
	pIO_Table->physics.sway_amp_t_ph = sqrt(pIO_Table->physics.sway_amp_t_ph2);


	//###各位相平面の加減速時の回転中心OFFSET値

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


	i_vlong_slew.iV2[0] = 5; i_vlong_slew.iV1[0] = 3; // 2段加速パターン　１組目　5ノッチ＋3ノッチ
	i_vlong_slew.iV2[1] = 4; i_vlong_slew.iV1[1] = 1; // 2段加速パターン　2組目　4ノッチ＋1ノッチ
	i_vlong_slew.iV2[2] = 3; i_vlong_slew.iV1[2] = 1; // 2段加速パターン　3組目　3ノッチ＋1ノッチ

	i_vlong_bh.iV2[0] = 5; i_vlong_bh.iV1[0] = 3; // 2段加速パターン　１組目　5ノッチ＋3ノッチ
	i_vlong_bh.iV2[1] = 4; i_vlong_bh.iV1[1] = 2; // 2段加速パターン　2組目　4ノッチ＋1ノッチ
	i_vlong_bh.iV2[2] = 3; i_vlong_bh.iV1[2] = 2; // 2段加速パターン　3組目　3ノッチ＋1ノッチ
	
	return;
};

void CAnalyst::routine_work(void *param) {
	Vector3 rel_lp = hl.r - hp.r;		//吊荷相対xyz
	ws << L" working!" << *(inf.psys_counter) % 100 << "  AS ptn n ;" << pMode->antisway_ptn_n << ": AS ctr n" << pMode->antisway_control_n << "__   AS ptn t ;" << pMode->antisway_ptn_t << ": AS ctr t" << pMode->antisway_control_t;
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
//### Update Anti-sway Control Mode##############
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
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] < g_spec.as_compl_nposLv[I_AS_LV_TRIGGER]) {		//目標が位置決め起動判定距離内
		if (pIO_Table->physics.sway_amp_n_ph > g_spec.as_compl_swayLv[I_AS_LV_TRIGGER]){			//振れがトリガ判定値以上
			pMode->antisway_control_n = AS_MOVE_ANTISWAY;
			pMode->antisway_ptn_n = AS_PTN_DMP;
		}
		else {
			if (pMode->antisway_control_n != AS_MOVE_COMPLETE) {
				pMode->antisway_control_n = AS_MOVE_ANTISWAY;
				pMode->antisway_ptn_n = AS_PTN_POS;
			}
			else {
				pMode->antisway_ptn_n = AS_PTN_0;
			}
		}
	}
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] > g_spec.as_compl_nposLv[I_AS_LV_POSITION]) {
		pMode->antisway_control_n = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_n = AS_PTN_POS;
	}
	else if (pIO_Table->physics.sway_amp_n_ph > g_spec.as_compl_swayLv[I_AS_LV_DAMPING]) {			//振れがダンピング判定値以上
		pMode->antisway_control_n = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_n = AS_PTN_DMP;
	}
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] > g_spec.as_compl_nposLv[I_AS_LV_TRIGGER]) {
		pMode->antisway_control_n = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_n = AS_PTN_POS;
	}
	else;

	//##### Tangent direction

	if (pIO_Table->ref.b_slew_manual_ctrl) {
		pMode->antisway_ptn_t = AS_PTN_0;
		pMode->antisway_control_t = AS_MOVE_INTERRUPT;
	}
	else if ((pIO_Table->physics.vL != 0.0)||(pMode->antisway_hoist == OPE_MODE_AS_ON)) {	//振止動作中
		pMode->antisway_ptn_h = AS_PTN_0;
		pMode->antisway_control_h = AS_MOVE_STANDBY;
	}
	else if ((pIO_Table->physics.sway_amp_t_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) &&		//目標が位置決め完了判定距離内
		(pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] < g_spec.as_compl_tposLv[I_AS_LV_COMPLE])) {	//振れが完了判定内
		pMode->antisway_ptn_t = AS_PTN_0;
		pMode->antisway_control_t = AS_MOVE_COMPLETE;
	}
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] < g_spec.as_compl_tposLv[I_AS_LV_TRIGGER]) {	//目標が位置決め起動判定距離内
		if (pIO_Table->physics.sway_amp_t_ph > g_spec.as_compl_swayLv[I_AS_LV_TRIGGER]) {	//振れがトリガ判定値以上
			pMode->antisway_control_t = AS_MOVE_ANTISWAY;
			pMode->antisway_ptn_t = AS_PTN_DMP;
		}
		else {
			if (pMode->antisway_control_t != AS_MOVE_COMPLETE) {
				pMode->antisway_control_t = AS_MOVE_ANTISWAY;
				pMode->antisway_ptn_t = AS_PTN_POS;
			}
			else 
				pMode->antisway_ptn_t = AS_PTN_0;
		}
	}
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] > g_spec.as_compl_tposLv[I_AS_LV_POSITION]) {
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_t = AS_PTN_POS;
	}
	else if (pIO_Table->physics.sway_amp_t_ph > g_spec.as_compl_swayLv[I_AS_LV_DAMPING]) { //振れがダンピング判定値以上
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_t = AS_PTN_DMP;
	}
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] > g_spec.as_compl_tposLv[I_AS_LV_TRIGGER]) {
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_t= AS_PTN_POS;
	}
	else;

	//##### Hoist direction

	if (pMode->antisway_hoist != OPE_MODE_AS_ON) {
		pMode->antisway_ptn_h = AS_PTN_0;
		pMode->antisway_control_h = AS_MOVE_DEACTIVATE;
	}
	else if (pIO_Table->ref.b_mh_manual_ctrl){
		pMode->antisway_ptn_h = AS_PTN_0;
		pMode->antisway_control_h = AS_MOVE_INTERRUPT;
	}
	else if (pIO_Table->physics.wth != 0.0) {
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
//###################################
void CAnalyst::cal_as_gain() {//振れ止めゲイン＝加速時間
	//### Damping Modeゲイン計算
	//#### 引込方向
	double temp_angle;

	if (pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID] < pIO_Table->physics.sway_amp_n_ph) {
		temp_angle = DEF_HPI*0.8;
	}
	else {
		temp_angle = DEF_HPI*0.8 * pIO_Table->physics.sway_amp_n_ph/ pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID];
		if (temp_angle < DEF_PI / 10.0) temp_angle = DEF_PI / 10.0;
	}
	
	pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID] = temp_angle / pIO_Table->physics.w0;	//Gain = time


	//####旋回方向

	if (pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID] < pIO_Table->physics.sway_amp_t_ph) {
		temp_angle = DEF_HPI * 0.8;
	}
	else {
		temp_angle = DEF_HPI*0.8 * pIO_Table->physics.sway_amp_t_ph / pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID];
		//下限設定
		if (temp_angle < DEF_PI / 4.0) temp_angle = DEF_PI / 4.0 * pIO_Table->physics.R / 40.0;
	}
	pIO_Table->auto_ctrl.as_gain_damp[AS_SLEW_ID] = temp_angle / pIO_Table->physics.w0;

	//### Positioning Modeゲイン計算
	//移動距離から加速時間計算
	pIO_Table->auto_ctrl.as_gain_pos[AS_BH_ID] = sqrt(pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] / g_spec.bh_acc[FWD_ACC]);
	pIO_Table->auto_ctrl.as_gain_pos[AS_SLEW_ID] = sqrt(pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] / g_spec.slew_acc[FWD_ACC]);
	
	//Positioning Modeで設定
	if ((pIO_Table->auto_ctrl.as_gain_pos[AS_BH_ID] > pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID]) && //位置移動量に比べ振れが小さい
		(pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] < 0.2)){											  //移動量が小さい
		pIO_Table->auto_ctrl.as_gain_pos[AS_BH_ID] = pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID];
	}
	if ((pIO_Table->auto_ctrl.as_gain_pos[AS_SLEW_ID] > pIO_Table->auto_ctrl.as_gain_damp[AS_SLEW_ID]) && 	//位置移動量に比べ振れが小さい
		(pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] < 0.005)) {												//距離が近い
		pIO_Table->auto_ctrl.as_gain_pos[AS_SLEW_ID] = pIO_Table->auto_ctrl.as_gain_damp[AS_SLEW_ID];
	}
	//Positioning Modeで上限設定
	if (pIO_Table->auto_ctrl.as_gain_pos[AS_BH_ID] > DEF_HPI / pIO_Table->physics.w0 * 0.5) {
		pIO_Table->auto_ctrl.as_gain_pos[AS_BH_ID] = pIO_Table->auto_ctrl.as_gain_pos[AS_BH_ID]*0.5;
	}

	if (pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] < 0.1) {
//		pIO_Table->auto_ctrl.as_gain_pos[AS_SLEW_ID] = pIO_Table->auto_ctrl.as_gain_pos[AS_SLEW_ID] * 0.5;
	}
	if (pIO_Table->auto_ctrl.as_gain_pos[AS_SLEW_ID] > DEF_HPI / pIO_Table->physics.w0) {
		pIO_Table->auto_ctrl.as_gain_pos[AS_SLEW_ID] = pIO_Table->auto_ctrl.as_gain_pos[AS_SLEW_ID]*0.8;
	}

	return;
};
//@@@
//#########################################################################
int CAnalyst::cal_job_recipe(int job_id, int mode) {
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
			if (cal_long_move_recipe(MOTION_ID_BH, p_bh_target, AUTO_PTN_MODE_AUTOMOVE) == NO_ERR_EXIST) {
				p_bh_target->ptn_status = PTN_STANDBY;
			}
		}
		else if ((pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] > check_d2) && (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] > 0.5)) {
			if (cal_short_move_recipe(MOTION_ID_BH, p_bh_target, AUTO_PTN_MODE_AUTOMOVE) == NO_ERR_EXIST) {
				p_bh_target->ptn_status = PTN_STANDBY;
			}
		}
		else {
			if (pMode->antisway_control_n == AS_MOVE_ANTISWAY) {
				if (cal_as_recipe(MOTION_ID_BH, p_bh_target, AUTO_PTN_MODE_AUTOMOVE) == NO_ERR_EXIST) {
					p_bh_target->ptn_status = PTN_STANDBY;
				}
			}
		}

		if (p_bh_target->ptn_status = PTN_STANDBY) {
			if (cal_long_move_recipe(MOTION_ID_SLEW, p_slew_target, AUTO_PTN_MODE_AUTOMOVE) == NO_ERR_EXIST) {
				p_slew_target->ptn_status = PTN_STANDBY;
			}
		}

		if (p_slew_target->ptn_status != PTN_STANDBY) return ERROR_ANA_CREAPE_AUTO_PATERN;
	}break;
	default:break;
	}

	return NO_ERR_EXIST;
};
//#########################################################################
int CAnalyst::cal_as_recipe(int motion_id, LPST_MOTION_UNIT target, int mode) {

	cal_as_gain();
	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 0;
	pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 0;
	pIO_Table->auto_ctrl.as_out_dir[AS_MH_ID] = 0;

	target->n_step = 0;

	switch (motion_id) {
	case MOTION_ID_BH: {
		if (pMode->antisway_control_n & AS_MOVE_ANTISWAY) {
			target->n_step = 4;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = pMode->antisway_ptn_n;

			//Step 1
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				// _p
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;
				// _t
				target->motions[0]._t = pIO_Table->physics.T*2.0;
				// low phase
				double start_offset = pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID] * pIO_Table->physics.w0;
				target->motions[0].phase1 = start_offset;//DEF_PI * 0.25;
														 // high phase
				target->motions[0].phase2 = -DEF_PI + start_offset; //* 0.75;

				target->motions[0]._v = 0.0;
			}
			//Step 2
			{
				target->motions[1].type = CTR_TYPE_ACC_AS;
				// _p
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_bh;
				// _t
				if (pMode->antisway_ptn_n == AS_PTN_DMP)
					target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID];
				else
					target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_pos[AS_BH_ID];
				// _v
				target->motions[1]._v = g_spec.bh_acc[FWD_ACC] * target->motions[1]._t;
			}
			//Step 3
			{
				target->motions[2].type = CTR_TYPE_DEC_V;
				// _p
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_bh;
				// _t
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID];
				target->motions[2]._v = 0.0;
			}
			//Step 4
			{
				target->motions[3].type = CTR_TYPE_TIME_WAIT;
				// _p
				target->motions[3]._p = pIO_Table->auto_ctrl.tgpos_bh;
				// _t
				target->motions[3]._t = PTN_CONFIRMATION_TIME;
				// _v
				target->motions[3]._v = 0.0;
			}
			//time_count
			for (int i = 0; i < 4; i++) {
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
			// _p
			target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;
			// _t
			target->motions[0]._t = PTN_CONFIRMATION_TIME;
			target->motions[0]._v = 0.0;
			for (int i = 0; i < 1; i++) {
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
		if (pMode->antisway_control_t &  AS_MOVE_ANTISWAY) {
			target->n_step = 4;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = pMode->antisway_ptn_t;

			//Step 1
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				// _p
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_slew;
				// _t
				target->motions[0]._t = pIO_Table->physics.T*2.0;

				double start_offset = pIO_Table->auto_ctrl.as_gain_damp[AS_SLEW_ID] * pIO_Table->physics.w0;
				// low phase
				target->motions[0].phase1 = start_offset;//DEF_PI * 0.25;
														 // high phase
				target->motions[0].phase2 = -DEF_PI + start_offset;// *0.75;

				target->motions[0]._v = 0.0;
			}
			//Step 2
			{
				target->motions[1].type = CTR_TYPE_ACC_AS;
				// _p
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_slew;
				// _t
				if (pMode->antisway_ptn_t == AS_PTN_DMP)
					target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_damp[AS_SLEW_ID];
				else
					target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_pos[AS_SLEW_ID];

				// _v
				target->motions[1]._v = g_spec.slew_acc[FWD_ACC] * target->motions[1]._t;
			}
			//Step 3
			{
				target->motions[2].type = CTR_TYPE_DEC_V;
				// _p
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_slew;
				// _t
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_damp[AS_SLEW_ID];
				// _v
				target->motions[2]._v = pIO_Table->physics.wth;
			}
			//Step 4
			{
				target->motions[3].type = CTR_TYPE_TIME_WAIT;
				// _p
				target->motions[3]._p = pIO_Table->auto_ctrl.tgpos_slew;
				// _t
				target->motions[3]._t = PTN_CONFIRMATION_TIME;
				// _v
				target->motions[3]._v = 0.0;
			}
			//time_count
			for (int i = 0; i < 4; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}
		else {
			//Step 1
			{
				target->n_step = 1;
				target->axis_type = SLW_AXIS;
				target->ptn_status = PTN_STANDBY;
				target->iAct = 0; //Initialize activated pattern
								  //Step 1
				target->motions[0].type = CTR_TYPE_TIME_WAIT;
				// _p
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_slew;
				// _t
				target->motions[0]._t = PTN_CONFIRMATION_TIME;

				target->motions[0]._v = 0.0;
			}
			//time_count
			for (int i = 0; i < 1; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
		}
	}break;
	case MOTION_ID_MH: {
		int step_count;
		target->axis_type = MH_AXIS;
		target->ptn_status = PTN_STANDBY;
		target->iAct = 0; //Initialize activated pattern
		target->motion_type = AS_PTN_MOVE_LONG2;

		//Step 1 位相待ち
		{
			target->n_step = 1;
			step_count = 0;
			target->motions[step_count].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
			target->motions[step_count]._p = pIO_Table->auto_ctrl.tgpos_h;	// _p
			target->motions[step_count]._t = pIO_Table->physics.T*2.0;		// _t
			target->motions[step_count].phase1 = DEF_PI / 4.0;				// low phase
			target->motions[step_count].phase2 = -DEF_PI/4.0; 				// high phase
			target->motions[step_count]._v = 0.0;
		}
		//Step 2 巻上
		{
			step_count += 1;
			target->n_step += 1;

			target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
			target->motions[step_count]._t = DEF_HPI/pIO_Table->physics.w0 - PTN_HOIST_ADJUST_TIME;
			target->motions[step_count]._v = -g_spec.hoist_notch_spd[NOTCH_MAX-1];
			target->motions[step_count]._p = target->motions[step_count - 1]._p
				+ target->motions[step_count]._t * target->motions[step_count]._v
				- target->motions[step_count]._v*target->motions[step_count]._v / g_spec.hoist_acc[FWD_ACC] / 2.0;
		}
		//Step 3 停止
		{
			step_count += 1;
			target->n_step += 1;

			target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
			target->motions[step_count]._t = abs(target->motions[step_count-1]._v/ g_spec.hoist_acc[FWD_ACC])+ PTN_HOIST_ADJUST_TIME;
			target->motions[step_count]._v = 0.0;
			target->motions[step_count]._p = target->motions[step_count - 1]._p
				+ target->motions[step_count-1]._v*target->motions[step_count-1]._v / g_spec.hoist_acc[FWD_ACC] / 2.0;
		}
		//Step 4 巻下
		{
			step_count += 1;
			target->n_step += 1;

			target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
			target->motions[step_count]._t = DEF_HPI / pIO_Table->physics.w0 - PTN_HOIST_ADJUST_TIME;
			target->motions[step_count]._v = g_spec.hoist_notch_spd[NOTCH_MAX-1];
			target->motions[step_count]._p = pIO_Table->auto_ctrl.tgpos_h 
				+ target->motions[step_count]._v*target->motions[step_count]._v / g_spec.hoist_acc[FWD_ACC] / 2.0;
		}
		//Step 5 停止
		{
			step_count += 1;
			target->n_step += 1;

			target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
			target->motions[step_count]._t = abs(target->motions[step_count - 1]._v / g_spec.hoist_acc[FWD_ACC]) + PTN_HOIST_ADJUST_TIME;
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

	}break;

	default: return 1;
	}
	return NO_ERR_EXIST;
};
//#########################################################################
int CAnalyst::cal_long_move_recipe(int motion_id, LPST_MOTION_UNIT target, int mode) {

	cal_as_gain();
	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 0;
	pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 0;
	pIO_Table->auto_ctrl.as_out_dir[AS_MH_ID] = 0;

	int n = 0;
	double dx, acc, Da;

	target->n_step = 0;

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
			if (Da >= 0.0) n = cal_move_pattern_bh(ptn_notch_freq[AS_BH_ID], Da, AS_PTN_MOVE_LONG);
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
			if (Da >= 0.0) n = cal_move_pattern_slew(ptn_notch_freq[AS_SLEW_ID], Da, AS_PTN_MOVE_LONG);
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
	return CAL_RESULT_POSITIVE;
};
//################################################################################
int CAnalyst::cal_long_move_recipe2(int motion_id, LPST_MOTION_UNIT target, int mode) {

	cal_as_gain();

	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 0;
	pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 0;
	pIO_Table->auto_ctrl.as_out_dir[AS_MH_ID] = 0;

	target->n_step = 0;

	int n;
	double t1, t2, t3, t4, dx, acc, Da;

	switch (motion_id) {
	case MOTION_ID_BH: {

		Da = pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID];//目標移動距離
		acc = g_spec.bh_acc[FWD_ACC];

		for (int i = 0; i < DEPTH_OF_2STEP_BH; i++) {

			i_vlong_bh.v1 = g_spec.bh_notch_spd[i_vlong_bh.iV1[i]];
			i_vlong_bh.v2 = g_spec.bh_notch_spd[i_vlong_bh.iV2[i]];
			t1 = i_vlong_bh.v1 / acc;
			t2 = 0.0;
			t3 = i_vlong_bh.v2 / acc - t1;
			n = int(t1 / pIO_Table->physics.T);

			if (i_vlong_bh.v2 < i_vlong_bh.v1) {
				i_vlong_bh.v2 = i_vlong_bh.v1 = 0.0;
				Da = 0.0; t2 = 0.0; break;
			}

			if (t1 < pIO_Table->physics.T * ((double)n + 0.5)) {
				dx = (i_vlong_bh.v2 * i_vlong_bh.v2 - 2.0 * i_vlong_bh.v1 * i_vlong_bh.v1) / acc + i_vlong_bh.v1 * (2 * n + 1)*pIO_Table->physics.T;
				if (Da > dx) {
					t2 = pIO_Table->physics.T * ((double)n + 0.5) - t1;
					t4 = (Da - dx) / i_vlong_bh.v2;
					break;
				}
			}
			else {
				dx = (i_vlong_bh.v2 * i_vlong_bh.v2 - 2.0 * i_vlong_bh.v1 *i_vlong_bh.v1) / acc + i_vlong_bh.v1 * (2 * n + 3)*pIO_Table->physics.T;
				if (Da > dx) {
					t2 = pIO_Table->physics.T * ((double)n + 1.5) - t1;
					t4 = (Da - dx) / i_vlong_bh.v2;
					break;
				}
			}
		}
		if (t2 == 0.0) return CAL_RESULT_NEGATIVE;//パターン作成不可

		if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] > 0.0) pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 1;
		else if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] < 0.0)pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = -1;
		else;

		if (Da <= 0.0) {//パターン不要
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
		else {
			int step_count = 0;

			target->n_step = 1;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_MOVE_LONG2;

			//Step 1 定速1段目
			{
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = t1 + t2;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * i_vlong_bh.v1;
				target->motions[step_count]._p = pIO_Table->physics.R
					+ target->motions[step_count]._v * target->motions[step_count]._t / 2.0
					+ target->motions[step_count]._v * t2;
			}

			//Step 2 定速2段目
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = t3 + t4;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * i_vlong_bh.v2;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ (target->motions[step_count]._v + target->motions[step_count - 1]._v) * t3 / 2.0
					+ target->motions[step_count]._v * t4;
			}
			//Step 3 定速3段目
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = t3 + t2;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * i_vlong_bh.v1;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ (target->motions[step_count - 1]._v + target->motions[step_count]._v) * t3 / 2.0
					+ target->motions[step_count]._v * t2;
			}
			//Step 4 定速4段目 = 停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = t1;
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

		for (int i = 0; i < DEPTH_OF_2STEP_SLEW; i++) {

			i_vlong_slew.v1 = g_spec.slew_notch_spd[i_vlong_slew.iV1[i]];
			i_vlong_slew.v2 = g_spec.slew_notch_spd[i_vlong_slew.iV2[i]];
			t1 = i_vlong_slew.v1 / acc;
			t2 = 0.0;
			t3 = i_vlong_slew.v2 / acc - t1;
			n = int(t1 / pIO_Table->physics.T);

			if (i_vlong_slew.v2 < i_vlong_slew.v1) {
				i_vlong_slew.v2 = i_vlong_slew.v1 = 0.0;
				Da = 0.0; t2 = 0.0; break;
			}

			if (t1 < pIO_Table->physics.T * ((double)n + 0.5)) {
				dx = (i_vlong_slew.v2 * i_vlong_slew.v2 - 2.0 * i_vlong_slew.v1 * i_vlong_slew.v1) / acc + i_vlong_slew.v1 * (2 * n + 1)*pIO_Table->physics.T;
				if (Da > dx) {
					t2 = pIO_Table->physics.T * ((double)n + 0.5) - t1;
					t4 = (Da - dx) / i_vlong_slew.v2;
					break;
				}
			}
			else {
				dx = (i_vlong_slew.v2 * i_vlong_slew.v2 - 2.0 * i_vlong_slew.v1 *i_vlong_slew.v1) / acc + i_vlong_slew.v1 * (2 * n + 3)*pIO_Table->physics.T;
				if (Da > dx) {
					t2 = pIO_Table->physics.T * ((double)n + 1.5) - t1;
					t4 = (Da - dx) / i_vlong_slew.v2;
					break;
				}
			}
		}
		if (t2 == 0.0) return CAL_RESULT_NEGATIVE;//パターン作成不可

		if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] > 0.0) pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 1;
		else if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] < 0.0)pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = -1;
		else;

		if (Da <= 0.0) {//パターン不要
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

		else {
			int step_count = 0;

			target->n_step = 1;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_MOVE_LONG2;

			//Step 1 定速1段目
			{
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = t1 + t2;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * i_vlong_slew.v1;
				target->motions[step_count]._p = pIO_Table->physics.R
					+ target->motions[step_count]._v * target->motions[step_count]._t / 2.0
					+ target->motions[step_count]._v * t2;
			}

			//Step 2 定速2段目
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = t3 + t4;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * i_vlong_slew.v2;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ (target->motions[step_count]._v + target->motions[step_count - 1]._v) * t3 / 2.0
					+ target->motions[step_count]._v * t4;
			}
			//Step 3 定速3段目
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = t3 + t2;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * i_vlong_slew.v1;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ (target->motions[step_count - 1]._v + target->motions[step_count]._v) * t3 / 2.0
					+ target->motions[step_count]._v * t2;
			}
			//Step 4 定速4段目 = 停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = t1;
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
//#########################################################################
int CAnalyst::cal_short_move_recipe(int motion_id, LPST_MOTION_UNIT target, int mode) {

	cal_as_gain();
	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 0;
	pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 0;
	pIO_Table->auto_ctrl.as_out_dir[AS_MH_ID] = 0;

	target->n_step = 0;

	switch (motion_id) {
	case MOTION_ID_BH: {

		if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] > 0.0) pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 1;
		else if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] < 0.0)pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = -1;
		else;

		//目標移動距離Da
		double Da = pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID];
		int n = 0;

		if (Da >= 0.0) n = cal_move_pattern_bh(ptn_notch_freq[AS_BH_ID], Da, AS_PTN_MOVE_SHORT);

		if ((n == 0) || (Da < 0.0)) {//パターン不要
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
		else {
			int step_count = 0;


			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_MOVE_SHORT;

			//Step 1 加速
			{
				step_count = 0;
				target->n_step = 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[n];
				target->motions[step_count]._p = pIO_Table->physics.R
					+ pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count]._v * (target->motions[step_count]._t - target->motions[step_count]._v / g_spec.bh_acc[FWD_ACC] / 2.0);
			}

			//Step 2　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count - 1]._v * target->motions[step_count - 1]._v / g_spec.bh_acc[FWD_ACC] / 2.0;
			}

			//Step 3　定速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[n];
				target->motions[step_count]._t = Da / g_spec.bh_notch_spd[n] - pIO_Table->physics.T / 3.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count]._v * (target->motions[step_count]._t - target->motions[step_count]._v / g_spec.bh_acc[FWD_ACC] / 2.0);
			}
			//Step 4　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count - 1]._v * target->motions[step_count - 1]._v / g_spec.bh_acc[FWD_ACC] / 2.0;
			}
			//Step 5 加速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[n];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count]._v * (target->motions[step_count]._t - target->motions[step_count]._v / g_spec.bh_acc[FWD_ACC] / 2.0);
			}

			//Step 6　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = target->motions[step_count - 1]._v / g_spec.bh_acc[FWD_ACC] + PTN_CONFIRMATION_TIME;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count - 1]._v * target->motions[step_count - 1]._v / g_spec.bh_acc[FWD_ACC] / 2.0;
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

		if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] > 0.0) pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 1;
		else if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] < 0.0)pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = -1;
		else;

		//目標移動距離Da

		double Da = pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID];
		int n = 0;

		if (Da >= 0.0) n = cal_move_pattern_slew(ptn_notch_freq[AS_SLEW_ID], Da, AS_PTN_MOVE_SHORT);

		if ((n == 0) || (Da < 0.0)) {//パターン不要
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

		else {

			int step_count = 0;

			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_MOVE_SHORT;

			//Step 1 加速
			{
				step_count = 0;
				target->n_step = 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];
				target->motions[step_count]._p = pIO_Table->physics.th
					+ pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count]._v * (target->motions[step_count]._t - target->motions[step_count]._v / g_spec.slew_acc[FWD_ACC] / 2.0);
			}
			//Step 2　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count - 1]._v * target->motions[step_count - 1]._v / g_spec.slew_acc[FWD_ACC] / 2.0;
			}

			//Step 3　定速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];
				target->motions[step_count]._t = Da / g_spec.slew_notch_spd[n] - pIO_Table->physics.T / 3.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count]._v * (target->motions[step_count]._t - target->motions[step_count]._v / g_spec.slew_acc[FWD_ACC] / 2.0);
			}
			//Step 4　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count - 1]._v * target->motions[step_count - 1]._v / g_spec.slew_acc[FWD_ACC] / 2.0;
			}
			//Step 5 加速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count]._v * (target->motions[step_count]._t - target->motions[step_count]._v / g_spec.slew_acc[FWD_ACC] / 2.0);
			}

			//Step 6　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = target->motions[step_count - 1]._v / g_spec.bh_acc[FWD_ACC] + PTN_CONFIRMATION_TIME;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count - 1]._v * target->motions[step_count - 1]._v / g_spec.slew_acc[FWD_ACC] / 2.0;
			}

		}

		//time_count
		for (int i = 0; i < target->n_step; i++) {
			target->motions[i].act_counter = 0;
			target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
		}
	}

	}
	return NO_ERR_EXIST;
};
//#########################################################################
int CAnalyst::cal_short_move_recipe2(int motion_id, LPST_MOTION_UNIT target, int mode) {

	cal_as_gain();
	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 0;
	pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 0;
	pIO_Table->auto_ctrl.as_out_dir[AS_MH_ID] = 0;

	target->n_step = 0;

	switch (motion_id) {
	case MOTION_ID_BH: {

		if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] > 0.0) pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 1;
		else if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] < 0.0)pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = -1;
		else;

		//目標移動距離Da
		double Da = pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID];
		int n = 0;

		double check_t = pIO_Table->physics.T / 4.0; // 0.5 PAI
		if (Da < (check_t * check_t * g_spec.bh_acc[FWD_ACC])*2.0)
			check_t = sqrt(Da / g_spec.bh_acc[FWD_ACC] / 2.0);

		if (Da < 0.0) {//パターン不要
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
		else {
			int step_count = 0;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_MOVE_SHORT2;

			//Step 1 加速
			{
				step_count = 0;
				target->n_step = 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = check_t;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[NOTCH_MAX - 1];
				target->motions[step_count]._p = pIO_Table->physics.R
					+ pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count]._t * target->motions[step_count]._t / g_spec.bh_acc[FWD_ACC] / 2.0;
			}

			//Step 2　減速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = check_t;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count]._t * target->motions[step_count]._t / g_spec.bh_acc[FWD_ACC] / 2.0;
			}

			//Step 3　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._t = DEF_PI / pIO_Table->physics.w0 - check_t * 2.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p;
			}
			//Step 4　加速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = check_t;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count]._t * target->motions[step_count]._t / g_spec.bh_acc[FWD_ACC] / 2.0;
			}
			//Step 5 減速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = check_t;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[NOTCH_MAX - 1];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count]._t * target->motions[step_count]._t / g_spec.bh_acc[FWD_ACC] / 2.0;
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

		if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] > 0.0) pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 1;
		else if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] < 0.0)pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = -1;
		else;

		//目標移動距離Da

		double Da = pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID];
		int n = 0;

		double check_t = pIO_Table->physics.T / 4.0; // 0.5 PAI
		if (Da < (2.0 * check_t * check_t * g_spec.slew_acc[FWD_ACC]))
			check_t = sqrt(Da / g_spec.slew_acc[FWD_ACC] / 2.0);

		if (Da < 0.0) {//パターン不要
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
		else {

			int step_count = 0;

			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_MOVE_SHORT2;

			//Step 1 加速
			{
				step_count = 0;
				target->n_step = 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = check_t;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[NOTCH_MAX - 1];
				target->motions[step_count]._p = pIO_Table->physics.th
					+ pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count]._t * target->motions[step_count]._t / g_spec.slew_acc[FWD_ACC] / 2.0;
			}
			//Step 2　減速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = check_t;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count]._t * target->motions[step_count]._t / g_spec.slew_acc[FWD_ACC] / 2.0;
			}

			//Step 3　停止
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._t = DEF_PI / pIO_Table->physics.w0 - 2.0 * check_t;
				target->motions[step_count]._p = target->motions[step_count - 1]._p;
			}
			//Step 4　加速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = check_t;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[NOTCH_MAX - 1];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count]._t * target->motions[step_count]._t / g_spec.slew_acc[FWD_ACC] / 2.0;
			}
			//Step 5 減速
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = check_t;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count]._t * target->motions[step_count]._t / g_spec.slew_acc[FWD_ACC] / 2.0;
			}
		}

		//time_count
		for (int i = 0; i < target->n_step; i++) {
			target->motions[i].act_counter = 0;
			target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
		}
	}

	}
	return NO_ERR_EXIST;
};
//### 移動振れ止めパターンのノッチ組み合わせ計算　引込　###########################
int CAnalyst::cal_move_pattern_bh(int * notch_freq, double Da, int mode) {
	int notch_flag = 0;
	double cal_Da = Da;
	for (int i = 0; i < NOTCH_MAX; i++) *(notch_freq + i) = 0;

	if (mode == AS_PTN_MOVE_LONG) {
		for (int n = NOTCH_MAX - 1; n > 0; n--) {
			if (notch_flag == 0) {
				double Vn = g_spec.bh_notch_spd[n];
				double chk_dist = Vn * pIO_Table->physics.T;
				if (cal_Da - chk_dist < 0.0) continue;
				else {
					cal_Da -= chk_dist;	notch_flag = n;
					break;
				}
			}
		}

		if (notch_flag == 0) return notch_flag;
		else {
			for (int n = notch_flag; n > 0; n--) {
				double Vn = g_spec.bh_notch_spd[n];
				double chk_dist = Vn * pIO_Table->physics.T;
				if (chk_dist > 0.0) {
					notch_freq[n] = int(cal_Da / chk_dist);
					cal_Da = cal_Da - notch_freq[n] * chk_dist;
				}
			}
		}
	}
	else if (mode == AS_PTN_MOVE_SHORT) {
		for (int n = NOTCH_MAX - 1; n > 0; n--) {
			if (notch_flag == 0) {
				double Vn = g_spec.bh_notch_spd[n];
				double chk_dist = Vn * (pIO_Table->physics.T / 3.0 + Vn / g_spec.bh_acc[FWD_ACC]);
				if (cal_Da - chk_dist < 0.0) continue;
				else {
					cal_Da -= chk_dist;	notch_flag = n;
					break;
				}
			}
		}

	}
	else;

	return notch_flag;
};
//### 移動振れ止めパターンのノッチ組み合わせ計算　旋回　############################
int CAnalyst::cal_move_pattern_slew(int * notch_freq, double Da, int mode) {
	int notch_flag = 0;
	double cal_Da = Da;
	for (int i = 0; i < NOTCH_MAX; i++) *(notch_freq + i) = 0;
	if (mode == AS_PTN_MOVE_LONG) {
		for (int n = NOTCH_MAX - 1; n > 0; n--) {
			if (notch_flag == 0) {
				double Vn = g_spec.slew_notch_spd[n];
				double chk_dist = Vn * pIO_Table->physics.T;
				if (cal_Da - chk_dist < 0.0) continue;
				else {
					cal_Da -= chk_dist;	notch_flag = n;
					break;
				}
			}
		}

		if (notch_flag == 0) return notch_flag;
		else {
			for (int n = notch_flag; n > 0; n--) {
				double Vn = g_spec.slew_notch_spd[n];
				double chk_dist = Vn * pIO_Table->physics.T;
				if (chk_dist > 0.0) {
					notch_freq[n] = int(cal_Da / chk_dist);
					cal_Da = cal_Da - notch_freq[n] * chk_dist;
				}
			}
		}
	}
	else if (mode == AS_PTN_MOVE_SHORT) {
		for (int n = NOTCH_MAX - 1; n > 0; n--) {
			if (notch_flag == 0) {
				double Vn = g_spec.slew_notch_spd[n];
				double chk_dist = Vn * (pIO_Table->physics.T / 3.0 + Vn / g_spec.slew_acc[FWD_ACC]);
				if (cal_Da - chk_dist < 0.0) continue;
				else {
					cal_Da -= chk_dist;	notch_flag = n;
					break;
				}
			}
		}

	}
	else;

	return notch_flag;
};



