#include "stdafx.h"
#include "CAnalyst.h"

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

	//MOB�v�Z
	//�ݓ_�̓���
	hp.timeEvolution(0.0);  //double t  �ݓ_�̌v�Z�ɂ͌o�ߎ��Ԃ͎g��Ȃ��̂�0.0

	//�ׂ݉̓���
	hl.timeEvolution(0.0);
	hl.r.add(hl.dr);
	hl.v.add(hl.dv);


	//IO TABLE �Z�b�g
	pIO_Table->physics.cp = hp.r;
	pIO_Table->physics.cv = hp.v;
	pIO_Table->physics.th = hp.th_sl;	//����p�x
	pIO_Table->physics.wth = hp.w_sl;	//����p���x
	pIO_Table->physics.R = hp.r_bm;		//�����ʒu
	pIO_Table->physics.vR = hp.v_bm;	//�������x

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

	hl.pHP = &hp; //�ݓ_�ƕR�t��

	return;
};

void CAnalyst::routine_work(void *param) {

	ws << L" working!" << *(inf.psys_counter) % 100 << " x:y ; " << hp.r.x << ":" << hp.r.y;
	tweet2owner(ws.str()); ws.str(L""); ws.clear();

	//�V�~�����[�^�v�Z
	if (pMode->auto_control != ENV_MODE_SIM2) cal_simulation();

};



