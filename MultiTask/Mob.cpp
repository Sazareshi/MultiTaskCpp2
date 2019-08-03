#include "stdafx.h"
#include "Mob.h"

//Mob_HP
Mob_HoistPoint::Mob_HoistPoint(){};
Mob_HoistPoint::~Mob_HoistPoint() {};

void Mob_HoistPoint::init_mob(double _dt, Vector3& _r, Vector3& _v) {
	dt = _dt;
	r.copy(_r);
	v.copy(_v);
	return;
}

Vector3 Mob_HoistPoint::A(double t, Vector3& r, Vector3& v) {
	return r;
} //Model of acceleration

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

	//�v�Z�덷�ɂ�郍�[�v������␳
	Vector3 hatL = L_.clone().normalize();
	// �␳�΂˒e����
	Vector3 ak = hatL.clone().multiplyScalor(-compensationK * (pHP->l_h - L_.length()));
	Vector3 v_ = v_.subVectors(v, pHP->v);
	// �␳�S����R��
	Vector3 agamma = hatL.clone().multiplyScalor(-compensationGamma * v_.dot(hatL));
	// ���͂ɂЂ��̒����̕␳�͂�������

	a.add(ak).add(agamma);

	return a;
} //Model of acceleration

double  Mob_HungLoad::S() {
	Vector3 v_ = v.clone().sub(pHP->v);
	double v_abs2 = v_.lengthSq();
	Vector3 vectmp;
	Vector3 vecL = vectmp.subVectors(r, pHP->r);
	return -m * (v_abs2 - pHP->a.dot(vecL) - GRAVITY_ACC * vecL.z + pHP->v_h*pHP->v_h + pHP->l_h*pHP->a_h) / (pHP->l_h*pHP->l_h);
}


