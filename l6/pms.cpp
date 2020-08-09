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

// �}�[�N�V�[�g�N���X�B
struct MarkSheet {
	static const char NUM_MIN = 1, NUM_MAX = 43;	// �I���ł��鐔���͈̔́B
	static const char NUM_CNT = 6;	// �g�ݍ��킹�鐔���̐��B
	static const char NUM_GROUP = 5;	// 1�̃}�[�N�V�[�g�̑g�ݍ��킹�����̐��B

	static const short WND_WIDTH = 711, WND_HEIGHT = 367;	// �E�B���h�E�T�C�Y�B
	static const short MARK_X = 167, MARK_Y = 76;	// �ŏ��̃}�[�N�̈ʒu�B
	static const char MARK_WIDTH = 5, MARK_HEIGHT = 18;	// �}�[�N�̃T�C�Y�B
	static const char MARK_GAPX = 14, MARK_GAPY = 10;	// �}�[�N�ƃ}�[�N�̊Ԋu�B
	static const char MARK_VALID = 3;	// �}�[�N�O�I���̋��e�͈́B

	static const char CMDID_ALLCLEAR = 1, CMDID_DELETE = 2;

	// �������B
	struct {
		struct {
			char num[NUM_CNT];	// �g�ݍ��킹�����B
			bool qp;	// �N�C�b�N�s�b�N�B
		} group[NUM_GROUP];
		char npn;	// �e�����B
		char persist;	// �p���񐔁B
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

// �}�[�N�V�[�g�Ǘ��N���X�B
struct ManageMS {
	static const char MAXCNT = 10;	// �}�[�N�V�[�g���쐬�ł��鐔�B

	char m_mscnt;
	HWND m_hwnd;
	MarkSheet *m_pms[MAXCNT];	// ���g�p�̃}�[�N�V�[�g�̒l��NULL�B

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

// �����������B
//   hwnd �e�E�B���h�E�n���h��
//   x �\��X�ʒu
//   y �\��Y�ʒu
MarkSheet::MarkSheet(HWND hwnd, short x, short y) {
	WNDCLASS wc;
	HWND hbtn;

	// �E�B���h�E�N���X�擾/�o�^�B
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

	// ���C���E�B���h�E�쐬�B
	if ((m_hwnd = CreateWindow(wc.lpszClassName, NULL, WS_CHILD, x, y,
		WND_WIDTH, WND_HEIGHT, hwnd, NULL, g_hinst, (LPVOID)this)) == NULL) return;

	// [�S���]�{�^���쐬�B
	if ((hbtn = CreateWindow(TEXT("BUTTON"), TEXT("�S���"), WS_CHILD | WS_TABSTOP | WS_VISIBLE, 35, 300,
		55, 22, m_hwnd, (HMENU)CMDID_ALLCLEAR, g_hinst, NULL)) == NULL) return;
	SendMessage(hbtn, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// [�j��]�{�^���쐬�B
	if ((m_hwnd_delbtn = CreateWindow(TEXT("BUTTON"), TEXT("�j��"), WS_CHILD | WS_DISABLED | WS_TABSTOP | WS_VISIBLE, 35, 330,
		55, 22, m_hwnd, (HMENU)CMDID_DELETE, g_hinst, NULL)) == NULL) return;
	SendMessage(m_hwnd_delbtn, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	ZeroMemory(&numdata, sizeof(numdata));	// ������񏉊����B

	ShowWindow(m_hwnd, SW_SHOWNORMAL);	// ���C���E�B���h�E�\���B
}

// �w�肵���g�ݍ��킹�ʒu�Ɏw�肵��������ݒ肷��B
// group��0�`4�Ȃ�A�g�ݍ��킹�����̈ʒu��group�̐���(num�����Ȃ�N�C�b�N�s�b�N)��ݒ肷��B
// group��5�Ȃ�e�����A6�Ȃ�p���񐔂�ݒ肷��B
// �w�肵�����������ɐݒ肳��Ă����ꍇ�͂��̐������폜����B
// �ݒ�ɐ��������ꍇ��true�A����ȊO��false��Ԃ��B
//   group �g�ݍ��킹�ʒu
//   num ����
bool MarkSheet::setNum(char group, char num) {
	char i, j, tmp, *pnum1, *pnum2;
	bool flg;

	if (group >= 0 && group <= 4) {	// �g�ݍ��킹�����H
		if (num < 0)	// �N�C�b�N�s�b�N�H
			numdata.group[group].qp = !numdata.group[group].qp;
		else {
			if (num < NUM_MIN || num > NUM_MAX || numdata.group[group].qp) return false;
			flg = false;

			// �w�肵�����ɐ������ݒ肳��Ă���ꍇ�͂��̐������폜����B
			for (i = 0; i < NUM_CNT; i++) {
				pnum1 = &numdata.group[group].num[i];
				if (*pnum1 != num) continue;
				*pnum1 = 0;
				flg = true;
				break;
			}

			if (!flg) {
				// �󂢂Ă��鐔�����擾���A��������Ύw�肵��������ݒ肷��B
				for (i = 0; i < NUM_CNT; i++) {
					pnum1 = &numdata.group[group].num[i];
					if (*pnum1 >= NUM_MIN && *pnum1 <= NUM_MAX) continue;
					*pnum1 = num;
					flg = true;
					break;
				}

				if (!flg) return false;
			}

			// �����������ɕ��בւ��B
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
	} else if (group == 5) {	// �e�����H
		if (num < 2 || num > 10) return false;
		numdata.npn = numdata.npn == num ? 0 : num;
	} else {	// �p���񐔁B
		if (num < 2 || num > 5) return false;
		numdata.persist = numdata.persist == num ? 0 : num;
	}

	g_pmms->setNumText(g_hwnd_num);
	g_isedit = true;
	return true;
}

// �N���b�N���̏����B
//   x �N���b�N����X�ʒu
//   y �N���b�N����Y�ʒu
void MarkSheet::cmdClick(short x, short y) {
	char group, num, numx, numy;
	TCHAR buf[64];
	RECT rect;

	// �N���b�N�����ʒu����ʒu�C���f�b�N�X�ɕϊ��B
	numx = (x - MARK_X + MARK_VALID) / (MARK_WIDTH + MARK_GAPX);
	numy = (y - MARK_Y + MARK_VALID) / (MARK_HEIGHT + MARK_GAPY);
	if (numx < 0 || numx > 27 || numy < 0 || numy > 9) return;

	// �N���b�N�����ʒu�����炩�ȃ}�[�N�O�Ȃ��return�B
	if (x < MARK_X - MARK_VALID || x > MARK_X + MARK_WIDTH + numx * (MARK_WIDTH + MARK_GAPX) + MARK_VALID - 1 ||
		y < MARK_Y - MARK_VALID || y > MARK_Y + MARK_HEIGHT + numy * (MARK_HEIGHT + MARK_GAPY) + MARK_VALID - 1)
			return;

	group = numx / NUM_GROUP;
	num = numx % 5 * 10 + numy + 1;	// �����ɕϊ��B

	if (num == 50) {	// ����H
		wsprintf(buf, TEXT("�g����%c ���������܂����H"), group + 'A');
		if (MessageBox(m_hwnd, buf, TEXT("pms"), MB_YESNO | MB_ICONEXCLAMATION) != IDYES) return;

		ZeroMemory(&numdata.group[group].num, sizeof(numdata.group[0].num));
		numdata.group[group].qp = false;
		num = -1;

		g_pmms->setNumText(g_hwnd_num);
		g_isedit = true;
	} else {
		if (num == 47) num = -1;	// �N�C�b�N�s�b�N�̏ꍇ�B

		if (group < 5) {	// �g�ݍ��킹�����H
			if (!setNum(group, num)) return;
		} else {
			if (num >= 12 && num <= 20) {	// �e�����H
				if (!setNum(5, num - 10)) return;
			} else if (num >= 22 && num <= 25)	// �p���񐔁H
				if (!setNum(6, num - 20)) return;
		}
	}

	// �ĕ`����w���B
	if (group == 5) {
		// �e�������p���񐔂̏ꍇ�́A���̕����S�́B
		rect.left = MARK_X + (MARK_WIDTH + MARK_GAPX) * 26;
		rect.top = MARK_Y + (MARK_HEIGHT + MARK_GAPY);
		rect.right = rect.left + (MARK_WIDTH + MARK_GAPX) * 2;
		rect.bottom = rect.top + (MARK_HEIGHT + MARK_GAPY) * 9;
	} else if (num < 0) {
		// �N�C�b�N�s�b�N������̏ꍇ�́A�g�ݍ��킹�����S�́B
		rect.left = MARK_X + (MARK_WIDTH + MARK_GAPX) * group * 5;
		rect.top = MARK_Y;
		rect.right = rect.left + (MARK_WIDTH + MARK_GAPX) * 5;
		rect.bottom = rect.top + (MARK_HEIGHT + MARK_GAPY) * 10;
	} else {
		// �w�肵�������̂݁B
		rect.left = MARK_X + (MARK_WIDTH + MARK_GAPX) * numx;
		rect.top = MARK_Y + (MARK_HEIGHT + MARK_GAPY) * numy;
		rect.right = rect.left + MARK_WIDTH;
		rect.bottom = rect.top + MARK_HEIGHT;
	}
	InvalidateRect(m_hwnd, &rect, false);
}

// [�S���]�̏����B
void MarkSheet::cmdAllClear() {
	RECT rect;

	if (MessageBox(m_hwnd, TEXT("���̃V�[�g�̂��ׂĂ̐������������܂����H"),
		TEXT("pms"), MB_YESNO | MB_ICONEXCLAMATION) != IDYES) return;

	ZeroMemory(&numdata, sizeof(numdata));	// ������񏉊����B
	g_pmms->setNumText(g_hwnd_num);

	// �S�̓I�ɍĕ`����w���B
	rect.left = MARK_X;
	rect.top = MARK_Y;
	rect.right = rect.left + (MARK_WIDTH + MARK_GAPX) * 28;
	rect.bottom = rect.top + (MARK_HEIGHT + MARK_GAPY) * 10;
	InvalidateRect(m_hwnd, &rect, false);
}

// [�j��]�̏����B
void MarkSheet::cmdDelete() {
	if (MessageBox(m_hwnd, TEXT("���̃V�[�g��j�����܂����H"),
		TEXT("pms"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES) g_pmms->deleteMarkSheet();
}

// �`�揈���B
//   hdc �`��Ώۂ̃f�o�C�X�R���e�L�X�g
void MarkSheet::Paint(HDC hdc) {
	char i, j;
	RECT rect = {0, 0, MARK_WIDTH, MARK_HEIGHT};
	PAINTSTRUCT ps;
	HDC hdcmem = CreateCompatibleDC(hdc);
	HBRUSH hbrush;
	HBITMAP hbm;

	// ���C���摜�B
	SelectObject(hdcmem, g_hbmp_sheet);
	BitBlt(hdc, 0, 0, WND_WIDTH, WND_HEIGHT, hdcmem, 0, 0, SRCCOPY);

	// �}�[�N�B
	hbm = CreateCompatibleBitmap(hdc, MARK_WIDTH, MARK_HEIGHT);
	SelectObject(hdcmem, hbm);
	hbrush = CreateSolidBrush(RGB(64, 64, 64));
	FillRect(hdcmem, &rect, hbrush);
	DeleteObject(hbrush);

	for (i = 0; i < NUM_GROUP; i++) {
		for (j = 0; j < NUM_CNT; j++) {
			if (numdata.group[i].qp)
				// �N�C�b�N�s�b�N�B
				BitBlt(hdc,
					MARK_X + (MARK_WIDTH + MARK_GAPX) * i * 5 + (MARK_WIDTH + MARK_GAPX) * 4,
					MARK_Y + (MARK_HEIGHT + MARK_GAPY) * 6, MARK_WIDTH, MARK_HEIGHT, hdcmem, 0, 0, SRCAND
				);
			else if (numdata.group[i].num[j] >= 1 && numdata.group[i].num[j] <= 43)
				// �g�ݍ��킹�����B
				BitBlt(hdc,
					MARK_X + (MARK_WIDTH + MARK_GAPX) * i * 5 + (MARK_WIDTH + MARK_GAPX) * ((numdata.group[i].num[j] - 1) / 10),
					MARK_Y + (MARK_HEIGHT + MARK_GAPY) * ((numdata.group[i].num[j] - 1) % 10),
					MARK_WIDTH, MARK_HEIGHT, hdcmem, 0, 0, SRCAND
				);
		}

		if (numdata.npn >= 2 && numdata.npn <= 10)
			// �e�����B
			BitBlt(hdc, MARK_X + (MARK_WIDTH + MARK_GAPX) * 26,
				MARK_Y + (MARK_HEIGHT + MARK_GAPY) * (numdata.npn - 1), MARK_WIDTH, MARK_HEIGHT, hdcmem, 0, 0, SRCAND
			);

		if (numdata.persist >= 2 && numdata.persist <= 5)
			// �p���񐔁B
			BitBlt(hdc, MARK_X + (MARK_WIDTH + MARK_GAPX) * 27,
				MARK_Y + (MARK_HEIGHT + MARK_GAPY) * (numdata.persist - 1), MARK_WIDTH, MARK_HEIGHT, hdcmem, 0, 0, SRCAND
			);
	}
	DeleteObject(hbm);

	DeleteDC(hdcmem);
}

// �E�B���h�E�v���V�[�W���B
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

// �����������B
//   hwnd �e�E�B���h�E�n���h��
ManageMS::ManageMS(HWND hwnd) {
	TCITEM tci;

	// �^�u�E�B���h�E�쐬�B
	if ((m_hwnd = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_VISIBLE,
		5, 5, 722, 397, hwnd, NULL, g_hinst, NULL)) == NULL) return;
	SendMessage(m_hwnd, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// �^�u�������B
	tci.mask = TCIF_TEXT;
	tci.pszText = TEXT("�J�[�h 1");
	TabCtrl_InsertItem(m_hwnd, 0, &tci);
	tci.pszText = TEXT("(�V�����J�[�h)");
	TabCtrl_InsertItem(m_hwnd, 1, &tci);

	// �}�[�N�V�[�g�������B
	ZeroMemory(&m_pms, sizeof(m_pms));
	m_pms[0] = new MarkSheet(m_hwnd, 5, 25);
	m_mscnt = 1;
}

// �j�������B
ManageMS::~ManageMS() {
	char i;

	for (i = 0; i < m_mscnt; i++) delete m_pms[i];
}

// ���ׂẴ}�[�N�V�[�g������������B
void ManageMS::initMarkSheet() {
	while (m_mscnt > 1) deleteMarkSheet();
	ZeroMemory(&m_pms[0]->numdata, sizeof(m_pms[0]->numdata));	// ������񏉊����B
	RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
	setNumText(g_hwnd_num);
}

// �t�@�C�����炷�ׂẴ}�[�N�V�[�g��ǂݍ��ށB
//   pfn �t�@�C����
void ManageMS::loadMarkSheet(LPCWSTR pfn) {
	static const char *BLANK_CHR = "\t\n\v\f\r ", *NUM_CHR = "0123456789";
	char buf[1024], group, ret = 1, i, j, k, *pnum1, *pnum2, *pbuf, *plen, *pchk;
	int val;
	bool group_cnt_flg;
	FILE *pf;
	MarkSheet *pms;

	if ((pf = _wfopen(pfn, L"r")) == NULL) {
		MessageBox(m_hwnd, TEXT("�ǂݍ��݂Ɏ��s���܂����B���̃t�@�C������I�����Ă��������B"),
			TEXT("pms"), MB_OK | MB_ICONEXCLAMATION);
		return;
	}
	if (g_isedit && MessageBox(m_hwnd, TEXT("���ׂẴV�[�g�̐������������܂����H"),
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
					// �������Ŏ��̑g�ݍ��킹�����ցB
					if ((ret = getMarkSheet(&pms, &group)) != 1) goto procend;
					group_cnt_flg = true;
					break;
				} else if (strnicmp(pbuf, "QP", 2) == 0) {
					if ((ret = getMarkSheet(&pms, &group)) != 1) goto procend;
					pms->numdata.group[group].qp = true;	// �N�C�b�N�s�b�N�B
					group_cnt_flg = true;
					break;
				} else if (strnicmp(pbuf, "N", 1) == 0) {
					// �e�����B
					pbuf += strcspn(pbuf + 1, NUM_CHR);
					if (pbuf >= plen) break;
					val = strtol(pbuf, &pchk, 10);
					if (val > 10) pms->numdata.npn = 10;
					else if (val >= 2) pms->numdata.npn = val;

					if (pchk == NULL) break;
					pbuf = pchk;
					if (pbuf >= plen) break;

					// �p���񐔁B
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

			// �����B
			val = strtol(pbuf, &pchk, 10);
			if (val >= MarkSheet::NUM_MIN && val <= MarkSheet::NUM_MAX) {
				for (j = 0; j < i && pms->numdata.group[group].num[j] != val; j++);	// �d���`�F�b�N�B

				if (j >= i) {	// �d���͂Ȃ����H
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

		// �����������ɕ��בւ��B
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
		MessageBox(m_hwnd, TEXT("�ǂݍ��݃G���[���������܂����B�A�v���P�[�V�������ċN�����Ă݂Ă��������B"),
			TEXT("pms"), MB_OK | MB_ICONEXCLAMATION);
	else if (ret == -1)
		MessageBox(m_hwnd, TEXT("�g�ݍ��킹�����͓r�����疳������܂����B"),
			TEXT("pms"), MB_OK | MB_ICONEXCLAMATION);
}

// ��{�I�ɂ͉������Ȃ����A�󋵂ɉ����ĐV�����}�[�N�V�[�g��ݒ肷��B
// �Ԓl��1�Ȃ�OK�A0�Ȃ�G���[�A-1�Ȃ�}�[�N�V�[�g�쐬�s�B
//   ppms �}�[�N�V�[�g
//   pgroup �g�ݍ��킹�����̃C���f�b�N�X
char ManageMS::getMarkSheet(MarkSheet **ppms, char *pgroup) {
	MarkSheet *pms;

	if (*pgroup < MarkSheet::NUM_GROUP) return 1;
	if (m_mscnt >= MAXCNT) return -1;

	// �V�����}�[�N�V�[�g���쐬���A���̃}�[�N�V�[�g��ݒ肷��B
	TabCtrl_SetCurSel(m_hwnd, m_mscnt);
	selTab();
	if ((pms = m_pms[m_mscnt - 1]) == NULL) return 0;
	*ppms = pms;
	*pgroup = 0;
	return 1;
}

// ���ׂẴ}�[�N�V�[�g���t�@�C���ɏ������ށB
//   pf �t�@�C��
void ManageMS::saveMarkSheet(FILE *pf) {
	char i, j, k, *pnum;
	MarkSheet *pms;

	for (i = 0; i < m_mscnt; i++) {
		pms = m_pms[i];

		for (j = 0; j < MarkSheet::NUM_GROUP; j++) {
			if (pms->numdata.group[j].qp)
				_ftprintf(pf, TEXT("QP"));
			else
				// �����B
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

		// �e�����ƌp���񐔁B
		_ftprintf(pf, TEXT("N %d %d"),
			pms->numdata.npn > 0 ? pms->numdata.npn : 1,
			pms->numdata.persist > 0 ? pms->numdata.persist : 1
		);

		if (i < m_mscnt - 1) _ftprintf(pf, TEXT("\n"));
	}
}

// ���ׂẴ}�[�N�V�[�g���������B
// �����Ȃ�true�A����ȊO��false��Ԃ��B
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

	// ���ׂẴ}�[�N�V�[�g�̈���\�ȑg�ݍ��킹���������邩�`�F�b�N�B
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
		MessageBox(m_hwnd, TEXT("����\�ȑg�ݍ��킹����������܂���B�L���ȑg�ݍ��킹�������}�[�N���Ă��������B"),
			TEXT("pms"), MB_OK | MB_ICONEXCLAMATION);
		return true;
	}

	// �v�����^�_�C�A���O�\���B
	ZeroMemory(&pd, sizeof(pd));
	pd.lStructSize = sizeof(PRINTDLG);
	pd.hwndOwner = m_hwnd;
	pd.hDevMode = NULL;
	pd.hDevNames = NULL;
	pd.hDC = NULL;
	pd.Flags = PD_NOPAGENUMS | PD_RETURNDC;
	if (!PrintDlg(&pd)) return true;

	flg = false;

	if (pd.Flags & PD_SELECTION) {	// �I�����Ă���}�[�N�V�[�g�݈̂�����H
		idx = TabCtrl_GetCurSel(m_hwnd);
		if (idx < 0 || idx > m_mscnt) goto procend;
		cnt = idx + 1;

		// �I�����Ă���}�[�N�V�[�g�̈���\�ȑg�ݍ��킹���������邩�`�F�b�N�B
		i = 0;
		for (j = 0; j < MarkSheet::NUM_GROUP; j++) {
			if (checkNum(idx, j, &same_mscnt, &same_group) == 0) continue;
			i = 1;
			break;
		}
		if (i != 1) {
			MessageBox(m_hwnd, TEXT("�I�������}�[�N�V�[�g�Ɉ���\�ȑg�ݍ��킹����������܂���B�L���ȑg�ݍ��킹�������}�[�N���Ă��������B"),
				TEXT("pms"), MB_OK | MB_ICONEXCLAMATION);
			flg = true;
			goto procend;
		}
	} else {
		idx = 0;
		cnt = m_mscnt;
	}

	// �v�����^�f�o�C�X�R���e�L�X�g�Đݒ�B
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

	// �}�[�N�B
	hbm = CreateCompatibleBitmap(pd.hDC, mark_width, mark_height);
	SelectObject(hdc, hbm);
	hbrush = CreateSolidBrush(RGB(0, 0, 0));
	rect.left = 0;
	rect.top = 0;
	rect.right = mark_width;
	rect.bottom = mark_height;
	FillRect(hdc, &rect, hbrush);
	DeleteObject(hbrush);

	// �}�[�N�̈���ݒ�擾�B
	GetWindowTextA(g_hwnd_posx, buf, sizeof(buf) / sizeof(buf[0]));
	posx = atoi(buf);
	GetWindowTextA(g_hwnd_posy, buf, sizeof(buf) / sizeof(buf[0]));
	posy = atoi(buf);
	GetWindowTextA(g_hwnd_gapx, buf, sizeof(buf) / sizeof(buf[0]));
	gapx = atoi(buf);
	GetWindowTextA(g_hwnd_gapy, buf, sizeof(buf) / sizeof(buf[0]));
	gapy = atoi(buf);

	// ����B
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
		if (k != 1) continue;	// ����\�ȑg�ݍ��킹�������Ȃ��ꍇ�͎��̃}�[�N�V�[�g�ցB

		if (StartPage(pd.hDC) <= 0) {
			flg = false;
			break;
		}

		for (j = 0; j < MarkSheet::NUM_GROUP; j++) {
			if (checkNum(i, j, &same_mscnt, &same_group) == 0) continue;

			pms = m_pms[i];
			if (pms->numdata.group[j].qp) {	// �N�C�b�N�s�b�N�H
				x = 3;
				y = 4 + j * 5;
				DRAW_MARK;
			} else	// �����B
				for (k = 0; k < MarkSheet::NUM_CNT; k++) {
					pnum = &pms->numdata.group[j].num[k];
					x = 9 - (*pnum - 1) % 10;
					y = (*pnum - 1) / 10 + j * 5;
					DRAW_MARK;
				}
		}
		if (pms->numdata.npn >= 2 && pms->numdata.npn <= 10) {
			// �e�����B
			x = 10 - pms->numdata.npn;
			y = 26;
			DRAW_MARK;
		}
		if (pms->numdata.persist >= 2 && pms->numdata.persist <= 10) {
			// �p���񐔁B
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

// �w�肵���E�B���h�E�ɑg�ݍ��킹������\������B
//   hwnd �g�ݍ��킹������\������E�B���h�E�n���h��
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
				// �����B
				for (k = 0; k < MarkSheet::NUM_CNT; k++) {
					if (k > 0) lstrcat(numstr, TEXT("-"));
					wsprintf(buf, TEXT("%02d"), pms->numdata.group[j].num[k]);
					lstrcat(numstr, buf);
				}

				if (chk == -1) {	// �d���H
					wsprintf(buf, TEXT(" (�J�[�h%d�̑g����%c�Əd��)"), same_mscnt + 1, same_group + 'A');
					lstrcat(numstr, buf);
				}
			}

			lstrcat(numstr, TEXT("\n"));
		}

		// ���z�B
		if (num > 0) {
			j = pms->numdata.npn > 0 ? pms->numdata.npn : 1;
			k = pms->numdata.persist > 0 ? pms->numdata.persist : 1;
			num = num * j * k * 200;
			money += num;

			if (i != idx) continue;
			wsprintf(buf, TEXT("\n���z�F\\%d\n"), num);
			lstrcat(numstr, buf);
		}
	}
	if (money > 0) {
		wsprintf(buf, TEXT("���\���ݘg���~�e�����~�p���񐔁~�P��\n\n�����z�F\\%d"), money);
		lstrcat(numstr, buf);
	}

	SetWindowText(hwnd, numstr);
}

// �w�肵���g�ݍ��킹�������d���Ȃ��ݒ肳��Ă��邩�`�F�b�N�B
// �Ԓl��1�Ȃ�OK�A0�Ȃ�NG�A-1�Ȃ�OK�����d���B
//   mscnt �}�[�N�V�[�g�̃C���f�b�N�X
//   group �g�ݍ��킹�����̃C���f�b�N�X
//   psame_mscnt �d�����̃}�[�N�V�[�g�̃C���f�b�N�X(�d�����Ă��Ȃ����̒l�͖���`)
//   psame_group �d�����̑g�ݍ��킹�����̃C���f�b�N�X(�d�����Ă��Ȃ����̒l�͖���`)
char ManageMS::checkNum(char mscnt, char group, char *psame_mscnt, char *psame_group) {
	char i, j, k, *pnum;
	bool flg = true;
	MarkSheet *pms = m_pms[mscnt];

	if (pms->numdata.group[group].qp) return 1;

	// �������S���ݒ肳��Ă��邩�`�F�b�N�B
	for (i = 0; i < MarkSheet::NUM_CNT; i++) {
		pnum = &pms->numdata.group[group].num[i];
		if (*pnum >= MarkSheet::NUM_MIN && *pnum <= MarkSheet::NUM_MAX) continue;
		flg = false;
		break;
	}
	if (!flg) return 0;

	// �w�肵���ʒu�܂ł̂��ׂĂ̑g�ݍ��킹�����Ƃ̏d���`�F�b�N�B
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

// �قȂ�^�u��I���������̏����B
void ManageMS::selTab() {
	int idx = TabCtrl_GetCurSel(m_hwnd), i;
	TCHAR buf[16];
	TCITEM tci;

	if (idx < 0) return;

	if (idx < m_mscnt) {
		// �I�������}�[�N�V�[�g�݂̂�\������B
		for (i = 0; i < m_mscnt; i++) ShowWindow(m_pms[i]->m_hwnd, SW_HIDE);
		ShowWindow(m_pms[idx]->m_hwnd, SW_SHOWNORMAL);
	} else {	// �ǉ������B
		if (m_mscnt >= MAXCNT) return;

		// �ǉ������}�[�N�V�[�g�݂̂�\������B
		for (i = 0; i < m_mscnt; i++) ShowWindow(m_pms[i]->m_hwnd, SW_HIDE);
		m_pms[m_mscnt] = new MarkSheet(m_hwnd, 5, 25);
		if (m_mscnt == 1) EnableWindow(m_pms[0]->m_hwnd_delbtn, true);
		if (m_mscnt > 0) EnableWindow(m_pms[m_mscnt]->m_hwnd_delbtn, true);

		wsprintf(buf, TEXT("�J�[�h %d"), m_mscnt + 1);
		tci.mask = TCIF_TEXT;
		tci.pszText = buf;
		TabCtrl_SetItem(m_hwnd, m_mscnt, &tci);

		g_isedit = true;

		if (++m_mscnt >= MAXCNT) return;
		tci.pszText = TEXT("(�V�����J�[�h)");
		TabCtrl_InsertItem(m_hwnd, m_mscnt, &tci);
		RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
	}
	setNumText(g_hwnd_num);
}

// �I������Ă���}�[�N�V�[�g���폜����B
void ManageMS::deleteMarkSheet() {
	int idx = TabCtrl_GetCurSel(m_hwnd), i;
	TCHAR buf[16];
	TCITEM tci;

	if (idx < 0 || idx >= m_mscnt) return;

	// �폜�B
	DestroyWindow(m_pms[idx]->m_hwnd);
	delete m_pms[idx];
	TabCtrl_DeleteItem(m_hwnd, idx);

	// �폜��A�f�[�^�𐮂���B
	m_mscnt--;
	tci.mask = TCIF_TEXT;
	for (i = idx; i < m_mscnt; i++) {
		m_pms[i] = m_pms[i + 1];

		wsprintf(buf, TEXT("�J�[�h %d"), i + 1);
		tci.pszText = buf;
		TabCtrl_SetItem(m_hwnd, i, &tci);
	}
	m_pms[m_mscnt] = NULL;

	// �V�����}�[�N�V�[�g���쐬�ł��Ȃ���Ԃō폜���ꂽ���̑Ώ��B
	if (m_mscnt == MAXCNT - 1) {
		tci.pszText = TEXT("(�V�����J�[�h)");
		TabCtrl_InsertItem(m_hwnd, m_mscnt, &tci);
	}

	if (m_mscnt == 1) EnableWindow(m_pms[0]->m_hwnd_delbtn, false);
	TabCtrl_SetCurSel(m_hwnd, idx > 0 ? idx - 1 : 0);
	selTab();
	g_isedit = true;
}

// �_�C�A���O���b�Z�[�W�����������ꍇ��true��Ԃ��B
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
		ofn.lpstrFilter = L"���ׂẴt�@�C�� (*.*)\0*.*\0�g�ݍ��킹�����t�@�C�� (*.txt)\0*.txt\0";
		ofn.nMaxFile = sizeof(fn) / sizeof(fn[0]);
		ofn.lpstrDefExt = L"txt";
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case CMDID_NEW:	// �V�K�쐬�B
			if (g_isedit && MessageBox(hwndDlg, TEXT("���ׂẴV�[�g�̐������������܂����H"),
				TEXT("pms"), MB_YESNO | MB_ICONEXCLAMATION) != IDYES) break;
				g_pmms->initMarkSheet();
				g_isedit = false;
			break;
		case CMDID_PRINT:	// ����B
			if (!g_pmms->printMarkSheet())
				MessageBox(hwndDlg, TEXT("����G���[���������܂����B�ēx����������Ă݂Ă��������B"),
					TEXT("pms"), MB_OK | MB_ICONEXCLAMATION);
			break;
		case CMDID_LOAD:	// �ǂݍ��݁B
			ofn.hwndOwner = hwndDlg;
			ofn.Flags = OFN_FILEMUSTEXIST;
			ofn.lpstrFile = fn;
			if (GetOpenFileNameW(&ofn)) g_pmms->loadMarkSheet(fn);
			break;
		case CMDID_SAVE:	// ���O��t���ĕۑ��B
			ofn.hwndOwner = hwndDlg;
			ofn.Flags = OFN_OVERWRITEPROMPT;
			ofn.lpstrFile = fn;
			if (!GetSaveFileName(&ofn)) break;

			if ((pf = _tfopen(fn, TEXT("w"))) == NULL) {
				MessageBox(hwndDlg, TEXT("�ۑ��Ɏ��s���܂����B�t�@�C������ύX���Ă��������B"),
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
		if (!g_isedit || MessageBox(hwndDlg, TEXT("�ύX���e�͕ۑ�����Ă��܂���B�I�����܂����H"),
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

	// ���d�N���h�~�B
	CreateMutex(NULL, FALSE, TEXT("pms"));
	if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;

	// �t�H���g�쐬�B
	if ((g_hfont = CreateFont(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, NULL)) == NULL) return 0;

	g_hinst = hInstance;
	g_hbmp_sheet = LoadImage(hInstance, MAKEINTRESOURCE(2), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_LOADMAP3DCOLORS);
	g_hbmp_print = LoadImage(hInstance, MAKEINTRESOURCE(3), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_LOADMAP3DCOLORS);

	InitCommonControls();	// �R�����R���g���[���������B

	// �g�b�v�E�B���h�E�쐬�B
	if ((hwnd = CreateDialog(hInstance, MAKEINTRESOURCE(1), NULL, DialogProc)) == NULL) return 0;

	// [�V�K�쐬]�{�^���쐬�B
	if ((hwnd_tmp = CreateWindow(TEXT("BUTTON"), TEXT("�V�K�쐬"), WS_CHILD | WS_TABSTOP | WS_VISIBLE,
		5, 407, 130, 40, hwnd, (HMENU)CMDID_NEW, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// [���]�{�^���쐬�B
	if ((hwnd_tmp = CreateWindow(TEXT("BUTTON"), TEXT("���"), WS_CHILD | WS_TABSTOP | WS_VISIBLE,
		145, 407, 130, 40, hwnd, (HMENU)CMDID_PRINT, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// [�ǂݍ���]�{�^���쐬�B
	if ((hwnd_tmp = CreateWindow(TEXT("BUTTON"), TEXT("�ǂݍ���"), WS_CHILD | WS_TABSTOP | WS_VISIBLE,
		5, 457, 130, 40, hwnd, (HMENU)CMDID_LOAD, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// [���O��t���ĕۑ�]�{�^���쐬�B
	if ((hwnd_tmp = CreateWindow(TEXT("BUTTON"), TEXT("���O��t���ĕۑ�"), WS_CHILD | WS_TABSTOP | WS_VISIBLE,
		145, 457, 130, 40, hwnd, (HMENU)CMDID_SAVE, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	//--------------------------------------------------

	// �u����ʒu�v�쐬�B
	if ((hwnd_tmp = CreateWindow(TEXT("STATIC"), TEXT("����ʒu"), WS_CHILD | WS_VISIBLE,
		280, 407, 60, 15, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// �}�[�N�̈���J�nX�ʒu�ݒ�e�L�X�g�쐬�B
	if ((g_hwnd_posx = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("200"),
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER, 280, 420, 40, 18, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hwnd_posx, WM_SETFONT, (WPARAM)g_hfont, FALSE);
	SendMessage(g_hwnd_posx, EM_SETLIMITTEXT, 4, 0);

	// �}�[�N�̈���J�nY�ʒu�ݒ�e�L�X�g�쐬�B
	if ((g_hwnd_posy = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("3630"),
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER, 322, 420, 40, 18, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hwnd_posy, WM_SETFONT, (WPARAM)g_hfont, FALSE);
	SendMessage(g_hwnd_posy, EM_SETLIMITTEXT, 4, 0);

	// �u����Ԋu�v�쐬�B
	if ((hwnd_tmp = CreateWindow(TEXT("STATIC"), TEXT("����Ԋu"), WS_CHILD | WS_VISIBLE,
		280, 445, 60, 15, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// �}�[�N�̈��X�Ԋu�ݒ�e�L�X�g�쐬�B
	if ((g_hwnd_gapx = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("635"),
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER, 280, 458, 40, 18, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hwnd_gapx, WM_SETFONT, (WPARAM)g_hfont, FALSE);
	SendMessage(g_hwnd_gapx, EM_SETLIMITTEXT, 4, 0);

	// �}�[�N�̈��Y�Ԋu�ݒ�e�L�X�g�쐬�B
	if ((g_hwnd_gapy = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("420"),
		WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_NUMBER, 322, 458, 40, 18, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hwnd_gapy, WM_SETFONT, (WPARAM)g_hfont, FALSE);
	SendMessage(g_hwnd_gapy, EM_SETLIMITTEXT, 4, 0);

	//--------------------------------------------------

	// ������̒��ӕ��쐬�B
	if ((hwnd_tmp = CreateWindow(TEXT("STATIC"), TEXT("������̕W���̗p���̌����͐}�̒ʂ�ł�"),
		WS_CHILD | WS_VISIBLE | SS_RIGHT, 175, 543, 250, 20, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(hwnd_tmp, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	// ���E�B���h�E�쐬�B
	if ((g_hwnd_num = CreateWindow(TEXT("STATIC"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE,
		427, 407, 300, 150, hwnd, NULL, hInstance, NULL)) == NULL) return 0;
	SendMessage(g_hwnd_num, WM_SETFONT, (WPARAM)g_hfont, FALSE);

	g_pmms = new ManageMS(hwnd);	// �}�[�N�V�[�g�Ǘ��E�B���h�E�쐬�B

	hicon = LoadImage(hInstance, MAKEINTRESOURCE(4), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
	SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hicon);

	ShowWindow(hwnd, SW_SHOWNORMAL);	// �g�b�v�E�B���h�E�\���B

	// �R�}���h���C������������ꍇ�A�g�ݍ��킹�����t�@�C�����R�}���h���C������ǂݍ��ށB
	ppargv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argc >= 2) g_pmms->loadMarkSheet(ppargv[1]);
	LocalFree(ppargv);

	// ���b�Z�[�W���[�v�B
	while (ret = GetMessage(&msg, NULL, 0, 0)) {
		if (ret == -1) break;

		// �{�^���̃t�H�[�J�X����K�؂ɏ�����(�q�E�B���h�E�D��ɂ��邱��)�A���b�Z�[�W����������B
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
