#include "stdafx.h"
#include "CAnalyst.h"
#include "SharedObjects.h"
#include "CPlayer.h"

extern CORDER_Table*	pOrder;				//���L������Order�N���X�|�C���^
extern CMODE_Table*		pMode;				//���L������Mode�N���X�|�C���^
extern ST_SPEC			g_spec;				//�N���[���d�l
extern CIO_Table*		pIO_Table;


CAnalyst::CAnalyst(){
	for (int i = 0; i < NUM_OF_AS; i++)	pIO_Table->auto_ctrl.phase_chk_range[i] = DEF_PI / 50.0;
}

CAnalyst::~CAnalyst(){
}



void CAnalyst::cal_simulation() {

	//##�����x�w�ߒl�v�Z  acc_cyl x:r y:th z:z

	double def_w = pIO_Table->physics.vR - pIO_Table->ref.bh_v;
	if (MY_ABS(def_w) <= ALLOWABLE_DEF_BH) hp.a_bm_ref = 0.0;	//0���w�ߔ���
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

	//##MOB�v�Z
	//�ݓ_�̓���
	hp.timeEvolution(0.0);  //double t  �ݓ_�̌v�Z�ɂ͌o�ߎ��Ԃ͎g��Ȃ��̂�0.0

	//�ׂ݉̓���
	hl.timeEvolution(0.0); //double t  �ׂ݉̌v�Z�ɂ͌o�ߎ��Ԃ͎g��Ȃ��̂�0.0
	
	hl.r.add(hl.dr);
	hl.v.add(hl.dv);


	//##IO TABLE �Z�b�g

	pIO_Table->physics.cp = hp.r; 		//�ݓ_xyz

	pIO_Table->physics.R = hp.r_bm;		//������
	pIO_Table->physics.th = hp.th_sl;	//����p�x
	pIO_Table->physics.ph = 0.0;		//�N���p�x

	pIO_Table->physics.cv = hp.v; 		//�ݓ_vx vy vz
	pIO_Table->physics.vR = hp.v_bm;	//�������ω����x
	pIO_Table->physics.wth = hp.w_sl;	//����p���x
	pIO_Table->physics.wph = 0.0;		//�N���p���x

	pIO_Table->physics.lp = hl.r;		//�݉�xyz
	pIO_Table->physics.lv = hl.v;		//�݉ב��xvx vy vz
		
	Vector3 rel_lp =  hl.r - hp.r;		//�݉ב���xyz
	Vector3 rel_lvp = hl.v - hp.v;		//�݉ב��Α��xvx vy vz

	
	double last_L = pIO_Table->physics.L;
	//###���[�v��
	pIO_Table->physics.L = hp.l_h;	
	pIO_Table->physics.vL = hp.v_h;	//�����x
	
	//###Z���Ƃ̊p�x

	double temp_r = sqrt(rel_lp.x * rel_lp.x + rel_lp.y * rel_lp.y);//XY���ʔ��a
	double temp_v = sqrt(rel_lvp.x * rel_lvp.x + rel_lvp.y * rel_lvp.y);//XY���ʔ��a���x
	pIO_Table->physics.lph = asin(temp_r / pIO_Table->physics.L);//�����Ƃ̊p�x

	//###Z���p�x�̈ʑ�����  x:Theata y:TheataDot/Omega
	pIO_Table->physics.PhPlane_r.x = temp_r / pIO_Table->physics.L;//r/L = theata
	if (pIO_Table->physics.PhPlane_r.x < DEF_001DEG)pIO_Table->physics.PhPlane_r.x = DEF_001DEG;//0����h�~�p����

	pIO_Table->physics.PhPlane_r.y = temp_v / (pIO_Table->physics.L * pIO_Table->physics.w0);//vr/L/w = thata dot/w

	double last_rz = pIO_Table->physics.PhPlane_r.z; //r�ʑ��O��l

	pIO_Table->physics.PhPlane_r.z = atan(pIO_Table->physics.PhPlane_r.y / pIO_Table->physics.PhPlane_r.x);
	buf_average_phase_rdz[i_buf_rdz] = pIO_Table->physics.PhPlane_r.z - last_rz; i_buf_rdz++;
	if (i_buf_rdz > MAX_SAMPLE_AVERAGE_RZ - 1) i_buf_rdz = 0;

	double temp_double=0.0;
	for (int i = 0; i < MAX_SAMPLE_AVERAGE_RZ; i++) temp_double += buf_average_phase_rdz[i];
	pIO_Table->physics.wPhPlane_r = temp_double/ MAX_SAMPLE_AVERAGE_RZ;


	pIO_Table->physics.sway_amp_r_ph2 = pIO_Table->physics.PhPlane_r.x * pIO_Table->physics.PhPlane_r.x + pIO_Table->physics.PhPlane_r.y * pIO_Table->physics.PhPlane_r.y;
	pIO_Table->physics.sway_amp_r_ph = sqrt(pIO_Table->physics.sway_amp_r_ph2);



	//###XY���ʊp�x
	double radious = pIO_Table->physics.L * sin(pIO_Table->physics.lph);
	if (radious < DEF_001DEG) radious = DEF_001DEG;
	double last_th = pIO_Table->physics.lth;
	pIO_Table->physics.lth = acos(rel_lp.x / radious);

	//�U��p���g��
	if (pIO_Table->physics.L > 1.0)	pIO_Table->physics.w0 = sqrt(DEF_G / pIO_Table->physics.L);
	else pIO_Table->physics.w0 = sqrt(DEF_G);
	//�U�����
	pIO_Table->physics.T = DEF_2PI / pIO_Table->physics.w0;

	//###xy���ʔ��a����(�����j�̈ʑ�����  x:Theata y:TheataDot/Omega�@z:Phi 
	pIO_Table->physics.PhPlane_n.x = (rel_lp.x * sin(pIO_Table->physics.th) + rel_lp.y *cos(pIO_Table->physics.th)) / pIO_Table->physics.L;
	pIO_Table->physics.PhPlane_n.y = (rel_lvp.x * sin(pIO_Table->physics.th) + rel_lvp.y *cos(pIO_Table->physics.th)) / (pIO_Table->physics.L * pIO_Table->physics.w0);

	if (abs(pIO_Table->physics.PhPlane_n.x) < 0.000001) {
		pIO_Table->physics.PhPlane_n.z = 0.0;
	}
	else {
		pIO_Table->physics.PhPlane_n.z = atan(pIO_Table->physics.PhPlane_n.y / pIO_Table->physics.PhPlane_n.x);
	}
	if (pIO_Table->physics.PhPlane_n.x < 0.0) {//�ʑ���-�΁`�΂ŕ\������
		if (pIO_Table->physics.PhPlane_n.y < 0.0) pIO_Table->physics.PhPlane_n.z -= DEF_PI;
		else pIO_Table->physics.PhPlane_n.z += DEF_PI;
	}
	pIO_Table->physics.sway_amp_n_ph2 = pIO_Table->physics.PhPlane_n.x * pIO_Table->physics.PhPlane_n.x + pIO_Table->physics.PhPlane_n.y * pIO_Table->physics.PhPlane_n.y;
	pIO_Table->physics.sway_amp_n_ph = sqrt(pIO_Table->physics.sway_amp_n_ph2);

	//###xy���ʐڐ������̈ʑ�����  x:Theata y:TheataDot/Omega�@z:Phi
	pIO_Table->physics.PhPlane_t.x = (rel_lp.x * cos(pIO_Table->physics.th) - rel_lp.y *sin(pIO_Table->physics.th)) / pIO_Table->physics.L;
	pIO_Table->physics.PhPlane_t.y = (rel_lvp.x * cos(pIO_Table->physics.th) - rel_lvp.y *sin(pIO_Table->physics.th)) / (pIO_Table->physics.L* pIO_Table->physics.w0);
	
	if (abs(pIO_Table->physics.PhPlane_t.x) < 0.000001) {
		pIO_Table->physics.PhPlane_t.z = 0.0;
	}
	else {
		pIO_Table->physics.PhPlane_t.z = atan(pIO_Table->physics.PhPlane_t.y / pIO_Table->physics.PhPlane_t.x);
	}


	if (pIO_Table->physics.PhPlane_t.x < 0.0) {//�ʑ���-�΁`�΂ŕ\������
		if (pIO_Table->physics.PhPlane_t.y < 0.0) pIO_Table->physics.PhPlane_t.z -= DEF_PI;
		else pIO_Table->physics.PhPlane_t.z += DEF_PI;
	}
	pIO_Table->physics.sway_amp_t_ph2 = pIO_Table->physics.PhPlane_t.x * pIO_Table->physics.PhPlane_t.x + pIO_Table->physics.PhPlane_t.y * pIO_Table->physics.PhPlane_t.y;
	pIO_Table->physics.sway_amp_t_ph = sqrt(pIO_Table->physics.sway_amp_t_ph2);


	//###�e�ʑ����ʂ̉��������̉�]���SOFFSET�l�@r0 rad

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

	hl.pHP = &hp; //�ݓ_�ƕR�t��

	pIO_Table->auto_ctrl.allowable_pos_overshoot_plus[AS_BH_ID]= 1.0;		//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s����
	pIO_Table->auto_ctrl.allowable_pos_overshoot_plus[AS_SLEW_ID] = 1.0;	//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s����
	pIO_Table->auto_ctrl.allowable_pos_overshoot_plus[AS_MH_ID] = 1.0;		//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s����

	pIO_Table->auto_ctrl.allowable_pos_overshoot_minus[AS_BH_ID] = 1.0;		//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s�t����
	pIO_Table->auto_ctrl.allowable_pos_overshoot_minus[AS_SLEW_ID] = 1.0;	//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s�t����
	pIO_Table->auto_ctrl.allowable_pos_overshoot_minus[AS_MH_ID] = 1.0;		//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s�t����

	//�ʑ��p���x���Ϗ����p�o�b�t�@������
	for (int i = 0; i < MAX_SAMPLE_AVERAGE_RZ; i++) buf_average_phase_rdz[i] = 0.0;
	i_buf_rdz = 0;
	
	//2STEP ���ړ��ő�ړ�����
	pIO_Table->auto_ctrl.Dmax_2step[AS_BH_ID] = 2.0 * g_spec.bh_notch_spd[NOTCH_MAX - 1] * g_spec.bh_notch_spd[NOTCH_MAX - 1] / g_spec.bh_acc[FWD_ACC];
	pIO_Table->auto_ctrl.Dmax_2step[AS_SLEW_ID] = 2.0 * g_spec.slew_notch_spd[NOTCH_MAX - 1] * g_spec.slew_notch_spd[NOTCH_MAX - 1] / g_spec.slew_acc[FWD_ACC];

	return;
};

void CAnalyst::routine_work(void *param) {
	Vector3 rel_lp = hl.r - hp.r;		//�݉ב���xyz
//	ws << L" working!" << *(inf.psys_counter) % 100 << "  AS ptn n ;" << pMode->antisway_ptn_n << ": AS ctr n" << pMode->antisway_control_n << "__   AS ptn t ;" << pMode->antisway_ptn_t << ": AS ctr t" << pMode->antisway_control_t;
	
	ws << L"BH AS:" << pMode->antisway_control_n << L"SLEW AS:" << pMode->antisway_control_t;
	tweet2owner(ws.str()); ws.str(L""); ws.clear();

	//�V�~�����[�^�v�Z
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
			pIO_Table->auto_ctrl.tgpos_slew = pIO_Table->physics.th;	//����ڕW�ʒu�͐���p�x
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

	if (pIO_Table->ref.b_bh_manual_ctrl) {	//�蓮���쒆
		pMode->antisway_ptn_n = AS_PTN_0;
		pMode->antisway_control_n = AS_MOVE_INTERRUPT;
	}
	else if ((pIO_Table->physics.vR != 0.0) || (pMode->antisway_hoist == OPE_MODE_AS_ON) ){	//�U�~���쒆
		pMode->antisway_ptn_n = AS_PTN_0;
		pMode->antisway_control_n = AS_MOVE_STANDBY;
	}
	else if ((pIO_Table->physics.sway_amp_n_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) &&			//�U�ꂪ�������x��
				(pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] < g_spec.as_compl_nposLv[I_AS_LV_COMPLE])){	//�ʒu���������x��
		pMode->antisway_ptn_n = AS_PTN_0;
		pMode->antisway_control_n = AS_MOVE_COMPLETE;
	}
#if 1
	else {
		pMode->antisway_control_n = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_n = AS_PTN_2ACCDEC;
	}
#else
	else if (pIO_Table->physics.sway_amp_n_ph > pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID]) {	//�U�ꂪ�����U�����
			pMode->antisway_control_n = AS_MOVE_ANTISWAY;
			pMode->antisway_ptn_n = AS_PTN_1STEP;
	}
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] < g_spec.as_compl_nposLv[I_AS_LV_TRIGGER]) {	//�ڕW���ʒu���ߋN�����苗����
		if (pIO_Table->physics.sway_amp_n_ph > g_spec.as_compl_swayLv[I_AS_LV_TRIGGER]){			//�U�ꂪ�g���K����l�ȏ�
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
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] <= pIO_Table->auto_ctrl.Dmax_2step[AS_BH_ID]) { //2��C���`���O�ő�ړ������ȉ�
		pMode->antisway_control_n = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_n = AS_PTN_2STEP_PP;
	}
	else{
		pMode->antisway_control_n = AS_MOVE_ANTISWAY;

		//2�i�������ŏ��ړ�����
		double Dmin = g_spec.bh_notch_spd[NOTCH_MAX - 1]*(pIO_Table->physics.T + g_spec.bh_notch_spd[NOTCH_MAX - 1] / g_spec.bh_acc[FWD_ACC]);
		//2�i������
		if (pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] <= Dmin) pMode->antisway_ptn_n = AS_PTN_3STEP;
		//3�i
		else pMode->antisway_ptn_n = AS_PTN_2ACCDEC;
	}
#endif
	//##### Tangent direction

	if (pIO_Table->ref.b_slew_manual_ctrl) {
		pMode->antisway_ptn_t = AS_PTN_0;
		pMode->antisway_control_t = AS_MOVE_INTERRUPT;
	}
	else if ((pIO_Table->physics.wth != 0.0)||(pMode->antisway_hoist == OPE_MODE_AS_ON)) {	//�U�~���쒆
		pMode->antisway_ptn_t = AS_PTN_0;
		pMode->antisway_control_t = AS_MOVE_STANDBY;
	}
	else if ((pIO_Table->physics.sway_amp_t_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) &&		//�ڕW���ʒu���ߊ������苗����
		(pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] < g_spec.as_compl_tposLv[I_AS_LV_COMPLE])) {	//�U�ꂪ���������
		pMode->antisway_ptn_t = AS_PTN_0;
		pMode->antisway_control_t = AS_MOVE_COMPLETE;
	}
#if 1
	else{	//�U�ꂪ�����U��ȏ�
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_t = AS_PTN_2ACCDEC;
	}
#else
	else if (pIO_Table->physics.sway_amp_t_ph > pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID]) {	//�U�ꂪ�����U��ȏ�
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_t = AS_PTN_1STEP;
	}
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] < g_spec.as_compl_tposLv[I_AS_LV_TRIGGER]) {	//�ڕW���ʒu���ߋN�����苗����
		if (pIO_Table->physics.sway_amp_t_ph > g_spec.as_compl_swayLv[I_AS_LV_TRIGGER]) {	//�U�ꂪ�g���K����l�ȏ�
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
	else if (pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] <= pIO_Table->auto_ctrl.Dmax_2step[AS_SLEW_ID]) { //2��C���`���O�ő�ړ������ȉ�
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_t = AS_PTN_2STEP_PP;
	}
	else {
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;

		//2�i�������ŏ��ړ�����
		double Dmin = g_spec.slew_notch_spd[NOTCH_MAX - 1] * (pIO_Table->physics.T + g_spec.slew_notch_spd[NOTCH_MAX - 1] / g_spec.bh_acc[FWD_ACC]);
		//2�i������
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
	else if (pIO_Table->physics.sway_amp_r_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) {	//�U�ꂪ���������
		pMode->antisway_ptn_h = AS_PTN_0;
		pMode->antisway_control_h = AS_MOVE_COMPLETE;
	}
	else if (pMode->antisway_control_h == AS_MOVE_COMPLETE) {	//�U�~������
		if (pIO_Table->physics.sway_amp_r_ph > g_spec.as_compl_swayLv[I_AS_LV_TRIGGER]) {	//�U�ꂪ�g���K����l�ȏ�
			pMode->antisway_control_h = AS_MOVE_ANTISWAY;
		}
		else {
				pMode->antisway_ptn_h = AS_PTN_0;
		}
	}
	else if (pIO_Table->physics.sway_amp_r_ph > g_spec.as_compl_swayLv[I_AS_LV_TRIGGER]) {	//�U�ꂪ�g���K����l�ȏ�
		pMode->antisway_control_h = AS_MOVE_ANTISWAY;
	}
	else;

	return;
};

//### �U��~�߃Q�C���v�Z�i1Step�@2Step�j##########################################
void CAnalyst::cal_as_gain(int motion_id, int type) {

	double R0,gain_limit1, gain_limit2;

	if (motion_id == AS_BH_ID) {

		gain_limit2 = g_spec.bh_notch_spd[NOTCH_MAX - 1] / g_spec.bh_acc[FWD_ACC];	//�ő呬�x�ɂ��������Ԑ���
		if (type == AS_PTN_1STEP) {
			R0 = pIO_Table->physics.sway_amp_n_ph;
			gain_limit1 = 4 * pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID];			//1��̃C���`���O�ŗ}���\�ȍő�U���́A4r0
			if (R0 > gain_limit1) R0 = gain_limit1;
			if (R0 > gain_limit2 * pIO_Table->physics.w0) R0 = gain_limit2 * pIO_Table->physics.w0;

			pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID] = acos(1 - 0.5 * R0 / pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID]);
			pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID] / pIO_Table->physics.w0;
		}
		else if(type == AS_PTN_2STEP_PN){
			//�����U��ʂł܂��A�Q�C����ݒ�
			if (pIO_Table->physics.sway_amp_n_ph < pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID]) {//�U��U���������U�����
				//2��̃C���`���O�ŐU��~�ߗp
				double temp_d = acos(1 - pIO_Table->physics.sway_amp_n_ph /4.0/ pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID]);
				pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = temp_d/ pIO_Table->physics.w0;
			}
			else {
				//2��ڂ̃C���`���O�ŐU��~�߁i�҂����Ԓ����^�C�v�j�p
				pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = sqrt(pIO_Table->physics.sway_amp_n_ph * pIO_Table->physics.L / g_spec.bh_acc[FWD_ACC]);
			}
			//�ő呬�x�ɂ��������Ԑ���
			if (pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] > gain_limit2) { 
				pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = gain_limit2;
			}
			//�ʑ���/2�ȉ��ɂ��鐧��
			if (pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] > DEF_HPI/ pIO_Table->physics.w0) {
				pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = DEF_HPI / pIO_Table->physics.w0;
			}
			pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID] = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] * pIO_Table->physics.w0;
		}
		else if (type == AS_PTN_2STEP_PP) {
			pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] = sqrt(0.5 * pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] / g_spec.bh_acc[FWD_ACC]);
			//�ő呬�x�ɂ��������Ԑ���
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
		gain_limit2 = g_spec.slew_notch_spd[NOTCH_MAX - 1] / g_spec.slew_acc[FWD_ACC];	//�ő呬�x�ɂ��������Ԑ���

		if (type == AS_PTN_1STEP) {
			R0 = pIO_Table->physics.sway_amp_t_ph;
			gain_limit1 = 4 * pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID]; //1��̃C���`���O�ŗ}���\�ȍő�U���́A4r0
			if (R0 > gain_limit1) R0 = gain_limit1;
			if (R0 > gain_limit2 * pIO_Table->physics.w0) R0 = gain_limit2 * pIO_Table->physics.w0;

			pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID] = acos(1 - 0.5 * R0 / pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID]);
			pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID] / pIO_Table->physics.w0;
		}
		else if (type == AS_PTN_2STEP_PN) {
			//�����U��ʂł܂��A�Q�C����ݒ�
			if (pIO_Table->physics.sway_amp_t_ph < pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID]) {//�U��U���������U�����
				//2��̃C���`���O�ŐU��~�ߗp
//				double temp_ph = acos(1 - pIO_Table->physics.sway_amp_t_ph / 4.0 / pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID] );
				double temp_ph = acos(1 - pIO_Table->physics.sway_amp_t_ph / 1.8 / pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID]);
				pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = temp_ph  /  pIO_Table->physics.w0;
			}
			else {
				//2��ڂ̃C���`���O�ŐU��~�߁i�҂����Ԓ����j�p
				pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = sqrt(pIO_Table->physics.sway_amp_t_ph * pIO_Table->physics.L / g_spec.slew_acc[FWD_ACC]);
			}

			//�ő呬�x�ɂ��������Ԑ���
			if (pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] > gain_limit2) {
				pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = gain_limit2;
			}
			//�ʑ���/2�ȉ��ɂ��鐧��
			if (pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] > DEF_HPI / pIO_Table->physics.w0) {
				pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = DEF_HPI / pIO_Table->physics.w0;
			}
			pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID] = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] * pIO_Table->physics.w0;
		}
		else if (type == AS_PTN_2STEP_PP) {
			pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = sqrt(0.5 * pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] / g_spec.slew_acc[FWD_ACC]);
			//�ő呬�x�ɂ��������Ԑ���
			if (pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] > gain_limit2) {
				pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] = gain_limit2;
			}
			//�ʑ���/2�ȉ��ɂ��鐧��
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

		check_d = pIO_Table->physics.T * g_spec.bh_notch_spd[1] //1�m�b�`������@�{�@�U��~��1��̈ړ�����
			+ g_spec.bh_acc[FWD_ACC] * pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID] * pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID];
		check_d2 = pIO_Table->physics.T * g_spec.bh_notch_spd[1] / 3.0 //1�m�b�` 1/3�����@�{�@1�m�b�`�C���`���O����
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

//### 1Step�p�^�[�� �@   #########################################################
int CAnalyst::cal_move_1Step(int motion_id, LPST_MOTION_UNIT target, int mode) {

	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	target->n_step = 0;//�X�e�b�v��������

	switch (motion_id) {
	case MOTION_ID_BH: {
		//�U��~�߈ړ������Z�b�g
		pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 0;//�ړ������s��

		cal_as_gain(AS_BH_ID,AS_PTN_1STEP);//�U��~�߃Q�C���v�Z

		if (pMode->antisway_control_n & AS_MOVE_ANTISWAY) {//�U��~�ߗL��
			target->n_step = 4;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = pMode->antisway_ptn_n;

			//Step 1�@�ʑ��҂�
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[0]._t = pIO_Table->physics.T*2.0;// _t�@�^�C���I�[�o�[2����
				//phase1 �}�C�i�X�����p�ʑ�
				target->motions[0].phase1 = pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID];
				//phase1 �v���X�����p�ʑ�
				target->motions[0].phase2 = -DEF_PI + pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID];// high phase
				target->motions[0]._v = 0.0;
				target->motions[0].opt_i1 = AS_PTN_1STEP;//�p�^�[���o�͎�����p
			}
			//Step 2�@����
			{
				target->motions[1].type = CTR_TYPE_ACC_AS;
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[1]._v = g_spec.bh_acc[FWD_ACC] * target->motions[1]._t;			// _v

			}
			//Step 3�@����
			{
				target->motions[2].type = CTR_TYPE_DEC_V;			
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p			
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[2]._v = 0.0;													// _v
			}
			//Step 4�@�����~�ҋ@
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

		//�������[�h�ł́A���񂪏I���܂Ŏ��̃p�^�[���ɓ���Ȃ��l�ɂ���@����v����
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

		//�U��~�߈ړ������Z�b�g
		pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 0;//�ړ������s��
		cal_as_gain(AS_SLEW_ID, AS_PTN_1STEP);//�U��~�߃Q�C���v�Z

		if (pMode->antisway_control_t &  AS_MOVE_ANTISWAY) {
			target->n_step = 4;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = pMode->antisway_ptn_t;

			//Step 1�@�ʑ��҂�
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[0]._t = pIO_Table->physics.T*2.0;								// _t�@�^�C���I�[�o�[2����
				target->motions[0].phase1 = pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID];		// low phase
				target->motions[0].phase2 = -DEF_PI+pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID];// high phase
				target->motions[0]._v = 0.0;
				target->motions[0].opt_i1 = AS_PTN_1STEP;//�p�^�[���o�͎�����p
			}
			//Step 2�@����
			{
				target->motions[1].type = CTR_TYPE_ACC_AS;
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[1]._v = g_spec.slew_acc[FWD_ACC] * target->motions[1]._t;
			}
			//Step 3�@����
			{
				target->motions[2].type = CTR_TYPE_DEC_V;
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p			
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[2]._v = 0.0;													// _v
			}
			//Step 4�@�����~�ҋ@
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
		//��������
	}break;
	default: return  ERR_NO_CASE_EXIST;
	}

	return NO_ERR_EXIST;
};

//### 2Step+-�p�^�[���@  #########################################################
int CAnalyst::cal_move_2Step_pn(int motion_id, LPST_MOTION_UNIT target, int mode) {

	double adjust_t_pos, adjust_t_sway;
	int adjust_count_pos, adjust_count_sway, adjust_dir;
	
	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	target->n_step = 0;//�X�e�b�v��������

	switch (motion_id) {
	case MOTION_ID_BH: {
		int large_sway_flag;
		if (pIO_Table->physics.sway_amp_n_ph < pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID]) {
			large_sway_flag = OFF;
		}
		else {
			large_sway_flag = ON;
		}

		//�U��~�߈ړ������Z�b�g
		if (large_sway_flag == ON) {
			if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] > 0) pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_PLUS;
			else pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_MINUS;
		}
		else {
			pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = 0;
		}
		pIO_Table->auto_ctrl.as_start_ph[AS_BH_ID] = 0;	//�U��~�ߊJ�n�ʑ������t���O�N���A
		cal_as_gain(AS_BH_ID, AS_PTN_2STEP_PN);//�U��~�߃Q�C���v�Z

	//�ڕW�ʒu�܂ł̈ʒu���킹�␳����,�@�ڕW�ʒu�܂ł̋����ɉ����Ă����ꂩ�̃X�e�b�v�̉������Ԃ𑝂₷
		adjust_t_pos = sqrt(pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID] * pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID]
			  + pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID] / g_spec.bh_acc[FWD_ACC]); 
		adjust_t_pos -= pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];
		adjust_count_pos = (int)(adjust_t_pos * 1000) / (int)play_scan_ms;

	//�␳���Ԃ�K�p����ړ������𔻒肷��
		if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] > 0.0) {
			adjust_dir = AS_DIR_PLUS;//�}�C�i�X���������炷
		}
		else {
			adjust_dir = AS_DIR_MINUS;
		}

	//�����U��U���ɂ���~�␳����,�@�U��U���ɉ����Ē�~���Ԃ̑���������
		double R = 2.0 * pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID] * (1.0 - cos(pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID]));
		double dph = pIO_Table->physics.sway_amp_n_ph / R;
		adjust_t_sway = dph / pIO_Table->physics.w0;
		adjust_count_sway = (int)(adjust_t_sway * 1000) / (int)play_scan_ms;

		if (pMode->antisway_control_n & AS_MOVE_ANTISWAY) {//�U��~�ߗL��
			target->n_step = 7;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0;								//Initialize activated pattern
			target->motion_type = AS_PTN_2STEP_PN;

			//Step 1�@�ʑ��҂�
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[0]._t = pIO_Table->physics.T*2.0;								// _t�@�^�C���I�[�o�[2����
				target->motions[0].phase1 = 0;													// low phase
				target->motions[0].phase2 = DEF_PI;												// high phase
				target->motions[0]._v = 0.0;
				target->motions[0].opt_i1 = AS_PTN_2STEP_PN;//�p�^�[���o�͎�����p
			}
			//Step 2�@����
			{
				target->motions[1].type = CTR_TYPE_ACC_AS_2PN;
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[1]._v = g_spec.bh_acc[FWD_ACC] * target->motions[1]._t;			// _v
				target->motions[1].opt_i1 = adjust_count_pos;	//�ʒu���킹�p�␳�^�C�}�[�J�E���g
				target->motions[1].opt_i2 = adjust_dir;			//�␳�^�C�}�[�K�p�ړ�����
			}
			//Step 3�@����
			{
				target->motions[2].type = CTR_TYPE_DEC_AS_2PN;
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p			
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[2]._v = 0.0;													// _v
				target->motions[2].opt_i1 = adjust_count_pos;	//�ʒu���킹�p�␳�^�C�}�[�J�E���g
				target->motions[2].opt_i2 = adjust_dir;			//�␳�^�C�}�[�K�p�ړ�����
			}
			//Step 4�@�ʑ��҂��ҋ@
			{
				target->motions[3].type = CTR_TYPE_TIME_WAIT_2PN;
				target->motions[3]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				if (large_sway_flag) {
					target->motions[3]._t = 2.0*(DEF_PI / pIO_Table->physics.w0 - pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID]);	// _t
					target->motions[3].opt_i1 = adjust_count_sway;	//�U��~�ߗp�␳�^�C�}�[�J�E���g
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

					if (phx < 0.0) { //�}�C�i�X�����́A�����𒴂��Ă���̂ŕϊ��K�v
						phx = DEF_PI + phx;
					}
					target->motions[3]._t = DEF_PI / pIO_Table->physics.w0;	// _t
					target->motions[3].opt_i1 = (int)(phx / pIO_Table->physics.w0 * 1000) / (int)play_scan_ms;	//�U��~�ߗp�␳�^�C�}�[�J�E���g
					target->motions[3].opt_i2 = AS_INIT_SWAY_SMALL;
				}
				target->motions[3]._v = 0.0;													// _v
			}
			//Step 5�@����
			{
				target->motions[4].type = CTR_TYPE_ACC_AS_2PN;
				target->motions[4]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[4]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[4]._v = g_spec.bh_acc[FWD_ACC] * target->motions[1]._t;			// _v
				target->motions[4].opt_i1 = adjust_count_pos;	//�ʒu���킹�p�␳�^�C�}�[�J�E���g
				target->motions[4].opt_i2 = adjust_dir;			//�␳�^�C�}�[�K�p�ړ�����
			}
			//Step 6�@����
			{
				target->motions[5].type = CTR_TYPE_DEC_AS_2PN;
				target->motions[5]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p			
				target->motions[5]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[5]._v = 0.0;													// _v
				target->motions[5].opt_i1 = adjust_count_pos;	//�ʒu���킹�p�␳�^�C�}�[�J�E���g
				target->motions[5].opt_i2 = adjust_dir;			//�␳�^�C�}�[�K�p�ړ�����
			}
			//Step 7�@�����~�ҋ@
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

		//�U��~�߈ړ������Z�b�g
		if (large_sway_flag == ON) {
			if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] > 0) pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_PLUS;
			else pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_MINUS;
		}
		else {
			pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = 0;
		}
		//�U��~�߈ړ������Z�b�g
		pIO_Table->auto_ctrl.as_start_ph[AS_SLEW_ID] = 0;//�U��~�ߊJ�n�ʑ������t���O�N���A
		cal_as_gain(AS_SLEW_ID, AS_PTN_2STEP_PN);//�U��~�߃Q�C���v�Z

		//�ڕW�ʒu�܂ł̈ʒu���킹�␳����,�@�ڕW�ʒu�܂ł̋����ɉ����Ă����ꂩ�̃X�e�b�v�̉������Ԃ����炷
		adjust_t_pos = sqrt(pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID] * pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID]
			+ pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID] / g_spec.slew_acc[FWD_ACC]);
		adjust_t_pos -= pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];

		adjust_count_pos = (int)(adjust_t_pos * 1000) / (int)play_scan_ms;

		//�␳���Ԃ�K�p����ړ������𔻒肷��
		if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] > 0.0) {
			adjust_dir = AS_DIR_PLUS;//�v���X�����𑝂₷
		}
		else {
			adjust_dir = AS_DIR_MINUS;
		}

		//�����U��U���ɂ���~�␳����,�@�U��U���ɉ����Ē�~���Ԃ̑���������
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

			//Step 1�@�ʑ��҂�
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_slew;// _p
				target->motions[0]._t = pIO_Table->physics.T*2.0;		// _t�@�^�C���I�[�o�[2����
				target->motions[0].phase1 = 0;							// low phase
				target->motions[0].phase2 = DEF_PI;						// high phase
				target->motions[0]._v = 0.0;
				target->motions[0].opt_i1 = AS_PTN_2STEP_PN;//�p�^�[���o�͎�����p
			}
			//Step 2�@����
			{
				target->motions[1].type = CTR_TYPE_ACC_AS_2PN;
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[1]._v = g_spec.slew_acc[FWD_ACC] * target->motions[1]._t;			// _v
				target->motions[1].opt_i1 = adjust_count_pos;	//�ʒu���킹�p�␳�^�C�}�[�J�E���g
				target->motions[1].opt_i2 = adjust_dir;			//�␳�^�C�}�[�K�p�ړ�����
			}
			//Step 3�@����
			{
				target->motions[2].type = CTR_TYPE_DEC_AS_2PN;
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p			
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[2]._v = 0.0;													// _v
				target->motions[2].opt_i1 = adjust_count_pos;	//�ʒu���킹�p�␳�^�C�}�[�J�E���g
				target->motions[2].opt_i2 = adjust_dir;			//�␳�^�C�}�[�K�p�ړ�����
			}
			//Step 4�@�����~�ҋ@
			{
				target->motions[3].type = CTR_TYPE_TIME_WAIT_2PN;
				target->motions[3]._p = pIO_Table->auto_ctrl.tgpos_bh;		// _p
				if (large_sway_flag) {
					target->motions[3]._t = 2.0*(DEF_PI / pIO_Table->physics.w0 - pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID]);									// _t
					target->motions[3].opt_i1 = adjust_count_sway;	//�U��~�ߗp�␳�^�C�}�[�J�E���g
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

					if (phx < 0.0) { //�}�C�i�X�����́A�����𒴂��Ă���̂ŕϊ��K�v
						phx = DEF_PI + phx;
					}
					target->motions[3]._t = DEF_PI / pIO_Table->physics.w0;	// _t
					target->motions[3].opt_i1 = (int)(phx / pIO_Table->physics.w0 * 1000) / (int)play_scan_ms;	//�U��~�ߗp�␳�^�C�}�[�J�E���g
					target->motions[3].opt_i2 = AS_INIT_SWAY_SMALL;
				}
				target->motions[3]._v = 0.0;													// _v
			}
			//Step 5�@����
			{
				target->motions[4].type = CTR_TYPE_ACC_AS_2PN;
				target->motions[4]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[4]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[4]._v = g_spec.slew_acc[FWD_ACC] * target->motions[4]._t;			// _v
				target->motions[4].opt_i1 = adjust_count_pos;	//�ʒu���킹�p�␳�^�C�}�[�J�E���g
				target->motions[4].opt_i2 = adjust_dir;			//�␳�^�C�}�[�K�p�ړ�����
			}
			//Step 6�@����
			{
				target->motions[5].type = CTR_TYPE_DEC_AS_2PN;
				target->motions[5]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p			
				target->motions[5]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[5]._v = 0.0;													// _v
				target->motions[5].opt_i1 = adjust_count_pos;	//�ʒu���킹�p�␳�^�C�}�[�J�E���g
				target->motions[5].opt_i2 = adjust_dir;			//�␳�^�C�}�[�K�p�ړ�����
			}
			//Step 7�@�����~�ҋ@
			{
				target->motions[6].type = CTR_TYPE_TIME_WAIT;
				target->motions[6]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[6]._t = PTN_CONFIRMATION_TIME*10;									// _t
				target->motions[6]._v = 0.0;													// _v
				target->motions[6].opt_i1 = 0;	//�U��~�ߗp�␳�^�C�}�[�J�E���g
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

			//Step 1 �ʑ��҂�
			{
				target->n_step = 1;
				step_count = 0;
				target->motions[step_count].type = CTR_TYPE_SINGLE_PHASE_WAIT;
				target->motions[step_count]._p = pIO_Table->auto_ctrl.tgpos_h;	// _p
				target->motions[step_count]._t = pIO_Table->physics.T*2.0;		// _t �^�C���I�[�o�[��2�����Ƃ���
				target->motions[step_count].phase1 = t0 * pIO_Table->physics.w0;		// low phase
				target->motions[step_count].phase2 = target->motions[step_count].phase1;// high phase
				target->motions[step_count]._v = 0.0;
			}
			//Step 2 ����
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
			//Step 3 ��~
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 4.0 - t0;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count - 1]._v*target->motions[step_count - 1]._v / g_spec.hoist_acc[FWD_ACC] / 2.0;
			}
			//Step 4 ����
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = tx;
				target->motions[step_count]._v = v1;
				target->motions[step_count]._p = pIO_Table->auto_ctrl.tgpos_h
					+ target->motions[step_count]._v*target->motions[step_count]._v / g_spec.hoist_acc[FWD_ACC] / 2.0;
			}
			//Step 5 ��~
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = t1;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = pIO_Table->auto_ctrl.tgpos_h;
			}
			//Step end ��
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

//### 2Step++�p�^�[���@  #########################################################
int CAnalyst::cal_move_2Step_pp(int motion_id, LPST_MOTION_UNIT target, int mode) {

	double adjust_t_sway;
	int adjust_count_sway;

	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;
	target->n_step = 0;

	switch (motion_id) {
	case MOTION_ID_BH: {

		//�U��~�߈ړ������Z�b�g
		if (pIO_Table->auto_ctrl.tgD[AS_BH_ID] > 0) {
			pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_PLUS;
		}
		else {
			pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] = AS_DIR_MINUS;
		}

		pIO_Table->auto_ctrl.as_start_ph[AS_BH_ID] = 0;	//�U��~�ߊJ�n�ʑ������t���O�N���A
		cal_as_gain(AS_BH_ID, AS_PTN_2STEP_PP);//�U��~�߃Q�C���v�Z

		//�����U��U���ɂ���~�␳����,�@�U��U���ɉ����Ē�~���Ԃ̑���������
		double R = 2.0 * pIO_Table->auto_ctrl.phase_acc_offset[AS_BH_ID] * (1.0 - cos(pIO_Table->auto_ctrl.as_gain_ph[AS_BH_ID]));
		double dph;
		if (R < 0.0001) { //0����h�~
			dph = 0.0;
		}
		else {
			dph = pIO_Table->physics.sway_amp_n_ph / R;
			if (dph > DEF_QPI) {//����ݒ�i�傫���U��͑Ή��s�j
				dph = DEF_QPI;
			}
		}
		adjust_t_sway = dph / pIO_Table->physics.w0;
		adjust_count_sway = (int)(adjust_t_sway * 1000) / (int)play_scan_ms;

		if (pMode->antisway_control_n & AS_MOVE_ANTISWAY) {//�U��~�ߗL��
			target->n_step = 7;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0;								//Initialize activated pattern
			target->motion_type = AS_PTN_2STEP_PP;

			//Step 1�@�ʑ��҂�
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[0]._t = pIO_Table->physics.T*2.0;								// _t�@�^�C���I�[�o�[2����
				target->motions[0].phase1 = 0;													// low phase
				target->motions[0].phase2 = DEF_PI;												// high phase
				target->motions[0]._v = 0.0;
				target->motions[0].opt_i1 = AS_PTN_2STEP_PP;//�p�^�[���o�͎�����p
			}
			//Step 2�@����
			{
				target->motions[1].type = CTR_TYPE_ACC_AS;
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[1]._v = g_spec.bh_acc[FWD_ACC] * target->motions[1]._t;			// _v
			}
			//Step 3�@����
			{
				target->motions[2].type = CTR_TYPE_DEC_V;
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p			
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[2]._v = 0.0;													// _v
			}
			//Step 4�@�ʑ��҂��ҋ@
			{
				target->motions[3].type = CTR_TYPE_TIME_WAIT_2PP;
				target->motions[3]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[3]._t = DEF_PI/pIO_Table->physics.w0 - 2.0* pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];									// _t
					target->motions[3]._v = 0.0;													// _v
				target->motions[3].opt_i1 = adjust_count_sway;	//�U��~�ߗp�␳�^�C�}�[�J�E���g
		//		target->motions[3].opt_i1 = 0;	//�U��~�ߗp�␳�^�C�}�[�J�E���g
			}
			//Step 5�@����
			{
				target->motions[4].type = CTR_TYPE_ACC_AS;
				target->motions[4]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p
				target->motions[4]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[4]._v = g_spec.bh_acc[FWD_ACC] * target->motions[1]._t;			// _v
			}
			//Step 6�@����
			{
				target->motions[5].type = CTR_TYPE_DEC_V;
				target->motions[5]._p = pIO_Table->auto_ctrl.tgpos_bh;							// _p			
				target->motions[5]._t = pIO_Table->auto_ctrl.as_gain_time[AS_BH_ID];			// _t
				target->motions[5]._v = 0.0;													// _v
			}
			//Step 7�@�����~�ҋ@
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
		//�U��~�߈ړ������Z�b�g
		if (pIO_Table->auto_ctrl.tgD[AS_SLEW_ID] > 0) pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_PLUS;
		else pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] = AS_DIR_MINUS;

		pIO_Table->auto_ctrl.as_start_ph[AS_SLEW_ID] = 0;//�U��~�ߊJ�n�ʑ������t���O�N���A

		cal_as_gain(AS_SLEW_ID, AS_PTN_2STEP_PP);//�U��~�߃Q�C���v�Z

		//�����U��U���ɂ���~�␳����,�@�U��U���ɉ����Ē�~���Ԃ̑���������
		double R = 2.0 * pIO_Table->auto_ctrl.phase_acc_offset[AS_SLEW_ID] * (1.0 - cos(pIO_Table->auto_ctrl.as_gain_ph[AS_SLEW_ID]));
		double dph;
		if (R < 0.0001) { //0����h�~
			dph = 0.0;
		}
		else {
			dph = pIO_Table->physics.sway_amp_t_ph / R;
			if (dph > DEF_QPI) {//����ݒ�i�傫���U��͑Ή��s�j
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

			//Step 1�@�ʑ��҂�
			{
				target->motions[0].type = CTR_TYPE_DOUBLE_PHASE_WAIT;
				target->motions[0]._p = pIO_Table->auto_ctrl.tgpos_slew;// _p
				target->motions[0]._t = pIO_Table->physics.T*2.0;		// _t�@�^�C���I�[�o�[2����
				target->motions[0].phase1 = 0;							// low phase
				target->motions[0].phase2 = DEF_PI;						// high phase
				target->motions[0]._v = 0.0;
				target->motions[0].opt_i1 = AS_PTN_2STEP_PP;//�p�^�[���o�͎�����p
			}
			//Step 2�@����
			{
				target->motions[1].type = CTR_TYPE_ACC_AS;
				target->motions[1]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[1]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[1]._v = g_spec.slew_acc[FWD_ACC] * target->motions[1]._t;			// _v
			}
			//Step 3�@����
			{
				target->motions[2].type = CTR_TYPE_DEC_V;
				target->motions[2]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p			
				target->motions[2]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[2]._v = 0.0;													// _v
			}
			//Step 4�@�����~�ҋ@
			{
				target->motions[3].type = CTR_TYPE_TIME_WAIT_2PP;
				target->motions[3]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[3]._t = DEF_PI / pIO_Table->physics.w0 - 2.0* pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];									// _t
				target->motions[3]._v = 0.0;													// _v
				target->motions[3].opt_i1 = adjust_count_sway;	//�U��~�ߗp�␳�^�C�}�[�J�E���g
			}
			//Step 5�@����
			{
				target->motions[4].type = CTR_TYPE_ACC_AS;
				target->motions[4]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[4]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[4]._v = g_spec.slew_acc[FWD_ACC] * target->motions[1]._t;			// _v
			}
			//Step 6�@����
			{
				target->motions[5].type = CTR_TYPE_DEC_V;
				target->motions[5]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p			
				target->motions[5]._t = pIO_Table->auto_ctrl.as_gain_time[AS_SLEW_ID];			// _t
				target->motions[5]._v = 0.0;													// _v
			}
			//Step 7�@�����~�ҋ@
			{
				target->motions[6].type = CTR_TYPE_TIME_WAIT;
				target->motions[6]._p = pIO_Table->auto_ctrl.tgpos_slew;						// _p
				target->motions[6]._t = PTN_CONFIRMATION_TIME;									// _t
				target->motions[6]._v = 0.0;													// _v
				target->motions[6].opt_i1 = 0;	//�U��~�ߗp�␳�^�C�}�[�J�E���g
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
		//��������
	}break;
	}
	return NO_ERR_EXIST;
};

//### ��`�p�^�[��       #########################################################
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

		Da = pIO_Table->auto_ctrl.tgD_abs[AS_BH_ID];//�ڕW�ړ�����
		acc = g_spec.bh_acc[FWD_ACC];

		dx = pIO_Table->physics.T * g_spec.bh_notch_spd[1] //1�m�b�`������@�{�@�U��~��1��̈ړ�����
			+ g_spec.bh_acc[FWD_ACC] * pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID] * pIO_Table->auto_ctrl.as_gain_damp[AS_BH_ID];

		if (Da < dx) return CAL_RESULT_NEGATIVE;//�p�^�[���쐬�s��

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

		if ((n == 0) || (Da < 0.0)) {//�p�^�[���s�v
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

			//Step 1 ��`�p�^�[����
			{
				target->motions[0].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = Da / g_spec.bh_notch_spd[n];
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[n];
				target->motions[step_count]._p = target->motions[0]._p = pIO_Table->physics.R +
					pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * (Da - target->motions[step_count]._v * target->motions[step_count]._v / g_spec.bh_acc[FWD_ACC] / 2.0);
			}
			//Step 2�` ��`�����p�^�[����
			{
				step_count += 1;
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = g_spec.bh_notch_spd[n] / g_spec.bh_acc[FWD_ACC];
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[0]._p = pIO_Table->physics.R + pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * Da;
			}
			//Step end ��
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

			//Step 1 ��{��`�p�^�[����
			{
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[n];
				target->motions[step_count]._p = pIO_Table->physics.R
					+ target->motions[step_count]._v * target->motions[step_count]._t
					- pIO_Table->auto_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count]._v * target->motions[step_count]._v / g_spec.bh_acc[FWD_ACC] / 2.0;
			}

			//Step 2�` ��`�����p�^�[����
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

			//Step end ��
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

		Da = pIO_Table->auto_ctrl.tgD_abs[AS_SLEW_ID];//�ڕW�ړ�����
		acc = g_spec.slew_acc[FWD_ACC];

		dx = pIO_Table->physics.T * g_spec.slew_notch_spd[1]
			+ acc * pIO_Table->auto_ctrl.as_gain_damp[AS_SLEW_ID] * pIO_Table->auto_ctrl.as_gain_damp[AS_SLEW_ID];

		if (Da < dx) return CAL_RESULT_NEGATIVE;//�p�^�[���쐬�s��

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


		if ((n == 0) || (Da <= 0.0)) {//�p�^�[���s�v
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

			//Step 1 ��`�p�^�[����
			{
				target->motions[0].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = Da / g_spec.slew_notch_spd[n];
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];
				target->motions[step_count]._p = target->motions[0]._p = pIO_Table->physics.th +
					pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * (Da - target->motions[step_count]._v * target->motions[step_count]._v / g_spec.slew_acc[FWD_ACC] / 2.0);
			}
			//Step 2�` ��`�����p�^�[����
			{
				step_count += 1;
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = g_spec.slew_notch_spd[n] / g_spec.slew_acc[FWD_ACC];
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[0]._p = target->motions[0]._p = pIO_Table->physics.th + pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * Da;
			}
			//Step end ��
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

			//Step 1 ��{��`�p�^�[����
			{
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T;
				target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];
				target->motions[step_count]._p = pIO_Table->physics.th
					+ target->motions[step_count]._v * target->motions[step_count]._t
					- pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count]._v * target->motions[step_count]._v / g_spec.slew_acc[FWD_ACC] / 2.0;
			}
			//Step 2�` ��`�����p�^�[����
			{
				for (; n > 0; n--) {

					if (ptn_notch_freq[AS_SLEW_ID][n] == 0) continue;

					step_count += 1;
					target->n_step += 1;

					target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
					target->motions[step_count]._v = pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];

					double  temp_d = (target->motions[step_count - 1]._v - target->motions[step_count]._v) / g_spec.slew_acc[FWD_ACC];
					if (temp_d < 0.0)temp_d *= -1.0;//���m�b�`���x�܂ł̌�������
					target->motions[step_count]._t = (double)ptn_notch_freq[AS_SLEW_ID][n] * pIO_Table->physics.T + temp_d;

					temp_d = 0.5*g_spec.slew_acc[FWD_ACC] * temp_d * temp_d;
					target->motions[step_count]._p = target->motions[step_count - 1]._p
						+ target->motions[step_count]._v * target->motions[step_count]._t
						- pIO_Table->auto_ctrl.as_out_dir[AS_SLEW_ID] * temp_d;
				}
			}
			//Step end ��
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

//### 2�i�������p�^�[���@#########################################################
int CAnalyst::cal_move_2accdec(int motion_id, LPST_MOTION_UNIT target, int mode) {

	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	target->n_step = 0;

	LPAS_NOTCH_SET p_as_notch_spd = &as_notch_spd;

	switch (motion_id) {
	case MOTION_ID_BH: {
	
		//�i�s�����Z�b�g�i�J�n�ʑ����ʗp�j
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

			//Step 1 �葬1�i��
			{
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_2nd[AS_BH_ID] + p_as_notch_spd->t_const_2nd[AS_BH_ID];
				target->motions[step_count]._v = dir * p_as_notch_spd ->v_2nd[AS_BH_ID];
				target->motions[step_count]._p = pIO_Table->physics.R
					+ target->motions[step_count]._v * p_as_notch_spd->t_acc_2nd[AS_BH_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_2nd[AS_BH_ID];
			}

			//Step 2 �葬2�i��
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
			//Step 3 �葬3�i��
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
			//Step 4 �葬4�i�� = ��~
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_2nd[AS_BH_ID];
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count - 1]._v * target->motions[step_count]._t / 2.0;
			}
			//Step end ��
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

			if (mode == AUTO_PTN_MODE_AUTOMOVE) {//�����ł͐���ʒu�҂�������@��ŗv����
				target->motions[target->n_step].type = CTR_TYPE_SLEW_WAIT;
				target->motions[target->n_step]._p = pIO_Table->auto_ctrl.tgpos_bh;
				target->motions[target->n_step]._t = PTN_ERROR_CHECK_TIME1;
				target->motions[target->n_step].time_count = (int)(target->motions[target->n_step]._t * 1000) / (int)play_scan_ms;
				target->motions[target->n_step]._v = 0.0;
				target->n_step += 1;
			}

		}
		else{//�p�^�[���s��
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

		//�i�s�����Z�b�g�i�J�n�ʑ����ʗp�j
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

			//Step 1 �葬1�i��
			{
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_2nd[AS_SLEW_ID] + p_as_notch_spd->t_const_2nd[AS_SLEW_ID];
				target->motions[step_count]._v = dir * p_as_notch_spd->v_2nd[AS_SLEW_ID];
				target->motions[step_count]._p = pIO_Table->physics.th
					+ target->motions[step_count]._v * p_as_notch_spd->t_acc_2nd[AS_SLEW_ID] / 2.0
					+ target->motions[step_count]._v * p_as_notch_spd->t_const_2nd[AS_SLEW_ID];
			}

			//Step 2 �葬2�i��
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
			//Step 3 �葬3�i��
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
			//Step 4 �葬4�i�� = ��~
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = p_as_notch_spd->t_acc_2nd[AS_SLEW_ID];
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ target->motions[step_count - 1]._v * target->motions[step_count]._t / 2.0;
			}
			//Step end ��
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

//### 3Step�ړ��p�^�[���@#########################################################
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

			//Step 1 ����
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

			//Step 2�@��~
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

			//Step 3�@�葬
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
			//Step 4�@��~
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
			//Step 5 ����
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

			//Step 6�@��~
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

		if (mode == AUTO_PTN_MODE_AUTOMOVE) {//�����ł͐���҂�������@��ŗv����
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

			//Step 1 ����
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

			//Step 2�@��~
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

			//Step 3�@�葬
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
			//Step 4�@��~
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
			//Step 5 ����
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

			//Step 6�@��~
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

//### �ړ��U��~�߃p�^�[���̃m�b�`�g�ݍ��킹�v�Z�@�����@###########################
// �߂�l�FTop�m�b�`,�@�p�^�[�������� 0
int CAnalyst::cal_notch_set(LPAS_NOTCH_SET notch_set, double Da, int mode, int type) {

	notch_set->i_notch_top[type] = 0; notch_set->i_notch_2nd[type] = 0;
	notch_set->v_top[type] = 0; notch_set->v_2nd[type] = 0;

	if (mode == AS_PTN_2ACCDEC) { //2�i�������p�^�[�� ���ʂ͂P�p�^�[���Œ�
		int n = 0;
		double tn,Dmin,t_const_temp;

		switch (type) {
		case AS_BH_ID:
			notch_set->i_notch_top[type] = 5; notch_set->i_notch_2nd[type] = 3;				//�m�b�`�̔z��ʒu�Z�b�g
			notch_set->v_top[type] = g_spec.bh_notch_spd[notch_set->i_notch_top[type]];		//�g�b�v�X�s�[�h
			notch_set->v_2nd[type] = g_spec.bh_notch_spd[notch_set->i_notch_2nd[type]];		//���ԃX�s�[�h
			notch_set->t_acc_2nd[type] = notch_set->v_2nd[type] / g_spec.bh_acc[FWD_ACC];	//��i�ڂ܂ł̉�������
			tn = g_spec.bh_notch_spd[5] / g_spec.bh_acc[FWD_ACC];
			notch_set->t_acc_top[type] = tn - notch_set->t_acc_2nd[type];					//��i�ڂ����i�܂ł̉�������

			if (tn <= pIO_Table->physics.T) t_const_temp = pIO_Table->physics.T - tn;		//�������̒葬���ԁi��������T�ȉ��j
			else							t_const_temp = 3 * pIO_Table->physics.T - tn;	//�������̒葬���ԁi��������T�ȏ�j

			Dmin = notch_set->v_top[type] * notch_set->v_top[type] / g_spec.bh_acc[FWD_ACC]	//TOP���x0�b�ł̈ړ�����
				+ t_const_temp * notch_set->v_2nd[type];
			notch_set->t_const_2nd[type] = t_const_temp / 2;
			notch_set->t_const_d[type] = (Da - Dmin) / notch_set->v_top[type];

			if (notch_set->t_const_d[type] > 0.0) {
				n = 5;
			}
			else {//TOP�m�b�`�ړ������s��

				notch_set->i_notch_top[type] = 4; notch_set->i_notch_2nd[type] = 2;				//�m�b�`�̔z��ʒu�Z�b�g
				notch_set->v_top[type] = g_spec.bh_notch_spd[notch_set->i_notch_top[type]];		//�g�b�v�X�s�[�h
				notch_set->v_2nd[type] = g_spec.bh_notch_spd[notch_set->i_notch_2nd[type]];		//���ԃX�s�[�h
				notch_set->t_acc_2nd[type] = notch_set->v_2nd[type] / g_spec.bh_acc[FWD_ACC];	//��i�ڂ܂ł̉�������
				tn = g_spec.bh_notch_spd[4] / g_spec.bh_acc[FWD_ACC];
				notch_set->t_acc_top[type] = tn - notch_set->t_acc_2nd[type];					//��i�ڂ����i�܂ł̉�������

				if (tn <= pIO_Table->physics.T) t_const_temp = pIO_Table->physics.T - tn;		//�������̒葬���ԁi��������T�ȉ��j
				else							t_const_temp = 3 * pIO_Table->physics.T - tn;	//�������̒葬���ԁi��������T�ȏ�j

				Dmin = notch_set->v_top[type] * notch_set->v_top[type] / g_spec.bh_acc[FWD_ACC]	//TOP���x0�b�ł̈ړ�����
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

	//		if (notch_set->t_const_d[type] < 0.0) n = 0;//�ړ������s��
	//		else n = 5;

			return n;
		default:
			return 0;
			break;
		}
	}
	else if (mode == AS_PTN_3STEP) { //3�i�p�^�[��
		int n,k,i_notch_max,i_notch;
		double base_phi, base_t, tn ,temp_d, temp_t, acc_phi,temp_double;

		switch (type) {
		case AS_BH_ID: {
			//Step1 �ړ���������ő�m�b�`��ݒ�
			for (i_notch_max = NOTCH_MAX - 1; i_notch_max > 0; i_notch_max--) {
				//�ݒ�m�b�`�ł̍ŏ��ړ�����
				temp_double = 3.0 * g_spec.bh_notch_spd[i_notch_max] * g_spec.bh_notch_spd[i_notch_max] / g_spec.bh_acc[FWD_ACC];
				if (Da > temp_double)break;
			}
			if (i_notch_max < 1) return 0;//�ړ������s��

			//Step2 ��̈ʑ���ݒ�(�ŏ��l�����߂�j
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
					if (temp_double > acc_phi) {//�������Ԃ̈ʑ��ω�����ݒ��菬����
						base_phi = temp_double;//��l�X�V
						break;
					}
				}
				if (base_phi > acc_phi) {//�������Ԃ̈ʑ��ω�����ݒ���傫��
					break;
				}
			}
			//Step3 �m�b�`�̌���
			base_t = base_phi / pIO_Table->physics.w0;//��ʑ��̎��Ԋ��Z
			for (i_notch = i_notch_max; i_notch > 0; i_notch--) {
				temp_d = 3.0 * g_spec.bh_notch_spd[i_notch] * g_spec.bh_notch_spd[i_notch] / g_spec.bh_acc[FWD_ACC];
				temp_d += 2.0 *  g_spec.bh_notch_spd[i_notch] * (base_t - (g_spec.bh_notch_spd[i_notch] / g_spec.bh_acc[FWD_ACC]));

				if (temp_d < Da) {
					break;
				}
			}
			if (i_notch < 1) {
				return 0;//�K���m�b�`����
			}
			else {
				tn = g_spec.bh_notch_spd[i_notch] / g_spec.bh_acc[FWD_ACC];
				notch_set->i_notch_top[type] = notch_set->i_notch_2nd[type] = i_notch;
				notch_set->v_top[type] = notch_set->v_2nd[type] = g_spec.bh_notch_spd[i_notch];
				notch_set->t_acc_top[type] = notch_set->t_acc_2nd[type] = tn;
				notch_set->t_const_2nd[type] = base_t - tn;
				notch_set->t_const_d[type] = (Da - temp_d) / notch_set->v_top[type];
				return i_notch; //�m�b�`�I������
			}
		}break;
		case AS_SLEW_ID: {
			//Step1 �ړ���������ő�m�b�`��ݒ�
			for (i_notch_max = NOTCH_MAX - 1; i_notch_max > 0; i_notch_max--) {
				//�ݒ�m�b�`�ł̍ŏ��ړ�����
				temp_double = 3.0 * g_spec.slew_notch_spd[i_notch_max] * g_spec.slew_notch_spd[i_notch_max] / g_spec.slew_acc[FWD_ACC];
				if (Da > temp_double)break;
			}
			if (i_notch_max < 1) return 0;//�ړ������s��

			//Step2 ��̈ʑ���ݒ�(�ŏ��l�����߂�j
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
					if (temp_double > acc_phi) {//�������Ԃ̈ʑ��ω�����ݒ��菬����
						base_phi = temp_double;//��l�X�V
						break;
					}
				}
				if (base_phi > acc_phi) {//�������Ԃ̈ʑ��ω�����ݒ���傫��
					break;
				}
			}
			//Step3 �m�b�`�̌���
			base_t = base_phi / pIO_Table->physics.w0;//��ʑ��̎��Ԋ��Z
			for (i_notch = i_notch_max; i_notch > 0; i_notch--) {
				temp_d = 3.0 * g_spec.slew_notch_spd[i_notch] * g_spec.slew_notch_spd[i_notch] / g_spec.slew_acc[FWD_ACC];
				temp_d += 2.0 *  g_spec.slew_notch_spd[i_notch] * (base_t - (g_spec.slew_notch_spd[i_notch] / g_spec.slew_acc[FWD_ACC]));

				if (temp_d < Da) {
					break;
				}
			}
			if (i_notch < 1) {
				return 0;//�K���m�b�`����
			}
			else {
				tn = g_spec.slew_notch_spd[i_notch] / g_spec.slew_acc[FWD_ACC];
				notch_set->i_notch_top[type] = notch_set->i_notch_2nd[type] = i_notch;
				notch_set->v_top[type] = notch_set->v_2nd[type] = g_spec.slew_notch_spd[i_notch];
				notch_set->t_acc_top[type] = notch_set->t_acc_2nd[type] = tn;
				notch_set->t_const_2nd[type] = base_t - tn;
				notch_set->t_const_d[type] = (Da - temp_d) / notch_set->v_top[type];
				return i_notch; //�m�b�`�I������
			}
		}break;
		default:
			return 0;//�Y���p�^�[��ID����
			break;
		}
	}
	else ;
	return 0xff;//�Y���p�^�[��ID����
}