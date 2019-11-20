#include "stdafx.h"
#include "CAnalyst.h"
#include "SharedObjects.h"
#include "CPlayer.h"

extern CORDER_Table*	pOrder;				//���L������Order�N���X�|�C���^
extern CMODE_Table*		pMode;				//���L������Mode�N���X�|�C���^
extern ST_SPEC			g_spec;				//�N���[���d�l
extern CIO_Table*		pIO_Table;


CAnalyst::CAnalyst(){
	for (int i = 0; i < NUM_OF_AS; i++)	pIO_Table->as_ctrl.phase_chk_range[i] = DEF_PI / 50.0;
}

CAnalyst::~CAnalyst(){
}


void CAnalyst::cal_simulation() {

	//##�����x�w�ߒl�v�Z  acc_cyl x:r y:th z:z

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

	//##MOB�v�Z
	//�ݓ_�̓���
	hp.timeEvolution(0.0);  //double t  �ݓ_�̌v�Z�ɂ͌o�ߎ��Ԃ͎g��Ȃ��̂�0.0

	//## Calicurate the motion of hung load
	hl.timeEvolution(0.0); //double t  �ݓ_�̌v�Z�ɂ͌o�ߎ��Ԃ͎g��Ȃ��̂�0.0
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
	pIO_Table->physics.lph = asin(temp_r / pIO_Table->physics.L);

	//###Z���p�x�̈ʑ�����  x:Theata y:TheataDot/Omega
	pIO_Table->physics.PhPlane_r.x = temp_r / pIO_Table->physics.L;
	if (pIO_Table->physics.PhPlane_r.x < DEF_001DEG)pIO_Table->physics.PhPlane_r.x = DEF_001DEG;

	pIO_Table->physics.PhPlane_r.y = temp_v / (pIO_Table->physics.L * pIO_Table->physics.w0);
	pIO_Table->physics.PhPlane_r.z = atan(pIO_Table->physics.PhPlane_r.y / pIO_Table->physics.PhPlane_r.x);

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

	//###xy���ʔ��a�����̈ʑ�����  x:Theata y:TheataDot/Omega�@z:Phi
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

	//###xy���ʐڐ������̈ʑ�����  x:Theata y:TheataDot/Omega�@z:Phi
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


	//###�e�ʑ����ʂ̉��������̉�]���SOFFSET�l

	pIO_Table->as_ctrl.phase_acc_offset[AS_SLEW_ID] = g_spec.slew_acc[FWD_ACC] * pIO_Table->physics.R / DEF_G;	//Offset of center of phase plane on acceleration
	pIO_Table->as_ctrl.phase_dec_offset[AS_SLEW_ID] = g_spec.slew_acc[FWD_DEC] * pIO_Table->physics.R / DEF_G;	//Offset of center of phase plane on deceleration
	pIO_Table->as_ctrl.phase_acc_offset[AS_BH_ID] = g_spec.bh_acc[FWD_ACC] / DEF_G;								//Offset of center of phase plane on acceleration
	pIO_Table->as_ctrl.phase_dec_offset[AS_BH_ID] = g_spec.bh_acc[FWD_DEC] / DEF_G;								//Offset of center of phase plane on deceleration

};

void CAnalyst::init_task(void *pobj) {
	set_panel_tip_txt();

	Vector3 _r(0.0, DEFAULT_BH, DEFAULT_HP_Z);
	Vector3 _v(0.0, 0.0, 0.0);

	hp.init_mob((double)inf.cycle_ms / 1000.0, _r, _v);

	_r.z -= DEFAULT_ROPE_L;
	hl.init_mob((double)inf.cycle_ms / 1000.0, _r, _v);

	hl.pHP = &hp; //�ݓ_�ƕR�t��

	pIO_Table->as_ctrl.allowable_pos_overshoot_plus[AS_BH_ID]= 1.0;		//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s����
	pIO_Table->as_ctrl.allowable_pos_overshoot_plus[AS_SLEW_ID] = 1.0;	//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s����
	pIO_Table->as_ctrl.allowable_pos_overshoot_plus[AS_MH_ID] = 1.0;	//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s����

	pIO_Table->as_ctrl.allowable_pos_overshoot_minus[AS_BH_ID] = 1.0;	//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s����
	pIO_Table->as_ctrl.allowable_pos_overshoot_minus[AS_SLEW_ID] = 1.0;	//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s����
	pIO_Table->as_ctrl.allowable_pos_overshoot_minus[AS_MH_ID] = 1.0;	//�U�~�ڕW�ʒu�I�[�o�[���e�l�@�i�s����
	
	return;
};

void CAnalyst::routine_work(void *param) {
	Vector3 rel_lp = hl.r - hp.r;		//�݉ב���xyz
	ws << L" working!" << *(inf.psys_counter) % 100 << "  AS ptn n ;" << pMode->antisway_ptn_n << ": AS ctr n" << pMode->antisway_control_n << "__   AS ptn t ;" << pMode->antisway_ptn_t << ": AS ctr t" << pMode->antisway_control_t;
	tweet2owner(ws.str()); ws.str(L""); ws.clear();

	//�V�~�����[�^�v�Z
	if (pMode->auto_control != ENV_MODE_SIM2) cal_simulation();

	cal_as_target();
	update_as_ctrl();

};
//##########################
LPST_JOB_ORDER CAnalyst::cal_job_recipe(int job_type) {
	return &(pOrder->job_A);
};
//##########################
void CAnalyst::cal_as_target() {
	if (pMode->antisway != OPE_MODE_AS_ON) {
		pIO_Table->as_ctrl.tgpos_bh = pIO_Table->physics.R;
		pIO_Table->as_ctrl.tgpos_slew = pIO_Table->physics.th;	//����ڕW�ʒu�͐���p�x
	}
	else{
		if(pMode->antisway_control_n == AS_MOVE_INTERRUPT)	pIO_Table->as_ctrl.tgpos_bh = pIO_Table->physics.R;
		if (pMode->antisway_control_t == AS_MOVE_INTERRUPT)	pIO_Table->as_ctrl.tgpos_slew = pIO_Table->physics.th;
	}

	pIO_Table->as_ctrl.tgD[AS_BH_ID] = pIO_Table->as_ctrl.tgpos_bh - pIO_Table->physics.R;
	pIO_Table->as_ctrl.tgD_abs[AS_BH_ID] = abs(pIO_Table->as_ctrl.tgD[AS_BH_ID]);
	pIO_Table->as_ctrl.tgD[AS_SLEW_ID] = pIO_Table->as_ctrl.tgpos_slew - pIO_Table->physics.th;
	pIO_Table->as_ctrl.tgD_abs[AS_SLEW_ID] = abs(pIO_Table->as_ctrl.tgD[AS_SLEW_ID]);

	return;
};

//#########################################################################
int CAnalyst::cal_as_recipe(int motion_id, LPST_MOTION_UNIT target, int mode) {

	cal_as_gain();
	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] = 0;
	pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] = 0;
	pIO_Table->as_ctrl.as_out_dir[AS_MH_ID] = 0;

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
				target->motions[0]._p = pIO_Table->as_ctrl.tgpos_bh;
				// _t
				target->motions[0]._t = pIO_Table->physics.T*2.0;
				// low phase
				double start_offset = pIO_Table->as_ctrl.as_gain_damp[AS_BH_ID] * pIO_Table->physics.w0;
				target->motions[0].phase1 = start_offset;//DEF_PI * 0.25;
				// high phase
				target->motions[0].phase2 = -DEF_PI + start_offset; //* 0.75;

				target->motions[0]._v = 0.0;
			}
			//Step 2
			{
				target->motions[1].type = CTR_TYPE_ACC_AS;
				// _p
				target->motions[1]._p = pIO_Table->as_ctrl.tgpos_bh;
				// _t
				if(pMode->antisway_ptn_n == AS_PTN_DMP)
					target->motions[1]._t = pIO_Table->as_ctrl.as_gain_damp[AS_BH_ID];
				else
					target->motions[1]._t = pIO_Table->as_ctrl.as_gain_pos[AS_BH_ID];
				// _v
				target->motions[1]._v = g_spec.bh_acc[FWD_ACC] * target->motions[1]._t;
			}
			//Step 3
			{
				target->motions[2].type = CTR_TYPE_DEC_V;
				// _p
				target->motions[2]._p = pIO_Table->as_ctrl.tgpos_bh;
				// _t
				target->motions[2]._t = pIO_Table->as_ctrl.as_gain_damp[AS_BH_ID];
				target->motions[2]._v = 0.0;
			}
			//Step 4
			{
				target->motions[3].type = CTR_TYPE_TIME_WAIT;
				// _p
				target->motions[3]._p = pIO_Table->as_ctrl.tgpos_bh;
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
			target->motions[0]._p = pIO_Table->as_ctrl.tgpos_bh;
			// _t
			target->motions[0]._t = PTN_CONFIRMATION_TIME;
			target->motions[0]._v = 0.0;
			for (int i = 0; i < 1; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}
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
					target->motions[0]._p = pIO_Table->as_ctrl.tgpos_slew;
					// _t
					target->motions[0]._t = pIO_Table->physics.T*2.0;

					double start_offset = pIO_Table->as_ctrl.as_gain_damp[AS_SLEW_ID] * pIO_Table->physics.w0;
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
					target->motions[1]._p = pIO_Table->as_ctrl.tgpos_slew;
					// _t
					if (pMode->antisway_ptn_t == AS_PTN_DMP)
						target->motions[1]._t = pIO_Table->as_ctrl.as_gain_damp[AS_SLEW_ID];
					else
						target->motions[1]._t = pIO_Table->as_ctrl.as_gain_pos[AS_SLEW_ID];
	
					// _v
					target->motions[1]._v = g_spec.slew_acc[FWD_ACC] * target->motions[1]._t;
				}
				//Step 3
				{
					target->motions[2].type = CTR_TYPE_DEC_V;
					// _p
					target->motions[2]._p = pIO_Table->as_ctrl.tgpos_slew;
					// _t
					target->motions[2]._t = pIO_Table->as_ctrl.as_gain_damp[AS_SLEW_ID];
					// _v
					target->motions[2]._v = pIO_Table->physics.wth;
				}
				//Step 4
				{
					target->motions[3].type = CTR_TYPE_TIME_WAIT;
					// _p
					target->motions[3]._p = pIO_Table->as_ctrl.tgpos_slew;
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
				target->motions[0]._p = pIO_Table->as_ctrl.tgpos_slew;
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
	default: return 1;
	}
	return NO_ERR_EXIST;
};
//#########################################################################
int CAnalyst::cal_long_move_recipe(int motion_id, LPST_MOTION_UNIT target, int mode) {

	cal_as_gain();
	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] = 0;
	pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] = 0;
	pIO_Table->as_ctrl.as_out_dir[AS_MH_ID] = 0;

	target->n_step = 0;

	switch (motion_id) {
	case MOTION_ID_BH: {

		if(pIO_Table->as_ctrl.tgD[AS_BH_ID] > 0.0) pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] = 1;
		else if(pIO_Table->as_ctrl.tgD[AS_BH_ID] < 0.0)pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] = -1;
		else;

		//�ڕW�ړ�����Da
		double Da = pIO_Table->as_ctrl.tgD_abs[AS_BH_ID];
		int n = 0;

		if(Da >= 0.0) n = cal_move_pattern_bh(ptn_notch_freq[AS_BH_ID], Da, AS_PTN_MOVE_LONG);

		if ((n== 0)||(Da < 0.0)) {//�p�^�[���s�v
			target->n_step = 1;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern

			//Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;
			// _p
			target->motions[0]._p = pIO_Table->as_ctrl.tgpos_bh;
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
			target->motion_type = AS_PTN_MOVE_LONG;

			//Step 1 ��{��`�p�^�[����
			{
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T;
				target->motions[step_count]._v = pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[n];
				target->motions[step_count]._p =   pIO_Table->physics.R 
					                             + target->motions[step_count]._v * target->motions[step_count]._t
												 - pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count]._v * target->motions[step_count]._v / g_spec.bh_acc[FWD_ACC] / 2.0;
			}

			//Step 2�` ��`�����p�^�[����
			{
				for (; n > 0; n--) {
					
					if (ptn_notch_freq[AS_BH_ID][n] == 0) continue;

					step_count += 1;
					target->n_step += 1;

					target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
					target->motions[step_count]._v = pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[n];

					double  temp_d = (target->motions[step_count - 1]._v - target->motions[step_count]._v) / g_spec.bh_acc[FWD_ACC];
					if (temp_d < 0.0)temp_d *= -1.0;
					target->motions[step_count]._t = (double)ptn_notch_freq[AS_BH_ID][n] * pIO_Table->physics.T + temp_d;

					temp_d = 0.5*g_spec.bh_acc[FWD_ACC] *temp_d * temp_d;
					target->motions[step_count]._p = target->motions[step_count-1]._p
													+ target->motions[step_count]._v * target->motions[step_count]._t
													- pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * temp_d;
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
				target->motions[step_count]._p = target->motions[step_count - 1]._p	- pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * temp_d;
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}

		}
	}break;

	case MOTION_ID_SLEW: {

		if (pIO_Table->as_ctrl.tgD[AS_SLEW_ID] > 0.0) pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] = 1;
		else if (pIO_Table->as_ctrl.tgD[AS_SLEW_ID] < 0.0)pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] = -1;
		else;

		//�ڕW�ړ�����Da

		double Da = pIO_Table->as_ctrl.tgD_abs[AS_SLEW_ID];
		int n = 0;

		if (Da >= 0.0) n = cal_move_pattern_slew(ptn_notch_freq[AS_SLEW_ID], Da, AS_PTN_MOVE_LONG);

		if ((n == 0) || (Da < 0.0)) {//�p�^�[���s�v
			target->n_step = 1;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern

			 //Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;
			target->motions[0]._p = pIO_Table->as_ctrl.tgpos_slew;	// _p
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
			target->motion_type = AS_PTN_MOVE_LONG;

			//Step 1 ��{��`�p�^�[����
			{
				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T;
				target->motions[step_count]._v = pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];
				target->motions[step_count]._p = pIO_Table->physics.th
					+ target->motions[step_count]._v * target->motions[step_count]._t
					- pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count]._v * target->motions[step_count]._v / g_spec.slew_acc[FWD_ACC] / 2.0;
			}
			//Step 2�` ��`�����p�^�[����
			{
				for (; n > 0; n--) {

					if (ptn_notch_freq[AS_SLEW_ID][n] == 0) continue;

					step_count += 1;
					target->n_step += 1;

					target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
					target->motions[step_count]._v = pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];

					double  temp_d = (target->motions[step_count - 1]._v - target->motions[step_count]._v) / g_spec.slew_acc[FWD_ACC];
					if (temp_d < 0.0)temp_d *= -1.0;//���m�b�`���x�܂ł̌�������
					target->motions[step_count]._t = (double)ptn_notch_freq[AS_SLEW_ID][n] * pIO_Table->physics.T + temp_d;

					temp_d = 0.5*g_spec.slew_acc[FWD_ACC] * temp_d * temp_d;
					target->motions[step_count]._p = target->motions[step_count - 1]._p
						+ target->motions[step_count]._v * target->motions[step_count]._t
						- pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * temp_d;
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
				target->motions[step_count]._p = target->motions[step_count - 1]._p - pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * temp_d;
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
	return NO_ERR_EXIST;
};
//#########################################################################
int CAnalyst::cal_short_move_recipe(int motion_id, LPST_MOTION_UNIT target, int mode) {

	cal_as_gain();
	CPlayer* pPly = (CPlayer*)VectpCTaskObj[g_itask.ply];
	unsigned int play_scan_ms = pPly->inf.cycle_ms;

	pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] = 0;
	pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] = 0;
	pIO_Table->as_ctrl.as_out_dir[AS_MH_ID] = 0;

	target->n_step = 0;

	switch (motion_id) {
	case MOTION_ID_BH: {

		if (pIO_Table->as_ctrl.tgD[AS_BH_ID] > 0.0) pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] = 1;
		else if (pIO_Table->as_ctrl.tgD[AS_BH_ID] < 0.0)pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] = -1;
		else;

		//�ڕW�ړ�����Da
		double Da = pIO_Table->as_ctrl.tgD_abs[AS_BH_ID];
		int n = 0;

		if (Da >= 0.0) n = cal_move_pattern_bh(ptn_notch_freq[AS_BH_ID], Da, AS_PTN_MOVE_SHORT);

		if ((n == 0) || (Da < 0.0)) {//�p�^�[���s�v
			target->n_step = 1;
			target->axis_type = BH_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_0;

			//Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;
			target->motions[0]._p = pIO_Table->as_ctrl.tgpos_bh;// _p
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

			//Step 1 ����
			{
				step_count = 0;
				target->n_step = 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[n];
				target->motions[step_count]._p = pIO_Table->physics.R
					+ pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count]._v * (target->motions[step_count]._t - target->motions[step_count]._v / g_spec.bh_acc[FWD_ACC] / 2.0);
			}

			//Step 2�@��~
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count - 1]._v * target->motions[step_count - 1]._v / g_spec.bh_acc[FWD_ACC] / 2.0;
			}

			//Step 3�@�葬
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._v = pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[n];
				target->motions[step_count]._t = Da / g_spec.bh_notch_spd[n] - pIO_Table->physics.T / 3.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count]._v * (target->motions[step_count]._t - target->motions[step_count]._v / g_spec.bh_acc[FWD_ACC] / 2.0);
			}
			//Step 4�@��~
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count - 1]._v * target->motions[step_count - 1]._v / g_spec.bh_acc[FWD_ACC] / 2.0;
			}
			//Step 5 ����
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * g_spec.bh_notch_spd[n];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count]._v * (target->motions[step_count]._t - target->motions[step_count]._v / g_spec.bh_acc[FWD_ACC] / 2.0);
			}

			//Step 6�@��~
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = target->motions[step_count - 1]._v / g_spec.bh_acc[FWD_ACC] + PTN_CONFIRMATION_TIME;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->as_ctrl.as_out_dir[AS_BH_ID] * target->motions[step_count - 1]._v * target->motions[step_count - 1]._v / g_spec.bh_acc[FWD_ACC] / 2.0;
			}

			//time_count
			for (int i = 0; i < target->n_step; i++) {
				target->motions[i].act_counter = 0;
				target->motions[i].time_count = (int)(target->motions[i]._t * 1000) / (int)play_scan_ms;
			}

		}
	}break;

	case MOTION_ID_SLEW: {

		if (pIO_Table->as_ctrl.tgD[AS_SLEW_ID] > 0.0) pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] = 1;
		else if (pIO_Table->as_ctrl.tgD[AS_SLEW_ID] < 0.0)pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] = -1;
		else;

		//�ڕW�ړ�����Da

		double Da = pIO_Table->as_ctrl.tgD_abs[AS_SLEW_ID];
		int n = 0;

		if (Da >= 0.0) n = cal_move_pattern_slew(ptn_notch_freq[AS_SLEW_ID], Da, AS_PTN_MOVE_SHORT);

		if ((n == 0) || (Da < 0.0)) {//�p�^�[���s�v
			target->n_step = 1;
			target->axis_type = SLW_AXIS;
			target->ptn_status = PTN_STANDBY;
			target->iAct = 0; //Initialize activated pattern
			target->motion_type = AS_PTN_0;
			//Step 1
			target->motions[0].type = CTR_TYPE_TIME_WAIT;
			target->motions[0]._p = pIO_Table->as_ctrl.tgpos_slew;	// _p
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

			//Step 1 ����
			{
				step_count = 0;
				target->n_step = 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];
				target->motions[step_count]._p = pIO_Table->physics.th
					+ pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count]._v * (target->motions[step_count]._t - target->motions[step_count]._v / g_spec.slew_acc[FWD_ACC] / 2.0);
			}
			//Step 2�@��~
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count - 1]._v * target->motions[step_count - 1]._v / g_spec.slew_acc[FWD_ACC] / 2.0;
			}

			//Step 3�@�葬
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._v = pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];
				target->motions[step_count]._t = Da / g_spec.slew_notch_spd[n] - pIO_Table->physics.T / 3.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count]._v * (target->motions[step_count]._t - target->motions[step_count]._v / g_spec.slew_acc[FWD_ACC] / 2.0);
			}
			//Step 4�@��~
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count - 1]._v * target->motions[step_count - 1]._v / g_spec.slew_acc[FWD_ACC] / 2.0;
			}
			//Step 5 ����
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = pIO_Table->physics.T / 6.0;
				target->motions[step_count]._v = pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * g_spec.slew_notch_spd[n];
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count]._v * (target->motions[step_count]._t - target->motions[step_count]._v / g_spec.slew_acc[FWD_ACC] / 2.0);
			}

			//Step 6�@��~
			{
				step_count += 1;
				target->n_step += 1;

				target->motions[step_count].type = CTR_TYPE_CONST_V_TIME;
				target->motions[step_count]._t = target->motions[step_count - 1]._v / g_spec.bh_acc[FWD_ACC] + PTN_CONFIRMATION_TIME;
				target->motions[step_count]._v = 0.0;
				target->motions[step_count]._p = target->motions[step_count - 1]._p
					+ pIO_Table->as_ctrl.as_out_dir[AS_SLEW_ID] * target->motions[step_count - 1]._v * target->motions[step_count - 1]._v / g_spec.slew_acc[FWD_ACC] / 2.0;
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
//### Update Anti-sway Control Mode################################################
void CAnalyst::update_as_ctrl() {

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
	else if (pIO_Table->physics.vR != 0.0){	//�U�~���쒆
		pMode->antisway_ptn_n = AS_PTN_0;
		pMode->antisway_control_n = AS_MOVE_STANDBY;
	}
	else if ((pIO_Table->physics.sway_amp_n_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) &&			//�U�ꂪ�������x��
				(pIO_Table->as_ctrl.tgD_abs[AS_BH_ID] < g_spec.as_compl_nposLv[I_AS_LV_COMPLE])){	//�ʒu���������x��
		pMode->antisway_ptn_n = AS_PTN_0;
		pMode->antisway_control_n = AS_MOVE_COMPLETE;
	}
	else if (pIO_Table->as_ctrl.tgD_abs[AS_BH_ID] < g_spec.as_compl_nposLv[I_AS_LV_TRIGGER]) {		//�ڕW���ʒu���ߋN�����苗����
		if (pIO_Table->physics.sway_amp_n_ph > g_spec.as_compl_swayLv[I_AS_LV_TRIGGER]){			//�U�ꂪ�g���K����l�ȏ�
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
	else if (pIO_Table->as_ctrl.tgD_abs[AS_BH_ID] > g_spec.as_compl_nposLv[I_AS_LV_POSITION]) {
		pMode->antisway_control_n = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_n = AS_PTN_POS;
	}
	else if (pIO_Table->physics.sway_amp_n_ph > g_spec.as_compl_swayLv[I_AS_LV_DAMPING]) {			//�U�ꂪ�_���s���O����l�ȏ�
		pMode->antisway_control_n = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_n = AS_PTN_DMP;
	}
	else if (pIO_Table->as_ctrl.tgD_abs[AS_BH_ID] > g_spec.as_compl_nposLv[I_AS_LV_TRIGGER]) {
		pMode->antisway_control_n = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_n = AS_PTN_POS;
	}
	else;

	//pMode->antisway_ptn_n = AS_PTN_DMP;

		//##### Tangent direction

	if (pIO_Table->ref.b_slew_manual_ctrl) {
		pMode->antisway_ptn_t = AS_PTN_0;
		pMode->antisway_control_t = AS_MOVE_INTERRUPT;
	}
	else if (pIO_Table->physics.wth != 0.0) {
		pMode->antisway_ptn_t = AS_PTN_0;
		pMode->antisway_control_t = AS_MOVE_STANDBY;
	}
	else if ((pIO_Table->physics.sway_amp_t_ph < g_spec.as_compl_swayLv[I_AS_LV_COMPLE]) &&		//�ڕW���ʒu���ߊ������苗����
		(pIO_Table->as_ctrl.tgD_abs[AS_SLEW_ID] < g_spec.as_compl_tposLv[I_AS_LV_COMPLE])) {	//�U�ꂪ���������
		pMode->antisway_ptn_t = AS_PTN_0;
		pMode->antisway_control_t = AS_MOVE_COMPLETE;
	}
	else if (pIO_Table->as_ctrl.tgD_abs[AS_SLEW_ID] < g_spec.as_compl_tposLv[I_AS_LV_TRIGGER]) {	//�ڕW���ʒu���ߋN�����苗����
		if (pIO_Table->physics.sway_amp_t_ph > g_spec.as_compl_swayLv[I_AS_LV_TRIGGER]) {	//�U�ꂪ�g���K����l�ȏ�
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
	else if (pIO_Table->as_ctrl.tgD_abs[AS_SLEW_ID] > g_spec.as_compl_tposLv[I_AS_LV_POSITION]) {
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_t = AS_PTN_POS;
	}
	else if (pIO_Table->physics.sway_amp_t_ph > g_spec.as_compl_swayLv[I_AS_LV_DAMPING]) { //�U�ꂪ�_���s���O����l�ȏ�
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_t = AS_PTN_DMP;
	}
	else if (pIO_Table->as_ctrl.tgD_abs[AS_SLEW_ID] > g_spec.as_compl_tposLv[I_AS_LV_TRIGGER]) {
		pMode->antisway_control_t = AS_MOVE_ANTISWAY;
		pMode->antisway_ptn_t= AS_PTN_POS;
	}
	else;
	return;
};
//################################################################################
void CAnalyst::cal_as_gain() {//�U��~�߃Q�C������������
	//### Damping Mode�Q�C���v�Z
	//#### ��������
	double temp_angle;

	if (pIO_Table->as_ctrl.phase_acc_offset[AS_BH_ID] < pIO_Table->physics.sway_amp_n_ph) {
		temp_angle = DEF_HPI*0.8;
	}
	else {
		temp_angle = DEF_HPI*0.8 * pIO_Table->physics.sway_amp_n_ph/ pIO_Table->as_ctrl.phase_acc_offset[AS_BH_ID];
		if (temp_angle < DEF_PI / 10.0) temp_angle = DEF_PI / 10.0;
	}
	
	pIO_Table->as_ctrl.as_gain_damp[AS_BH_ID] = temp_angle / pIO_Table->physics.w0;	//Gain = time


	//####�������

	if (pIO_Table->as_ctrl.phase_acc_offset[AS_SLEW_ID] < pIO_Table->physics.sway_amp_t_ph) {
		temp_angle = DEF_HPI * 0.8;
	}
	else {
		temp_angle = DEF_HPI*0.8 * pIO_Table->physics.sway_amp_t_ph / pIO_Table->as_ctrl.phase_acc_offset[AS_SLEW_ID];
		//�����ݒ�
		if (temp_angle < DEF_PI / 4.0) temp_angle = DEF_PI / 4.0 * pIO_Table->physics.R / 40.0;
	}
	pIO_Table->as_ctrl.as_gain_damp[AS_SLEW_ID] = temp_angle / pIO_Table->physics.w0;

	//### Positioning Mode�Q�C���v�Z
	//�ړ���������������Ԍv�Z
	pIO_Table->as_ctrl.as_gain_pos[AS_BH_ID] = sqrt(pIO_Table->as_ctrl.tgD_abs[AS_BH_ID] / g_spec.bh_acc[FWD_ACC]);
	pIO_Table->as_ctrl.as_gain_pos[AS_SLEW_ID] = sqrt(pIO_Table->as_ctrl.tgD_abs[AS_SLEW_ID] / g_spec.slew_acc[FWD_ACC]);
	
	//Positioning Mode�Őݒ�
	if ((pIO_Table->as_ctrl.as_gain_pos[AS_BH_ID] > pIO_Table->as_ctrl.as_gain_damp[AS_BH_ID]) && //�ʒu�ړ��ʂɔ�אU�ꂪ������
		(pIO_Table->as_ctrl.tgD_abs[AS_BH_ID] < 0.2)){											  //�ړ��ʂ�������
		pIO_Table->as_ctrl.as_gain_pos[AS_BH_ID] = pIO_Table->as_ctrl.as_gain_damp[AS_BH_ID];
	}
	if ((pIO_Table->as_ctrl.as_gain_pos[AS_SLEW_ID] > pIO_Table->as_ctrl.as_gain_damp[AS_SLEW_ID]) && 	//�ʒu�ړ��ʂɔ�אU�ꂪ������
		(pIO_Table->as_ctrl.tgD_abs[AS_SLEW_ID] < 0.005)) {												//�������߂�
		pIO_Table->as_ctrl.as_gain_pos[AS_SLEW_ID] = pIO_Table->as_ctrl.as_gain_damp[AS_SLEW_ID];
	}
	//Positioning Mode�ŏ���ݒ�
	if (pIO_Table->as_ctrl.as_gain_pos[AS_BH_ID] > DEF_HPI / pIO_Table->physics.w0 * 0.5) {
		pIO_Table->as_ctrl.as_gain_pos[AS_BH_ID] = pIO_Table->as_ctrl.as_gain_pos[AS_BH_ID]*0.5;
	}

	if (pIO_Table->as_ctrl.tgD_abs[AS_SLEW_ID] < 0.1) {
//		pIO_Table->as_ctrl.as_gain_pos[AS_SLEW_ID] = pIO_Table->as_ctrl.as_gain_pos[AS_SLEW_ID] * 0.5;
	}
	if (pIO_Table->as_ctrl.as_gain_pos[AS_SLEW_ID] > DEF_HPI / pIO_Table->physics.w0) {
		pIO_Table->as_ctrl.as_gain_pos[AS_SLEW_ID] = pIO_Table->as_ctrl.as_gain_pos[AS_SLEW_ID]*0.8;
	}

	return;
};
//################################################################################
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
//################################################################################
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


