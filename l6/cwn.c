#include <windows.h>
#include "../common.c"

char g_encnt;
HWND g_hwwn[7], g_hwen[30][7], g_hwttl;
HFONT g_hfont;
WNDPROC g_editproc, g_staticproc;

void print_hit_stat()
{
	char hit[8], i, j, k, l;
	TCHAR buf1[200], buf2[5];

	for (i = 0; i < 8; i++) hit[i] = 0;
	for (i = 0; i < g_encnt; i++) {
		j = 0;
		for (k = 0; k < 6; k++) {
			GetWindowText(g_hwwn[k], buf1, 3);
			for (l = 0; l < k; l++) {
				GetWindowText(g_hwwn[l], buf2, 3);
				if (atoi(buf1) == atoi(buf2)) break;
			}
			if (l < k) continue;
			for (l = 0; l < 6; l++) {
				if (atoi(buf1) != (char)GetProp(g_hwen[i][l], TEXT("NUM"))) continue;
				j++;
				break;
			}
		}
		if (j == 5) {
			GetWindowText(g_hwwn[6], buf1, 3);
			for (l = 0; l < 6; l++) {
				GetWindowText(g_hwwn[l], buf2, 3);
				if (atoi(buf1) == atoi(buf2)) break;
			}
			if (l == 6)
				for (l = 0; l < 6; l++) {
					if (atoi(buf1) != (char)GetProp(g_hwen[i][l], TEXT("NUM"))) continue;
					j = 7;
					break;
				}
		}
		hit[j]++;
	}

	_stprintf(buf1, TEXT(
		"0 : %2d %10f%%\n"
		"1 : %2d %10f%%\n"
		"2 : %2d %10f%%\n"
		"3 : %2d %10f%%\n"
		"4 : %2d %10f%%\n"
		"5 : %2d %10f%%\n"
		"5b: %2d %10f%%\n"
		"6 : %2d %10f%%\n"),
		hit[0], (float)hit[0] / (float)g_encnt * 100,
		hit[1], (float)hit[1] / (float)g_encnt * 100,
		hit[2], (float)hit[2] / (float)g_encnt * 100,
		hit[3], (float)hit[3] / (float)g_encnt * 100,
		hit[4], (float)hit[4] / (float)g_encnt * 100,
		hit[5], (float)hit[5] / (float)g_encnt * 100,
		hit[7], (float)hit[7] / (float)g_encnt * 100,
		hit[6], (float)hit[6] / (float)g_encnt * 100);
	SetWindowText(g_hwttl, buf1);
}

LRESULT CALLBACK proc_edit(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char i, j;
	TCHAR buf1[5], buf2[5];

	switch (uMsg) {
	case WM_KILLFOCUS:
		GetWindowText(hwnd, buf1, 3);
		i = atoi(buf1);
		if (i < 1 || i > 43) SetWindowText(hwnd, NULL);

		for (i = 0; i < 6; i++) {
			GetWindowText(g_hwwn[i], buf1, 3);
			for (j = i + 1; j < 7; j++) {
				GetWindowText(g_hwwn[j], buf2, 3);
				if (atoi(buf1) == atoi(buf2)) SetWindowText(g_hwwn[j], NULL);
			}
		}
	case WM_KEYUP:
		for (i = 0; i < g_encnt; i++)
			for (j = 0; j < 7; j++) RedrawWindow(g_hwen[i][j], NULL, NULL, RDW_INVALIDATE);
		print_hit_stat();
		break;
	}
	return CallWindowProc(g_editproc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK proc_static1(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char num, i, j;
	TCHAR buf[5];
	RECT rect;
	HDC hdc;
	PAINTSTRUCT ps;

	switch (uMsg) {
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		SetBkMode(hdc, TRANSPARENT);
		SelectObject(hdc, g_hfont);

		num = (char)GetProp(hwnd, TEXT("NUM"));

		for (i = 0; i < 7; i++) {
			GetWindowText(g_hwwn[i], buf, 3);
			if (atoi(buf) == num) break;
		}

		SetTextColor(hdc, i < 6 ? RGB(255, 0, 0) : i < 7 ? RGB(0, 0, 255) : RGB(0, 0, 0));
		wsprintf(buf, TEXT("%d"), num);
		GetClientRect(hwnd, &rect);
		DrawText(hdc, buf, -1, &rect, DT_CENTER);

		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		RemoveProp(hwnd, TEXT("NUM"));
		return 0;
	}
	return CallWindowProc(g_staticproc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK proc_static2(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char hit, idx, i, j;
	TCHAR buf1[5], buf2[5];
	RECT rect;
	HDC hdc;
	PAINTSTRUCT ps;

	switch (uMsg) {
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		SetBkColor(hdc, GetSysColor(COLOR_MENU));
		SelectObject(hdc, g_hfont);

		hit = 0;
		idx = (char)GetProp(hwnd, TEXT("IDX"));
		for (i = 0; i < 6; i++) {
			GetWindowText(g_hwwn[i], buf1, 3);
			for (j = 0; j < i; j++) {
				GetWindowText(g_hwwn[j], buf2, 3);
				if (atoi(buf1) == atoi(buf2)) break;
			}
			if (j < i) continue;
			for (j = 0; j < 6; j++) {
				if (atoi(buf1) != (char)GetProp(g_hwen[idx][j], TEXT("NUM"))) continue;
				hit++;
				break;
			}
		}
		if (hit == 5) {
			GetWindowText(g_hwwn[6], buf1, 3);
			for (i = 0; i < 6; i++) {
				GetWindowText(g_hwwn[i], buf2, 3);
				if (atoi(buf1) == atoi(buf2)) break;
			}
			if (i == 6)
				for (i = 0; i < 6; i++) {
					if (atoi(buf1) != (char)GetProp(g_hwen[idx][i], TEXT("NUM"))) continue;
					hit = 7;
					break;
				}
		}

		SetTextColor(hdc, hit < 3 ? RGB(0, 0, 0) : hit < 7 ? RGB(255, 0, 0) : RGB(0, 0, 255));
		wsprintf(buf1, TEXT("%d"), hit < 7 ? hit : hit - 2);
		GetClientRect(hwnd, &rect);
		DrawText(hdc, buf1, -1, &rect, DT_CENTER);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		RemoveProp(hwnd, TEXT("IDX"));
		return 0;
	}
	return CallWindowProc(g_staticproc, hwnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char i, j;
	TCHAR buf1[3], buf2[3];
	HDC hdc;
	PAINTSTRUCT ps;

	switch (uMsg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case 1:
			for (i = 0; i < 5; i++)
				for (j = i + 1; j < 6; j++) {
					GetWindowText(g_hwwn[i], buf1, 3);
					GetWindowText(g_hwwn[j], buf2, 3);
					if ((lstrlen(buf1) > 0 ? atoi(buf1) : 99) <= (lstrlen(buf2) > 0 ? atoi(buf2) : 99)) continue;
					SetWindowText(g_hwwn[i], buf2);
					SetWindowText(g_hwwn[j], buf1);
				}
			break;
		}
		return TRUE;
	case WM_PAINT:
		hdc = BeginPaint(hwndDlg, &ps);
		MoveToEx(hdc, 5, 40, NULL);
		LineTo(hdc, 325, 40);
		EndPaint(hwndDlg, &ps);
		return TRUE;
	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		return TRUE;
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	char num[6], i, j;
	FILE *f;
	MSG msg;
	HWND hwnd, hwbtn;

	CreateMutex(NULL, FALSE, TEXT("cwn"));
	if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;

	if ((g_hfont = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FIXED_PITCH, NULL)) == NULL) return 0;

	if ((hwnd = CreateDialog(hInstance, MAKEINTRESOURCE(1), NULL, DialogProc)) == NULL) return 0;

	for (i = 0; i < 6; i++) {
		if ((g_hwwn[i] = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER,
		i * 24 + 10, 10, 22, 19, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
		SendMessage(g_hwwn[i], WM_SETFONT, (WPARAM)g_hfont, FALSE);
		g_editproc = (WNDPROC)GetWindowLong(g_hwwn[i], GWL_WNDPROC);
		SetWindowLong(g_hwwn[i], GWL_WNDPROC, (int)proc_edit);
	}
	if ((g_hwwn[6] = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL, WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER,
	162, 10, 22, 19, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hwwn[6], WM_SETFONT, (WPARAM)g_hfont, FALSE);
	SetWindowLong(g_hwwn[6], GWL_WNDPROC, (int)proc_edit);

	if ((hwbtn = CreateWindow(TEXT("BUTTON"), TEXT("Sort"), WS_CHILD | WS_TABSTOP | WS_VISIBLE,
	197, 10, 40, 19, hwnd, (HMENU)1, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwbtn, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	i = 0;
	if ((f = _tfopen(TEXT("exp.txt"), TEXT("r"))) != NULL) {
		for (; i < 30 && get_number(f, num, 6, 6, 1, 43, 0, 6, 0); i++) {
			for (j = 0; j < 6; j++) {
				if ((g_hwen[i][j] = CreateWindowEx(WS_EX_STATICEDGE, TEXT("STATIC"), NULL, WS_CHILD | WS_VISIBLE | ES_CENTER,
				j * 24 + 10, i * 20 + 50, 22, 15, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
				SetProp(g_hwen[i][j], TEXT("NUM"), (HANDLE)num[j]);
				g_staticproc = (WNDPROC)GetWindowLong(g_hwen[i][j], GWL_WNDPROC);
				SetWindowLong(g_hwen[i][j], GWL_WNDPROC, (int)proc_static1);
			}
			if ((g_hwen[i][6] = CreateWindow(TEXT("STATIC"), NULL, WS_CHILD | WS_VISIBLE,
			158, i * 20 + 51, 10, 15, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
			SetProp(g_hwen[i][6], TEXT("IDX"), (HANDLE)i);
			SetWindowLong(g_hwen[i][6], GWL_WNDPROC, (int)proc_static2);
		}
		fclose(f);
	}
	g_encnt = i;

	if ((g_hwttl = CreateWindow(TEXT("STATIC"), NULL, WS_CHILD | WS_VISIBLE,
	195, 50, 130, 110, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hwttl, WM_SETFONT, (WPARAM)g_hfont, FALSE);
	print_hit_stat();

	SetFocus(g_hwwn[0]);
	ShowWindow(hwnd, SW_SHOWNORMAL);
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (IsDialogMessage(hwnd, &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	DeleteObject(g_hfont);
	return msg.wParam;
}
