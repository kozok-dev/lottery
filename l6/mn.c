#include <windows.h>
#include "../common.c"

int g_num[6];
HWND g_hnum[43], g_hlb, g_hstc;
HFONT g_hfont;
WNDPROC g_staticproc;

LRESULT CALLBACK proc_static(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char num, i;
	TCHAR buf[32];
	RECT rect;
	HDC hdc;
	HBRUSH hbrush;
	PAINTSTRUCT ps;

	switch (uMsg) {
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		SetBkMode(hdc, TRANSPARENT);
		SelectObject(hdc, g_hfont);
		GetClientRect(hwnd, &rect);

		num = (char)GetProp(hwnd, TEXT("NUM"));
		for (i = 0; i < 6 && g_num[i] != num; i++);
		if (i < 6) {
			hbrush = CreateSolidBrush(RGB(0, 255, 0));
			FillRect(hdc, &rect, hbrush);
			DeleteObject(hbrush);
		} else
			FillRect(hdc, &rect, (HBRUSH)COLOR_WINDOW);

		wsprintf(buf, TEXT("%d"), num);
		DrawText(hdc, buf, -1, &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		RemoveProp(hwnd, TEXT("NUM"));
		return 0;
	}
	return CallWindowProc(g_staticproc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK proc_wnd(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int i;
	char hit[7], j, k, l;
	TCHAR buf[256];

	switch (uMsg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case 1:
			switch (HIWORD(wParam)) {
			case LBN_SELCHANGE:
				if ((i = SendMessage(g_hlb, LB_GETCURSEL, 0, 0)) == LB_ERR ||
				SendMessage(g_hlb, LB_GETTEXT, i, (LPARAM)buf) == LB_ERR) break;
				_stscanf(buf, TEXT("%d|%d|%d|%d|%d|%d"),
				&g_num[0], &g_num[1], &g_num[2], &g_num[3], &g_num[4], &g_num[5]);

				for (i = 0; i < 7; i++) hit[i] = 0;
				for (i = 1; i < 9; i++) {
					j = 0;
					for (k = i; k < 44; k += 10)
						for (l = 0; l < 6; l++)
							if (g_num[l] == k || g_num[l] == k + 1 || g_num[l] == k + 2) j++;
					hit[j]++;
				}

				_stprintf(buf, TEXT("0: %d\n1: %d\n2: %d\n3: %d\n4: %d\n5: %d\n6: %d"),
				hit[0], hit[1], hit[2], hit[3], hit[4], hit[5], hit[6]);
				SetWindowText(g_hstc, buf);

				for (i = 0; i < 43; i++) RedrawWindow(g_hnum[i], NULL, NULL, RDW_INVALIDATE);
				break;
			}
			break;
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	char num[6], i, j, k, l;
	short hit[7], cnt;
	TCHAR buf[256];
	FILE *f;
	WNDCLASS wc;
	MSG msg;
	HWND hwnd, hstc;

	wc.style = 0;
	wc.lpfnWndProc = proc_wnd;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("mn");
	if (RegisterClass(&wc) == 0) return 0;

	if ((g_hfont = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
	OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, NULL)) == NULL) return 0;

	if ((hwnd = CreateWindow(wc.lpszClassName, wc.lpszClassName, WS_MINIMIZEBOX | WS_SYSMENU,
	200, 100, 160, 445, NULL, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	if ((g_hlb = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), NULL,
	WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 5, 5, 145, 80,
	hwnd, (HMENU)1, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hlb, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	if ((g_hstc = CreateWindow(TEXT("STATIC"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE,
	5, 80, 30, 95, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hstc, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	for (i = 0; i < 10; i++)
		for (j = i; j < 43; j += 10) {
			if ((g_hnum[j] = CreateWindow(TEXT("STATIC"), NULL,
			WS_BORDER | WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
			j / 10 * 20 + 40, i * 22 + 80, 17, 20, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
			SendMessage(g_hnum[j], WM_SETFONT, (WPARAM)g_hfont, FALSE);
			SetProp(g_hnum[j], TEXT("NUM"), (HANDLE)(j + 1));
			g_staticproc = (WNDPROC)GetWindowLong(g_hnum[j], GWL_WNDPROC);
			SetWindowLong(g_hnum[j], GWL_WNDPROC, (int)proc_static);
		}

	for (i = 0; i < 7; i++) hit[i] = 0;
	cnt = 0;
	if ((f = fopen(strlen(lpCmdLine) > 0 ? lpCmdLine : "exp.txt", "r")) != NULL) {
		while (get_number(f, num, 6, 6, 1, 43, 0, 6, 0)) {
			wsprintf(buf, TEXT("%2d|%2d|%2d|%2d|%2d|%2d"),
			num[0], num[1], num[2], num[3], num[4], num[5]);
			if (SendMessage(g_hlb, LB_ADDSTRING, 0, (LPARAM)buf) < 0) break;

			for (i = 1; i < 9; i++) {
				j = 0;
				for (k = i; k < 44; k += 10)
					for (l = 0; l < 6; l++)
						if (num[l] == k || num[l] == k + 1 || num[l] == k + 2) j++;
				hit[j]++;
			}
			cnt++;
		}
		fclose(f);
	}
	if ((hstc = CreateWindow(TEXT("STATIC"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE,
	5, 305, 125, 110, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(hstc, WM_SETFONT, (WPARAM)g_hfont, FALSE);
	_stprintf(buf,
		TEXT("Count: %d\n")
		TEXT("0: %4d %f\n")
		TEXT("1: %4d %f\n")
		TEXT("2: %4d %f\n")
		TEXT("3: %4d %f\n")
		TEXT("4: %4d %f\n")
		TEXT("5: %4d %f\n")
		TEXT("6: %4d %f"), cnt,
		hit[0], (float)hit[0] / (float)cnt,
		hit[1], (float)hit[1] / (float)cnt,
		hit[2], (float)hit[2] / (float)cnt,
		hit[3], (float)hit[3] / (float)cnt,
		hit[4], (float)hit[4] / (float)cnt,
		hit[5], (float)hit[5] / (float)cnt,
		hit[6], (float)hit[6] / (float)cnt
	);
	SetWindowText(hstc, buf);

	ShowWindow(hwnd, SW_SHOWNORMAL);
	while (GetMessage(&msg, NULL, 0, 0)) DispatchMessage(&msg);
	DeleteObject(g_hfont);
	return msg.wParam;
}
