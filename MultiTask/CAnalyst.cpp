#include "stdafx.h"
#include "CAnalyst.h"
#include "SharedObjects.h"

extern CORDER_Table*	pOrder;				//���L������Order�N���X�|�C���^
extern CMODE_Table*		pMode;				//���L������Mode�N���X�|�C���^
extern ST_SPEC			g_spec;				//�N���[���d�l
extern CIO_Table*		pIO_Table;


CAnalyst::CAnalyst(){
}

CAnalyst::~CAnalyst(){
}


void CAnalyst::cal_simulation() {

	//�����x�w�ߒl�v�Z  acc_cyl x:r y:th z:z

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

	//MOB�v�Z
	//�ݓ_�̓���
	hp.timeEvolution(0.0);  //double t  �ݓ_�̌v�Z�ɂ͌o�ߎ��Ԃ͎g��Ȃ��̂�0.0

	//�ׂ݉̓���
	hl.timeEvolution(0.0); //double t  �ݓ_�̌v�Z�ɂ͌o�ߎ��Ԃ͎g��Ȃ��̂�0.0
	hl.r.add(hl.dr);
	hl.v.add(hl.dv);


	//IO TABLE �Z�b�g

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
		
	Vector3 rel_lp = hp.r - hl.r;		//�݉ב���xyz
	Vector3 rel_lvp = hp.v - hl.v;		//�݉ב��Α��xvx vy vz

	
	double last_L = pIO_Table->physics.L;
	//���[�v��
	pIO_Table->physics.L = hp.l_h;	
	pIO_Table->physics.vL = hp.v_h;	//�����x
	
	//�@Z���Ƃ̊p�x
	double last_ph = pIO_Table->physics.lph;
	double temp_f = sqrt(rel_lp.x * rel_lp.x + rel_lp.y * rel_lp.y);
	pIO_Table->physics.lph = asin(temp_f/ pIO_Table->physics.L);
	
	//Z���p�x�̈ʑ�����  x:OmegaTheata y:TheataDot
	pIO_Table->physics.PhPlane_r.x = pIO_Table->physics.lph * pIO_Table->physics.w0;
	pIO_Table->physics.PhPlane_r.y = (pIO_Table->physics.lph - last_ph) / (double)inf.period / 1000.0;
	pIO_Table->physics.PhPlane_r.z = 0.0;


	//  XY���ʊp�x
	double R = pIO_Table->physics.L * sin(pIO_Table->physics.lph);
	if (R < 0.0001) R = 0.0001;

	double last_th = pIO_Table->physics.lth;
	pIO_Table->physics.lth = acos(rel_lp.x);
	
	
	if(pIO_Table->physics.L < 1.0)
		pIO_Table->physics.w0 = sqrt(DEF_G / pIO_Table->physics.L);//�U��p���g��
	pIO_Table->physics.T = DEF_2PI / pIO_Table->physics.w0;			//�U�����

	//xy���ʔ��a�����̈ʑ�����  x:OmegaTheata y:TheataDot
	pIO_Table->physics.PhPlane_n.x = rel_lp.x * cos(-pIO_Table->physics.th) - rel_lp.y *sin(-pIO_Table->physics.th) / pIO_Table->physics.L;
	pIO_Table->physics.PhPlane_n.y = (rel_lvp.x * cos(-pIO_Table->physics.th) - rel_lvp.y *sin(-pIO_Table->physics.th))/ pIO_Table->physics.L;
	pIO_Table->physics.PhPlane_n.z = 0.0;

	//xy���ʐڐ������̈ʑ�����  x:OmegaTheata y:TheataDot
	pIO_Table->physics.PhPlane_t.x = rel_lp.x * sin(-pIO_Table->physics.th) + rel_lp.y *cos(-pIO_Table->physics.th) / pIO_Table->physics.L;;
	pIO_Table->physics.PhPlane_t.y = rel_lvp.x * sin(-pIO_Table->physics.th) - rel_lvp.y *cos(-pIO_Table->physics.th) / pIO_Table->physics.L;;
	pIO_Table->physics.PhPlane_t.z= 0.0;
	
	
};

void CAnalyst::init_task(void *pobj) {
	set_panel_tip_txt();

	Vector3 _r(0.0, DEFAULT_BH, DEFAULT_HP_Z);
	Vector3 _v(0.0, 0.0, 0.0);

	hp.init_mob((double)inf.cycle_ms / 1000.0, _r, _v);

	_r.z -= DEFAULT_ROPE_L;
	hl.init_mob((double)inf.cycle_ms / 1000.0, _r, _v);

	hl.pHP = &hp; //�ݓ_�ƕR�t��

	return;
};

void CAnalyst::routine_work(void *param) {

	ws << L" working!" << *(inf.psys_counter) % 100 << " x:y ; " << hp.r.x << ":" << hp.r.y;
	tweet2owner(ws.str()); ws.str(L""); ws.clear();

	//�V�~�����[�^�v�Z
	if (pMode->auto_control != ENV_MODE_SIM2) cal_simulation();

};

LPST_JOB_ORDER CAnalyst::cal_job_recipe(int job_type) {
	return &(pOrder->job_A);
};


