#define UNICODE
#define _UNICODE
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <commctrl.h>
#define HIM2PIX_X(him) ((double)(him) / 2540 * GetDeviceCaps(pd.hDC, LOGPIXELSX) + 0.5)
#define HIM2PIX_Y(him) ((double)(him) / 2540 * GetDeviceCaps(pd.hDC, LOGPIXELSY) + 0.5)
#define DRAW_MARK BitBlt(pd.hDC, HIM2PIX_X(posx + gapx * x - 4 * y), \
	HIM2PIX_Y((double)posy + (double)gapy * y + (3.5 * (double)x + 2.5 * (double)y)), \
	mark_width, mark_height, hdc, 0, 0, SRCCOPY)
#define CMDID_NEW 1
#define CMDID_PRINT 2
#define CMDID_LOAD 3
#define CMDID_SAVE 4

bool g_isedit;
HWND g_hwnd_num, g_hwnd_posx, g_hwnd_posy, g_hwnd_gapx, g_hwnd_gapy;
HINSTANCE g_hinst;
HANDLE g_hbmp_sheet, g_hbmp_print;
HFONT g_hfont;

// マークシートクラス。
struct MarkSheet {
	static const char NUM_MIN = 1, NUM_MAX = 43;	// 選択できる数字の範囲。
	static const char NUM_CNT = 6;	// 組み合わせる数字の数。
	static const char NUM_GROUP = 5;	// 1つのマークシートの組み合わせ数字の数。

	static const short WND_WIDTH = 711, WND_HEIGHT = 367;	// ウィンドウサイズ。
	static const short MARK_X = 167, MARK_Y = 76;	// 最初のマークの位置。
	static const char MARK_WIDTH = 5, MARK_HEIGHT = 18;	// マークのサイズ。
	static const char MARK_GAPX = 14, MARK_GAPY = 10;	// マークとマークの間隔。
	static const char MARK_VALID = 3;	// マーク外選択の許容範囲。

	static const char CMDID_ALLCLEAR = 1, CMDID_DELETE = 2;

	// 数字情報。
	struct {
		struct {
			char num[NUM_CNT];	// 組み合わせ数字。
			bool qp;	// クイックピック。
		} group[NUM_GROUP];
		char npn;	// 各口数。
		char persist;	// 継続回数。
	} numdata;

	HWND m_hwnd, m_hwnd_delbtn;

	MarkSheet(HWND, short, short);
	bool setNum(char, char);
	void cmdClick(short, short);
	void cmdAllClear();
	void cmdDelete();
	void Paint(HDC);
	static LRESULT CALLBACK procWnd(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// マークシート管理クラス。
struct ManageMS {
	static const char MAXCNT = 10;	// マークシートを作成できる数。

	char m_mscnt;
	HWND m_hwnd;
	MarkSheet *m_pms[MAXCNT];	// 未使用のマークシートの値はNULL。

	ManageMS(HWND);
	~ManageMS();
	void initMarkSheet();
	void loadMarkSheet(LPCWSTR);
	char getMarkSheet(MarkSheet **, char *);
	void saveMarkSheet(FILE *);
	bool printMarkSheet();
	void setNumText(HWND);
	char checkNum(char, char, char *, char *);
	void selTab();
	void deleteMarkSheet();
	bool isDlgMsg(PMSG);
} *g_pmms;

// 初期化処理。
//   hwnd 親ウィンドウハンドル
//   x 表示X位置
//   y 表示Y位置
MarkSheet::MarkSheet(HWND hwnd, short x, short y) {
	WNDCLASS wc;
	HWND hbtn;

	// ウィンドウクラス取得/登録。
	if (!GetClassInfo(g_hinst, TEXT("pms"), &wc)) {
		wc.style = 0;
		wc.lpfnWndProc = procWnd;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = g_hinst;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(g_hinst, MAKEINTRESOURCE(5));
		wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = TEXT("pms");
		if (RegisterClass(&wc) == 0) return;
	}

	// メインウィンドウ作成。
	if ((m_hwnd = CreateWindow(wc.lpszClassName, NULL, WS_CHILD, x, y,
		WND_WIDTH, WND_HEIGHT, hwnd, NULL, g_hinst, (LPVOID)this)) == NULL) return;

	// [全取消]ボタン作成。
	if ((hbtn = CreateWindow(TEXT("BUTTON"), TEXT("全取消"), WS_CHILD | WS_TABSTOP | WS_VISIBLE, 35, 300,
		55, 22, m_hwnd, (HMENU)CMDID_ALLCLEAR, g_hinst, NULL)) == NULL) return;
	SendMessage(hbtn, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// [破棄]ボタン作成。
	if ((m_hwnd_delbtn = CreateWindow(TEXT("BUTTON"), TEXT("破棄"), WS_CHILD | WS_DISABLED | WS_TABSTOP | WS_VISIBLE, 35, 330,
		55, 22, m_hwnd, (HMENU)CMDID_DELETE, g_hinst, NULL)) == NULL) return;
	SendMessage(m_hwnd_delbtn, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	ZeroMemory(&numdata, sizeof(numdata));	// 数字情報初期化。

	ShowWindow(m_hwnd, SW_SHOWNORMAL);	// メインウィンドウ表示。
}

// 指定した組み合わせ位置に指定した数字を設定する。
// groupが0～4なら、組み合わせ数字の位置がgroupの数字(numが負ならクイックピック)を設定する。
// groupが5なら各口数、6なら継続回数を設定する。
// 指定した数字が既に設定されていた場合はその数字を削除する。
// 設定に成功した場合はtrue、それ以外はfalseを返す。
//   group 組み合わせ位置
//   num 数字
bool MarkSheet::setNum(char group, char num) {
	char i, j, tmp, *pnum1, *pnum2;
	bool flg;

	if (group >= 0 && group <= 4) {	// 組み合わせ数字？
		if (num < 0)	// クイックピック？
			numdata.group[group].qp = !numdata.group[group].qp;
		else {
			if (num < NUM_MIN || num > NUM_MAX || numdata.group[group].qp) return false;
			flg = false;

			// 指定した既に数字が設定されている場合はその数字を削除する。
			for (i = 0; i < NUM_CNT; i++) {
				pnum1 = &numdata.group[group].num[i];
				if (*pnum1 != num) continue;
				*pnum1 = 0;
				flg = true;
				break;
			}

			if (!flg) {
				// 空いている数字を取得し、成功すれば指定した数字を設定する。
				for (i = 0; i < NUM_CNT; i++) {
					pnum1 = &numdata.group[group].num[i];
					if (*pnum1 >= NUM_MIN && *pnum1 <= NUM_MAX) continue;
					*pnum1 = num;
					flg = true;
					break;
				}

				if (!flg) return false;
			}

			// 数字を昇順に並べ替え。
			for (i = 0; i < NUM_CNT - 1; i++)
				for (j = i + 1; j < NUM_CNT; j++) {
					pnum1 = &numdata.group[group].num[i];
					pnum2 = &numdata.group[group].num[j];
					if (*pnum1 <= *pnum2 && *pnum1 >= NUM_MIN && *pnum1 <= NUM_MAX ||
						*pnum2 < NUM_MIN || *pnum2 > NUM_MAX) continue;
					tmp = *pnum1;
					*pnum1 = *pnum2;
					*pnum2 = tmp;
				}
		}
	} else if (group == 5) {	// 各口数？
		if (num < 2 || num > 10) return false;
		numdata.npn = numdata.npn == num ? 0 : num;
	} else {	// 継続回数。
		if (num < 2 || num > 5) return false;
		numdata.persist = numdata.persist == num ? 0 : num;
	}

	g_pmms->setNumText(g_hwnd_num);
	g_isedit = true;
	return true;
}

// クリック時の処理。
//   x クリックしたX位置
//   y クリックしたY位置
void MarkSheet::cmdClick(short x, short y) {
	char group, num, numx, numy;
	TCHAR buf[64];
	RECT rect;

	// クリックした位置から位置インデックスに変換。
	numx = (x - MARK_X + MARK_VALID) / (MARK_WIDTH + MARK_GAPX);
	numy = (y - MARK_Y + MARK_VALID) / (MARK_HEIGHT + MARK_GAPY);
	if (numx < 0 || numx > 27 || numy < 0 || numy > 9) return;

	// クリックした位置が明らかなマーク外ならばreturn。
	if (x < MARK_X - MARK_VALID || x > MARK_X + MARK_WIDTH + numx * (MARK_WIDTH + MARK_GAPX) + MARK_VALID - 1 ||
		y < MARK_Y - MARK_VALID || y > MARK_Y + MARK_HEIGHT + numy * (MARK_HEIGHT + MARK_GAPY) + MARK_VALID - 1)
			return;

	group = numx / NUM_GROUP;
	num = numx % 5 * 10 + numy + 1;	// 数字に変換。

	if (num == 50) {	// 取消？
		wsprintf(buf, TEXT("組合せ%c を取り消しますか？"), group + 'A');
		if (MessageBox(m_hwnd, buf, TEXT("pms"), MB_YESNO | MB_ICONEXCLAMATION) != IDYES) return;

		ZeroMemory(&numdata.group[group].num, sizeof(numdata.group[0].num));
		numdata.group[group].qp = false;
		num = -1;

		g_pmms->setNumText(g_hwnd_num);
		g_isedit = true;
	} else {
		if (num == 47) num = -1;	// クイックピックの場合。

		if (group < 5) {	// 組み合わせ数字？
			if (!setNum(group, num)) return;
		} else {
			if (num >= 12 && num <= 20) {	// 各口数？
				if (!setNum(5, num - 10)) return;
			} else if (num >= 22 && num <= 25)	// 継続回数？
				if (!setNum(6, num - 20)) return;
		}
	}

	// 再描画を指示。
	if (group == 5) {
		// 各口数か継続回数の場合は、その部分全体。
		rect.left = MARK_X + (MARK_WIDTH + MARK_GAPX) * 26;
		rect.top = MARK_Y + (MARK_HEIGHT + MARK_GAPY);
		rect.right = rect.left + (MARK_WIDTH + MARK_GAPX) * 2;
		rect.bottom = rect.top + (MARK_HEIGHT + MARK_GAPY) * 9;
	} else if (num < 0) {
		// クイックピックか取消の場合は、組み合わせ数字全体。
		rect.left = MARK_X + (MARK_WIDTH + MARK_GAPX) * group * 5;
		rect.top = MARK_Y;
		rect.right = rect.left + (MARK_WIDTH + MARK_GAPX) * 5;
		rect.bottom = rect.top + (MARK_HEIGHT + MARK_GAPY) * 10;
	} else {
		// 指定した数字のみ。
		rect.left = MARK_X + (MARK_WIDTH + MARK_GAPX) * numx;
		rect.top = MARK_Y + (MARK_HEIGHT + MARK_GAPY) * numy;
		rect.right = rect.left + MARK_WIDTH;
		rect.bottom = rect.top + MARK_HEIGHT;
	}
	InvalidateRect(m_hwnd, &rect, false);
}

// [全取消]の処理。
void MarkSheet::cmdAllClear() {
	RECT rect;

	if (MessageBox(m_hwnd, TEXT("このシートのすべての数字を取り消しますか？"),
		TEXT("pms"), MB_YESNO | MB_ICONEXCLAMATION) != IDYES) return;

	ZeroMemory(&numdata, sizeof(numdata));	// 数字情報初期化。
	g_pmms->setNumText(g_hwnd_num);

	// 全体的に再描画を指示。
	rect.left = MARK_X;
	rect.top = MARK_Y;
	rect.right = rect.left + (MARK_WIDTH + MARK_GAPX) * 28;
	rect.bottom = rect.top + (MARK_HEIGHT + MARK_GAPY) * 10;
	InvalidateRect(m_hwnd, &rect, false);
}

// [破棄]の処理。
void MarkSheet::cmdDelete() {
	if (MessageBox(m_hwnd, TEXT("このシートを破棄しますか？"),
		TEXT("pms"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES) g_pmms->deleteMarkSheet();
}

// 描画処理。
//   hdc 描画対象のデバイスコンテキスト
void MarkSheet::Paint(HDC hdc) {
	char i, j;
	RECT rect = {0, 0, MARK_WIDTH, MARK_HEIGHT};
	PAINTSTRUCT ps;
	HDC hdcmem = CreateCompatibleDC(hdc);
	HBRUSH hbrush;
	HBITMAP hbm;

	// メイン画像。
	SelectObject(hdcmem, g_hbmp_sheet);
	BitBlt(hdc, 0, 0, WND_WIDTH, WND_HEIGHT, hdcmem, 0, 0, SRCCOPY);

	// マーク。
	hbm = CreateCompatibleBitmap(hdc, MARK_WIDTH, MARK_HEIGHT);
	SelectObject(hdcmem, hbm);
	hbrush = CreateSolidBrush(RGB(64, 64, 64));
	FillRect(hdcmem, &rect, hbrush);
	DeleteObject(hbrush);

	for (i = 0; i < NUM_GROUP; i++) {
		for (j = 0; j < NUM_CNT; j++) {
			if (numdata.group[i].qp)
				// クイックピック。
				BitBlt(hdc,
					MARK_X + (MARK_WIDTH + MARK_GAPX) * i * 5 + (MARK_WIDTH + MARK_GAPX) * 4,
					MARK_Y + (MARK_HEIGHT + MARK_GAPY) * 6, MARK_WIDTH, MARK_HEIGHT, hdcmem, 0, 0, SRCAND
				);
			else if (numdata.group[i].num[j] >= 1 && numdata.group[i].num[j] <= 43)
				// 組み合わせ数字。
				BitBlt(hdc,
					MARK_X + (MARK_WIDTH + MARK_GAPX) * i * 5 + (MARK_WIDTH + MARK_GAPX) * ((numdata.group[i].num[j] - 1) / 10),
					MARK_Y + (MARK_HEIGHT + MARK_GAPY) * ((numdata.group[i].num[j] - 1) % 10),
					MARK_WIDTH, MARK_HEIGHT, hdcmem, 0, 0, SRCAND
				);
		}

		if (numdata.npn >= 2 && numdata.npn <= 10)
			// 各口数。
			BitBlt(hdc, MARK_X + (MARK_WIDTH + MARK_GAPX) * 26,
				MARK_Y + (MARK_HEIGHT + MARK_GAPY) * (numdata.npn - 1), MARK_WIDTH, MARK_HEIGHT, hdcmem, 0, 0, SRCAND
			);

		if (numdata.persist >= 2 && numdata.persist <= 5)
			// 継続回数。
			BitBlt(hdc, MARK_X + (MARK_WIDTH + MARK_GAPX) * 27,
				MARK_Y + (MARK_HEIGHT + MARK_GAPY) * (numdata.persist - 1), MARK_WIDTH, MARK_HEIGHT, hdcmem, 0, 0, SRCAND
			);
	}
	DeleteObject(hbm);

	DeleteDC(hdcmem);
}

// ウィンドウプロシージャ。
LRESULT CALLBACK MarkSheet::procWnd(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc;
	MarkSheet *pms = (MarkSheet *)GetWindowLong(hwnd, GWL_USERDATA);

	switch (uMsg) {
	case WM_CREATE:
		pms = (MarkSheet *)((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLong(hwnd, GWL_USERDATA, (LONG)pms);
		return 0;
	case WM_LBUTTONDOWN:
		pms->cmdClick((short)LOWORD(lParam), (short)HIWORD(lParam));
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case CMDID_ALLCLEAR:
			pms->cmdAllClear();
			break;
		case CMDID_DELETE:
			pms->cmdDelete();
			break;
		}
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		pms->Paint(hdc);
		EndPaint(hwnd, &ps);
		return 0;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// 初期化処理。
//   hwnd 親ウィンドウハンドル
ManageMS::ManageMS(HWND hwnd) {
	TCITEM tci;

	// タブウィンドウ作成。
	if ((m_hwnd = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_VISIBLE,
		5, 5, 722, 397, hwnd, NULL, g_hinst, NULL)) == NULL) return;
	SendMessage(m_hwnd, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// タブ初期化。
	tci.mask = TCIF_TEXT;
	tci.pszText = TEXT("カード 1");
	TabCtrl_InsertItem(m_hwnd, 0, &tci);
	tci.pszText = TEXT("(新しいカード)");
	TabCtrl_InsertItem(m_hwnd, 1, &tci);

	// マークシート初期化。
	ZeroMemory(&m_pms, sizeof(m_pms));
	m_pms[0] = new MarkSheet(m_hwnd, 5, 25);
	m_mscnt = 1;
}

// 破棄処理。
ManageMS::~ManageMS() {
	char i;

	for (i = 0; i < m_mscnt; i++) delete m_pms[i];
}

// すべてのマークシートを初期化する。
void ManageMS::initMarkSheet() {
	while (m_mscnt > 1) deleteMarkSheet();
	ZeroMemory(&m_pms[0]->numdata, sizeof(m_pms[0]->numdata));	// 数字情報初期化。
	RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
	setNumText(g_hwnd_num);
}

// ファイルからすべてのマークシートを読み込む。
//   pfn ファイル名
void ManageMS::loadMarkSheet(LPCWSTR pfn) {
	static const char *BLANK_CHR = "\t\n\v\f\r ", *NUM_CHR = "0123456789";
	char buf[1024], group, ret = 1, i, j, k, *pnum1, *pnum2, *pbuf, *plen, *pchk;
	int val;
	bool group_cnt_flg;
	FILE *pf;
	MarkSheet *pms;

	if ((pf = _wfopen(pfn, L"r")) == NULL) {
		MessageBox(m_hwnd, TEXT("読み込みに失敗しました。他のファイル名を選択してください。"),
			TEXT("pms"), MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	if (g_isedit && MessageBox(m_hwnd, TEXT("すべてのシートの数字を取り消しますか？"),
		TEXT("pms"), MB_YESNO | MB_ICONEXCLAMATION) != IDYES) {
		fclose(pf);
		return;
	}

	initMarkSheet();
	pms = m_pms[0];
	group = 0;

	for (;;) {
		if (fgets(buf, sizeof(buf) / sizeof(buf[0]), pf) == NULL) break;
		i = 0;
		group_cnt_flg = false;

		for (pbuf = buf, plen = buf + strlen(buf); ; ) {
			if (i == 0) {
				pbuf += strspn(pbuf, BLANK_CHR);
				if (pbuf >= plen) break;

				if (strncmp(pbuf, "|", 1) == 0) {
					// 無条件で次の組み合わせ数字へ。
					if ((ret = getMarkSheet(&pms, &group)) != 1) goto procend;
					group_cnt_flg = true;
					break;
				} else if (strnicmp(pbuf, "QP", 2) == 0) {
					if ((ret = getMarkSheet(&pms, &group)) != 1) goto procend;
					pms->numdata.group[group].qp = true;	// クイックピック。
					group_cnt_flg = true;
					break;
				} else if (strnicmp(pbuf, "N", 1) == 0) {
					// 各口数。
					pbuf += strcspn(pbuf + 1, NUM_CHR);
					if (pbuf >= plen) break;
					val = strtol(pbuf, &pchk, 10);
					if (val > 10) pms->numdata.npn = 10;
					else if (val >= 2) pms->numdata.npn = val;

					if (pchk == NULL) break;
					pbuf = pchk;
					if (pbuf >= plen) break;

					// 継続回数。
					pbuf += strcspn(pbuf, NUM_CHR);
					if (pbuf >= plen) break;
					val = strtol(pbuf, &pchk, 10);
					if (val > 5) pms->numdata.persist = 5;
					else if (val >= 2) pms->numdata.persist = val;

					break;
				}
			}

			pbuf += strcspn(pbuf, NUM_CHR);
			if (pbuf >= plen) break;

			// 数字。
			val = strtol(pbuf, &pchk, 10);
			if (val >= MarkSheet::NUM_MIN && val <= MarkSheet::NUM_MAX) {
				for (j = 0; j < i && pms->numdata.group[group].num[j] != val; j++);	// 重複チェック。

				if (j >= i) {	// 重複はないか？
					if ((ret = getMarkSheet(&pms, &group)) != 1) goto procend;
					pms->numdata.group[group].num[i++] = val;
					group_cnt_flg = true;
					if (i >= MarkSheet::NUM_CNT) break;
				}
			}

			if (pchk == NULL) break;
			pbuf = pchk;
			if (pbuf >= plen) break;
		}

		if (!group_cnt_flg) continue;

		// 数字を昇順に並べ替え。
		if (!pms->numdata.group[group].qp)
			for (j = 0; j < i - 1; j++)
				for (k = j + 1; k < i; k++) {
					pnum1 = &pms->numdata.group[group].num[j];
					pnum2 = &pms->numdata.group[group].num[k];
					if (*pnum1 <= *pnum2) continue;
					val = *pnum1;
					*pnum1 = *pnum2;
					*pnum2 = val;
				}

		group++;
	}

procend:
	fclose(pf);

	TabCtrl_SetCurSel(m_hwnd, 0);
	selTab();
	g_isedit = false;

	if (ret == 0)
		MessageBox(m_hwnd, TEXT("読み込みエラーが発生しました。アプリケーションを再起動してみてください。"),
			TEXT("pms"), MB_OK | MB_ICONEXCLAMATION);
	else if (ret == -1)
		MessageBox(m_hwnd, TEXT("組み合わせ数字は途中から無視されました。"),
			TEXT("pms"), MB_OK | MB_ICONEXCLAMATION);
}

// 基本的には何もしないが、状況に応じて新しいマークシートを設定する。
// 返値が1ならOK、0ならエラー、-1ならマークシート作成不可。
//   ppms マークシート
//   pgroup 組み合わせ数字のインデックス
char ManageMS::getMarkSheet(MarkSheet **ppms, char *pgroup) {
	MarkSheet *pms;

	if (*pgroup < MarkSheet::NUM_GROUP) return 1;
	if (m_mscnt >= MAXCNT) return -1;

	// 新しいマークシートを作成し、そのマークシートを設定する。
	TabCtrl_SetCurSel(m_hwnd, m_mscnt);
	selTab();
	if ((pms = m_pms[m_mscnt - 1]) == NULL) return 0;
	*ppms = pms;
	*pgroup = 0;
	return 1;
}

// すべてのマークシートをファイルに書き込む。
//   pf ファイル
void ManageMS::saveMarkSheet(FILE *pf) {
	char i, j, k, *pnum;
	MarkSheet *pms;

	for (i = 0; i < m_mscnt; i++) {
		pms = m_pms[i];

		for (j = 0; j < MarkSheet::NUM_GROUP; j++) {
			if (pms->numdata.group[j].qp)
				_ftprintf(pf, TEXT("QP"));
			else
				// 数字。
				for (k = 0; k < MarkSheet::NUM_CNT; k++) {
					if (k > 0) _ftprintf(pf, TEXT("|"));

					pnum = &pms->numdata.group[j].num[k];
					if (*pnum < MarkSheet::NUM_MIN || *pnum > MarkSheet::NUM_MAX) {
						if (k < MarkSheet::NUM_CNT - 1) _ftprintf(pf, TEXT("  "));
					} else
						_ftprintf(pf, TEXT("%2d"), *pnum);
				}

			_ftprintf(pf, TEXT("\n"));
		}

		// 各口数と継続回数。
		_ftprintf(pf, TEXT("N %d %d"),
			pms->numdata.npn > 0 ? pms->numdata.npn : 1,
			pms->numdata.persist > 0 ? pms->numdata.persist : 1
		);

		if (i < m_mscnt - 1) _ftprintf(pf, TEXT("\n"));
	}
}

// すべてのマークシートを印刷する。
// 成功ならtrue、それ以外はfalseを返す。
bool ManageMS::printMarkSheet() {
	char buf[8], cnt, x, y, i, j, k, same_mscnt, same_group, *pnum;
	short mark_x, mark_y, mark_width, mark_height;
	int posx, posy, gapx, gapy, idx;
	bool flg;
	RECT rect;
	HDC hdc = NULL;
	HBITMAP hbm = NULL;
	HBRUSH hbrush;
	PRINTDLG pd;
	DOCINFO di;
	DEVMODE *pdm = NULL;
	MarkSheet *pms;

	// すべてのマークシートの印刷可能な組み合わせ数字があるかチェック。
	k = 0;
	for (i = 0; i < m_mscnt; i++) {
		for (j = 0; j < MarkSheet::NUM_GROUP; j++) {
			if (checkNum(i, j, &same_mscnt, &same_group) == 0) continue;
			k = 1;
			break;
		}
		if (k == 1) break;
	}
	if (k != 1) {
		MessageBox(m_hwnd, TEXT("印刷可能な組み合わせ数字がありません。有効な組み合わせ数字をマークしてください。"),
			TEXT("pms"), MB_OK | MB_ICONEXCLAMATION);
		return true;
	}

	// プリンタダイアログ表示。
	ZeroMemory(&pd, sizeof(pd));
	pd.lStructSize = sizeof(PRINTDLG);
	pd.hwndOwner = m_hwnd;
	pd.hDevMode = NULL;
	pd.hDevNames = NULL;
	pd.hDC = NULL;
	pd.Flags = PD_NOPAGENUMS | PD_RETURNDC;
	if (!PrintDlg(&pd)) return true;

	flg = false;

	if (pd.Flags & PD_SELECTION) {	// 選択しているマークシートのみ印刷か？
		idx = TabCtrl_GetCurSel(m_hwnd);
		if (idx < 0 || idx > m_mscnt) goto procend;
		cnt = idx + 1;

		// 選択しているマークシートの印刷可能な組み合わせ数字があるかチェック。
		i = 0;
		for (j = 0; j < MarkSheet::NUM_GROUP; j++) {
			if (checkNum(idx, j, &same_mscnt, &same_group) == 0) continue;
			i = 1;
			break;
		}
		if (i != 1) {
			MessageBox(m_hwnd, TEXT("選択したマークシートに印刷可能な組み合わせ数字がありません。有効な組み合わせ数字をマークしてください。"),
				TEXT("pms"), MB_OK | MB_ICONEXCLAMATION);
			flg = true;
			goto procend;
		}
	} else {
		idx = 0;
		cnt = m_mscnt;
	}

	// プリンタデバイスコンテキスト再設定。
	if ((pdm = (DEVMODE *)GlobalLock(pd.hDevMode)) == NULL) goto procend;
	pdm->dmOrientation = DMORIENT_PORTRAIT;
	pdm->dmPaperSize = 0;
	pdm->dmPaperLength = 1610;
	pdm->dmPaperWidth = 825;
	pdm->dmColor = DMCOLOR_MONOCHROME;
	if (ResetDC(pd.hDC, pdm) == NULL) goto procend;
	GlobalUnlock(pd.hDevMode);
	pdm = NULL;

	hdc = CreateCompatibleDC(pd.hDC);

	mark_width = HIM2PIX_X(400);
	mark_height = HIM2PIX_Y(100);

	// マーク。
	hbm = CreateCompatibleBitmap(pd.hDC, mark_width, mark_height);
	SelectObject(hdc, hbm);
	hbrush = CreateSolidBrush(RGB(0, 0, 0));
	rect.left = 0;
	rect.top = 0;
	rect.right = mark_width;
	rect.bottom = mark_height;
	FillRect(hdc, &rect, hbrush);
	DeleteObject(hbrush);

	// マークの印刷設定取得。
	GetWindowTextA(g_hwnd_posx, buf, sizeof(buf) / sizeof(buf[0]));
	posx = atoi(buf);
	GetWindowTextA(g_hwnd_posy, buf, sizeof(buf) / sizeof(buf[0]));
	posy = atoi(buf);
	GetWindowTextA(g_hwnd_gapx, buf, sizeof(buf) / sizeof(buf[0]));
	gapx = atoi(buf);
	GetWindowTextA(g_hwnd_gapy, buf, sizeof(buf) / sizeof(buf[0]));
	gapy = atoi(buf);

	// 印刷。
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = TEXT("NuSTL Mark sheet");
	di.lpszOutput = NULL;
	di.lpszDatatype = NULL;
	di.fwType = 0;
	if (StartDoc(pd.hDC, &di) <= 0) goto procend;
	flg = true;
	for (i = idx; i < cnt; i++) {
		k = 0;
		for (j = 0; j < MarkSheet::NUM_GROUP; j++) {
			if (checkNum(i, j, &same_mscnt, &same_group) == 0) continue;
			k = 1;
			break;
		}
		if (k != 1) continue;	// 印刷可能な組み合わせ数字がない場合は次のマークシートへ。

		if (StartPage(pd.hDC) <= 0) {
			flg = false;
			break;
		}

		for (j = 0; j < MarkSheet::NUM_GROUP; j++) {
			if (checkNum(i, j, &same_mscnt, &same_group) == 0) continue;

			pms = m_pms[i];
			if (pms->numdata.group[j].qp) {	// クイックピック？
				x = 3;
				y = 4 + j * 5;
				DRAW_MARK;
			} else	// 数字。
				for (k = 0; k < MarkSheet::NUM_CNT; k++) {
					pnum = &pms->numdata.group[j].num[k];
					x = 9 - (*pnum - 1) % 10;
					y = (*pnum - 1) / 10 + j * 5;
					DRAW_MARK;
				}
		}
		if (pms->numdata.npn >= 2 && pms->numdata.npn <= 10) {
			// 各口数。
			x = 10 - pms->numdata.npn;
			y = 26;
			DRAW_MARK;
		}
		if (pms->numdata.persist >= 2 && pms->numdata.persist <= 10) {
			// 継続回数。
			x = 10 - pms->numdata.persist;
			y = 27;
			DRAW_MARK;
		}

		if (EndPage(pd.hDC) <= 0) {
			flg = false;
			break;
		}
	}
	if (EndDoc(pd.hDC) <= 0) flg = false;

procend:
	if (pdm != NULL) GlobalUnlock(pd.hDevMode);
	if (hbm != NULL) DeleteObject(hbm);
	if (hdc != NULL) DeleteDC(hdc);
	if (pd.hDC != NULL) DeleteDC(pd.hDC);
	return flg;
}

// 指定したウィンドウに組み合わせ数字を表示する。
//   hwnd 組み合わせ数字を表示するウィンドウハンドル
void ManageMS::setNumText(HWND hwnd) {
	char chk, same_mscnt, same_group;
	int idx = TabCtrl_GetCurSel(m_hwnd), money, num, i, j, k;
	TCHAR numstr[512], buf[64];
	MarkSheet *pms;

	if (idx < 0 || idx >= m_mscnt) return;
	money = 0;
	numstr[0] = TEXT('\0');

	for (i = 0; i < m_mscnt; i++) {
		pms = m_pms[i];
		num = 0;

		for (j = 0; j < MarkSheet::NUM_GROUP; j++) {
			if ((chk = checkNum(i, j, &same_mscnt, &same_group)) == 0) continue;
			num++;
			if (i != idx) continue;

			if (pms->numdata.group[j].qp)
				lstrcat(numstr, TEXT("QP"));
			else {
				// 数字。
				for (k = 0; k < MarkSheet::NUM_CNT; k++) {
					if (k > 0) lstrcat(numstr, TEXT("-"));
					wsprintf(buf, TEXT("%02d"), pms->numdata.group[j].num[k]);
					lstrcat(numstr, buf);
				}

				if (chk == -1) {	// 重複？
					wsprintf(buf, TEXT(" (カード%dの組合せ%cと重複)"), same_mscnt + 1, same_group + 'A');
					lstrcat(numstr, buf);
				}
			}

			lstrcat(numstr, TEXT("\n"));
		}

		// 金額。
		if (num > 0) {
			j = pms->numdata.npn > 0 ? pms->numdata.npn : 1;
			k = pms->numdata.persist > 0 ? pms->numdata.persist : 1;
			num = num * j * k * 200;
			money += num;

			if (i != idx) continue;
			wsprintf(buf, TEXT("\n金額：\\%d\n"), num);
			lstrcat(numstr, buf);
		}
	}
	if (money > 0) {
		wsprintf(buf, TEXT("※申込み枠数×各口数×継続回数×単価\n\n総金額：\\%d"), money);
		lstrcat(numstr, buf);
	}

	SetWindowText(hwnd, numstr);
}

// 指定した組み合わせ数字が重複なく設定されているかチェック。
// 返値が1ならOK、0ならNG、-1ならOKだが重複。
//   mscnt マークシートのインデックス
//   group 組み合わせ数字のインデックス
//   psame_mscnt 重複時のマークシートのインデックス(重複していない時の値は未定義)
//   psame_group 重複時の組み合わせ数字のインデックス(重複していない時の値は未定義)
char ManageMS::checkNum(char mscnt, char group, char *psame_mscnt, char *psame_group) {
	char i, j, k, *pnum;
	bool flg = true;
	MarkSheet *pms = m_pms[mscnt];

	if (pms->numdata.group[group].qp) return 1;

	// 数字が全部設定されているかチェック。
	for (i = 0; i < MarkSheet::NUM_CNT; i++) {
		pnum = &pms->numdata.group[group].num[i];
		if (*pnum >= MarkSheet::NUM_MIN && *pnum <= MarkSheet::NUM_MAX) continue;
		flg = false;
		break;
	}
	if (!flg) return 0;

	// 指定した位置までのすべての組み合わせ数字との重複チェック。
	for (i = 0; i <= mscnt; i++)
		for (j = 0; j < (i < mscnt ? MarkSheet::NUM_GROUP : group); j++) {
			flg = false;
			for (k = 0; k < MarkSheet::NUM_CNT; k++) {
				if (pms->numdata.group[group].num[k] == m_pms[i]->numdata.group[j].num[k]) continue;
				flg = true;
				break;
			}
			if (flg) continue;

			*psame_mscnt = i;
			*psame_group = j;
			return -1;
		}

	return 1;
}

// 異なるタブを選択した時の処理。
void ManageMS::selTab() {
	int idx = TabCtrl_GetCurSel(m_hwnd), i;
	TCHAR buf[16];
	TCITEM tci;

	if (idx < 0) return;

	if (idx < m_mscnt) {
		// 選択したマークシートのみを表示する。
		for (i = 0; i < m_mscnt; i++) ShowWindow(m_pms[i]->m_hwnd, SW_HIDE);
		ShowWindow(m_pms[idx]->m_hwnd, SW_SHOWNORMAL);
	} else {	// 追加処理。
		if (m_mscnt >= MAXCNT) return;

		// 追加したマークシートのみを表示する。
		for (i = 0; i < m_mscnt; i++) ShowWindow(m_pms[i]->m_hwnd, SW_HIDE);
		m_pms[m_mscnt] = new MarkSheet(m_hwnd, 5, 25);
		if (m_mscnt == 1) EnableWindow(m_pms[0]->m_hwnd_delbtn, true);
		if (m_mscnt > 0) EnableWindow(m_pms[m_mscnt]->m_hwnd_delbtn, true);

		wsprintf(buf, TEXT("カード %d"), m_mscnt + 1);
		tci.mask = TCIF_TEXT;
		tci.pszText = buf;
		TabCtrl_SetItem(m_hwnd, m_mscnt, &tci);

		g_isedit = true;

		if (++m_mscnt >= MAXCNT) return;
		tci.pszText = TEXT("(新しいカード)");
		TabCtrl_InsertItem(m_hwnd, m_mscnt, &tci);
		RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
	}
	setNumText(g_hwnd_num);
}

// 選択されているマークシートを削除する。
void ManageMS::deleteMarkSheet() {
	int idx = TabCtrl_GetCurSel(m_hwnd), i;
	TCHAR buf[16];
	TCITEM tci;

	if (idx < 0 || idx >= m_mscnt) return;

	// 削除。
	DestroyWindow(m_pms[idx]->m_hwnd);
	delete m_pms[idx];
	TabCtrl_DeleteItem(m_hwnd, idx);

	// 削除後、データを整える。
	m_mscnt--;
	tci.mask = TCIF_TEXT;
	for (i = idx; i < m_mscnt; i++) {
		m_pms[i] = m_pms[i + 1];

		wsprintf(buf, TEXT("カード %d"), i + 1);
		tci.pszText = buf;
		TabCtrl_SetItem(m_hwnd, i, &tci);
	}
	m_pms[m_mscnt] = NULL;

	// 新しいマークシートが作成できない状態で削除された時の対処。
	if (m_mscnt == MAXCNT - 1) {
		tci.pszText = TEXT("(新しいカード)");
		TabCtrl_InsertItem(m_hwnd, m_mscnt, &tci);
	}

	if (m_mscnt == 1) EnableWindow(m_pms[0]->m_hwnd_delbtn, false);
	TabCtrl_SetCurSel(m_hwnd, idx > 0 ? idx - 1 : 0);
	selTab();
	g_isedit = true;
}

// ダイアログメッセージを処理した場合はtrueを返す。
bool ManageMS::isDlgMsg(PMSG pmsg) {
	char i;
	bool is_dlgmsg = false;

	for (i = 0; i < m_mscnt; i++)
		if (IsDialogMessage(m_pms[i]->m_hwnd, pmsg)) is_dlgmsg = true;
	return is_dlgmsg;
}

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char chk;
	FILE *pf;
	WCHAR fn[MAX_PATH] = {L'\0'};
	PAINTSTRUCT ps;
	HDC hdc, hdcmem;
	NMHDR *pnmh;
	static OPENFILENAMEW ofn;

	switch (uMsg) {
	case WM_INITDIALOG:
		ofn.lStructSize = sizeof(OPENFILENAMEW);
		ofn.lpstrFilter = L"すべてのファイル (*.*)\0*.*\0組み合わせ数字ファイル (*.txt)\0*.txt\0";
		ofn.nMaxFile = sizeof(fn) / sizeof(fn[0]);
		ofn.lpstrDefExt = L"txt";
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case CMDID_NEW:	// 新規作成。
			if (g_isedit && MessageBox(hwndDlg, TEXT("すべてのシートの数字を取り消しますか？"),
				TEXT("pms"), MB_YESNO | MB_ICONEXCLAMATION) != IDYES) break;
				g_pmms->initMarkSheet();
				g_isedit = false;
			break;
		case CMDID_PRINT:	// 印刷。
			if (!g_pmms->printMarkSheet())
				MessageBox(hwndDlg, TEXT("印刷エラーが発生しました。再度印刷し直してみてください。"),
					TEXT("pms"), MB_OK | MB_ICONEXCLAMATION);
			break;
		case CMDID_LOAD:	// 読み込み。
			ofn.hwndOwner = hwndDlg;
			ofn.Flags = OFN_FILEMUSTEXIST;
			ofn.lpstrFile = fn;
			if (GetOpenFileNameW(&ofn)) g_pmms->loadMarkSheet(fn);
			break;
		case CMDID_SAVE:	// 名前を付けて保存。
			ofn.hwndOwner = hwndDlg;
			ofn.Flags = OFN_OVERWRITEPROMPT;
			ofn.lpstrFile = fn;
			if (!GetSaveFileName(&ofn)) break;

			if ((pf = _tfopen(fn, TEXT("w"))) == NULL) {
				MessageBox(hwndDlg, TEXT("保存に失敗しました。ファイル名を変更してください。"),
					TEXT("pms"), MB_OK | MB_ICONEXCLAMATION);
				break;
			}
			g_pmms->saveMarkSheet(pf);
			g_isedit = false;
			fclose(pf);
			break;
		}
		return TRUE;
	case WM_NOTIFY:
		pnmh = (NMHDR *)lParam;
		if (pnmh->hwndFrom == g_pmms->m_hwnd && pnmh->code == TCN_SELCHANGE) g_pmms->selTab();
		return TRUE;
	case WM_PAINT:
		hdc = BeginPaint(hwndDlg, &ps);
		hdcmem = CreateCompatibleDC(hdc);
		SelectObject(hdcmem, g_hbmp_print);
		BitBlt(hdc, 380, 407, 41, 132, hdcmem, 0, 0, SRCCOPY);
		DeleteDC(hdcmem);
		EndPaint(hwndDlg, &ps);
		return TRUE;
	case WM_DROPFILES:
		DragQueryFileW((HDROP)wParam, 0, fn, sizeof(fn) / sizeof(fn[0]));
		g_pmms->loadMarkSheet(fn);
		return TRUE;
	case WM_CLOSE:
		if (!g_isedit || MessageBox(hwndDlg, TEXT("変更内容は保存されていません。終了しますか？"),
			TEXT("pms"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES) DestroyWindow(hwndDlg);
		return TRUE;
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int argc;
	BOOL ret;
	LPWSTR *ppargv;
	WNDCLASS wc;
	MSG msg;
	HWND hwnd, hwnd_tmp;
	HANDLE hicon;

	// 多重起動防止。
	CreateMutex(NULL, FALSE, TEXT("pms"));
	if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;

	// フォント作成。
	if ((g_hfont = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, NULL)) == NULL) return 0;

	g_hinst = hInstance;
	g_hbmp_sheet = LoadImage(hInstance, MAKEINTRESOURCE(2), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_LOADMAP3DCOLORS);
	g_hbmp_print = LoadImage(hInstance, MAKEINTRESOURCE(3), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_LOADMAP3DCOLORS);

	InitCommonControls();	// コモンコントロール初期化。

	// トップウィンドウ作成。
	if ((hwnd = CreateDialog(hInstance, MAKEINTRESOURCE(1), NULL, DialogProc)) == NULL) return 0;

	// [新規作成]ボタン作成。
	if ((hwnd_tmp = CreateWindow(TEXT("BUTTON"), TEXT("新規作成"), WS_CHILD | WS_TABSTOP | WS_VISIBLE,
		5, 407, 130, 40, hwnd, (HMENU)CMDID_NEW, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// [印刷]ボタン作成。
	if ((hwnd_tmp = CreateWindow(TEXT("BUTTON"), TEXT("印刷"), WS_CHILD | WS_TABSTOP | WS_VISIBLE,
		145, 407, 130, 40, hwnd, (HMENU)CMDID_PRINT, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// [読み込み]ボタン作成。
	if ((hwnd_tmp = CreateWindow(TEXT("BUTTON"), TEXT("読み込み"), WS_CHILD | WS_TABSTOP | WS_VISIBLE,
		5, 457, 130, 40, hwnd, (HMENU)CMDID_LOAD, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// [名前を付けて保存]ボタン作成。
	if ((hwnd_tmp = CreateWindow(TEXT("BUTTON"), TEXT("名前を付けて保存"), WS_CHILD | WS_TABSTOP | WS_VISIBLE,
		145, 457, 130, 40, hwnd, (HMENU)CMDID_SAVE, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	//--------------------------------------------------

	// 「印刷位置」作成。
	if ((hwnd_tmp = CreateWindow(TEXT("STATIC"), TEXT("印刷位置"), WS_CHILD | WS_VISIBLE,
		280, 407, 60, 15, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// マークの印刷開始X位置設定テキスト作成。
	if ((g_hwnd_posx = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("200"),
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER, 280, 420, 40, 18, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hwnd_posx, WM_SETFONT, (WPARAM)g_hfont, FALSE);
	SendMessage(g_hwnd_posx, EM_SETLIMITTEXT, 4, 0);

	// マークの印刷開始Y位置設定テキスト作成。
	if ((g_hwnd_posy = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("3630"),
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER, 322, 420, 40, 18, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hwnd_posy, WM_SETFONT, (WPARAM)g_hfont, FALSE);
	SendMessage(g_hwnd_posy, EM_SETLIMITTEXT, 4, 0);

	// 「印刷間隔」作成。
	if ((hwnd_tmp = CreateWindow(TEXT("STATIC"), TEXT("印刷間隔"), WS_CHILD | WS_VISIBLE,
		280, 445, 60, 15, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// マークの印刷X間隔設定テキスト作成。
	if ((g_hwnd_gapx = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("635"),
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER, 280, 458, 40, 18, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hwnd_gapx, WM_SETFONT, (WPARAM)g_hfont, FALSE);
	SendMessage(g_hwnd_gapx, EM_SETLIMITTEXT, 4, 0);

	// マークの印刷Y間隔設定テキスト作成。
	if ((g_hwnd_gapy = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("420"),
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER, 322, 458, 40, 18, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hwnd_gapy, WM_SETFONT, (WPARAM)g_hfont, FALSE);
	SendMessage(g_hwnd_gapy, EM_SETLIMITTEXT, 4, 0);

	//--------------------------------------------------

	// 印刷時の注意文作成。
	if ((hwnd_tmp = CreateWindow(TEXT("STATIC"), TEXT("※印刷の標準の用紙の向きは図の通りです"),
		WS_CHILD | WS_VISIBLE | SS_RIGHT, 175, 543, 250, 20, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// 情報ウィンドウ作成。
	if ((g_hwnd_num = CreateWindow(TEXT("STATIC"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE,
		427, 407, 300, 150, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hwnd_num, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	g_pmms = new ManageMS(hwnd);	// マークシート管理ウィンドウ作成。

	hicon = LoadImage(hInstance, MAKEINTRESOURCE(4), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hicon);

	ShowWindow(hwnd, SW_SHOWNORMAL);	// トップウィンドウ表示。

	// コマンドライン引数がある場合、組み合わせ数字ファイルをコマンドラインから読み込む。
	ppargv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argc >= 2) g_pmms->loadMarkSheet(ppargv[1]);
	LocalFree(ppargv);

	// メッセージループ。
	while (ret = GetMessage(&msg, NULL, 0, 0)) {
		if (ret == -1) break;

		// ボタンのフォーカス等を適切に処理し(子ウィンドウ優先にすること)、メッセージを処理する。
		if (g_pmms->isDlgMsg(&msg) || IsDialogMessage(hwnd, &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	DeleteObject(g_hfont);
	DeleteObject(g_hbmp_sheet);
	DeleteObject(g_hbmp_print);
	DeleteObject(hicon);
	delete g_pmms;
	return msg.wParam;
}
