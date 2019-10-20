#pragma once
#include <windowsx.h>
#include <commctrl.h>

#define IDC_CHART_START_PB		10601
#define IDC_CHART_STOP_PB		10602
#define IDC_CHART_RADIO_GRAPH	10603
#define IDC_CHART_RADIO_PHASE	10604


#define ID_CHART_TIMER	1


#define CHART_NUM		4
#define PLOT_ITEM_NUM	4
#define CHART_WND_X		0
#define CHART_WND_Y		0
#define CHART_WND_W		800
#define CHART_WND_H		650

#define CHART_GRAPH		0
#define CHART_PHASE		1

#define CHART_100			50	//100%�h�b�g��
#define CHART_DOT_H			140	//�P�̃`���[�g�̍���
#define CHART_DOT_W			720	//�P�̃`���[�g�̕�
#define CHART_MARGIN_X		50	//�`���[�g�����o���|�C���g
#define CHART_DURATION_DEF	10000	//�`���[�g�\���͈̓f�t�H���gmsec

#define PHASE_NUM			3
#define PHASE_DOT_H			200	//�P�̈ʑ��`���[�g�̍���
#define PHASE_DOT_W			200	//�P�̈ʑ��`���[�g�̕�
#define PHASE_DURATION_DEF	10000	//�ʑ��\������msec
#define PHASE_INTERVAL		100	//�ʑ��\���Ԋu
#define PHASE_MARGIN_X		50	//�ʑ��`���[�g�����o���|�C���g
#define PHASE_MARGIN_Y		50	//�ʑ��`���[�g�����o���|�C���g
#define PHASE_MAX_DEGREE	20	//�ʑ��`���[�g�\���ő�p�x


typedef struct _stCHART_DISP
{
	int chart_w, chart_h;			//�`���[�g��ʂ̕��A����
	HDC hdc_mem0;					//������ʃ������f�o�C�X�R���e�L�X�g
	HDC hdc_mem_bg;					//�w�i��ʃ������f�o�C�X�R���e�L�X�g
	HDC hdc_mem_graph;				//�O���t��ʃ������f�o�C�X�R���e�L�X�g
	HDC hdc_mem_inf;				//������ʃ������f�o�C�X�R���e�L�X�g
	HFONT hfont_inftext;			//�e�L�X�g�p�t�H���g
	BLENDFUNCTION bf;				//�����ߐݒ�\����

	int plot_interval_ms;				//�ʑ����ʕ\���Ԋumsec
	
	int g_write_line;				//�`���[�g�������C��
	bool bPrimalyFirst;				//����ʕ\��
	int disp_type;					//0:graph 1:phase
	int g_screen_disp_time;			//graph��ʕ\������
	int p_screen_disp_time;			//�ʑ���ʕ\������
	POINT g_origin[CHART_NUM];		//�O���t�̌��_
	POINT p_origin[PHASE_NUM];		//�ʑ����ʂ̌��_

	int g_ms_per_dot;					//1dot�������msec
	int p_max_degree;					//�ʑ����ʕ\���ő�p�x


}ST_CHART_DISP, *LPST_CHART_DISP;






class Chart
{
public:
	Chart();
	~Chart();

	static ST_CHART_DISP st_disp;
	static HWND hwnd_startPB;
	static HWND hwnd_stopPB;
	static HWND hwnd_radio_g;
	static HWND hwnd_radio_p;
	static UINT	chart_interval_ms;

	static HINSTANCE hInst;
	
	static void open_chart(HINSTANCE hInst, HWND hwnd_parent);
	
	static double ploting_data[CHART_NUM][PLOT_ITEM_NUM];
	static HWND hwnd_chart;

	static int plot_graph(int iline, double* pPlotting);
	static int plot_phase(double* pPlotting);
	static void init_chart();
	static LRESULT CALLBACK ChartWndProc(HWND, UINT, WPARAM, LPARAM);
	static int chart_type;
};

