#include "stdafx.h"
#include "Mob.h"
#include "CommonFormat.h"

extern ST_SPEC			g_spec;				//クレーン仕様

//Mob_HP
Mob_HoistPoint::Mob_HoistPoint(){};
Mob_HoistPoint::~Mob_HoistPoint() {};

void Mob_HoistPoint::init_mob(double _dt, Vector3& _r, Vector3& _v) {
	dt = _dt;
	r.copy(_r);
	v.copy(_v);

	l_h = DEFAULT_ROPE_L;	//ロープ長
	v_h = 0.0;				//巻速度;
	a_h = 0.0;				//巻加速度;

	th_sl = DEFAULT_SLEW;	//旋回角度
	w_sl = 0.0;				//旋回角速度
	dw_sl = 0.0;			//旋回角加速度

	r_bm = DEFAULT_BH;		//引込位置
	v_bm = 0.0;				//引込速度
	a_bm = 0.0;				//引込加速度

	

	acc_rec = Vector3(0.0, 0.0, 0.0);
	a_h_ref = 0.0;		//巻加速度指令
	dw_sl_ref = 0.0;	//旋回角速度指令
	a_bm_ref = 0.0;		//旋回角速度指令acc_cyl_ref = Vector3(0.0, 0.0, 0.0);

	return;
}

Vector3 Mob_HoistPoint::A(double t, Vector3& r, Vector3& v) {

	double a_er = a_bm - r_bm * w_sl * w_sl;
	double a_eth = r_bm *  dw_sl + 2.0 * v_bm * w_sl;
	Vector3 acc_cyl(a_er,a_eth,0.0);

	acc_rec = acc_cyl.cyl2rec(acc_cyl,th_sl);
	
	return acc_rec;

} //Model of acceleration

void Mob_HoistPoint::timeEvolution(double t) {

	A(0.0, r, v);//吊点の加速度演算

	a_bm = (dt*a_bm_ref + HP_Tf * a_bm) / (dt + HP_Tf); //一次遅れ
	dw_sl = (dt*dw_sl_ref + HP_Tf * dw_sl) / (dt + HP_Tf); //一次遅れ

	a_h	= (dt*a_h_ref + HP_Tf * a_h) / (dt + HP_Tf); //一次遅れ

	v_bm = v_bm + dt * a_bm;
	if (v_bm* v_bm < 0.00001)v_bm = 0.0;

	w_sl = w_sl + dt * dw_sl;
	if (dw_sl_ref*dw_sl_ref < 0.00000000001 && w_sl* w_sl < 0.00002)w_sl = 0.0;

	v_h = v_h + dt * a_h;
	if (v_h* v_h < 0.00001)v_h = 0.0;


	r_bm += dt * v_bm;

	th_sl += dt * w_sl;
	if (th_sl >= DEF_2PI) th_sl -= DEF_2PI;
	if (th_sl <= -DEF_2PI) th_sl += DEF_2PI;

	l_h += dt * v_h;
	 
	v.x = v_bm * sin(th_sl) + r_bm * w_sl * cos(th_sl);
	v.y = v_bm * cos(th_sl) - r_bm * w_sl * sin(th_sl);
	v.z = 0.0;

	r.x = r_bm * sin(th_sl);
	r.y = r_bm * cos(th_sl);
	r.z = g_spec.boom_height;

}


//Mob_HLoad
Mob_HungLoad::Mob_HungLoad() {};
Mob_HungLoad::~Mob_HungLoad() {};

void Mob_HungLoad::init_mob(double _dt, Vector3& _r, Vector3& _v) {
	dt = _dt;
	r.copy(_r);
	v.copy(_v);
	m = 10000.0;//10 ton
	//m = 40000.0;//40 ton
	return;
}

Vector3 Mob_HungLoad::A(double t, Vector3& r, Vector3& v) {
	Vector3 a;
	Vector3 L_;

	L_ = L_.subVectors(r, pHP->r);

	double Sdivm = S() / m;

	a = L_.clone().multiplyScalor(Sdivm);
	a.z -= DEF_G;

	//計算誤差によるロープ長ずれ補正
	Vector3 hatL = L_.clone().normalize();
	// 補正ばね弾性力
	Vector3 ak = hatL.clone().multiplyScalor(-compensationK * (pHP->l_h - L_.length()));
	Vector3 v_ = v_.subVectors(v, pHP->v);
	// 補正粘性抵抗力
	Vector3 agamma = hatL.clone().multiplyScalor(-compensationGamma * v_.dot(hatL));
	// 張力にひもの長さの補正力を加える

	a.add(ak).add(agamma);

	return a;
} //Model of acceleration

double  Mob_HungLoad::S() {
	Vector3 v_ = v.clone().sub(pHP->v);
	double v_abs2 = v_.lengthSq();
	Vector3 vectmp;
	Vector3 vecL = vectmp.subVectors(r, pHP->r);

	return -m * (v_abs2 - pHP->acc_rec.dot(vecL) - GRAVITY_ACC * vecL.z - (pHP->a_h * pHP->l_h + pHP->v_h*pHP->v_h)) / (pHP->l_h*pHP->l_h);

}


