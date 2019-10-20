#include "stdafx.h"
#include "Chart.h"
extern WCHAR szWindowClass[];

Chart::Chart(){}
Chart::~Chart(){}

double Chart::ploting_data[CHART_NUM][PLOT_ITEM_NUM];
HWND Chart::hwnd_chart;
ST_CHART_DISP Chart::st_disp;
HWND Chart::hwnd_startPB;
HWND Chart::hwnd_stopPB;
HWND Chart::hwnd_radio_g;
HWND Chart::hwnd_radio_p;
HINSTANCE Chart::hInst;
UINT Chart::chart_interval_ms;
int Chart::chart_type;


void Chart::init_chart() {


	st_disp.disp_type = CHART_GRAPH;
	if (st_disp.disp_type == CHART_GRAPH) SendMessage(hwnd_radio_g, BM_SETCHECK, BST_CHECKED, 0L);
	if (st_disp.disp_type == CHART_PHASE) SendMessage(hwnd_radio_p, BM_SETCHECK, BST_CHECKED, 0L);


	st_disp.g_write_line = 0;
	for (int i = 0; i < CHART_NUM; i++) {
		st_disp.g_origin[i].x = CHART_MARGIN_X;
		st_disp.g_origin[i].y= CHART_DOT_H / 2 + CHART_DOT_H * i +25;
	}
	for (int i = 0; i < PHASE_NUM; i++) {
		st_disp.p_origin[i].x = (PHASE_MARGIN_X  + PHASE_DOT_W )*i + PHASE_DOT_W / 2;
		st_disp.p_origin[i].y = PHASE_MARGIN_Y + PHASE_DOT_H/2;
	}
	
	st_disp.g_screen_disp_time = CHART_DURATION_DEF;
	st_disp.p_screen_disp_time = PHASE_DURATION_DEF;
	st_disp.g_ms_per_dot = st_disp.g_screen_disp_time / CHART_DOT_W;
	st_disp.plot_interval_ms = PHASE_INTERVAL;

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

	Chart::hInst = hInst;
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

int Chart::plot_graph(int iline, double* pPlotting) {
	SelectObject(st_disp.hdc_mem_graph, GetStockObject(DC_BRUSH));
	SetDCBrushColor(st_disp.hdc_mem_graph, RGB(192, 0, 192));
	SelectObject(st_disp.hdc_mem_graph, GetStockObject(NULL_PEN));
	Ellipse(st_disp.hdc_mem_graph, 50, 50, 10, 10);
	return 0;
}
int Chart::plot_phase(double* pPlotting) {
	return 0;
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

		hwnd_startPB = CreateWindow(
			L"BUTTON", L"Start",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			CHART_WND_W - 100, CHART_WND_H-70, 38, 25, hWnd, (HMENU)IDC_CHART_START_PB, Chart::hInst, NULL);
		hwnd_stopPB = CreateWindow(
			L"BUTTON", L"Stop",
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			CHART_WND_W - 58, CHART_WND_H-70, 38, 25, hWnd, (HMENU)IDC_CHART_STOP_PB, Chart::hInst, NULL);
		hwnd_radio_g = CreateWindow(
			L"BUTTON", L"graph",
			WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
			CHART_WND_W - 150, 5, 60, 25, hWnd, (HMENU)IDC_CHART_RADIO_GRAPH, Chart::hInst, NULL);
		hwnd_radio_p = CreateWindow(
			L"BUTTON", L"phase",
			WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
			CHART_WND_W - 80, 5, 60, 25, hWnd, (HMENU)IDC_CHART_RADIO_PHASE, Chart::hInst, NULL);



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

		SelectObject(st_disp.hdc_mem_bg, GetStockObject(DC_PEN));
		SetDCPenColor(st_disp.hdc_mem_bg, RGB(128, 128, 128));
		for(int i = 0; i < CHART_NUM; i++) {
			MoveToEx(st_disp.hdc_mem_bg, st_disp.g_origin[i].x, st_disp.g_origin[i].y, NULL);
			LineTo(st_disp.hdc_mem_bg, st_disp.g_origin[i].x + CHART_DOT_W, st_disp.g_origin[i].y);

			MoveToEx(st_disp.hdc_mem_bg, st_disp.g_origin[i].x, st_disp.g_origin[i].y + CHART_DOT_H/2 - 5, NULL);
			LineTo(st_disp.hdc_mem_bg, st_disp.g_origin[i].x, st_disp.g_origin[i].y - CHART_DOT_H / 2 +5);

		}
		





		//TransparentBlt(st_disp.hdc_mem0, 0, 0, st_disp.chart_w, st_disp.chart_h, st_disp.hdc_mem_graph, 0, 0, st_disp.chart_w, st_disp.chart_h, RGB(255, 255, 255));
		//BitBlt(st_disp.hdc_mem0, 0, 0, st_disp.chart_w, st_disp.chart_h, st_disp.hdc_mem_bg, 0, 0, SRCCOPY);


	}break;

	case WM_TIMER:{
		if (st_disp.disp_type == CHART_GRAPH) {
			
			st_disp.g_write_line+= st_disp.g_ms_per_dot * st_disp.plot_interval_ms;
			if (st_disp.g_write_line >= CHART_WND_W * 2) {
				st_disp.g_write_line -= CHART_WND_W * 2;
			}
			int iline=1;
			double tem_d=0.1;
			plot_graph(iline, &tem_d);
		}
		else if (st_disp.disp_type == CHART_PHASE) {
			;
		}
		else;
		InvalidateRect(Chart::hwnd_chart, NULL, FALSE);
	}break;
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDC_CHART_START_PB:
			SetTimer(hwnd_chart, ID_CHART_TIMER, Chart::chart_interval_ms,NULL);
		break;
		case IDC_CHART_STOP_PB:
			KillTimer(hwnd_chart, ID_CHART_TIMER);
		break;
		case IDC_CHART_RADIO_GRAPH:
			st_disp.disp_type = CHART_GRAPH;
			SendMessage(hwnd_radio_g, BM_SETCHECK, BST_CHECKED, 0L);
			SendMessage(hwnd_radio_p, BM_SETCHECK, BST_UNCHECKED, 0L);
		break;
		case IDC_CHART_RADIO_PHASE:
			st_disp.disp_type = CHART_PHASE;
			SendMessage(hwnd_radio_p, BM_SETCHECK, BST_CHECKED, 0L);
			SendMessage(hwnd_radio_g, BM_SETCHECK, BST_UNCHECKED, 0L);
			break;


		}break;
	case WM_PAINT: {
		PAINTSTRUCT ps;

		hdc = BeginPaint(hWnd, &ps);
		TransparentBlt(st_disp.hdc_mem0, 0, 0, st_disp.chart_w, st_disp.chart_h, st_disp.hdc_mem_bg, 0, 0, st_disp.chart_w, st_disp.chart_h, RGB(255, 255, 255));
		TransparentBlt(st_disp.hdc_mem0, 0, 0, st_disp.chart_w, st_disp.chart_h, st_disp.hdc_mem_graph, 0, 0, st_disp.chart_w, st_disp.chart_h, RGB(255, 255, 255));

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