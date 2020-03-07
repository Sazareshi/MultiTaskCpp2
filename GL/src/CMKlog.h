#pragma once

#include <Windows.h>
#include "MKlog_ini.h"

namespace MKlog
{
#define LOG_RECORD_MAX		8	//LOG�L�^���ő�l
#define LOG_ITEM_MAX		128 //LOG�ɋL�^����ITEM���ő�l
#define LABEL_LEN_MAX		24 //LOG�ɋL�^����ITEM���x�������ő吔
#define TIMER_ID_BASE		300 //TIMER ID�w��l

#define LOG_STATUS_STANDBY	0 //LOG�L�^�̏��
#define LOG_STATUS_ACTIVE	1 //LOG�L�^�̏��
#define LOG_STATUS_PAUSE	2 //LOG�L�^�̏��
#define LOG_STATUS_STOP		3 //LOG�L�^�̏��
#define LOG_STATUS_ERROR	-1 //LOG�L�^�̏��

#define EXT_OF_CSVFILE			L".csv"		//csv�t�@�C���g���q
#define DELIMITER_OF_FILE		L"_"		//�t�@�C������؂蕶��


	typedef struct stMKlogSet {
		FILE*	fp;
		HANDLE	hf;
		WCHAR	szType[128];				//BASIC:������ŋL�^��������@TRAP:������Ŏw�萔�T���v�����O�@EVENT�F�C�x���g�g���K�ŋL�^
		WCHAR	fname[256];					//�t�@�C����
		WCHAR	labes[LOG_ITEM_MAX][LABEL_LEN_MAX];//���x���e�L�X�g
		std::wstring filename;
		int		ms_period;					//�T���v�����O�����@msec NEGATIVE VALUE:�C�x���g�^
		int		n_item;						//�T���v�����O�f�[�^���ڐ�
		int		timeset;					//���ԋL�^�^�C�v
		int		data_type[LOG_ITEM_MAX];	//0:double 1:int 2:bool
		double*	p_double[LOG_ITEM_MAX];		//�L�^�f�[�^�̃A�h���X�}�b�v
		int*	p_int[LOG_ITEM_MAX];		//�L�^�f�[�^�̃A�h���X�}�b�v
		bool*	p_bool[LOG_ITEM_MAX];		//�L�^�f�[�^�̃A�h���X�}�b�v
		UINT	timerID;
		int		status;						//0:STANDBY 1:ACTIVE 2:PAUSE 3:STOP NEGATIVE VALUE:ERROR
		int		max_sample;					//�T���v�����O�ő吔
	}ST_MKLOG_SET, *PST_MKLOG_SET;

	class CMKlog
	{
	public:
		CMKlog() {};
		~CMKlog(){};

		static	WCHAR szInipath[_MAX_PATH], szDrive[_MAX_DRIVE], szPath[_MAX_PATH], szFName[_MAX_FNAME], szExt[_MAX_EXT];//ini�t�@�C���p�X�擾�p
		static	WCHAR szLogfolder1[_MAX_PATH];
		static	ST_MKLOG_SET mklogset[LOG_RECORD_MAX];
		static	int nlogs;
		static  std::wstring log_path;
		static  HWND hwnd_client;
		static	double def_double;	//�Q�ƃ|�C���^�����ݒ�p�@�_�~�[�f�[�^
		static	bool def_bool;		//�Q�ƃ|�C���^�����ݒ�p�@�_�~�[�f�[�^

		int init_logfunc();//�������@�ݒ�t�B���ǂݍ���

		static int start_record(int logID, HWND hWnd);//int logID, HWND hWnd:�N���C�A���g���Ń^�C�}�[����������ꍇ�̃C�x���g��MWindow
		static int end_record(int logID);
		static int pause_record(int logID);
		static int add_record(int logID);
		static int set_label(int logID);

		int set_double_data(double* pd, int logID, int i_item) { if((logID<LOG_RECORD_MAX)&&(i_item<LOG_ITEM_MAX)) mklogset[logID].p_double[i_item] = pd; return 0; };
		int set_int_data(int* pi, int logID, int i_item) { if ((logID<LOG_RECORD_MAX) && (i_item<LOG_ITEM_MAX)) mklogset[logID].p_int[i_item] = pi; return 0; };
		int set_bool_data(bool* pb, int logID, int i_item) { if ((logID<LOG_RECORD_MAX) && (i_item<LOG_ITEM_MAX)) mklogset[logID].p_bool[i_item] = pb; return 0; };


		static VOID CALLBACK MKlogTimerProc(HWND hwnd, UINT msg, UINT idEvent, DWORD dwTime);


		static std::wstring getDatetimeStr();
	};


#define MK_LOGSET_1				0 //LOG SET�z��Index
#define MK_LOGSET_2				1 //LOG SET�z��Index
#define MK_LOGSET_3				2 //LOG SET�z��Index

#define MK_DATA_TYPE_DOUBLE		0 //�L�^�f�[�^�̌^�@double
#define MK_DATA_TYPE_INT		1 //�L�^�f�[�^�̌^�@int
#define MK_DATA_TYPE_BOOL		2 //�L�^�f�[�^�̌^�@bool
}





