#include "stdafx.h"
#include "Mob.h"
#include "CommonFormat.h"

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

	

	acc_cyl = Vector3(0.0, 0.0, 0.0);
	acc_cyl_ref = Vector3(0.0, 0.0, 0.0);


	return;
}

Vector3 Mob_HoistPoint::A(double t, Vector3& r, Vector3& v) {

	double a_er = acc_cyl.x - r_bm * w_sl * w_sl;
	double a_eth = r_bm * acc_cyl.y + 2.0 * v_bm * w_sl;
	Vector3 acc_cyl2(a_er,a_eth,acc_cyl_ref.z);

	acc_rec = acc_cyl2.cyl2rec(acc_cyl2,th_sl);
	
	return acc_rec;

} //Model of acceleration

void Mob_HoistPoint::timeEvolution(double t) {

	acc_cyl = (dt*acc_cyl_ref + HP_Tf * acc_cyl) / (dt + HP_Tf); //一次遅れ

	v_bm = v_bm + dt * acc_cyl.x;
	if (v_bm* v_bm < 0.00001)v_bm = 0.0;
	w_sl = w_sl + dt * acc_cyl.y;
	if (w_sl* w_sl < 0.000000001)w_sl = 0.0;

	r_bm += dt * v_bm;
	th_sl += dt * w_sl;
	if (th_sl >= DEF_2PI) th_sl -= DEF_2PI;
	 
	v.x = v_bm * sin(th_sl) + r_bm * w_sl * cos(th_sl);
	v.y = v_bm * cos(th_sl) - r_bm * w_sl * sin(th_sl);

	r.x = r_bm * sin(th_sl);
	r.y = r_bm * cos(th_sl);
	r.z = DEFAULT_HP_Z;

}


//Mob_HLoad
Mob_HungLoad::Mob_HungLoad() {};
Mob_HungLoad::~Mob_HungLoad() {};

void Mob_HungLoad::init_mob(double _dt, Vector3& _r, Vector3& _v) {
	dt = _dt;
	r.copy(_r);
	v.copy(_v);
	m = 1000.0;//1 ton
	return;
}

Vector3 Mob_HungLoad::A(double t, Vector3& r, Vector3& v) {
	Vector3 a;
	Vector3 L_;

	L_ = L_.subVectors(r, pHP->r);

	double Sdivm = S() / m;

	//	a.x = Sdivm * (r.x - r_box.x);
	//	a.y = Sdivm * (r.y - r_box.y);
	//	a.z = -G + Sdivm * (r.z - r_box.z);

	a = L_.clone().multiplyScalor(Sdivm);
	a.z -= GRAVITY_ACC;

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
	return -m * (v_abs2 - pHP->acc_rec.dot(vecL) - GRAVITY_ACC * vecL.z + pHP->v_h*pHP->v_h + pHP->l_h*pHP->a_h) / (pHP->l_h*pHP->l_h);
}


