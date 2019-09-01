#pragma once
#include <windowsx.h>
#include <commctrl.h>

#define CHART_NUM		4
#define PLOT_ITEM_NUM	4
#define CHART_WND_X		0
#define CHART_WND_Y		0
#define CHART_WND_W		800
#define CHART_WND_H		600

typedef struct _stCHART_DISP
{
	int chart_w, chart_h;			//�`���[�g��ʂ̕��A����
	HDC hdc_mem0;					//������ʃ������f�o�C�X�R���e�L�X�g
	HDC hdc_mem_bg;					//�w�i��ʃ������f�o�C�X�R���e�L�X�g
	HDC hdc_mem_graph;				//�O���t��ʃ������f�o�C�X�R���e�L�X�g
	HDC hdc_mem_inf;				//������ʃ������f�o�C�X�R���e�L�X�g
	HFONT hfont_inftext;			//�e�L�X�g�p�t�H���g
	BLENDFUNCTION bf;				//�����ߐݒ�\����
}ST_CHART_DISP, *LPST_CHART_DISP;

class Chart
{
public:
	Chart();
	~Chart();

	static ST_CHART_DISP st_disp;
	
	static void open_chart(HINSTANCE hInst, HWND hwnd_parent);
	
	static double ploting_data[CHART_NUM][PLOT_ITEM_NUM];
	static HWND hwnd_chart;
	static int set_ploting_data(double* pPlotting);
	static LRESULT CALLBACK ChartWndProc(HWND, UINT, WPARAM, LPARAM);
};

