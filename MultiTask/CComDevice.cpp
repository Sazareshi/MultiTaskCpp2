#include "stdafx.h"
#include "CComDevice.h"
#include "CPublicRelation.h"

extern CORDER_Table*	pOrder;

CComDevice::CComDevice(){}

CComDevice::~CComDevice(){}

bool CComDevice::get_UI() {
	CPublicRelation* pPR = (CPublicRelation *) VectpCTaskObj[g_itask.pr];
	pOrder->ui.notch_slew = pPR->ui_table.notch_slew;
	pOrder->ui.notch_bh = pPR->ui_table.notch_bh;

	return FALSE;
};