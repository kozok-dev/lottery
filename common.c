#include <stdio.h>
#include <tchar.h>
#include <emmintrin.h>

/* ファイルから数値を取得して配列に設定する。
f ファイル。
num 設定する配列(cnt1とcnt2が同じなら要素数がcnt2以上、そうでないならcnt2+1以上であること)。
cnt1～cnt2 設定する数値の数の範囲。
r1～r2 設定する数値の範囲。
digit 設定する数値は1桁ずつで重複を許すか。
sort 並べ替える範囲(cntより小さいこと)。
stpos 設定を開始する数値の位置。*/
char get_number(FILE *f, char *num, char cnt1, char cnt2, char r1, char r2, char digit, char sort, char stpos)
{
	char buf[1024], i, j, k, *bp, *len, *chk;
	int val;

	for (;;) {
		if (fgets(buf, 1024, f) == NULL) return 0;
		i = j = 0;

		for (bp = buf, len = buf + strlen(buf); ; ) {
			bp += strcspn(bp, "0123456789");
			if (bp >= len) break;

			val = digit && j >= stpos ? *bp - '0' : strtol(bp, &chk, 10);

			if (j < stpos)
				j++;
			else
				if (val >= r1 && val <= r2) {
					if (digit)
						k = i;
					else
						for (k = 0; k < i && num[k] != val; k++);

					if (k >= i) {
						num[i++] = val;
						if (i >= cnt2) break;
					}
				}

			if (digit && j >= stpos)
				bp++;
			else {
				if (chk == NULL) break;
				bp = chk;
			}
			if (bp >= len) break;
		}

		if (i >= cnt1) break;
	}
	if (cnt1 < cnt2) num[i] = -1;

	if (sort > i) sort = i;
	for (i = 0; i < sort - 1; i++)
		for (j = i + 1; j < sort; j++) {
			if (num[i] <= num[j]) continue;
			val = num[i];
			num[i] = num[j];
			num[j] = val;
		}
	return 1;
}

char add_chr(char **str, char chr)
{
	char *tmp;
	size_t len = 2;

	if (*str != NULL) len += strlen(*str);

	tmp = (char *)realloc(*str, len * sizeof(char));
	if (tmp == NULL) return 0;

	*str = tmp;
	(*str)[len - 2] = chr;
	(*str)[len - 1] = '\0';
	return 1;
}

char add_col(char ***col, char **str)
{
	char **tmp;
	size_t len = 1;

	if (*col != NULL) len += _msize(*col) / sizeof(*col);

	tmp = (char **)realloc(*col, len * sizeof(char *));
	if (tmp == NULL) return 0;

	*col = tmp;
	(*col)[len - 1] = *str;

	*str = NULL;
	return 1;
}

char add_list(char ****list, char ***col)
{
	char ***tmp;
	size_t len = 1;

	if (*list != NULL) len += _msize(*list) / sizeof(*list);

	tmp = (char ***)realloc(*list, len * sizeof(char **));
	if (tmp == NULL) return 0;

	*list = tmp;
	(*list)[len - 1] = *col;

	*col = NULL;
	return 1;
}

/* CSV二次元配列用のメモリを開放する。
list 二次元配列。*/
void free_csv_list(char ****list)
{
	size_t i, j;

	if (*list == NULL) return;

	for (i = _msize(*list) / sizeof(*list) - 1; i >= 0; i--) {
		for (j = _msize((*list)[i]) / sizeof((*list)[0]) - 1; j >= 0; j--) free((*list)[i][j]);
		free((*list)[i]);
	}
	free(*list);
	*list = NULL;
}

/* CSVファイルを二次元配列の文字列にする。
list 二次元配列の設定先。
filename ファイル名。*/
char get_csv_list(char ****list, TCHAR *filename)
{
	char **col, *str, buf[1000], quote_flag, out_flag, succ = 0;
	size_t len_buf, pos;
	FILE *f;

	f = _tfopen(filename, _T("r"));
	if (f == NULL) return 0;

	*list = NULL;
	col = NULL;
	str = NULL;
	out_flag = 2;
	quote_flag = 0;

	while (fgets(buf, sizeof(buf) / sizeof(buf[0]), f) != NULL) {
		len_buf = strlen(buf);
		pos = 0;

		/* CSVの内容を1文字ずつ解析していく。*/
		while (pos < len_buf) {
			out_flag = 0;

			switch (buf[pos]) {
			case ',':	/* カンマの場合。*/
				if (quote_flag) {
					if (!add_chr(&str, buf[pos])) goto procend;
				} else {
					/* 確定。*/
					if (!add_col(&col, &str)) goto procend;
					out_flag = 1;
				}
				pos++;
				break;

			case '"':	/* ダブルクォーテーションの場合。*/
				if (quote_flag) {
					if (pos + 1 < len_buf && buf[pos + 1] == '"') {
						/* ダブルクォーテーション内にダブルクォーテーションだった。*/
						if (!add_chr(&str, buf[pos])) goto procend;
						pos += 2;
					} else {
						/* ダブルクォーテーション終了。*/
						pos++;
						quote_flag = 0;
					}
				} else {
					/* ダブルクォーテーション開始。*/
					pos++;
					quote_flag = 1;
				}
				break;

			case '\n':	/* 改行の場合。*/
				if (quote_flag) {
					if (!add_chr(&str, buf[pos])) goto procend;
				} else {
					/* 確定。*/
					if (!add_col(&col, &str)) goto procend;
					if (!add_list(list, &col)) goto procend;
					out_flag = 2;
				}
				pos++;
				break;

			default:	/* その他の文字の場合。*/
				if (!add_chr(&str, buf[pos])) goto procend;
				pos++;
				break;
			}
		}
	}

	if (out_flag < 2) {	/* 最後の内容が残っている？ */
		if (out_flag < 1) {
			if (!add_col(&col, &str)) goto procend;
		}

		if (!add_list(list, &col)) goto procend;
	}

	succ = 1;

procend:
	fclose(f);
	if (succ == 1) return 1;

	free(str);
	if (col != NULL) {
		for (pos = _msize(col) / sizeof(col) - 1; pos >= 0; pos--) free(col[pos]);
		free(col);
	}
	free_csv_list(list);
	return 0;
}

struct {
	char sse2;
	int range, shift, normal_sw;
	unsigned int index, base;
	double normal_save;
	union {
		__m128i m[156];
		unsigned int x[624];
	} u;
} g_mt;

void init_mt(unsigned int s)
{
	static unsigned int parity[] = {1, 0, 0, 0x13c9e684};
	int inner = 0, i, j;
	unsigned int p, work;

	for (g_mt.u.x[0] = s, p = 1; p < 624; p++) g_mt.u.x[p] = s = 1812433253 * (s ^ (s >> 30)) + p;

	g_mt.index = 624; g_mt.range = g_mt.normal_sw = 0;
	_asm {
		push edx
		pushfd
		pop eax
		xor eax, 200000h
		push eax
		popfd
		pushfd
		pop edx
		cmp eax, edx
		jnz ret0
		mov eax, 1
		cpuid
		and edx, 4000000h
		jnz ret1
ret0:
		mov g_mt.sse2, 0
		jmp procend
ret1:
		mov g_mt.sse2, 1
procend:
		pop edx
	}
	for (i = 0; i < 4; i++) inner ^= g_mt.u.x[i] & parity[i];
	for (i = 16; i > 0; i >>= 1) inner ^= inner >> i;
	inner &= 1;
	if (inner == 1) return;
	for (i = 0; i < 4; i++)
		for (j = 0, work = 1; j < 32; j++, work <<= 1)
			if (work & parity[i]) {
				g_mt.u.x[i] ^= work;
				return;
			}
}

unsigned int next_mt()
{
	if (g_mt.index == 0) init_mt(time(NULL));
	if (g_mt.index == 624) {
		if (g_mt.sse2) {
			int i;
			__m128i *t = g_mt.u.m, c, d, v, x, y, z, mask;

			mask = _mm_set_epi32(0xbffffff6, 0xbffaffff, 0xddfecb7f, 0xdfffffef);
			c = _mm_load_si128(&t[154]);
			d = _mm_load_si128(&t[155]);
			for (i = 0; i < 34; i++) {
				x = _mm_load_si128(&t[i]);
				y = _mm_srli_epi32(t[i + 122], 11);
				z = _mm_srli_si128(c, 1);
				v = _mm_slli_epi32(d, 18);
				z = _mm_xor_si128(z, x);
				z = _mm_xor_si128(z, v);
				x = _mm_slli_si128(x, 1);
				y = _mm_and_si128(y, mask);
				z = _mm_xor_si128(z, x);
				z = _mm_xor_si128(z, y);
				_mm_store_si128(&t[i], z);
				c = d; d = z;
			}
			for (; i < 156; i++) {
				x = _mm_load_si128(&t[i]);
				y = _mm_srli_epi32(t[i - 34], 11);
				z = _mm_srli_si128(c, 1);
				v = _mm_slli_epi32(d, 18);
				z = _mm_xor_si128(z, x);
				z = _mm_xor_si128(z, v);
				x = _mm_slli_si128(x, 1);
				y = _mm_and_si128(y, mask);
				z = _mm_xor_si128(z, x);
				z = _mm_xor_si128(z, y);
				_mm_store_si128(&t[i], z);
				c = d; d = z;
			}
		} else {
			unsigned int *a = g_mt.u.x, *b = a + 488, *c = a + 616, *d = a + 620, *e = a + 624, x;

			do {
				x = a[3] ^ (a[3] << 8) ^ (a[2] >> 24) ^ ((b[3] >> 11) & 0xbffffff6);
				a[3]= x ^ (c[3] >> 8) ^ (d[3] << 18);

				x = a[2] ^ (a[2] << 8) ^ (a[1] >> 24) ^ ((b[2] >> 11) & 0xbffaffff);
				a[2] = x ^ ((c[2] >> 8) | (c[3] << 24)) ^ (d[2] << 18);

				x = a[1] ^ (a[1] << 8) ^ (a[0] >> 24) ^ ((b[1] >> 11) & 0xddfecb7f);
				a[1] = x ^ ((c[1] >> 8) | (c[2] << 24)) ^ (d[1] << 18);

				x = a[0] ^ (a[0] << 8) ^ ((b[0] >> 11) & 0xdfffffef);
				a[0] = x ^ ((c[0] >> 8) | (c[1] << 24)) ^ (d[0] << 18);

				c = d; d = a; a += 4; b += 4;
				if (b == e) b -= 624;
			} while (a != e);
		}
		g_mt.index = 0;
	}
	return g_mt.u.x[g_mt.index++];
}

int next_int_ex(int range)
{
	int shift;
	unsigned int y, base, remain;

	if (range <= 0) return 0;
	if (range != g_mt.range) {
		g_mt.base = g_mt.range = range;
		for (g_mt.shift = 0; g_mt.base <= (1 << 30); g_mt.shift++) g_mt.base <<= 1;
	}
	for (;;) {
		y = next_mt() >> 1;
		if (y < g_mt.base) return (int)(y >> g_mt.shift);
		base = g_mt.base; shift = g_mt.shift; y -= base;
		remain = (1 << 31) - base;
		for (; remain >= (unsigned int)range; remain -= base) {
			for (; base > remain; base >>= 1) shift--;
			if (y < base) return (int)(y >> shift);
			else y -= base;
		}
	}
}
