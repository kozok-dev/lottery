#include <stdarg.h>
#include "../common.c"
#define NUMMAX 6000
#define ITEMNAME "合計    大小         奇数偶数     数字集団             下一桁       同番台  " \
	"隣接数字差       セット球 ﾚ ﾋ ﾄ 平出回数 当選間隔                          過去当選コ\n"
#define COLNAME "数1\t数2\t数3\t数4\t数5\t数6\t合計\t大小\t奇偶\t集1\t集1T\t集2\t集2T\t" \
				"下\t下T\t番台\t番台T\t差\t差T\tｾｯﾄ\tｾｯﾄT\tﾚ\tﾋ\tﾄ\t平\t平T\t" \
				"当間\t当間s\t当間a\tﾎｯﾄ\th0\th1\th2\th3\th4\th5\th5b\th6\n"
#define SEPAL "-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n"

char wn[NUMMAX][7], hit0max[6], hit0min[6], hit1max[6], hit1min[6], hit2max[6], hit2min[6], hit3max[6], hit4max[6], hit5max[6];
short wncnt, wnum_cnt1[44], wnum_cnt2[44], wnum_cnt3[44], wnum_cnt4[44], h0max = -1, h0min = 999, h1max = -1, h1min = 999, h2max = -1, h2min = 999, h3max = -1, h4max = -1, h5max = -1;
int sum_cnt[244], big_cnt[7], odd_cnt[7], seq_cnt[7], samepast_cnt[7], avg_cnt[7], sub_cnt[7], same1_cnt[7], same2_cnt[7], group_cnt[11], num_cnt[44], h5_cnt, h5b_cnt, h6_cnt;

struct {
	char en[44], encnt, hit, *wn;
	short cnt, gap;
} chthh[12][NUMMAX / 10], *cht1hh = chthh[0], *chtm1hh = chthh[1], *cht2hh = chthh[2],
*cht3hh = chthh[3], *chtm3hh = chthh[4], *cht4hh = chthh[5], *chtm4hh = chthh[6], *cht5hh = chthh[7],
*cht6hh = chthh[8], *cht7hh = chthh[9], *chtm7hh = chthh[10], *cht8hh = chthh[11];
char *chtname[] = {
	"統計一覧 予", "統計一覧 削", "最後当選 予", "当選期間 予", "当選期間 削", "飛後当選 予",
	"飛後当選 削", "多重当選 予", "前後数当 予", "後追い数 予", "後追い数 削", "占星術 予"
};

/* 合計数字。*/
short sum_number(const char *num)
{
	return num[0] + num[1] + num[2] + num[3] + num[4] + num[5];
}

/* 大小数字。*/
void big_number(const char *num, short *b, short *tb, short g)
{
	short i, j;

	*b = 0;
	for (i = 0; i < 6; i++)
		if (num[i] > 21) (*b)++;

	*tb = *b;
	for (i = 1; i < (g < 4 ? g + 1 : 5); i++)
		for (j = 0; j < 6; j++)
			if (wn[g - i][j] > 21) (*tb)++;
}

/* 奇数偶数比率。*/
void odd_number(const char *num, short *o, short *to, short g)
{
	short i, j;

	*o = 0;
	for (i = 0; i < 6; i++)
		if (num[i] & 1) (*o)++;

	*to = *o;
	for (i = 1; i < (g < 4 ? g + 1 : 5); i++)
		for (j = 0; j < 6; j++)
			if (wn[g - i][j] & 1) (*to)++;
}

/* 数字集団。*/
void group_number(const char *num, char *g1, char *g2, char t)
{
	char g[3], ch[6], chf, i, j, k = 0;

	for (i = 0; i < 6; i++) {
		if (t)
			g1[i] = (num[i] - 1) % 7 + 'A';	/* セット球。*/
		else
			/* 6分割。*/
			if (num[i] < 8) g1[i] = 'A';
			else if (num[i] < 15) g1[i] = 'B';
			else if (num[i] < 22) g1[i] = 'C';
			else if (num[i] < 29) g1[i] = 'D';
			else if (num[i] < 36) g1[i] = 'E';
			else g1[i] = 'F';

		ch[i] = 0;
	}

	/* 分割タイプ取得。*/
	g[0] = g[1] = g[2] = 0;
	for (i = 0; i < 5; i++) {
		if (ch[i]) continue;
		chf = 0;
		for (j = i + 1; j < 6; j++) {
			if (g1[i] != g1[j]) continue;
			g[k] += chf ? 1 : 2;
			ch[j] = 1;
			chf = 1;
		}
		if (chf) k++;
	}
	for (i = 0; i < 2; i++)
		for (j = i + 1; j < 3; j++) {
			if (g[i] >= g[j]) continue;
			k = g[i];
			g[i] = g[j];
			g[j] = k;
		}
	if (g[0] == 2 && g[1] == 0 && g[2] == 0) g1[6] = 'b';	/* 例：AABCDE */
	else if (g[0] == 2 && g[1] == 2 && g[2] == 0) g1[6] = 'c';	/* 例：AABBCD */
	else if (g[0] == 2 && g[1] == 2 && g[2] == 2) g1[6] = 'd';	/* 例：AABBCC */
	else if (g[0] == 3 && g[1] == 0) g1[6] = 'e';	/* 例：AAABCD */
	else if (g[0] == 3 && g[1] == 2) g1[6] = 'f';	/* 例：AAABBC */
	else if (g[0] == 3 && g[1] == 3) g1[6] = 'g';	/* 例：AAABBB */
	else if (g[0] == 4 && g[1] == 0) g1[6] = 'h';	/* 例：AAAABC */
	else if (g[0] == 4 && g[1] == 2) g1[6] = 'i';	/* 例：AAAABB */
	else if (g[0] == 5) g1[6] = 'j';	/* 例：AAAAAB */
	else if (g[0] == 6) g1[6] = 'k';	/* 例：AAAAAA */
	else g1[6] = 'a';	/* 例：ABCDEF */

	/* 9分割。*/
	for (i = 0; i < 9; i++) g2[i] = 0;
	for (i = 0; i < 6; i++)
		if (num[i] < 6) g2[0]++;
		else if (num[i] < 11) g2[1]++;
		else if (num[i] < 16) g2[2]++;
		else if (num[i] < 21) g2[3]++;
		else if (num[i] < 26) g2[4]++;
		else if (num[i] < 31) g2[5]++;
		else if (num[i] < 36) g2[6]++;
		else if (num[i] < 41) g2[7]++;
		else g2[8]++;
	g2[9] = 0;
	for (i = 0; i < 9; i++)
		if (g2[i] > 1) g2[9] += g2[i];
}

/* 下1桁。同番台。*/
void same_number(const char *num, char *s1, char *s2)
{
	char ch1[6], ch2[6], chf1, chf2, i, j, k, l, m, n;

	/* 下1桁。*/
	for (i = 0; i < 11; i++) s1[i] = 0;
	for (i = 0; i < 6; i++) s1[num[i] % 10]++;

	/* 同番台。*/
	for (i = 0; i < 6; i++) s2[i] = ch1[i] = ch2[i] = 0;
	for (i = 0; i < 6; i++) s2[num[i] / 10]++;

	/* 下1桁と同番台の数を計算。*/
	for (i = 0; i < 5; i++) {
		chf1 = chf2 = 0;
		k = num[i] % 10;
		m = num[i] / 10;

		for (j = i + 1; j < 6; j++) {
			l = num[j] % 10;
			n = num[j] / 10;

			if (k == l && !ch1[j]) {	/* 下1桁一致？ */
				s1[10] += chf1 ? 1 : 2;
				ch1[j] = 1;
				chf1 = 1;
			}
			if (m == n && !ch2[j]) {	/* 同番台一致？ */
				s2[5] += chf2 ? 1 : 2;
				ch2[j] = 1;
				chf2 = 1;
			}
		}
	}
}

/* 隣接数字差。*/
void sub_number(const char *num, char *s)
{
	char ch[5], chf, i, j;

	for (i = 0; i < 5; i++) {
		s[i] = num[i + 1] - num[i];
		ch[i] = 0;
	}
	s[5] = 0;

	for (i = 0; i < 4; i++) {
		if (ch[i]) continue;
		chf = 0;
		for (j = i + 1; j < 5; j++) {
			if (s[i] != s[j]) continue;
			s[5] += chf ? 1 : 2;
			ch[j] = 1;
			chf = 1;
		}
	}
}

/* 連番。*/
char seq_number(const char *num)
{
	char cnt = 0, i = 0, j;

	while (i < 5) {
		for (j = i + 1; j < 6 && num[i] >= num[j] - j + i; j++) cnt += i < j - 1 ? 1 : 2;
		i = j;
	}
	return cnt;
}

/* ヒキ/飛び数字。*/
char same_past_number(const char *num, short g)
{
	char cnt = 0, i, j;

	if (--g < 0) return 0;

	for (i = 0; i < 6; i++)
		for (j = 0; j < 6; j++)
			if (wn[g][i] == num[j]) {
				cnt++;
				break;
			}
	return cnt;
}

/* 平均出現回数。*/
void avg_number(const char *num, char *a, short g)
{
	short wnumcnt[44], wnumavg, i, j;

	for (i = 1; i < 44; i++) wnumcnt[i] = 0;
	for (i = 0; i < g; i++)
		for (j = 0; j < 6; j++) wnumcnt[wn[i][j]]++;
	wnumavg = (float)g * 6 / 43 + 0.5;

	a[6] = 0;
	for (i = 0; i < 6; i++)
		if (wnumcnt[num[i]] < wnumavg)
			a[i] = 'l';
		else {
			a[i] = 'w';
			a[6]++;
		}
}

/* ホット数字。*/
void hot_number(const char *num, char *g1, char *g2, short *s, short *t, short g)
{
	char ch[6], chf;
	short i, j, k;

	g1[6] = 0;
	for (i = 0; i < 6; i++) {
		/* ホット数字。*/
		if ((j = g - 10) < 0)
			g1[6] = 6;
		else
			for (; j < g; j++) {
				for (k = 0; k < 6 && num[i] != wn[j][k]; k++);
				if (k == 6) continue;
				g1[6]++;
				break;
			}

		/* 当選間隔。*/
		g1[i] = 0;
		for (j = g - 1; j >= 0; j--) {
			for (k = 0; k < 6 && num[i] != wn[j][k]; k++);
			if (k < 6) break;
			g1[i]++;
		}
	}

	for (i = 0; i < 37; i++) g2[i] = 0;
	*s = 0;
	for (i = 0; i < 6; i++) {
		g2[g1[i] < 36 ? g1[i] : 36]++;
		*s += g1[i];
	}
	for (i = 0; i < 6; i++) ch[i] = 0;
	*t = 0;
	for (i = 0; i < 5; i++) {
		if (ch[i]) continue;
		chf = 0;
		for (j = i + 1; j < 6; j++) {
			if (g1[i] != g1[j]) continue;
			*t += chf ? 1 : 2;
			ch[j] = 1;
			chf = 1;
		}
	}
}

/* 過去当選数字。*/
void past_winning_number(const char *num, short *h, short g)
{
	short i, j, k, l;

	for (i = 0; i < 8; i++) h[i] = 0;

	for (i = wncnt - g; i < wncnt; i++) {
		j = 0;
		for (k = 0; k < 6; k++)
			for (l = 0; l < 6; l++)
				if (wn[i][k] == num[l]) j++;

		if (j == 5)	/* 2等 or 3等 */
			for (k = 0; k < 6; k++)
				if (wn[i][6] == num[k]) {
					j = 7;	/* 2等 */
					break;
				}
		h[j]++;
	}
}

/* 指定した数字が含まれている数。*/
char cnt_number(const char *num, ...)
{
	char cnt = 0, i, j;
	va_list al;

	va_start(al, num);
	while ((i = va_arg(al, char)) > 0)
		for (j = 0; j < 6; j++)
			if (i == num[j]) cnt++;
	va_end(al);
	return cnt;
}

/* マークした数字が特定パターンになっているかどうか。*/
char mark_number(const char *num)
{
	char m[7], i, j, k, l;

	for (i = 0; i < 7; i++) m[i] = 0;
	for (i = 1; i < 9; i++) {
		j = 0;
		for (k = i; k < 44; k += 10)
			for (l = 0; l < 6; l++)
				if (num[l] == k || num[l] == k + 1 || num[l] == k + 2) j++;
		m[j]++;
	}
	if (m[0] > 2 || m[1] > 5 || m[2] > 5 || m[3] > 3 || m[4] > 1 || m[5] > 0 || m[6] > 0) return 1;

	for (i = 0; i < 6; i++) {
		j = 0;
		for (k = 0; k < 6; k++)
			if (num[i] == num[k] - 11 || num[i] == num[k] - 2 || num[i] == num[k] + 9) j++;
		if (j >= 2) return 1;

		j = 0;
		for (k = 0; k < 6; k++)
			if (num[i] == num[k] - 9 || num[i] == num[k] + 2 || num[i] == num[k] + 11) j++;
		if (j >= 2) return 1;
	}

	for (i = 0; i < 4; i++) {
		j = k = 0;
		for (l = i + 1; l < 6; l++) {
			if (num[i] < 22 && num[i] % 10 != 0 && num[i] % 10 != 9) {
				if (num[l] != num[i] + 11 && num[l] != num[i] + 22) continue;
				if (j > 0) return 1;
				j++;
			}

			if (num[i] < 26 && num[i] % 10 != 1 && num[i] % 10 != 2) {
				if (num[l] != num[i] + 9 && num[l] != num[i] + 18) continue;
				if (k > 0) return 1;
				k++;
			}
		}
	}

	return 0;
}

/* 過去当たり数字連続重複。*/
char same_past_seq_winning_number(const char *num, short g)
{
	short i, j, k;

	for (i = wncnt - g; i < wncnt; i++)
		for (j = 0; j < 4; j++)
			for (k = 0; k < 4; k++)
				if (wn[i][j] == num[k] &&
				wn[i][j + 1] == num[k + 1] &&
				wn[i][j + 2] == num[k + 2]) return 1;
	return 0;
}

void write_data(const char *, FILE *, FILE *, short);
#define SCRTYPE 4
#include "../scr.c"

void write_data(const char *num, FILE *f1, FILE *f2, short flag)
{
	char a1[11], a2[37];
	short h[8], i, j, k;

	if (g_ec != 1 || flag != -2)
		fprintf(f1, "[%2d|%2d|%2d|%2d|%2d|%2d] ", num[0], num[1], num[2], num[3], num[4], num[5]);
	if (f2 != NULL) fprintf(f2, "%d\t%d\t%d\t%d\t%d\t%d\t", num[0], num[1], num[2], num[3], num[4], num[5]);

	i = sum_number(num);
	if (flag == -2) sum_cnt[i]++;
	if (g_ec != 1 || flag != -2) fprintf(f1, "%3d%+4d ", i, i - 132);
	if (f2 != NULL) fprintf(f2, "%d\t", i);

	k = flag < 0 ? wncnt : flag;
	big_number(num, &i, &j, k);
	if (flag == -2) big_cnt[i]++;
	if (g_ec != 1 || flag != -2) {
		k = k < 4 ? (k + 1) * 6 : 30;
		fprintf(f1, "%d:%d %2d/%2d%+3d ", i, 6 - i, j, k - j, j - k + j);
	}
	if (f2 != NULL) fprintf(f2, "s%d:%d\t", i, 6 - i);

	k = flag < 0 ? wncnt : flag;
	odd_number(num, &i, &j, k);
	if (flag == -2) odd_cnt[i]++;
	if (g_ec != 1 || flag != -2) {
		k = k < 4 ? (k + 1) * 6 : 30;
		fprintf(f1, "%d:%d %2d/%2d%+3d ", i, 6 - i, j, k - j, j - k + j);
	}
	if (f2 != NULL) fprintf(f2, "s%d:%d\t", i, 6 - i);

	group_number(num, a1, a2, 0);
	if (flag == -2) group_cnt[a1[6] - 'a']++;
	if (g_ec != 1 || flag != -2)
		fprintf(f1, "%c%c%c%c%c%c:%c-%d%d%d%d%d%d%d%d%d:%d ",
		a1[0], a1[1], a1[2], a1[3], a1[4], a1[5], a1[6],
		a2[0], a2[1], a2[2], a2[3], a2[4], a2[5], a2[6], a2[7], a2[8], a2[9]);
	if (f2 != NULL) fprintf(f2, "%c%c%c%c%c%c\t%c\ts%d%d%d%d%d%d%d%d%d\t%d\t",
	a1[0], a1[1], a1[2], a1[3], a1[4], a1[5], a1[6],
	a2[0], a2[1], a2[2], a2[3], a2[4], a2[5], a2[6], a2[7], a2[8], a2[9]);

	same_number(num, a1, a2);
	if (flag == -2) {
		same1_cnt[a1[10]]++;
		same2_cnt[a2[5]]++;
	}
	if (g_ec != 1 || flag != -2)
		fprintf(f1, "%d%d%d%d%d%d%d%d%d%d:%d %d%d%d%d%d:%d ",
		a1[0], a1[1], a1[2], a1[3], a1[4], a1[5], a1[6], a1[7], a1[8], a1[9], a1[10],
		a2[0], a2[1], a2[2], a2[3], a2[4], a2[5]);
	if (f2 != NULL) fprintf(f2, "s%d%d%d%d%d%d%d%d%d%d\t%d\ts%d%d%d%d%d\t%d\t",
	a1[0], a1[1], a1[2], a1[3], a1[4], a1[5], a1[6], a1[7], a1[8], a1[9], a1[10],
	a2[0], a2[1], a2[2], a2[3], a2[4], a2[5]);

	sub_number(num, a1);
	if (flag == -2) sub_cnt[a1[5]]++;
	if (g_ec != 1 || flag != -2)
		fprintf(f1, "%2d-%2d-%2d-%2d-%2d:%d ", a1[0], a1[1], a1[2], a1[3], a1[4], a1[5]);
	if (f2 != NULL) fprintf(f2, "s%2d-%2d-%2d-%2d-%2d\t%d\t", a1[0], a1[1], a1[2], a1[3], a1[4], a1[5]);

	group_number(num, a1, a2, 1);
	if (g_ec != 1 || flag != -2)
		fprintf(f1, "%c%c%c%c%c%c:%c ", a1[0], a1[1], a1[2], a1[3], a1[4], a1[5], a1[6]);
	if (f2 != NULL) fprintf(f2, "%c%c%c%c%c%c\t%c\t", a1[0], a1[1], a1[2], a1[3], a1[4], a1[5], a1[6]);

	i = seq_number(num);
	j = same_past_number(num, flag < 0 ? wncnt : flag);
	k = same_past_number(num, flag < 0 ? wncnt - 1 : flag - 1);
	if (flag == -2) {
		seq_cnt[i]++;
		samepast_cnt[j]++;
	}
	if (g_ec != 1 || flag != -2) fprintf(f1, "%d %d %d ", i, j, k);
	if (f2 != NULL) fprintf(f2, "%d\t%d\t%d\t", i, j, k);

	avg_number(num, a1, flag < 0 ? wncnt : flag);
	if (flag == -2) avg_cnt[a1[6]]++;
	if (g_ec != 1 || flag != -2)
		fprintf(f1, "%c%c%c%c%c%c:%d ", a1[0], a1[1], a1[2], a1[3], a1[4], a1[5], a1[6]);
	if (f2 != NULL) fprintf(f2, "%c%c%c%c%c%c\t%d\t", a1[0], a1[1], a1[2], a1[3], a1[4], a1[5], a1[6]);

	hot_number(num, a1, a2, &i, &j, flag < 0 ? wncnt : flag);
	if (g_ec != 1 || flag != -2)
		fprintf(f1, "%2d+%2d+%2d+%2d+%2d+%2d=%3d %9f %d ", a1[0], a1[1], a1[2], a1[3], a1[4], a1[5], i, (float)i / 6, a1[6]);
	if (f2 != NULL) fprintf(f2, "s%2d+%2d+%2d+%2d+%2d+%2d\t%d\t%f\t%d\t", a1[0], a1[1], a1[2], a1[3], a1[4], a1[5], i, (float)i / 6, a1[6]);

	past_winning_number(num, h, wncnt);
	if (flag == -2) {
		h5_cnt += h[5];
		h5b_cnt += h[7];
		h6_cnt += h[6];
	}
	if (g_ec != 1 || flag != -2)
		fprintf(f1, "(%4d %4d %3d %3d %2d %d %d %d)\n", h[0], h[1], h[2], h[3], h[4], h[5], h[7], h[6]);
	if (f2 != NULL) fprintf(f2, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", h[0], h[1], h[2], h[3], h[4], h[5], h[7], h[6]);

	if (flag == -2) {
		for (i = 0; i < 6; i++) num_cnt[num[i]]++;

		if (h[0] > h0max) {
			for (i = 0; i < 6; i++) hit0max[i] = num[i];
			h0max = h[0];
		}
		if (h[0] < h0min) {
			for (i = 0; i < 6; i++) hit0min[i] = num[i];
			h0min = h[0];
		}
		if (h[1] > h1max) {
			for (i = 0; i < 6; i++) hit1max[i] = num[i];
			h1max = h[1];
		}
		if (h[1] < h1min) {
			for (i = 0; i < 6; i++) hit1min[i] = num[i];
			h1min = h[1];
		}
		if (h[2] > h2max) {
			for (i = 0; i < 6; i++) hit2max[i] = num[i];
			h2max = h[2];
		}
		if (h[2] < h2min) {
			for (i = 0; i < 6; i++) hit2min[i] = num[i];
			h2min = h[2];
		}
		if (h[3] > h3max) {
			for (i = 0; i < 6; i++) hit3max[i] = num[i];
			h3max = h[3];
		}
		if (h[4] > h4max) {
			for (i = 0; i < 6; i++) hit4max[i] = num[i];
			h4max = h[4];
		}
		if (h[5] > h5max) {
			for (i = 0; i < 6; i++) hit5max[i] = num[i];
			h5max = h[5];
		}
	}
}

void main(int argc, char **argv)
{
	char en[NUMMAX][6], num[6], numcnt[44][44], *numrnd;
	char cht1[44], cht2[44], cht3[44], cht4[44], cht5[44], cht6[44], cht7[44], cht8[44], *cp;
	char chtm1[44], chtm3[44], chtm4[44], chtm7[44], *cmp;
	char alnum[44][12], maxday[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	char *alstr[] = {
		"03/21", "04/20", "05/21", "06/21", "07/23", "08/23",
		"09/23", "10/23", "11/22", "12/22", "01/20", "02/19"
	};
	short chth1[7], chth2[7], chth3[7], chth4[7], chth5[7], chth6[7], chth7[7], chth8[7], chtmh1[7], chtmh3[7], chtmh4[7], chtmh7[7];
	short a1[37], a2[37], actwncnt, anatype = argc > 1 ? atoi(argv[1]) : 0, anacnt = argc > 2 ? atoi(argv[2]) : 13, anahit = argc > 3 ? atoi(argv[3]) : 6;
	int cht1max = 0, cht1min = 0, cht1avg = 0, cht1sd = 0, cht1cnt = 0, cht1gap = 0;
	int chtm1max = 0, chtm1min = 0, chtm1avg = 0, chtm1sd = 0, chtm1cnt = 0, chtm1gap = 0;
	int cht2max = 0, cht2min = 0, cht2avg = 0, cht2sd = 0, cht2cnt = 0, cht2gap = 0;
	int cht3max = 0, cht3min = 0, cht3avg = 0, cht3sd = 0, cht3cnt = 0, cht3gap = 0;
	int chtm3max = 0, chtm3min = 0, chtm3avg = 0, chtm3sd = 0, chtm3cnt = 0, chtm3gap = 0;
	int cht4max = 0, cht4min = 0, cht4avg = 0, cht4sd = 0, cht4cnt = 0, cht4gap = 0;
	int chtm4max = 0, chtm4min = 0, chtm4avg = 0, chtm4sd = 0, chtm4cnt = 0, chtm4gap = 0;
	int cht5max = 0, cht5min = 0, cht5avg = 0, cht5sd = 0, cht5cnt = 0, cht5gap = 0;
	int cht6max = 0, cht6min = 0, cht6avg = 0, cht6sd = 0, cht6cnt = 0, cht6gap = 0;
	int cht7max = 0, cht7min = 0, cht7avg = 0, cht7sd = 0, cht7cnt = 0, cht7gap = 0;
	int chtm7max = 0, chtm7min = 0, chtm7avg = 0, chtm7sd = 0, chtm7cnt = 0, chtm7gap = 0;
	int cht8max = 0, cht8min = 0, cht8avg = 0, cht8sd = 0, cht8cnt = 0, cht8gap = 0;
	int i, j, k, l, m, n, o, p, q, r;
	float wnumavg, chtp1, chtp2, chtp3, chtp4, chtp5, chtp6, chtp7, chtp8, chtmp1, chtmp3, chtmp4, chtmp7, f, chtp[44];
	float chta1, chta2, chta3, chta4, chta5, chta6, chta7, chta8, chtma1, chtma3, chtma4, chtma7;
	FILE *f1, *f2 = NULL;

	/* 当選データ読み込み。*/
	if ((f1 = fopen("wn.txt", "r")) == NULL) return;
	for (wncnt = 0; wncnt < NUMMAX && get_number(f1, wn[wncnt], 7, 7, 1, 43, 0, 6, 0); wncnt++)
		for (i = 0; i < 6; i++) wnum_cnt1[wn[wncnt][i]]++;
	fclose(f1);

	if (anatype > 0) {
		if ((f1 = fopen("anas.txt", anatype == 1 && anacnt < 0 ? "w" : "a")) == NULL) return;
	} else {
		/* 予想データ読み込み。*/
		i = 0;
		if ((f1 = fopen("exp.txt", "r")) != NULL) {
			for (; i < NUMMAX && get_number(f1, en[i], 6, 6, 1, 43, 0, 6, 0); i++);
			fclose(f1);
		}

		if (!init_script(wncnt) || (f1 = fopen("ana.txt", "w")) == NULL) {
			free(g_code);
			return;
		}

		printf("0");
		if (i > 0) {
			fprintf(f1, "予想数字            " ITEMNAME);
			if (g_arf && (f2 = fopen("ana_exp.txt", "w")) != NULL) fprintf(f2, COLNAME);
			for (j = 0; j < i; j++) {
				printf("\b\b\b\b\b%d", j + 1);
				write_data(en[j], f1, f2, -1);
			}
			fprintf(f1, SEPAL);
			if (f2 != NULL) fclose(f2);
		}
		printf("\n");

		fprintf(f1, "当選数字            " ITEMNAME);
		if (g_arf && (f2 = fopen("ana_wn.txt", "w")) != NULL) fprintf(f2, COLNAME);
		if (wncnt > g_wnrange) fprintf(f1, "...\n");
		for (i = wncnt - g_wnrange; i < wncnt; i++) {
			printf("\b\b\b\b\b%d", i + 1);
			write_data(wn[i], f1, f2, i);
		}
		fprintf(f1, SEPAL);
		if (f2 != NULL) fclose(f2);
		printf("\n");

		printf("0");
		if (g_rndcnt > 0) {
			fprintf(f1, "ランダム数字        " ITEMNAME);
			if (g_arf && (f2 = fopen("ana_rnd.txt", "w")) != NULL) fprintf(f2, COLNAME);
			numrnd = (char *)calloc(wncnt * 6, sizeof(char));
			i = 0;
			for (j = 1; j < 44; j++)
				for (k = 0; k < wnum_cnt1[j]; k++) numrnd[i++] = j;
			for (i = 0; i < g_rndcnt; i++) {
				printf("\b\b\b\b\b%d", i + 1);

				/* ランダム数字生成。*/
				for (j = 0; j < 6; j++) {
					for (;;) {
						n = numrnd[next_int_ex(wncnt * 6)];
						for (k = 0; k < j && num[k] != n; k++);
						if (k < j) continue;
						num[j] = n;
						break;
					}
					for (k = j; k > 0 && num[k] <= num[k - 1]; k--) {
						n = num[k];
						num[k] = num[k - 1];
						num[k - 1] = n;
					}
				}

				write_data(num, f1, f2, -1);
			}
			free(numrnd);
			fprintf(f1, SEPAL);
			if (f2 != NULL) fclose(f2);
		}
		printf("\n");

		if (g_ec > 0) {
			fprintf(f1, "予想候補数字        " ITEMNAME);
			if (g_arf && (f2 = fopen("ana_ec.txt", "w")) != NULL) fprintf(f2, COLNAME);
			o = write_ec(wncnt, f1, f2);
			printf("\n");
			if (f2 != NULL) fclose(f2);

			fprintf(f1, "\n予想候補数字統計\n");
			fprintf(f1, "数: %d\n\n[合計]\n", o);
			for (i = 21; i < 244; i++)
				if (sum_cnt[i] > 0) fprintf(f1, "%3d %5d %9f%%\n", i, sum_cnt[i], (float)sum_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[大小]\n");
			for (i = 0; i < 7; i++)
				if (big_cnt[i] > 0) fprintf(f1, "%d:%d %7d %9f%%\n", i, 6 - i, big_cnt[i], (float)big_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[奇数偶数]\n");
			for (i = 0; i < 7; i++)
				if (odd_cnt[i] > 0) fprintf(f1, "%d:%d %7d %9f%%\n", i, 6 - i, odd_cnt[i], (float)odd_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[数字集団]\n");
			for (i = 0; i < 11; i++)
				if (group_cnt[i] > 0) fprintf(f1, "%c %7d %9f%%\n", i + 'a', group_cnt[i], (float)group_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[下1桁]\n");
			for (i = 0; i < 7; i++)
				if (same1_cnt[i] > 0) fprintf(f1, "%d %7d %9f%%\n", i, same1_cnt[i], (float)same1_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[同番台]\n");
			for (i = 2; i < 7; i++)
				if (same2_cnt[i] > 0) fprintf(f1, "%d %7d %9f%%\n", i, same2_cnt[i], (float)same2_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[隣接数字差]\n");
			for (i = 0; i < 7; i++)
				if (sub_cnt[i] > 0) fprintf(f1, "%d %7d %9f%%\n", i, sub_cnt[i], (float)sub_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[連番]\n");
			for (i = 0; i < 7; i++)
				if (seq_cnt[i] > 0) fprintf(f1, "%d %7d %9f%%\n", i, seq_cnt[i], (float)seq_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[ヒキ]\n");
			for (i = 0; i < 7; i++)
				if (samepast_cnt[i] > 0) fprintf(f1, "%d %7d %9f%%\n", i, samepast_cnt[i], (float)samepast_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[平均出現回数]\n");
			for (i = 0; i < 7; i++)
				if (avg_cnt[i] > 0) fprintf(f1, "%d %7d %9f%%\n", i, avg_cnt[i], (float)avg_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[過去当選コ]\n");
			fprintf(f1, "5  %6d %9f%%\n", h5_cnt, (float)h5_cnt / (float)o * 100);
			fprintf(f1, "5b %6d %9f%%\n", h5b_cnt, (float)h5b_cnt / (float)o * 100);
			fprintf(f1, "6  %6d %9f%%\n", h6_cnt, (float)h6_cnt / (float)o * 100);
			fprintf(f1, "\n[数字]\n");
			for (i = 1; i < 44; i++)
				if (num_cnt[i] > 0) fprintf(f1, "%2d %6d %9f%%\n", i, num_cnt[i], (float)num_cnt[i] / (float)o * 100);
			fprintf(f1, SEPAL);

			if (g_arf && (f2 = fopen("ana_mm.txt", "w")) != NULL) {
				write_data(hit0max, f2, NULL, -1);
				write_data(hit0min, f2, NULL, -1);
				write_data(hit1max, f2, NULL, -1);
				write_data(hit1min, f2, NULL, -1);
				write_data(hit2max, f2, NULL, -1);
				write_data(hit2min, f2, NULL, -1);
				write_data(hit3max, f2, NULL, -1);
				write_data(hit4max, f2, NULL, -1);
				write_data(hit5max, f2, NULL, -1);
				fclose(f2);
			}
		}
	}

	actwncnt = wncnt;
	chtp1 = chtp2 = chtp3 = chtp4 = chtp5 = chtp6 = chtp7 = chtp8 = chtmp1 = chtmp3 = chtmp4 = chtmp7 = chta1 = chta2 = chta3 = chta4 = chta5 = chta6 = chta7 = chta8 = chtma1 = chtma3 = chtma4 = chtma7 = 0;
	for (i = 0; i < 7; i++) chth1[i] = chth2[i] = chth3[i] = chth4[i] = chth5[i] = chth6[i] = chth7[i] = chth8[i] = chtmh1[i] = chtmh3[i] = chtmh4[i] = chtmh7[i] = 0;
	r = 0;
	for (wncnt = anacnt < 0 ? 50 : actwncnt - anacnt; wncnt <= actwncnt - (anatype > 0); wncnt++) {
		printf("\b\b\b\b%d", wncnt);
		wnumavg = (float)wncnt * 6 / 43;
		if (wncnt < actwncnt) r++;
		for (i = 1; i < 44; i++) wnum_cnt1[i] = 0;
		for (i = 0; i < wncnt; i++)
			for (j = 0; j < 6; j++) wnum_cnt1[wn[i][j]]++;

		if (wncnt == actwncnt) fprintf(f1, "統計一覧\n");
		cp = cht1;
		cmp = chtm1;
		for (i = 1; i < 44; i++) {
			j = k = l = m = n = o = 0;
			for (p = 0; p < wncnt; p++) {
				for (q = 0; q < 6 && wn[p][q] != i; q++);
				if (q < 6) {
					if (k < j) k = j;
					l = j;
					m += j;
					n += j * j;
					o++;
					j = 0;
				} else
					j++;
			}
			f = (float)m / (float)o;

			if (wncnt == actwncnt) fprintf(f1, "%2d: %2d %f | %2d %2d %9f %9f | %f %3d %10f%%\n", i,
				j, (float)j / f,	/* ハズレ。ハズレ/当選間隔平均。*/
				k, l, f, sqrt((float)n / (float)o - f * f),	/* 最長当選間隔。直前の当選間隔。当選間隔平均。当選間隔標準偏差。*/
				wnumavg, wnum_cnt1[i], (float)wnum_cnt1[i] / wnumavg * 100);	/* 当選期待値。当選合計。当選比率。*/

			if ((float)j / f >= 1 && (float)wnum_cnt1[i] / wnumavg * 100 >= 100) *cp++ = i;
			else if ((float)j / f < 1 && (float)wnum_cnt1[i] / wnumavg * 100 < 100) *cmp++ = i;
		}

		*cp = 0;
		cp = cht1;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n予:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %2d", *cp);
			else
				for (j = 0; j < 6; j++) {
					if (wn[wncnt][j] != *cp) continue;
					i++;
					break;
				}
			if (*++cp) {
				if (wncnt == actwncnt) fprintf(f1, ",");
			} else
				break;
		}
		if (wncnt < actwncnt) {
			chtp1 += (float)i / (float)(cp - cht1);
			chth1[i]++;
			chta1 += (float)(cp - cht1);

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				cht1hh->cnt = wncnt + 1;
				cht1hh->wn = wn[wncnt];
				cht1hh->gap = cht1gap;
				cht1hh->hit = i;
				cht1hh->encnt = cp - cht1;
				memcpy(cht1hh->en, cht1, cht1hh->encnt);
				cht1hh++;
			}
			if (i < anahit)
				cht1gap++;
			else {
				if (cht1max < cht1gap) cht1max = cht1gap;
				if (cht1min > cht1gap || cht1cnt == 0) cht1min = cht1gap;
				cht1avg += cht1gap;
				cht1sd += cht1gap * cht1gap;
				cht1cnt++;
				cht1gap = 0;
			}
		}

		*cmp = 0;
		cmp = chtm1;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n削:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %2d", *cmp);
			else
				for (j = 0; j < 6; j++) {
					if (wn[wncnt][j] != *cmp) continue;
					i++;
					break;
				}
			if (*++cmp) {
				if (wncnt == actwncnt) fprintf(f1, ",");
			} else
				break;
		}
		if (wncnt < actwncnt) {
			chtmp1 += (float)i / (float)(cmp - chtm1);
			chtmh1[i]++;
			chtma1 += (float)(cmp - chtm1);

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				chtm1hh->cnt = wncnt + 1;
				chtm1hh->wn = wn[wncnt];
				chtm1hh->gap = chtm1gap;
				chtm1hh->hit = i;
				chtm1hh->encnt = cmp - chtm1;
				memcpy(chtm1hh->en, chtm1, chtm1hh->encnt);
				chtm1hh++;
			}
			if (i < anahit)
				chtm1gap++;
			else {
				if (chtm1max < chtm1gap) chtm1max = chtm1gap;
				if (chtm1min > chtm1gap || chtm1cnt == 0) chtm1min = chtm1gap;
				chtm1avg += chtm1gap;
				chtm1sd += chtm1gap * chtm1gap;
				chtm1cnt++;
				chtm1gap = 0;
			}
		}
		if (wncnt == actwncnt) fprintf(f1, "\n" SEPAL);

		/* ------------------------------------------------------------ */

		if (wncnt == actwncnt) {
			fprintf(f1, "最後当選\n    |");
			for (i = 1; i < 44; i++) fprintf(f1, "%2d|", i);
			fprintf(f1, "\n");
		}
		for (i = wncnt - 40; i < wncnt; i++) {
			if (wncnt == actwncnt) fprintf(f1, "%4d|", i + 1);
			for (j = 1; j < 44; j++) {
				k = -1;
				for (l = i; l >= 0; l--) {
					for (m = 0; m < 6; m++) {
						if (wn[l][m] != j) continue;
						k = i - l;
						break;
					}
					if (k >= 0) break;
				}

				if (wncnt == actwncnt)
					if (k) fprintf(f1, "%2d|", k < 0 ? i + 1 : k);
					else fprintf(f1, "##|");
			}
			if (wncnt == actwncnt) fprintf(f1, "\n");
		}

		cp = cht2;
		for (i = 1; i < 44; i++) {
			/* 当選間隔長期間で当選して短期間にまた当選して現在のハズレ短期間。*/
			j = k = 0;
			l = wncnt;
			for (m = 0; m < 3; m++) {
				for (l--; l >= 0; l--) {
					for (n = 0; n < 6 && wn[l][n] != i; n++);
					if (n < 6) {
						k = 0;
						break;
					} else {
						k++;
						if (m < 2 && k > 3) {
							j = -1;
							break;
						} else if (m == 2 && k > 11) {
							j = 1;
							break;
						}
					}
				}
				if (j) break;
			}
			if (j == 1) {
				*cp++ = i;
				continue;
			}

			/* 過去数回の当選間隔が段々と長くなった後に当選。*/
			j = k = 0;
			l = wncnt;
			for (m = 0; m < 4; m++) {
				for (l--; l >= 0; l--) {
					for (n = 0; n < 6 && wn[l][n] != i; n++);
					if (n < 6) {
						if (m == 1) {
							if (k < 8) j = 1;
							o = k;
						} else if (m == 2 && (k > o || k < 8) || m == 3 && k < 3)
							j = 1;
						k = 0;
						break;
					} else {
						k++;
						if (m == 0 && k > 3 || m == 2 && k > 15 || m == 3 && k > 8) {
							j = 1;
							break;
						}
					}
				}
				if (j) break;
			}
			if (!j) {
				*cp++ = i;
				continue;
			}

			/* 過去数回の当選間隔が段々と短くなった後に当選。*/
			j = k = 0;
			l = wncnt;
			for (m = 0; m < 4; m++) {
				for (l--; l >= 0; l--) {
					for (n = 0; n < 6 && wn[l][n] != i; n++);
					if (n < 6) {
						if (m == 2) {
							if (k < 8) j = 1;
							o = k;
						} else if (m == 1 && k < 3 || m == 3 && k < o)
							j = 1;
						k = 0;
						break;
					} else {
						k++;
						if (m == 0 && k > 3 || m == 1 && k > 8 || m == 2 && k > 15) {
							j = 1;
							break;
						}
					}
				}
				if (j) break;
			}
			if (!j) {
				*cp++ = i;
				continue;
			}

			/* 同じ当選間隔±1が複数続いている。*/
			j = k = 0;
			l = wncnt;
			for (m = 0; m < 5; m++) {
				for (l--; l >= 0; l--) {
					for (n = 0; n < 6 && wn[l][n] != i; n++);
					if (n < 6) {
						if (m == 0)
							o = k;
						else {
							if (k < o - 1 || m == 1 && k > o + 1) j = 1;
							else if (m == 1) o = k;
						}
						k = 0;
						break;
					} else {
						k++;
						if (m > 1 && k > o + 1) {
							j = 1;
							break;
						}
					}
				}
				if (j) break;
			}
			if (m > 2 && m < 5) {
				*cp++ = i;
				continue;
			}

			/* 当選間隔短期間以内に当選して短期間ハズレている。*/
			j = k = 0;
			l = wncnt;
			for (m = 0; m < 3; m++) {
				for (l--; l >= 0; l--) {
					for (n = 0; n < 6 && wn[l][n] != i; n++);
					if (n < 6) {
						if (m == 0) o = k < 4 ? 0 : 1;
						else if (m > 0 && (o == 0 && k == 5 || o == 1 && k < 4)) j = 1;
						k = 0;
						break;
					} else if (++k > 5) {
						j = -1;
						break;
					}
				}
				if (j) break;
			}
			if (j == 1) {
				*cp++ = i;
				continue;
			}
		}

		*cp = 0;
		cp = cht2;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n予:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %2d", *cp);
			else
				for (j = 0; j < 6; j++) {
					if (wn[wncnt][j] != *cp) continue;
					i++;
					break;
				}
			if (*++cp) {
				if (wncnt == actwncnt) fprintf(f1, ",");
			} else
				break;
		}
		if (wncnt < actwncnt) {
			chtp2 += (float)i / (float)(cp - cht2);
			chth2[i]++;
			chta2 += (float)(cp - cht2);

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				cht2hh->cnt = wncnt + 1;
				cht2hh->wn = wn[wncnt];
				cht2hh->gap = cht2gap;
				cht2hh->hit = i;
				cht2hh->encnt = cp - cht2;
				memcpy(cht2hh->en, cht2, cht2hh->encnt);
				cht2hh++;
			}
			if (i < anahit)
				cht2gap++;
			else {
				if (cht2max < cht2gap) cht2max = cht2gap;
				if (cht2min > cht2gap || cht2cnt == 0) cht2min = cht2gap;
				cht2avg += cht2gap;
				cht2sd += cht2gap * cht2gap;
				cht2cnt++;
				cht2gap = 0;
			}
		}
		if (wncnt == actwncnt) fprintf(f1, "\n" SEPAL);

		/* ------------------------------------------------------------ */

		if (wncnt == actwncnt) fprintf(f1, "当選期間\n");
		for (i = 1; i < 44; i++) wnum_cnt2[i] = wnum_cnt3[i] = wnum_cnt4[i] = 0;
		cp = cht3;
		cmp = chtm3;
		for (i = 0; i < 6; i++) {
			for (j = wncnt - 50; j < wncnt; j++) wnum_cnt2[wn[j][i]]++;
			for (j = wncnt - 25; j < wncnt; j++) wnum_cnt3[wn[j][i]]++;
			for (j = wncnt - 10; j < wncnt; j++) wnum_cnt4[wn[j][i]]++;
		}
		for (i = 1; i < 44; i++) {
			j = -1;
			for (k = wncnt - 1; k >= 0; k--) {
				for (l = 0; l < 6; l++) {
					if (wn[k][l] != i) continue;
					j = wncnt - k - 1;
					break;
				}
				if (j >= 0) break;
			}

			if (wncnt == actwncnt) fprintf(f1, "%2d: %3d %f%% | %2d %f%% | %2d %f%% | %d %f%% | %2d\n", i,
				wnum_cnt1[i], (float)wnum_cnt1[i] / ((float)wncnt * 6) * 100,	/* 全ての回数。*/
				wnum_cnt2[i], (float)wnum_cnt2[i] / (50 * 6) * 100,	/* 直前50回数。*/
				wnum_cnt3[i], (float)wnum_cnt3[i] / (25 * 6) * 100,	/* 直前25回数。*/
				wnum_cnt4[i], (float)wnum_cnt4[i] / (10 * 6) * 100,	/* 直前10回数。*/
				j);	/* ハズレ。*/

			if ((float)wnum_cnt1[i] / ((float)wncnt * 6) * 100 > (float)wnum_cnt4[i] / (10 * 6) * 100 &&
			j < 12) *cp++ = i;
			else if ((float)wnum_cnt1[i] / ((float)wncnt * 6) * 100 < (float)wnum_cnt2[i] / (50 * 6) * 100)
				*cmp++ = i;
		}

		*cp = 0;
		cp = cht3;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n予:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %2d", *cp);
			else
				for (j = 0; j < 6; j++) {
					if (wn[wncnt][j] != *cp) continue;
					i++;
					break;
				}
			if (*++cp) {
				if (wncnt == actwncnt) fprintf(f1, ",");
			} else
				break;
		}
		if (wncnt < actwncnt) {
			chtp3 += (float)i / (float)(cp - cht3);
			chth3[i]++;
			chta3 += (float)(cp - cht3);

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				cht3hh->cnt = wncnt + 1;
				cht3hh->wn = wn[wncnt];
				cht3hh->gap = cht3gap;
				cht3hh->hit = i;
				cht3hh->encnt = cp - cht3;
				memcpy(cht3hh->en, cht3, cht3hh->encnt);
				cht3hh++;
			}
			if (i < anahit)
				cht3gap++;
			else {
				if (cht3max < cht3gap) cht3max = cht3gap;
				if (cht3min > cht3gap || cht3cnt == 0) cht3min = cht3gap;
				cht3avg += cht3gap;
				cht3sd += cht3gap * cht3gap;
				cht3cnt++;
				cht3gap = 0;
			}
		}

		*cmp = 0;
		cmp = chtm3;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n削:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %2d", *cmp);
			else
				for (j = 0; j < 6; j++) {
					if (wn[wncnt][j] != *cmp) continue;
					i++;
					break;
				}
			if (*++cmp) {
				if (wncnt == actwncnt) fprintf(f1, ",");
			} else
				break;
		}
		if (wncnt < actwncnt) {
			chtmp3 += (float)i / (float)(cmp - chtm3);
			chtmh3[i]++;
			chtma3 += (float)(cmp - chtm3);

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				chtm3hh->cnt = wncnt + 1;
				chtm3hh->wn = wn[wncnt];
				chtm3hh->gap = chtm3gap;
				chtm3hh->hit = i;
				chtm3hh->encnt = cmp - chtm3;
				memcpy(chtm3hh->en, chtm3, chtm3hh->encnt);
				chtm3hh++;
			}
			if (i < anahit)
				chtm3gap++;
			else {
				if (chtm3max < chtm3gap) chtm3max = chtm3gap;
				if (chtm3min > chtm3gap || chtm3cnt == 0) chtm3min = chtm3gap;
				chtm3avg += chtm3gap;
				chtm3sd += chtm3gap * chtm3gap;
				chtm3cnt++;
				chtm3gap = 0;
			}
		}
		if (wncnt == actwncnt) fprintf(f1, "\n" SEPAL);

		/* ------------------------------------------------------------ */

		if (wncnt == actwncnt) fprintf(f1, "飛後当選\n      ");
		for (i = 0; i < 37; i++) {
			a2[i] = 0;
			if (wncnt == actwncnt) fprintf(f1, "|%2d", i);
		}
		if (wncnt == actwncnt) fprintf(f1, "以上\n");
		for (i = 1; i < 44; i++) {
			for (j = 0; j < 37; j++) a1[j] = 0;
			if (wncnt == actwncnt) fprintf(f1, "%2d: ", i);

			j = 0;
			for (k = 0; k < wncnt; k++) {
				for (l = 0; l < 6 && wn[k][l] != i; l++);
				if (l < 6) {
					a1[j < 37 ? j : 36]++;
					j = 0;
				} else
					j++;
			}

			if (wncnt == actwncnt) fprintf(f1, "%2d", j);
			for (j = 0; j < 37; j++)
				if (a1[j]) {
					a2[j] += a1[j];
					if (wncnt == actwncnt) fprintf(f1, "|%2d", a1[j]);
				} else
					if (wncnt == actwncnt) fprintf(f1, "|  ");
			if (wncnt == actwncnt) fprintf(f1, "\n");
		}
		if (wncnt == actwncnt) {
			fprintf(f1, "AG:   ", i);
			for (i = 0; i < 37; i++) fprintf(f1, "|%2d", (int)((float)a2[i] / 43 + 0.5));
			fprintf(f1, "\n");
		}

		cp = cht4;
		cmp = chtm4;
		for (i = 1; i < 44; i++) {
			for (j = 0; j < 37; j++) a1[j] = 0;
			j = 0;
			for (k = 0; k < wncnt; k++) {
				for (l = 0; l < 6 && wn[k][l] != i; l++);
				if (l < 6) {
					a1[j < 37 ? j : 36]++;
					j = 0;
				} else
					j++;
			}
			if (j > 36) j = 36;

			k = (float)a2[j] / 43 + 0.5;
			if (k > 0 && a1[j] < k) *cp++ = i;
			else if (a1[j] >= k + 2) *cmp++ = i;
		}

		*cp = 0;
		cp = cht4;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n予:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %2d", *cp);
			else
				for (j = 0; j < 6; j++) {
					if (wn[wncnt][j] != *cp) continue;
					i++;
					break;
				}
			if (*++cp) {
				if (wncnt == actwncnt) fprintf(f1, ",");
			} else
				break;
		}
		if (wncnt < actwncnt) {
			chtp4 += (float)i / (float)(cp - cht4);
			chth4[i]++;
			chta4 += (float)(cp - cht4);

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				cht4hh->cnt = wncnt + 1;
				cht4hh->wn = wn[wncnt];
				cht4hh->gap = cht4gap;
				cht4hh->hit = i;
				cht4hh->encnt = cp - cht4;
				memcpy(cht4hh->en, cht4, cht4hh->encnt);
				cht4hh++;
			}
			if (i < anahit)
				cht4gap++;
			else {
				if (cht4max < cht4gap) cht4max = cht4gap;
				if (cht4min > cht4gap || cht4cnt == 0) cht4min = cht4gap;
				cht4avg += cht4gap;
				cht4sd += cht4gap * cht4gap;
				cht4cnt++;
				cht4gap = 0;
			}
		}

		*cmp = 0;
		cmp = chtm4;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n削:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %2d", *cmp);
			else
				for (j = 0; j < 6; j++) {
					if (wn[wncnt][j] != *cmp) continue;
					i++;
					break;
				}
			if (*++cmp) {
				if (wncnt == actwncnt) fprintf(f1, ",");
			} else
				break;
		}
		if (wncnt < actwncnt) {
			chtmp4 += (float)i / (float)(cmp - chtm4);
			chtmh4[i]++;
			chtma4 += (float)(cmp - chtm4);

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				chtm4hh->cnt = wncnt + 1;
				chtm4hh->wn = wn[wncnt];
				chtm4hh->gap = chtm4gap;
				chtm4hh->hit = i;
				chtm4hh->encnt = cmp - chtm4;
				memcpy(chtm4hh->en, chtm4, chtm4hh->encnt);
				chtm4hh++;
			}
			if (i < anahit)
				chtm4gap++;
			else {
				if (chtm4max < chtm4gap) chtm4max = chtm4gap;
				if (chtm4min > chtm4gap || chtm4cnt == 0) chtm4min = chtm4gap;
				chtm4avg += chtm4gap;
				chtm4sd += chtm4gap * chtm4gap;
				chtm4cnt++;
				chtm4gap = 0;
			}
		}
		if (wncnt == actwncnt) fprintf(f1, "\n" SEPAL);

		/* ------------------------------------------------------------ */

		if (wncnt == actwncnt) fprintf(f1, "多重当選\n  ");
		for (i = 1; i < 5; i++) {
			a2[i] = 0;
			if (wncnt == actwncnt) fprintf(f1, "| %d", i);
		}
		if (wncnt == actwncnt) fprintf(f1, "|多当比率\n");
		i = j = 0;
		for (k = 1; k < 44; k++) {
			for (l = 1; l < 5; l++) a1[l] = 0;
			if (wncnt == actwncnt) fprintf(f1, "%2d", k);

			l = m = n = 0;
			for (o = 0; o < wncnt; o++) {
				for (p = 0; p < 6 && wn[o][p] != k; p++);
				if (p < 6) {
					if (l) {	/* 多重当選？ */
						a1[l < 5 ? l : 5]++;
						m++;
					}
					l++;
					n++;
				} else
					l = 0;
			}
			i += m;
			j += n;

			if (wncnt == actwncnt) {
				for (l = 1; l < 5; l++)
					if (a1[l]) fprintf(f1, "|%2d", a1[l]);
					else fprintf(f1, "|  ");
				fprintf(f1, "|%9f%%\n", (float)m / (float)n * 100);
			}
		}
		if (wncnt == actwncnt) fprintf(f1, "平均          |%9f%%\n", (float)i / (float)j * 100);

		cp = cht5;
		for (k = 0; k < 6; k++) {
			for (l = 1; l < 5; l++) a1[l] = 0;

			l = m = n = 0;
			for (o = 0; o < wncnt; o++) {
				for (p = 0; p < 6 && wn[o][p] != wn[wncnt - 1][k]; p++);
				if (p < 6) {
					if (l) {	/* 多重当選？ */
						a1[l < 5 ? l : 5]++;
						m++;
					}
					l++;
					n++;
				} else
					l = 0;
			}

			if ((float)m / (float)n * 100 < (float)i / (float)j * 100) *cp++ = wn[wncnt - 1][k];
		}

		*cp = 0;
		cp = cht5;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n予:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %2d", *cp);
			else
				for (j = 0; j < 6; j++) {
					if (wn[wncnt][j] != *cp) continue;
					i++;
					break;
				}
			if (*++cp) {
				if (wncnt == actwncnt) fprintf(f1, ",");
			} else
				break;
		}
		if (wncnt < actwncnt) {
			chtp5 += (float)i / (float)(cp - cht5);
			chth5[i]++;
			chta5 += (float)(cp - cht5);

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				cht5hh->cnt = wncnt + 1;
				cht5hh->wn = wn[wncnt];
				cht5hh->gap = cht5gap;
				cht5hh->hit = i;
				cht5hh->encnt = cp - cht5;
				memcpy(cht5hh->en, cht5, cht5hh->encnt);
				cht5hh++;
			}
			if (i < anahit)
				cht5gap++;
			else {
				if (cht5max < cht5gap) cht5max = cht5gap;
				if (cht5min > cht5gap || cht5cnt == 0) cht5min = cht5gap;
				cht5avg += cht5gap;
				cht5sd += cht5gap * cht5gap;
				cht5cnt++;
				cht5gap = 0;
			}
		}
		if (wncnt == actwncnt) fprintf(f1, "\n" SEPAL);

		/* ------------------------------------------------------------ */

		if (wncnt == actwncnt) fprintf(f1, "当間追跡\n");
		for (i = 0; i < 37; i++) a1[i] = 0;
		for (i = wncnt - 5; i < wncnt; i++)
			for (j = 0; j < 6; j++) {
				k = 0;
				for (l = i - 1; l >= 0; l--) {
					for (m = 0; m < 6 && wn[i][j] != wn[l][m]; m++);
					if (m < 6) break;
					k++;
				}
				a1[k < 36 ? k : 36]++;
			}
		if (wncnt == actwncnt) {
			for (i = 0; i < 37; i++) {
				fprintf(f1, "%2d: %d |", i, a1[i]);

				j = 0;
				for (k = 1; k < 44; k++) {
					l = 0;
					for (m = wncnt - 1; m >= 0 && l < 36; m--) {
						for (o = 0; o < 6 && wn[m][o] != k; o++);
						if (o < 6) break;
						l++;
					}
					if (i != l) continue;
					if (j) fprintf(f1, ",");
					fprintf(f1, " %2d", k);
					j = 1;
				}
				fprintf(f1, "\n");
			}
			fprintf(f1, SEPAL);
		}

		/* ------------------------------------------------------------ */

		if (wncnt == actwncnt) fprintf(f1, "前後数当\n");
		i = j = 0;
		for (k = 1; k < 44; k++) {
			l = m = 0;
			for (n = 1; n < wncnt; n++)
				for (o = 0; o < 6; o++) {
					if (wn[n][o] != k) continue;
					for (p = 0; p < 6; p++)
						if (wn[n - 1][p] == (k > 1 ? k - 1 : 43)) l++;	/* 数字-1 */
						else if (wn[n - 1][p] == (k < 43 ? k + 1 : 1)) m++;	/* 数字+1 */
					break;
				}
			i += l;
			j += m;

			if (wncnt == actwncnt) fprintf(f1, "%2d: %3d | %2d %9f%% | %2d %9f%%\n", k, wnum_cnt1[k],
				l, (float)l / (float)wnum_cnt1[k] * 100,
				m, (float)m / (float)wnum_cnt1[k] * 100);
		}
		i = (float)i / 43 + 0.5;
		j = (float)j / 43 + 0.5;

		if (wncnt == actwncnt) fprintf(f1, "AG: %3d | %2d %9f%% | %2d %9f%%\n", (int)(wnumavg + 0.5),
			i, (float)i / wnumavg * 100, j, (float)j / wnumavg * 100);

		cp = cht6;
		for (k = 0; k < 6; k++) {
			l = m = 0;
			for (n = 1; n < wncnt; n++)
				for (o = 0; o < 6; o++) {
					if (wn[n][o] != wn[wncnt - 1][k]) continue;
					for (p = 0; p < 6; p++)
						if (wn[n - 1][p] == (wn[wncnt - 1][k] > 1 ? wn[wncnt - 1][k] - 1 : 43))
							l++;	/* 数字-1 */
						else if (wn[n - 1][p] == (wn[wncnt - 1][k] < 43 ? wn[wncnt - 1][k] + 1 : 1))
							m++;	/* 数字+1 */
					break;
				}

			if ((float)l / (float)wnum_cnt1[wn[wncnt - 1][k]] * 100 < (float)i / wnumavg * 100)
				*cp++ = (wn[wncnt - 1][k] > 1 ? wn[wncnt - 1][k] - 1 : 43);
			if ((float)m / (float)wnum_cnt1[wn[wncnt - 1][k]] * 100 < (float)j / wnumavg * 100)
				*cp++ = (wn[wncnt - 1][k] < 43 ? wn[wncnt - 1][k] + 1 : 1);
		}

		*cp = 0;
		for (i = 0; cht6[i]; i++)
			for (j = i + 1; cht6[j]; j++) {
				if (cht6[i] <= cht6[j]) continue;
				k = cht6[i];
				cht6[i]= cht6[j];
				cht6[j] = k;
			}
		cp = cht6;
		i = j = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n予:");
		for (;;) {
			for (k = 0; &cht6[k] < cp && cht6[k] != *cp; k++);
			if (&cht6[k] >= cp) {
				if (wncnt == actwncnt) {
					if (k) fprintf(f1, ",");
					fprintf(f1, " %d", *cp);
				} else
					for (k = 0; k < 6; k++) {
						if (wn[wncnt][k] != *cp) continue;
						i++;
						break;
					}
				j++;
			}
			if (!*++cp) break;
		}
		if (wncnt < actwncnt) {
			chtp6 += (float)i / (float)(cp - cht6);
			chth6[i]++;
			chta6 += (float)j;

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				cht6hh->cnt = wncnt + 1;
				cht6hh->wn = wn[wncnt];
				cht6hh->gap = cht6gap;
				cht6hh->hit = i;
				cht6hh->encnt = j;
				cp = cht6;
				for (j = 0; ; ) {
					for (k = 0; &cht6[k] < cp && cht6[k] != *cp; k++);
					if (&cht6[k] >= cp) cht6hh->en[j++] = *cp;
					if (!*++cp) break;
				}
				cht6hh++;
			}
			if (i < anahit)
				cht6gap++;
			else {
				if (cht6max < cht6gap) cht6max = cht6gap;
				if (cht6min > cht6gap || cht6cnt == 0) cht6min = cht6gap;
				cht6avg += cht6gap;
				cht6sd += cht6gap * cht6gap;
				cht6cnt++;
				cht6gap = 0;
			}
		}
		if (wncnt == actwncnt) fprintf(f1, "\n" SEPAL);

		/* ------------------------------------------------------------ */

		if (wncnt == actwncnt) fprintf(f1, "後追い数\n  ");
		for (i = 0; i < 6; i++) {
			if (wncnt == actwncnt) fprintf(f1, "|%2d", wn[wncnt - 1][i]);
			for (j = 1; j < 44; j++) numcnt[i][j] = 0;
			a1[i] = 0;

			for (j = 0; j < wncnt - 1; j++)
				for (k = 0; k < 6; k++) {
					if (wn[wncnt - 1][i] != wn[j][k]) continue;
					for (l = 0; l < 6; l++) numcnt[i][wn[j + 1][l]]++;
					break;
				}
		}
		if (wncnt == actwncnt) fprintf(f1, "|計\n");
		for (i = 1; i < 44; i++) {
			if (wncnt == actwncnt) fprintf(f1, "%2d", i);
			j = 0;
			for (k = 0; k < 6; k++) {
				if (wncnt == actwncnt) fprintf(f1, "|%2d", numcnt[k][i]);
				j += numcnt[k][i];
				a1[k] += numcnt[k][i];
			}
			if (wncnt == actwncnt) fprintf(f1, "|%d\n", j);
		}
		if (wncnt == actwncnt) fprintf(f1, "AG");
		i = 0;
		for (j = 0; j < 6; j++) {
			if (wncnt == actwncnt) fprintf(f1, "|%2d", (int)((float)a1[j] / 43 + 0.5));
			i += a1[j];
		}
		i = (float)i / 43 + 0.5;
		if (wncnt == actwncnt) fprintf(f1, "|%2d\n", i);

		cp = cht7;
		cmp = chtm7;
		for (j = 1; j < 44; j++) {
			k = 0;
			for (l = 0; l < 6; l++) k += numcnt[l][j];

			if (k > i + 6) *cp++ = j;
			else if (k < i - 3) *cmp++ = j;
		}

		*cp = 0;
		cp = cht7;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n予:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %2d", *cp);
			else
				for (j = 0; j < 6; j++) {
					if (wn[wncnt][j] != *cp) continue;
					i++;
					break;
				}
			if (*++cp) {
				if (wncnt == actwncnt) fprintf(f1, ",");
			} else
				break;
		}
		if (wncnt < actwncnt) {
			chtp7 += (float)i / (float)(cp - cht7);
			chth7[i]++;
			chta7 += (float)(cp - cht7);

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				cht7hh->cnt = wncnt + 1;
				cht7hh->wn = wn[wncnt];
				cht7hh->gap = cht7gap;
				cht7hh->hit = i;
				cht7hh->encnt = cp - cht7;
				memcpy(cht7hh->en, cht7, cht7hh->encnt);
				cht7hh++;
			}
			if (i < anahit)
				cht7gap++;
			else {
				if (cht7max < cht7gap) cht7max = cht7gap;
				if (cht7min > cht7gap || cht7cnt == 0) cht7min = cht7gap;
				cht7avg += cht7gap;
				cht7sd += cht7gap * cht7gap;
				cht7cnt++;
				cht7gap = 0;
			}
		}

		*cmp = 0;
		cmp = chtm7;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n削:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %2d", *cmp);
			else
				for (j = 0; j < 6; j++) {
					if (wn[wncnt][j] != *cmp) continue;
					i++;
					break;
				}
			if (*++cmp) {
				if (wncnt == actwncnt) fprintf(f1, ",");
			} else
				break;
		}
		if (wncnt < actwncnt) {
			chtmp7 += (float)i / (float)(cmp - chtm7);
			chtmh7[i]++;
			chtma7 += (float)(cmp - chtm7);

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				chtm7hh->cnt = wncnt + 1;
				chtm7hh->wn = wn[wncnt];
				chtm7hh->gap = chtm7gap;
				chtm7hh->hit = i;
				chtm7hh->encnt = cmp - chtm7;
				memcpy(chtm7hh->en, chtm7, chtm7hh->encnt);
				chtm7hh++;
			}
			if (i < anahit)
				chtm7gap++;
			else {
				if (chtm7max < chtm7gap) chtm7max = chtm7gap;
				if (chtm7min > chtm7gap || chtm7cnt == 0) chtm7min = chtm7gap;
				chtm7avg += chtm7gap;
				chtm7sd += chtm7gap * chtm7gap;
				chtm7cnt++;
				chtm7gap = 0;
			}
		}
		if (wncnt == actwncnt) fprintf(f1, "\n" SEPAL);

		/* ------------------------------------------------------------ */

		if (wncnt == actwncnt) {
			fprintf(f1, "同伴数字\n");
			fprintf(f1, "   ");
			for (i = 1; i < 44; i++) fprintf(f1, "|%2d", i);
			fprintf(f1, "\n");
		}
		for (i = 1; i < 44; i++) {
			for (j = 1; j < 44; j++) numcnt[i][j] = 0;
			for (j = 0; j < wncnt; j++)
				for (k = 0; k < 6; k++) {
					if (wn[j][k] != i) continue;
					for (l = 0; l < 6; l++) numcnt[i][wn[j][l]]++;
					break;
				}

			if (wncnt == actwncnt) {
				fprintf(f1, "%2d ", i);
				j = (float)wncnt * 0.02 + 0.5;
				for (k = 1; k < 44; k++)
					if (i == k || numcnt[i][k] < j) fprintf(f1, "|  ");
					else fprintf(f1, "|%2d", numcnt[i][k]);
				fprintf(f1, "\n");
			}
		}
		if (wncnt == actwncnt) fprintf(f1, SEPAL);

		/* ------------------------------------------------------------ */

		if (wncnt == actwncnt) fprintf(f1, "となり組\n");
		i = 0;
		for (j = 1; j < 43; j++) {
			k = 0;
			for (l = 0; l < wncnt; l++)
				for (m = 0; m < 5; m++) {
					if (wn[l][m] != j || wn[l][m + 1] != j + 1) continue;
					k++;
					break;
				}
			i += k;

			if (wncnt == actwncnt) fprintf(f1, "%2d-%2d: %2d %3d %9f%%\n", j, j + 1, k, wnum_cnt1[j], (float)k / (float)wnum_cnt1[j] * 100);
		}
		f = (float)i / 42;
		if (wncnt == actwncnt) {
			fprintf(f1, "A V G: %2d %3d %9f%%\n", (int)(f + 0.5), (int)(wnumavg + 0.5), f / wnumavg * 100);
			fprintf(f1, SEPAL);
		}

		/* ------------------------------------------------------------ */

		if (wncnt == actwncnt) fprintf(f1, "占星術\n");
		for (i = 1; i < 44; i++)
			for (j = 0; j < 12; j++) alnum[i][j] = 0;
		i = 2000;
		j = 9;
		k = 5;
		p = 4;
		for (l = 0; l < wncnt; l++) {
			maxday[1] = i % 4 == 0 && i % 100 != 0 || i % 400 == 0 ? 29 : 28;	/* 閏年。*/

			m = (j + 1) * 100 + k;
			if (m >= 321 && m <= 419) n = 0;
			else if (m >= 420 && m <= 520) n = 1;
			else if (m >= 521 && m <= 620) n = 2;
			else if (m >= 621 && m <= 722) n = 3;
			else if (m >= 723 && m <= 822) n = 4;
			else if (m >= 823 && m <= 922) n = 5;
			else if (m >= 923 && m <= 1022) n = 6;
			else if (m >= 1023 && m <= 1121) n = 7;
			else if (m >= 1122 && m <= 1221) n = 8;
			else if (m >= 1222 && m <= 1231 || m >= 101 && m <= 119) n = 9;
			else if (m >= 120 && m <= 218) n = 10;
			else /* if (m >= 219 && m <= 320) */ n = 11;
			for (o = 0; o < 6; o++) alnum[wn[l][o]][n]++;

			do {
				if (i > 2011 || i == 2011 && (j > 2 || j == 2 && k == 31)) {
					k += p;
					p = 7 - p;	/* 週2回。*/
				} else
					k += 7;	/* 週1回。*/
				if (k > maxday[j]) {
					k -= maxday[j];

					if (j < 11)
						j++;
					else {
						j = 0;
						i++;
					}
				}
			} while (j == 11 && k == 31 || j == 0 && k >= 1 && k <= 3);	/* 年末年始をとばす。*/
		}

		m = (j + 1) * 100 + k;
		if (m >= 321 && m <= 419) n = 0;
		else if (m >= 420 && m <= 520) n = 1;
		else if (m >= 521 && m <= 620) n = 2;
		else if (m >= 621 && m <= 722) n = 3;
		else if (m >= 723 && m <= 822) n = 4;
		else if (m >= 823 && m <= 922) n = 5;
		else if (m >= 923 && m <= 1022) n = 6;
		else if (m >= 1023 && m <= 1121) n = 7;
		else if (m >= 1122 && m <= 1221) n = 8;
		else if (m >= 1222 && m <= 1231 || m >= 101 && m <= 119) n = 9;
		else if (m >= 120 && m <= 218) n = 10;
		else /* if (m >= 219 && m <= 320) */ n = 11;

		if (wncnt == actwncnt) {
			fprintf(f1, "  |");
			for (i = 0; i < 12; i++) fprintf(f1, "%s|", alstr[i]);
			fprintf(f1, "\n");
			for (i = 1; i < 44; i++) {
				fprintf(f1, "%2d|", i);
				for (j = 0; j < 12; j++) fprintf(f1, "%2d   |", alnum[i][j]);
				fprintf(f1, "\n");
			}
			fprintf(f1, "AG|");
			for (i = 0; i < 12; i++) {
				j = 0;
				for (k = 1; k < 44; k++) j += alnum[k][i];
				fprintf(f1, "%5.2f|", (float)j / 43);
			}
			fprintf(f1, "\n");
		}

		i = 0;
		for (j = 1; j < 44; j++) i += alnum[j][n];
		i = (float)i / 43 + 0.5;
		cp = cht8;
		for (j = 1; j < 44; j++)
			if (alnum[j][n] > i) *cp++ = j;
		*cp = 0;
		cp = cht8;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n予:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %2d", *cp);
			else
				for (j = 0; j < 6; j++) {
					if (wn[wncnt][j] != *cp) continue;
					i++;
					break;
				}
			if (*++cp) {
				if (wncnt == actwncnt) fprintf(f1, ",");
			} else
				break;
		}
		if (wncnt < actwncnt) {
			chtp8 += (float)i / (float)(cp - cht8);
			chth8[i]++;
			chta8 += (float)(cp - cht8);

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				cht8hh->cnt = wncnt + 1;
				cht8hh->wn = wn[wncnt];
				cht8hh->gap = cht8gap;
				cht8hh->hit = i;
				cht8hh->encnt = cp - cht8;
				memcpy(cht8hh->en, cht8, cht8hh->encnt);
				cht8hh++;
			}
			if (i < anahit)
				cht8gap++;
			else {
				if (cht8max < cht8gap) cht8max = cht8gap;
				if (cht8min > cht8gap || cht8cnt == 0) cht8min = cht8gap;
				cht8avg += cht8gap;
				cht8sd += cht8gap * cht8gap;
				cht8cnt++;
				cht8gap = 0;
			}
		}
		if (wncnt == actwncnt) fprintf(f1, "\n" SEPAL);
	}

	/* ------------------------------------------------------------ */

	if (anatype < 2) {
		fprintf(f1, "スコア(");
		if (anatype > 0)
			if (anacnt < 0) fprintf(f1, "50回～");
			else fprintf(f1, "過去%d回 ", anacnt);
		fprintf(f1, "%d回)\n", wncnt);

		chtp1 = chtp1 / (float)r * 100;
		chtmp1 = chtmp1 / (float)r * 100;
		chtp2 = chtp2 / (float)r * 100;
		chtp3 = chtp3 / (float)r * 100;
		chtmp3 = chtmp3 / (float)r * 100;
		chtp4 = chtp4 / (float)r * 100;
		chtmp4 = chtmp4 / (float)r * 100;
		chtp5 = chtp5 / (float)r * 100;
		chtp6 = chtp6 / (float)r * 100;
		chtp7 = chtp7 / (float)r * 100;
		chtmp7 = chtmp7 / (float)r * 100;
		chtp8 = chtp8 / (float)r * 100;
		chta1 /= (float)r;
		chtma1 /= (float)r;
		chta2 /= (float)r;
		chta3 /= (float)r;
		chtma3 /= (float)r;
		chta4 /= (float)r;
		chtma4 /= (float)r;
		chta5 /= (float)r;
		chta6 /= (float)r;
		chta7 /= (float)r;
		chtma7 /= (float)r;
		chta8 /= (float)r;

		cp = cht1;
		fprintf(f1, "\n統計一覧 予 %4d %4d %4d %3d %3d %3d %2d %9f %9f%%",
			chth1[0], chth1[1], chth1[2], chth1[3], chth1[4], chth1[5], chth1[6], chta1, chtp1);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %2d", *cp++);
				if (*cp) fprintf(f1, ",");
				else break;
			}
		}
		cmp = chtm1;
		fprintf(f1, "\n         削 %4d %4d %4d %3d %3d %3d %2d %9f %9f%%",
			chtmh1[0], chtmh1[1], chtmh1[2], chtmh1[3], chtmh1[4], chtmh1[5], chtmh1[6], chtma1, chtmp1);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %2d", *cmp++);
				if (*cmp) fprintf(f1, ",");
				else break;
			}
		}

		cp = cht2;
		fprintf(f1, "\n最後当選 予 %4d %4d %4d %3d %3d %3d %2d %9f %9f%%",
			chth2[0], chth2[1], chth2[2], chth2[3], chth2[4], chth2[5], chth2[6], chta2, chtp2);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %2d", *cp++);
				if (*cp) fprintf(f1, ",");
				else break;
			}
		}

		cp = cht3;
		fprintf(f1, "\n当選期間 予 %4d %4d %4d %3d %3d %3d %2d %9f %9f%%",
			chth3[0], chth3[1], chth3[2], chth3[3], chth3[4], chth3[5], chth3[6], chta3, chtp3);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %2d", *cp++);
				if (*cp) fprintf(f1, ",");
				else break;
			}
		}
		cmp = chtm3;
		fprintf(f1, "\n         削 %4d %4d %4d %3d %3d %3d %2d %9f %9f%%",
			chtmh3[0], chtmh3[1], chtmh3[2], chtmh3[3], chtmh3[4], chtmh3[5], chtmh3[6], chtma3, chtmp3);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %2d", *cmp++);
				if (*cmp) fprintf(f1, ",");
				else break;
			}
		}

		cp = cht4;
		fprintf(f1, "\n飛後当選 予 %4d %4d %4d %3d %3d %3d %2d %9f %9f%%",
			chth4[0], chth4[1], chth4[2], chth4[3], chth4[4], chth4[5], chth4[6], chta4, chtp4);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %2d", *cp++);
				if (*cp) fprintf(f1, ",");
				else break;
			}
		}
		cmp = chtm4;
		fprintf(f1, "\n         削 %4d %4d %4d %3d %3d %3d %2d %9f %9f%%",
			chtmh4[0], chtmh4[1], chtmh4[2], chtmh4[3], chtmh4[4], chtmh4[5], chtmh4[6], chtma4, chtmp4);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %2d", *cmp++);
				if (*cmp) fprintf(f1, ",");
				else break;
			}
		}

		cp = cht5;
		fprintf(f1, "\n多重当選 予 %4d %4d %4d %3d %3d %3d %2d %9f %9f%%",
			chth5[0], chth5[1], chth5[2], chth5[3], chth5[4], chth5[5], chth5[6], chta5, chtp5);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %2d", *cp++);
				if (*cp) fprintf(f1, ",");
				else break;
			}
		}

		cp = cht6;
		fprintf(f1, "\n前後数当 予 %4d %4d %4d %3d %3d %3d %2d %9f %9f%%",
			chth6[0], chth6[1], chth6[2], chth6[3], chth6[4], chth6[5], chth6[6], chta6, chtp6);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				for (i = 0; &cht6[i] < cp && cht6[i] != *cp; i++);
				if (&cht6[i] < cp)
					*cp++;
				else {
					if (i) fprintf(f1, ",");
					fprintf(f1, " %2d", *cp++);
				}
				if (!*cp) break;
			}
		}

		cp = cht7;
		fprintf(f1, "\n後追い数 予 %4d %4d %4d %3d %3d %3d %2d %9f %9f%%",
			chth7[0], chth7[1], chth7[2], chth7[3], chth7[4], chth7[5], chth7[6], chta7, chtp7);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %2d", *cp++);
				if (*cp) fprintf(f1, ",");
				else break;
			}
		}
		cmp = chtm7;
		fprintf(f1, "\n         削 %4d %4d %4d %3d %3d %3d %2d %9f %9f%%",
			chtmh7[0], chtmh7[1], chtmh7[2], chtmh7[3], chtmh7[4], chtmh7[5], chtmh7[6], chtma7, chtmp7);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %2d", *cmp++);
				if (*cmp) fprintf(f1, ",");
				else break;
			}
		}

		cp = cht8;
		fprintf(f1, "\n占星術   予 %4d %4d %4d %3d %3d %3d %2d %9f %9f%%",
			chth8[0], chth8[1], chth8[2], chth8[3], chth8[4], chth8[5], chth8[6], chta8, chtp8);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %2d", *cp++);
				if (*cp) fprintf(f1, ",");
				else break;
			}
		}

		if (anatype > 0) fprintf(f1, "\n" SEPAL);
		else fprintf(f1, "\n最新回のボーナス数字                                          : %2d\n\n", wn[actwncnt - 1][6]);
	}

	if (anatype <= 0) {
		wnumavg = 0;
		for (i = 1; i < 44; i++) {
			f = 0;
			for (cp = cht1; *cp; cp++) {
				if (i != *cp) continue;
				f += chtp1;
				break;
			}
			for (cmp = chtm1; *cmp; cmp++) {
				if (i != *cmp) continue;
				f -= chtp1 - chtmp1;
				break;
			}
			for (cp = cht2; *cp; cp++) {
				if (i != *cp) continue;
				f += chtp2;
				break;
			}
			for (cp = cht3; *cp; cp++) {
				if (i != *cp) continue;
				f += chtp3;
				break;
			}
			for (cmp = chtm3; *cmp; cmp++) {
				if (i != *cmp) continue;
				f -= chtp3 - chtmp3;
				break;
			}
			for (cp = cht4; *cp; cp++) {
				if (i != *cp) continue;
				f += chtp4;
				break;
			}
			for (cmp = chtm4; *cmp; cmp++) {
				if (i != *cmp) continue;
				f -= chtp4 - chtmp4;
				break;
			}
			for (cp = cht5; *cp; cp++) {
				if (i != *cp) continue;
				f += chtp5;
				break;
			}
			for (cp = cht6; *cp; cp++) {
				if (i != *cp) continue;
				f += chtp6;
				break;
			}
			for (cp = cht7; *cp; cp++) {
				if (i != *cp) continue;
				f += chtp7;
				break;
			}
			for (cmp = chtm7; *cmp; cmp++) {
				if (i != *cmp) continue;
				f -= chtp7 - chtmp7;
				break;
			}
			for (cp = cht8; *cp; cp++) {
				if (i != *cp) continue;
				f += chtp8;
				break;
			}
			chtp[i] = f;
			wnumavg += f;
		}
		f = wnumavg / 43 + 6;
		for (i = 1; i < 44; i++)
			if (f < chtp[i]) fprintf(f1, "%2d: %9f\n", i, chtp[i]);
	}

	if (anatype == 2) {
		if (anacnt < 0) fprintf(f1, "50回～");
		else fprintf(f1, "過去%d回 ", anacnt);
		fprintf(f1, "%d回 %dhit", wncnt, anahit);
		if (anahit < 6) fprintf(f1, "以上");
		fprintf(f1, "\n");

		f = (float)cht1avg / (float)cht1cnt;
		fprintf(f1, "統計一覧 予 %4d %4d %4d %11f %10f\n", cht1gap, cht1min,  cht1max, f, sqrt((float)cht1sd / (float)cht1cnt - f * f));
		f = (float)chtm1avg / (float)chtm1cnt;
		fprintf(f1, "         削 %4d %4d %4d %11f %10f\n", chtm1gap, chtm1min, chtm1max,f, sqrt((float)chtm1sd / (float)chtm1cnt - f * f));
		f = (float)cht2avg / (float)cht2cnt;
		fprintf(f1, "最後当選 予 %4d %4d %4d %11f %10f\n", cht2gap, cht2min,  cht2max, f, sqrt((float)cht2sd / (float)cht2cnt - f * f));
		f = (float)cht3avg / (float)cht3cnt;
		fprintf(f1, "当選期間 予 %4d %4d %4d %11f %10f\n", cht3gap, cht3min,  cht3max, f, sqrt((float)cht3sd / (float)cht3cnt - f * f));
		f = (float)chtm3avg / (float)chtm3cnt;
		fprintf(f1, "         削 %4d %4d %4d %11f %10f\n", chtm3gap, chtm3min, chtm3max,f, sqrt((float)chtm3sd / (float)chtm3cnt - f * f));
		f = (float)cht4avg / (float)cht4cnt;
		fprintf(f1, "飛後当選 予 %4d %4d %4d %11f %10f\n", cht4gap, cht4min,  cht4max, f, sqrt((float)cht4sd / (float)cht4cnt - f * f));
		f = (float)chtm4avg / (float)chtm4cnt;
		fprintf(f1, "         削 %4d %4d %4d %11f %10f\n", chtm4gap, chtm4min, chtm4max,f, sqrt((float)chtm4sd / (float)chtm4cnt - f * f));
		f = (float)cht5avg / (float)cht5cnt;
		fprintf(f1, "多重当選 予 %4d %4d %4d %11f %10f\n", cht5gap, cht5min,  cht5max, f, sqrt((float)cht5sd / (float)cht5cnt - f * f));
		f = (float)cht6avg / (float)cht6cnt;
		fprintf(f1, "前後数当 予 %4d %4d %4d %11f %10f\n", cht6gap, cht6min,  cht6max, f, sqrt((float)cht6sd / (float)cht6cnt - f * f));
		f = (float)cht7avg / (float)cht7cnt;
		fprintf(f1, "後追い数 予 %4d %4d %4d %11f %10f\n", cht7gap, cht7min,  cht7max, f, sqrt((float)cht7sd / (float)cht7cnt - f * f));
		f = (float)chtm7avg / (float)chtm7cnt;
		fprintf(f1, "         削 %4d %4d %4d %11f %10f\n", chtm7gap, chtm7min, chtm7max,f, sqrt((float)chtm7sd / (float)chtm7cnt - f * f));
		f = (float)cht8avg / (float)cht8cnt;
		fprintf(f1, "占星術   予 %4d %4d %4d %11f %10f\n\n", cht8gap, cht8min, cht8max, f, sqrt((float)cht8sd / (float)cht8cnt - f * f));
	} else if (anatype == 3)
		for (i = 0; i < 12; i++) {
			fprintf(f1, "%s\n", chtname[i]);
			for (cht1hh = chthh[i]; cht1hh->cnt > 0; cht1hh++) {
				fprintf(f1, "%4d [%2d|%2d|%2d|%2d|%2d|%2d](%2d) %4d %dh %2d:", cht1hh->cnt,
				cht1hh->wn[0], cht1hh->wn[1], cht1hh->wn[2], cht1hh->wn[3], cht1hh->wn[4],
				cht1hh->wn[5], cht1hh->wn[6], cht1hh->gap, cht1hh->hit, cht1hh->encnt);
				for (j = 0; j < cht1hh->encnt; j++) {
					if (j > 0) fprintf(f1, ",");
					fprintf(f1, " %2d", cht1hh->en[j]);
				}
				fprintf(f1, "\n");
			}
			fprintf(f1, "\n");
		}

	fclose(f1);
}
