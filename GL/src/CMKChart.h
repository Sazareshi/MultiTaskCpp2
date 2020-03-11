#pragma once



namespace MKChart
{
//#include "CMKChart_ini.h"
#include <Windows.h>
//#include <windowsx.h>
#include <commctrl.h>
#include <time.h>

#define NAME_OF_INIFILE		L"mkchart"	//iniファイル名
#define EXT_OF_INIFILE		L"ini"		//iniファイル拡張子

#define SET1_SECT_OF_INIFILE					L"SET1"			//CHART1 機能パラメータセクション
#define SET2_SECT_OF_INIFILE					L"SET2"			//CHART2 機能パラメータセクション
#define SET3_SECT_OF_INIFILE					L"SET3"			//CHART3 機能パラメータセクション
#define SET4_SECT_OF_INIFILE					L"SET4"			//CHART4 機能パラメータセクション

#define SET_CHART_TYPE_KEY_OF_INIFILE			L"CHART_TYPE"
#define SET_CHART_X_KEY_OF_INIFILE				L"CHART_WND_X"
#define SET_CHART_Y_KEY_OF_INIFILE				L"CHART_WND_Y"
#define SET_CHART_W_KEY_OF_INIFILE				L"CHART_WND_W"
#define SET_CHART_H_KEY_OF_INIFILE				L"CHART_WND_H"
#define SET_CHART_PLOT_MS_KEY_OF_INIFILE		L"PLOT_MS"

#define CHART_WND_X		600
#define CHART_WND_Y		100
#define CHART_WND_W		800
#define CHART_WND_H		650

#define CHART_GRAPH		0
#define CHART_SCATTER	1

#define GRAPH_CHART_100				50	//100%ドット数
#define GRAPH_CHART_DOT_H			140	//１つのチャートの高さ
#define GRAPH_CHART_DOT_W			720	//１つのチャートの幅
#define SCAT_CHART_100				100	//100%ドット数
#define SCAT_CHART_DOT_H			300	//１つのチャートの高さ
#define SCAT_CHART_DOT_W			300	//１つのチャートの幅
#define CHART_MARGIN_X				50	//チャート書き出しポイント
#define CHART_SPEED_DEF				10000	//チャート表示範囲デフォルトmsec
#define GRAPH_CHART_BOOL_H			4	//BOOLデータON時の高さ

#define PHASE_NUM			3
#define PHASE_DURATION_DEF	10000	//位相表示時間msec
#define PHASE_INTERVAL		100	//位相表示間隔
#define PHASE_MARGIN_X		25	//位相チャート書き出しポイント
#define PHASE_MARGIN_Y		10	//位相チャート書き出しポイント
#define PHASE_MAX_DEGREE	20	//位相チャート表示最大角度



#define MK_CHART_WND_MAX		4		//表示可能チャートウィンドウ数
#define MK_CHART_MAX_PER_WND	4		//1ウィンドウ当たり表示可能チャート数
#define MK_MAX_GRAPH_PER_CHART	4		//1つのチャートに重ねて表示可能なグラフ数
#define MK_MAX_BOOL_PER_CHART	4		//１つのグラフに重ねて表示可能なON/OFFデータ数

#define MK_CHART_TYPE_BASIC		0		//X:時間　Y:DATA
#define MK_CHART_TYPE_SCATTER	1		//散布図

#define MK_CHART1				0
#define MK_CHART2				1
#define MK_CHART3				2
#define MK_CHART4				3

#define MK_DATA_TYPE_NULL		0
#define MK_DATA_TYPE_TIME		1
#define MK_DATA_TYPE_DOUBLE		2
#define MK_DATA_TYPE_INT		3
#define MK_DATA_TYPE_BOOL		4

#define MK_CHART_BUF_MAX		600
#define MK_RANGE_100PERCENT		100000

#define MK_CHART_REFRESH_MS     10000

#define MK_DATA_CODE_X			0
#define MK_DATA_CODE_Y			1
#define MK_ADDRESS_INT			0
#define MK_ADDRESS_DOUBLE		0
#define MK_ADDRESS_BOOL_MAX		4
#define MK_ADDRESS_BOOL1		0
#define MK_ADDRESS_BOOL2		1
#define MK_ADDRESS_BOOL3		2
#define MK_ADDRESS_BOOL4		3

//操作ボタンID
#define IDC_CHART_START_PB		10601
#define IDC_CHART_PAUSE_PB		10602
#define IDC_CHART_RADIO_OPT1	10603
#define IDC_CHART_RADIO_OPT2	10604

//起動タイマーID
#define ID_CHART_TIMER			100

//メッセージID
#define WM_MKCHART_INIT			0x0400



	union MK_plot {				//チャートにプロットするデータ　ON/OFFは4点
		INT32 i_data;
		bool b_data[4];
	};
	typedef struct _stMK_plot {
		INT32 x;
		MK_plot y;
	}ST_PLOT_ELEMENT, *LPST_PLOT_ELEMENT;

	typedef struct _stPLOT_BUF{
		double* pd[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART];
		int*    pi[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART];
		bool*	pb[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART][MK_MAX_BOOL_PER_CHART];
	}ST_PLOT_BUF, *LPST_PLOT_BUF;
	typedef struct _stRANGE_SET //100%の値をセット
	{
		double d100[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART];
		int    i100[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART];
	}ST_RANGE_SET, *LPST_RANGE_SET;

#define MK_CHART_INIT				0x0001
#define MK_CHART_ACTIVE				0x0002
#define MK_CHART_PAUSE				0x0004
#define MK_CHART_NO_LINE			0x0100
#define MK_CHART_NO_MARK			0x0200
#define MK_CHART_STATUS_DEF_BASIC	0x0203
#define MK_CHART_STATUS_DEF_SCATTER	0x0103
	
	typedef struct _stCHART_SET
	{
		int chart_status;
		int chart_type;
		int num_of_plot;//プロット数
		int chart_x, chart_y, chart_w, chart_h;	//チャート画面の位置　幅、高さ
		int graph_field_w, graph_field_h;//GRAPHチャートのビットマップサイズ
		int plot_interval_ms;					//表示間隔msec
		int refresh_interval;				//グラフ表示クリア間隔(プロット回数）
		DWORD	start_time_ms;					//チャート開始時間
		DWORD	latast_plot_time_ms;			//最後にプロットした時間

	//Window
		HWND hwnd_chart;				//チャートウィンドウのハンドル
		HWND hwnd_chart_startPB;		//チャートスタートPBのハンドル
		HWND hwnd_chart_pausePB;			//チャートストップPBのハンドル
		HWND hwnd_chart_opt1_radio;		//チャートOption1PBのハンドル
		HWND hwnd_chart_opt2_radio;		//チャートOption2PBのハンドル
		HBITMAP hBmap_mem0;
		HBITMAP hBmap_bg;
		HBITMAP hBmap_graph;
		HDC hdc_mem0;					//合成画面メモリデバイスコンテキスト
		HDC hdc_mem_bg;					//背景画面メモリデバイスコンテキスト
		HDC hdc_mem_graph;				//グラフ画面メモリデバイスコンテキスト
		HDC hdc_mem_inf;				//文字画面メモリデバイスコンテキスト
		HFONT hfont_inftext;			//テキスト用フォント
		BLENDFUNCTION bf;				//半透過設定構造体

		int g_ms_per_dot;							//1dotあたりのmsec
		int dot100percent_x, dot100percent_y;		//100%に割り当てるdot数
		POINT g_origin[MK_MAX_GRAPH_PER_CHART];		//グラフの原点

		bool bPrimaryDisp;							//時間チャートは2画像切り替え

		ST_PLOT_BUF pdata[2];											//プロットデータのアドレス　x軸とy軸
		ST_RANGE_SET value100[2];										//プロットデータ100%の値　時間は、100秒=100%とする
		int	data_type[2][MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART];	//プロットデータの型



		ST_PLOT_ELEMENT	plot_data[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART][MK_CHART_BUF_MAX];//チャートプロットバッファ ログ記録用
		int plot_buf_index;				//plot_dataの書き込みindex
		int plot_x;						//plotするx座標
		int spd_dot_per_sec;			//plot速度
		int graph_count[MK_CHART_MAX_PER_WND];

		UINT_PTR timerID;//プロットタイマーID

		POINT plot_p[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART][MK_MAX_BOOL_PER_CHART];
		
		HPEN hpen[MK_MAX_GRAPH_PER_CHART];
		HBRUSH hbrush[MK_MAX_GRAPH_PER_CHART];

	}ST_CHART_SET, *LPST_CHART_SET;

	
	class CMKChart
	{
	public:
		CMKChart();
		~CMKChart();

		static	WCHAR szInipath[_MAX_PATH], szDrive[_MAX_DRIVE], szPath[_MAX_PATH], szFName[_MAX_FNAME], szExt[_MAX_EXT];//iniファイルパス取得用
		static	WCHAR szLogfolder1[_MAX_PATH];
		static  std::wstring log_path;
		static	ST_CHART_SET mkchartset[MK_CHART_WND_MAX];
		static HINSTANCE hInst;

		static	int init_chartfunc();
		static	int init_chart(int chartID);
		static	int close_chart(int chartID);
		static	int set_double_data(double* pd, int chart_WND_ID, int i_chart, int i_item, double d_100, bool is_x);
		static	int set_int_data(int* pi, int chart_WND_ID, int i_chart, int i_item, int i_100, bool is_x);
		static	int set_bool_data(bool* pb, int chart_WND_ID, int i_chart, int i_item, int i_bool);
		static	int set_chart_spd(int chart_WND_ID, int speed_ms);//チャートのスピード　チャート幅記録にかかる時間msec
		static	int set_reflesh_time(int chart_WND_ID, int period_ms);

		static HWND open_chart(int chartID, HWND hwnd_parent);
		static LRESULT CALLBACK ChartWndProc_A(HWND, UINT, WPARAM, LPARAM);//Time-Data Graph
		static LRESULT CALLBACK ChartWndProc_B(HWND, UINT, WPARAM, LPARAM);//Data-Data Graph
		
		static	int chart_start(int chartID, HWND hwnd_parent);
		static int get_chartID(HWND hWnd) { for (int i = 0; i < MK_CHART_WND_MAX; i++) { if (hWnd == mkchartset[i].hwnd_chart) return i; } return 3; }
		static	int set_graph(int chart_WND_ID);
	};
}
