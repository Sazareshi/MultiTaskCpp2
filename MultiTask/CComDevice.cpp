#include "stdafx.h"
#include "CComDevice.h"
#include "CPublicRelation.h"

extern CORDER_Table*	pOrder;
extern CMODE_Table*	pMode;
extern CIO_Table*	pIO_Table;

CComDevice::CComDevice(){}

CComDevice::~CComDevice(){}


void CComDevice::init_task(void *pobj) {
	set_panel_tip_txt();
	return;
};

void CComDevice::routine_work(void *param) {

	ws << L" working!" << *(inf.psys_counter)%100;
	tweet2owner(ws.str()); ws.str(L""); ws.clear();

};


