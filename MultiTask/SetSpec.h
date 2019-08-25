#pragma once

#include "CommonFormat.h"

ST_SPEC g_spec =
{
	{ 0.0,	0.0315,	0.04,	0.0525,	0.0735,	0.105 },//	rad/s	slew_notch_spd[NOTCH_MAX];
	{ 0.0,	0.058,	0.175,	0.292,	0.406,	0.583 },//	m/s		hoist_notch_spd[NOTCH_MAX];
	{ 0.0,	0.1,	0.3,	0.5,	0.7,	1.0 },	//	m/s		bh_notch_spd[NOTCH_MAX];
	{ 0.0,	0.04,	0.125,	0.25,	0.416,	0.416 },//	m/s		gantry_notch_spd[NOTCH_MAX];
	{ 0.0,	0.105,	0.131,	0.131,	0.131,	0.131 },//	rad/s	hook_notch_spd[NOTCH_MAX];
	{ 0.0,	0.25,	0.25,	0.25,	0.25,	0.25 },	//	m/s		operm_notch_spd[NOTCH_MAX];

	{ 0.00875,	-0.00875,	0.00875,	-0.00875 },	//	rad/s2	slew_acc[ACCELERATION_MAX];
	{ 0.387,	-0.387,	0.387,	-0.387 },			//	m/s2	hoist_acc[ACCELERATION_MAX];
	{ 0.333,	-0.333,	0.333,	-0.333 },			//	m/s2	bh_acc[ACCELERATION_MAX];
	{ 0.0812,	-0.0812,	0.0812,	-0.0812 },		//	m/s2	gantry_acc[ACCELERATION_MAX];
	{ 0.044,	-0.044,		0.044,	-0.044 },		//	rad/s2	hook_acc[ACCELERATION_MAX];
	{ 0.0125,	-0.0125,	0.0125,	-0.0125 },		//	m/s2	operm_acc[ACCELERATION_MAX];

	25.0	//double boom_height;
};
