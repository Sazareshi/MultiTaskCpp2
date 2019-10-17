#include "stdafx.h"
#include "Chart.h"
extern WCHAR szWindowClass[];

Chart::Chart(){}
Chart::~Chart(){}

double Chart::ploting_data[CHART_NUM][PLOT_ITEM_NUM];
HWND Chart::hwnd_chart;
ST_CHART_DISP Chart::st_disp;


void Chart::init_chart() {
	st_disp.disp_type = CHART_GRAPH;
	st_disp.g_write_line = 0;
	for (int i = 0; i < CHART_NUM; i++) {
		st_disp.g_origin[i].x = CHART_MARGIN_X;
		st_disp.g_origin[i].y= CHART_DOT_H / 2 + CHART_DOT_H * i;
	}
	for (int i = 0; i < PHASE_NUM; i++) {
		st_disp.p_origin[i].x = (PHASE_MARGIN_X  + PHASE_DOT_W )*i + PHASE_DOT_W / 2;
		st_disp.p_origin[i].y = PHASE_MARGIN_Y + PHASE_DOT_H/2;
	}
	
	st_disp.g_screen_disp_time = CHART_DURATION_DEF;
	st_disp.p_screen_disp_time = PHASE_DURATION_DEF;
	st_disp.g_ms_per_dot = st_disp.g_screen_disp_time * 1000 / CHART_DOT_W;
	st_disp.p_ms_interval = PHASE_INTERVAL;

	st_disp.chart_w = CHART_WND_W * 4;
	st_disp.chart_h = CHART_WND_H;

	//表示フォント
	st_disp.hfont_inftext = CreateFont(
		12,						//int cHeight
		0,						//int cWidth
		0,						//int cEscapement
		0,						//int cOrientation
		0,						//int cWeight
		FALSE,					//DWORD bItalic
		FALSE,					//DWORD bUnderline
		FALSE,					//DWORD bStrikeOut
		SHIFTJIS_CHARSET,		//DWORD iCharSet
		OUT_DEFAULT_PRECIS,		//DWORD iOutPrecision
		CLIP_DEFAULT_PRECIS,	//DWORD iClipPrecision
		PROOF_QUALITY,			//DWORD iQuality
		FIXED_PITCH | FF_MODERN,//DWORD iPitchAndFamily
		TEXT("Arial")			//LPCWSTR pszFaceName
	);

	st_disp.hdc_mem0 = CreateCompatibleDC(NULL);
	st_disp.hdc_mem_bg = CreateCompatibleDC(NULL);
	st_disp.hdc_mem_graph = CreateCompatibleDC(NULL);


	return;
}
/*########################################################################
チャートWindow Create & 表示
##########################################################################*/
void Chart::open_chart(HINSTANCE hInst, HWND hwnd_parent) {
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = Chart::ChartWndProc;// !CALLBACKでreturnを返していないとWindowClassの登録に失敗する
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("myChart");
	wc.hIconSm = NULL;
	ATOM fb = RegisterClassExW(&wc);

	hwnd_chart = CreateWindow(TEXT("myChart"),
		TEXT("MY CHART"),
		WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION,
		CHART_WND_X,
		CHART_WND_Y,
		CHART_WND_W,
		CHART_WND_H,
		hwnd_parent,
		0,
		hInst,
		NULL);

	ShowWindow(hwnd_chart, SW_SHOW);
	UpdateWindow(hwnd_chart);
}

/*########################################################################
チャートWindowのCall　Back関数
##########################################################################*/
LRESULT Chart::ChartWndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {

	BITMAP bmp_info;
	static HWND hinfchk;
	int nFramX, nFramY, nCaption, scrw, scrh, winw, winh, winx, winy, mob_speed = 4;
	HDC hdc;
	TCHAR szBuf[128];
	HMENU hsubmenu, hpopmenu;
	HWND tmpwnd;
	

	switch (msg) {
	case WM_CREATE: {
		nFramX = GetSystemMetrics(SM_CXSIZEFRAME);		//ウィンドウ周囲の幅
		nFramY = GetSystemMetrics(SM_CYSIZEFRAME);		//ウィンドウ周囲の高さ
		nCaption = GetSystemMetrics(SM_CYCAPTION);		//タイトルバーの高さ
		scrw = GetSystemMetrics(SM_CXSCREEN);			//プライマモニタの幅
		scrh = GetSystemMetrics(SM_CYSCREEN);			//プライマモニタの高さ

		init_chart();
	
		hdc = GetDC(hWnd);
		HBITMAP hDummy = CreateCompatibleBitmap(hdc, st_disp.chart_w, st_disp.chart_h);
		SelectObject(st_disp.hdc_mem0, hDummy);		//合成画像（背景＋グラフ）
		hDummy = CreateCompatibleBitmap(hdc, st_disp.chart_w, st_disp.chart_h);
		SelectObject(st_disp.hdc_mem_bg, hDummy);	//背景画像
		hDummy = CreateCompatibleBitmap(hdc, st_disp.chart_w, st_disp.chart_h);
		SelectObject(st_disp.hdc_mem_graph, hDummy);//グラフ画像

		PatBlt(st_disp.hdc_mem0, 0, 0, st_disp.chart_w, st_disp.chart_h, WHITENESS);//HDC hdc, int x,int y, int w, int h,DWORD rop
		PatBlt(st_disp.hdc_mem_bg, 0, 0, st_disp.chart_w, st_disp.chart_h, WHITENESS);//HDC hdc, int x,int y, int w, int h,DWORD rop
		PatBlt(st_disp.hdc_mem_graph, 0, 0, st_disp.chart_w, st_disp.chart_h, WHITENESS);//HDC hdc, int x,int y, int w, int h,DWORD rop

		ReleaseDC(hWnd, hdc);		
		DeleteObject(hDummy);

		//SelectObject(st_disp.hdc_mem_bg, GetStockObject(BLACK_PEN));

		POINT linkpt[2];
		POINT toPT;

	//	linkpt[0].x = 25; linkpt[0].y = 75; linkpt[1].x = 800; linkpt[1].y = 75;
	//	SelectObject(st_disp.hdc_mem0, GetStockObject(DC_PEN));
	//	SetDCPenColor(st_disp.hdc_mem0, RGB(128, 0, 0));
	//	MoveToEx(st_disp.hdc_mem0, linkpt[0].x, linkpt[0].y, NULL);
	//	LineTo(st_disp.hdc_mem0, linkpt[1].x, linkpt[1].y);
	//	linkpt[0].x = 25; linkpt[0].y = 225; linkpt[1].x = 800; linkpt[1].y = 225;

		SelectObject(st_disp.hdc_mem_bg, GetStockObject(DC_PEN));
		SetDCPenColor(st_disp.hdc_mem_bg, RGB(128, 128, 128));
		for(int i = 0; i < CHART_NUM; i++) {
			MoveToEx(st_disp.hdc_mem_bg, st_disp.g_origin[i].x, st_disp.g_origin[i].y, NULL);
			LineTo(st_disp.hdc_mem_bg, st_disp.g_origin[i].x + CHART_DOT_W, st_disp.g_origin[i].y);

			MoveToEx(st_disp.hdc_mem_bg, st_disp.g_origin[i].x, st_disp.g_origin[i].y + CHART_DOT_H/2 - 5, NULL);
			LineTo(st_disp.hdc_mem_bg, st_disp.g_origin[i].x, st_disp.g_origin[i].y - CHART_DOT_H / 2 +5);

		}
		


/*
		SelectObject(st_disp.hdc_mem_bg, GetStockObject(DC_BRUSH));
		SetDCBrushColor(st_disp.hdc_mem_bg, RGB(192, 0, 192));
		SelectObject(st_disp.hdc_mem_bg, GetStockObject(NULL_PEN));
		Ellipse(st_disp.hdc_mem_bg, 50, 50, 10, 10);
*/

		TransparentBlt(st_disp.hdc_mem0, 0, 0, st_disp.chart_w, st_disp.chart_h, st_disp.hdc_mem_bg, 0, 0, st_disp.chart_w, st_disp.chart_h, RGB(255, 255, 255));
		//BitBlt(st_disp.hdc_mem0, 0, 0, st_disp.chart_w, st_disp.chart_h, st_disp.hdc_mem_bg, 0, 0, SRCCOPY);


	}break;
	case WM_PAINT: {
		PAINTSTRUCT ps;

		hdc = BeginPaint(hWnd, &ps);
		BitBlt(
			hdc,				//HDC hdc
			0,					//int x
			0,					//int y
			st_disp.chart_w,	//int cx
			st_disp.chart_h,	//int c
			st_disp.hdc_mem0,	//HDC hdcSrc
			0,					//int x1
			0,					//int y1
			SRCCOPY//DWORD rop
		);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_KEYDOWN:
		if (wp == 'Q') {
			DestroyWindow(hWnd);
		}
		break;
	default:
		return DefWindowProc(hWnd, msg, wp, lp);
	}
	return 0;
}