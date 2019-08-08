#include "stdafx.h"
#include "Mob.h"

//Mob_HP
Mob_HoistPoint::Mob_HoistPoint(){};
Mob_HoistPoint::~Mob_HoistPoint() {};

void Mob_HoistPoint::init_mob(double _dt, Vector3& _r, Vector3& _v) {
	dt = _dt;
	r.copy(_r);
	v.copy(_v);

	l_h = DEFAULT_ROPE_L;	//ƒ[ƒv’·
	v_h = 0.0;				//Šª‘¬“x;
	a_h = 0.0;				//Šª‰Á‘¬“x;

	th_sl = DEFAULT_SLEW;	//ù‰ñŠp“x
	w_sl = 0.0;				//ù‰ñŠp‘¬“x
	dw_sl = 0.0;			//ù‰ñŠp‰Á‘¬“x

	r_bm = DEFAULT_BH;		//ˆøžˆÊ’u
	v_bm = 0.0;				//ù‰ñŠp‘¬“x
	a_bm = 0.0;				//ù‰ñŠp‰Á‘¬“x

	return;
}

Vector3 Mob_HoistPoint::A(double t, Vector3& r, Vector3& v) {

	acc = (dt*acc_ref + HP_Tf * acc) / (dt + HP_Tf); //ˆêŽŸ’x‚ê

	return acc;
} //Model of acceleration

void Mob_HoistPoint::timeEvolution(double t) {

	Vector3 v1 = V(t, r, v);
	Vector3 a1 = A(t, r, v);

	Vector3 _v1 = Vector3(r.x + v1.x*dt / 2.0, r.y + v1.y*dt / 2.0, r.z + v1.z*dt / 2.0);
	Vector3 _a1 = Vector3(v.x + a1.x*dt / 2.0, v.y + a1.y*dt / 2.0, v.z + a1.z*dt / 2.0);
	Vector3 v2 = V(t + dt / 2.0, _v1, _a1);
	Vector3 a2 = A(t + dt / 2.0, _v1, _a1);

	Vector3 _v2 = Vector3(r.x + v2.x*dt / 2.0, r.y + v2.y*dt / 2.0, r.z + v2.z*dt / 2.0);
	Vector3 _a2 = Vector3(v.x + a2.x*dt / 2.0, v.y + a2.y*dt / 2.0, v.z + a2.z*dt / 2.0);
	Vector3 v3 = V(t + dt / 2.0, _v2, _a2);
	Vector3 a3 = A(t + dt / 2.0, _v2, _a2);

	Vector3 _v3 = Vector3(r.x + v3.x*dt, r.y + v3.y*dt, r.z + v3.z*dt);
	Vector3 _a3 = Vector3(v.x + a3.x*dt, v.y + a3.y*dt, v.z + a3.z*dt);
	Vector3 v4 = V(t + dt, _v3, _a3);
	Vector3 a4 = A(t + dt, _v3, _a3);

	dr.x = dt / 6.0 * (v1.x + 2.0 * v2.x + 2.0 * v3.x + v4.x);
	dr.y = dt / 6.0 * (v1.y + 2.0 * v2.y + 2.0 * v3.y + v4.y);
	dr.z = dt / 6.0 * (v1.z + 2.0 * v2.z + 2.0 * v3.z + v4.z);
	dv.x = dt / 6.0 * (a1.x + 2.0 * a2.x + 2.0 * a3.x + a4.x);
	dv.y = dt / 6.0 * (a1.y + 2.0 * a2.y + 2.0 * a3.y + a4.y);
	dv.z = dt / 6.0 * (a1.z + 2.0 * a2.z + 2.0 * a3.z + a4.z);

}


//Mob_HLoad
Mob_HungLoad::Mob_HungLoad() {};
Mob_HungLoad::~Mob_HungLoad() {};

void Mob_HungLoad::init_mob(double _dt, Vector3& _r, Vector3& _v) {
	dt = _dt;
	r.copy(_r);
	v.copy(_v);
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

	//ŒvŽZŒë·‚É‚æ‚éƒ[ƒv’·‚¸‚ê•â³
	Vector3 hatL = L_.clone().normalize();
	// •â³‚Î‚Ë’e«—Í
	Vector3 ak = hatL.clone().multiplyScalor(-compensationK * (pHP->l_h - L_.length()));
	Vector3 v_ = v_.subVectors(v, pHP->v);
	// •â³”S«’ïR—Í
	Vector3 agamma = hatL.clone().multiplyScalor(-compensationGamma * v_.dot(hatL));
	// ’£—Í‚É‚Ð‚à‚Ì’·‚³‚Ì•â³—Í‚ð‰Á‚¦‚é

	a.add(ak).add(agamma);

	return a;
} //Model of acceleration

double  Mob_HungLoad::S() {
	Vector3 v_ = v.clone().sub(pHP->v);
	double v_abs2 = v_.lengthSq();
	Vector3 vectmp;
	Vector3 vecL = vectmp.subVectors(r, pHP->r);
	return -m * (v_abs2 - pHP->acc.dot(vecL) - GRAVITY_ACC * vecL.z + pHP->v_h*pHP->v_h + pHP->l_h*pHP->a_h) / (pHP->l_h*pHP->l_h);
}


