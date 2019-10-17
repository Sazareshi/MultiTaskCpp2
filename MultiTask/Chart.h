#pragma once
#include <windowsx.h>
#include <commctrl.h>

#define CHART_NUM		4
#define PLOT_ITEM_NUM	4
#define CHART_WND_X		0
#define CHART_WND_Y		0
#define CHART_WND_W		800
#define CHART_WND_H		600

#define CHART_GRAPH		0
#define CHART_PHASE		1

#define CHART_100			50	//100%ドット数
#define CHART_DOT_H			140	//１つのチャートの高さ
#define CHART_DOT_W			720	//１つのチャートの幅
#define CHART_MARGIN_X		50	//チャート書き出しポイント
#define CHART_DURATION_DEF	10	//チャート表示範囲デフォルト秒

#define PHASE_NUM			3
#define PHASE_DOT_H			200	//１つの位相チャートの高さ
#define PHASE_DOT_W			200	//１つの位相チャートの幅
#define PHASE_DURATION_DEF	10	//位相表示時間秒
#define PHASE_INTERVAL		100	//位相表示間隔
#define PHASE_MARGIN_X		50	//位相チャート書き出しポイント
#define PHASE_MARGIN_Y		50	//位相チャート書き出しポイント
#define PHASE_MAX_DEGREE	20	//位相チャート表示最大角度


typedef struct _stCHART_DISP
{
	int chart_w, chart_h;			//チャート画面の幅、高さ
	HDC hdc_mem0;					//合成画面メモリデバイスコンテキスト
	HDC hdc_mem_bg;					//背景画面メモリデバイスコンテキスト
	HDC hdc_mem_graph;				//グラフ画面メモリデバイスコンテキスト
	HDC hdc_mem_inf;				//文字画面メモリデバイスコンテキスト
	HFONT hfont_inftext;			//テキスト用フォント
	BLENDFUNCTION bf;				//半透過設定構造体

	int g_write_line;				//チャート書込ライン
	bool bPrimalyFirst;				//第一画面表示
	int disp_type;					//0:graph 1:phase
	int g_screen_disp_time;			//graph画面表示時間
	int p_screen_disp_time;			//位相画面表示時間
	POINT g_origin[CHART_NUM];		//グラフの原点
	POINT p_origin[PHASE_NUM];		//位相平面の原点

	int g_ms_per_dot;					//1dotあたりのmsec
	int p_ms_interval;					//位相平面表示間隔msec
	int p_max_degree;					//位相平面表示最大角度


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
	static void init_chart();
	static LRESULT CALLBACK ChartWndProc(HWND, UINT, WPARAM, LPARAM);
};

