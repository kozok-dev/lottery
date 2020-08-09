#include "../common.c"
#define NUMMAX 9999
#define ITEMNAME "合計  大小         奇数偶数     隣接数字差 " \
				"T ﾚ ﾋ ﾄ 平回数 当選間隔                        過去当選コ\n"
#define COLNAME "数1\t数2\t数3\t数4\t合計\t大小\t奇偶\t差\t差T\t" \
				"T\tﾚ\tﾋ\tﾄ\t平\t平T\t当間\t当間s\t当間ﾚa\tﾎｯﾄ\t無\tB\tS\n"
#define SEPAL "-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n"

char wn[NUMMAX][4], hitbmax[4], hitsmax[4];
short wncnt, wnum_cnt1[40], wnum_cnt2[40], wnum_cnt3[40], wnum_cnt4[40], hbmax = -1, hsmax = -1;
short sum_cnt[37], big_cnt[5], odd_cnt[5], seq_cnt[5], samepast_cnt[5], avg_cnt[5], sub_cnt[5], num_cnt[40], hb_cnt, hs_cnt;

struct {
	char en[40], encnt, hit, *wn;
	short cnt, gap;
} chthh[11][NUMMAX / 10], *cht1hh = chthh[0], *chtm1hh = chthh[1], *cht2hh = chthh[2],
*cht3hh = chthh[3], *chtm3hh = chthh[4], *cht4hh = chthh[5], *chtm4hh = chthh[6],
*cht5hh = chthh[7], *cht6hh = chthh[8], *cht7hh = chthh[9], *chtm7hh = chthh[10];
char *chtname[] = {
	"統計一覧 予", "統計一覧 削", "最後当選 予", "当選期間 予", "当選期間 削", "飛後当選 予",
	"飛後当選 削", "多重当選 予", "前後数当 予", "後追い数 予", "後追い数 削"
};

/* 合計数字。*/
char sum_number(const char *num)
{
	return num[0] + num[1] + num[2] + num[3];
}

/* 大小数字。*/
void big_number(const char *num, short *b, short *tb, short g)
{
	short i, j;

	*b = 0;
	for (i = 0; i < 4; i++)
		if (num[i] > 4) (*b)++;

	*tb = *b;
	for (i = 1; i < (g < 4 ? g + 1 : 5); i++)
		for (j = 0; j < 4; j++)
			if (wn[g - i][j] > 4) (*tb)++;
}

/* 奇数偶数比率。*/
void odd_number(const char *num, short *o, short *to, short g)
{
	short i, j;

	*o = 0;
	for (i = 0; i < 4; i++)
		if (num[i] & 1) (*o)++;

	*to = *o;
	for (i = 1; i < (g < 4 ? g + 1 : 5); i++)
		for (j = 0; j < 4; j++)
			if (wn[g - i][j] & 1) (*to)++;
}

/* 隣接数字差。*/
void sub_number(const char *num, char *s)
{
	char ch[3], chf, i, j;

	for (i = 0; i < 3; i++) {
		s[i] = num[i + 1] - num[i];
		ch[i] = 0;
	}
	s[3] = 0;

	for (i = 0; i < 3; i++) {
		if (ch[i]) continue;
		chf = 0;
		for (j = i + 1; j < 3; j++) {
			if (s[i] != s[j]) continue;
			s[3] += chf ? 1 : 2;
			ch[j] = 1;
			chf = 1;
		}
	}
}

/* 組み合わせ数字の種類。*/
char type_number(const char *num)
{
	char ch[4], cnt = 0, val, w = 0, i, j;

	for (i = 0; i < 4; i++) ch[i] = 0;
	for (i = 0; i < 3; i++) {
		val = 2;
		for (j = i + 1; j < 4; j++) {
			if (num[i] != num[j] || ch[j]) continue;
			if (cnt == 2 && val == 2) w = 1;
			else cnt += val;
			ch[j] = 1;
			val = 1;
		}
	}

	switch (cnt) {
		case 0:
			return 's';
		case 2:
			return w ? 'W' : 'w';
		case 3:
			return 't';
		default:
			return 'a';
	}
}

/* 連番。*/
char seq_number(const char *num)
{
	char cnt = 0, i = 0, j;

	while (i < 3) {
		for (j = i + 1; j < 4 && num[i] == num[j] - j + i; j++) cnt += i < j - 1 ? 1 : 2;
		i = j;
	}
	return cnt;
}

/* ヒキ/飛び数字。*/
char same_past_number(const char *num, short g)
{
	char ch[4], cnt = 0, i, j;

	if (--g < 0) return 0;

	for (i = 0; i < 4; i++) ch[i] = 0;
	for (i = 0; i < 4; i++)
		for (j = 0; j < 4; j++) {
			if (wn[g][i] != num[j] || ch[j]) continue;
			cnt++;
			ch[j] = 1;
			break;
		}
	return cnt;
}

/* 平均出現回数。*/
void avg_number(const char *num, char *a, short g)
{
	short wnumcnt[40], wnumavg, i, j;

	for (i = 0; i < 40; i++) wnumcnt[i] = 0;
	for (i = 0; i < g; i++)
		for (j = 0; j < 4; j++)
			wnumcnt[wn[i][j] + j * 10]++;
	wnumavg = (float)g / 10 + 0.5;

	a[4] = 0;
	for (i = 0; i < 4; i++)
		if (wnumcnt[num[i] + i * 10] < wnumavg)
			a[i] = 'l';
		else {
			a[i] = 'w';
			a[4]++;
		}
}

/* ホット数字。*/
void hot_number(const char *num, char *g1, char *g2, short *s, short *t, short g)
{
	short i, j, k;

	g1[4] = 0;
	for (i = 0; i < 4; i++) {
		/* ホット数字。*/
		if ((j = g - 10) < 0)
			g1[4] = 4;
		else
			for (; j < g; j++) {
				for (k = 0; k < 4 && num[i] + i * 10 != wn[j][k] + k * 10; k++);
				if (k == 4) continue;
				g1[4]++;
				break;
			}

		/* 当選間隔。*/
		g1[i] = 0;
		for (j = g - 1; j >= 0; j--) {
			for (k = 0; k < 4 && num[i] + i * 10 != wn[j][k] + k * 10; k++);
			if (k < 4) break;
			g1[i]++;
		}
	}

	*s = 0;
	for (i = 0; i < 4; i++) *s += g1[i];
}

/* 過去当選数字。*/
void past_winning_number(const char *num, short *h, short g)
{
	short ch[4], i, j, k, l;

	for (i = 0; i < 3; i++) h[i] = 0;

	for (i = wncnt - g; i < wncnt; i++) {
		for (j = 0; j < 4 && num[j] + j * 10 == wn[i][j] + j * 10; j++);
		if (j < 4) {
			for (j = 0; j < 4; j++) ch[j] = 0;
			j = 0;
			for (k = 0; k < 4; k++)
				for (l = 0; l < 4; l++) {
					if (num[k] != wn[i][l] || ch[l]) continue;
					j++;
					ch[l] = 1;
					break;
				}
			h[j == 4]++;	/* 0:ハズレ、1:ボックス。*/
		} else
			h[2]++;	/* ストレート。*/
	}
}

/* 過去当たり数字連続重複。*/
char same_past_seq_winning_number(const char *num, short g)
{
	short i, j, k;

	for (i = wncnt - g; i < wncnt; i++)
		for (j = 0; j < 3; j++)
			for (k = 0; k < 3; k++)
				if (wn[i][j] == num[k] && wn[i][j + 1] == num[k + 1]) return 1;
	return 0;
}

void write_data(const char *, FILE *, FILE *, short);
#define SCRTYPE 2
#include "../scr.c"

void write_data(const char *num, FILE *f1, FILE *f2, short flag)
{
	char a[5];
	short h[3], i, j, k;

	fprintf(f1, "[%d|%d|%d|%d] ", num[0], num[1], num[2], num[3]);
	if (f2 != NULL) fprintf(f2, "%d\t%d\t%d\t%d\t", num[0], num[1], num[2], num[3]);

	i = sum_number(num);
	if (flag == -2) sum_cnt[i]++;
	fprintf(f1, "%2d%+3d ", i, i - 18);
	if (f2 != NULL) fprintf(f2, "%d\t", i);

	k = flag < 0 ? wncnt : flag;
	big_number(num, &i, &j, k);
	if (flag == -2) big_cnt[i]++;
	k = k < 4 ? (k + 1) * 4 : 20;
	fprintf(f1, "%d:%d %2d/%2d%+3d ", i, 4 - i, j, k - j, j - k + j);
	if (f2 != NULL) fprintf(f2, "s%d:%d\t", i, 4 - i);

	k = flag < 0 ? wncnt : flag;
	odd_number(num, &i, &j, k);
	if (flag == -2) odd_cnt[i]++;
	k = k < 4 ? (k + 1) * 4 : 20;
	fprintf(f1, "%d:%d %2d/%2d%+3d ", i, 4 - i, j, k - j, j - k + j);
	if (f2 != NULL) fprintf(f2, "s%d:%d\t", i, 4 - i);

	sub_number(num, a);
	if (flag == -2) sub_cnt[a[3]]++;
	fprintf(f1, "%+d.%+d.%+d:%d ", a[0], a[1], a[2], a[3]);
	if (f2 != NULL) fprintf(f2, "s%+d.%+d.%+d\t%d\t", a[0], a[1], a[2], a[3]);

	i = type_number(num);
	fprintf(f1, "%c ", i);
	if (f2 != NULL) fprintf(f2, "%c\t", i);

	i = seq_number(num);
	j = same_past_number(num, flag < 0 ? wncnt : flag);
	k = same_past_number(num, flag < 0 ? wncnt - 1 : flag - 1);
	if (flag == -2) {
		seq_cnt[i]++;
		samepast_cnt[j]++;
	}
	fprintf(f1, "%d %d %d ", i, j, k);
	if (f2 != NULL) fprintf(f2, "%d\t%d\t%d\t", i, j, k);

	avg_number(num, a, flag < 0 ? wncnt : flag);
	if (flag == -2) avg_cnt[a[4]]++;
	fprintf(f1, "%c%c%c%c:%d ", a[0], a[1], a[2], a[3], a[4]);
	if (f2 != NULL) fprintf(f2, "%c%c%c%c\t%d\t", a[0], a[1], a[2], a[3], a[4]);

	hot_number(num, a, NULL, &i, NULL, flag < 0 ? wncnt : flag);
	fprintf(f1, "%3d+%3d+%3d+%3d=%3d %9f %d ", a[0], a[1], a[2], a[3], i, (float)i / 4, a[4]);
	if (f2 != NULL) fprintf(f2, "s%3d+%3d+%3d+%3d\t%d\t%f\t%d\t", a[0], a[1], a[2], a[3], i, (float)i / 4, a[4]);

	past_winning_number(num, h, wncnt);
	if (flag == -2) {
		hb_cnt += h[1] > 0;
		hs_cnt += h[2] > 0;
	}
	fprintf(f1, "(%d %2d %2d)\n", h[0], h[1], h[2]);
	if (f2 != NULL) fprintf(f2, "%d\t%d\t%d\n", h[0], h[1], h[2]);

	if (flag == -2) {
		for (i = 0; i < 4; i++) num_cnt[num[i] + i * 10]++;

		if (h[1] > hbmax) {
			for (i = 0; i < 4; i++) hitbmax[i] = num[i];
			hbmax = h[1];
		}
		if (h[2] > hsmax) {
			for (i = 0; i < 4; i++) hitsmax[i] = num[i];
			hsmax = h[2];
		}
	}
}

void main(int argc, char **argv)
{
	static char *numstr[] = {
		"0***", "1***", "2***", "3***", "4***", "5***", "6***", "7***", "8***", "9***",
		"*0**", "*1**", "*2**", "*3**", "*4**", "*5**", "*6**", "*7**", "*8**", "*9**",
		"**0*", "**1*", "**2*", "**3*", "**4*", "**5*", "**6*", "**7*", "**8*", "**9*",
		"***0", "***1", "***2", "***3", "***4", "***5", "***6", "***7", "***8", "***9"
	};
	char en[NUMMAX][4], num[4], numcnt[4][40], *numrnd;
	char cht1[40], cht2[40], cht3[40], cht4[40], cht5[40], cht6[80], cht7[40], *cp;
	char chtm1[40], chtm3[40], chtm4[40], chtm7[40], *cmp;
	short chth1[5], chth2[5], chth3[5], chth4[5], chth5[5], chth6[5], chth7[5], chtmh1[5], chtmh3[5], chtmh4[5], chtmh7[5];
	short a1[37], a2[37], actwncnt, anatype = argc > 1 ? atoi(argv[1]) : 0, anacnt = argc > 2 ? atoi(argv[2]) : 13, anahit = argc > 3 ? atoi(argv[3]) : 4;
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
	int i, j, k, l, m, n, o, p, q, r;
	float wnumavg, chtp1, chtp2, chtp3, chtp4, chtp5, chtp6, chtp7, chtmp1, chtmp3, chtmp4, chtmp7, f, chtp[40];
	float chta1, chta2, chta3, chta4, chta5, chta6, chta7, chtma1, chtma3, chtma4, chtma7;
	FILE *f1, *f2 = NULL;

	/* 当選データ読み込み。*/
	if ((f1 = fopen("wn.txt", "r")) == NULL) return;
	for (wncnt = 0; wncnt < NUMMAX && get_number(f1, wn[wncnt], 4, 4, 0, 9, 1, 0, 0); wncnt++)
		for (i = 0; i < 4; i++) wnum_cnt1[wn[wncnt][i] + i * 10]++;
	fclose(f1);

	if (anatype > 0) {
		if ((f1 = fopen("anas.txt", anatype == 1 && anacnt < 0 ? "w" : "a")) == NULL) return;
	} else {
		/* 予想データ読み込み。*/
		i = 0;
		if ((f1 = fopen("exp.txt", "r")) != NULL) {
			for (; i < NUMMAX && get_number(f1, en[i], 4, 4, 0, 9, 1, 0, 0); i++);
			fclose(f1);
		}

		if (!init_script(wncnt) || (f1 = fopen("ana.txt", "w")) == NULL) {
			free(g_code);
			return;
		}

		printf("0");
		if (i > 0) {
			fprintf(f1, "予想数字  " ITEMNAME);
			if (g_arf && (f2 = fopen("ana_exp.txt", "w")) != NULL) fprintf(f2, COLNAME);
			for (j = 0; j < i; j++) {
				printf("\b\b\b\b\b%d", j + 1);
				write_data(en[j], f1, f2, -1);
			}
			fprintf(f1, SEPAL);
			if (f2 != NULL) fclose(f2);
		}
		printf("\n");

		fprintf(f1, "当選数字  " ITEMNAME);
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
			fprintf(f1, "ﾗﾝﾀﾞﾑ数字 " ITEMNAME);
			if (g_arf && (f2 = fopen("ana_rnd.txt", "w")) != NULL) fprintf(f2, COLNAME);
			numrnd = (char *)calloc(wncnt, sizeof(char));
			for (i = 0; i < g_rndcnt; i++) {
				printf("\b\b\b\b\b%d", i + 1);

				/* ランダム数字生成。*/
				for (j = 0; j < 4; j++) {
					k = j * 10;
					l = 0;
					for (m = k; m < k + 10; m++)
						for (n = 0; n < wnum_cnt1[m]; n++) numrnd[l++] = m - k;

					num[j] = numrnd[next_int_ex(wncnt)];
				}

				write_data(num, f1, f2, -1);
			}
			free(numrnd);
			fprintf(f1, SEPAL);
			if (f2 != NULL) fclose(f2);
		}
		printf("\n");

		if (g_ec > 0) {
			fprintf(f1, "候補数字  " ITEMNAME);
			if (g_arf && (f2 = fopen("ana_ec.txt", "w")) != NULL) fprintf(f2, COLNAME);
			o = write_ec(wncnt, f1, f2);
			printf("\n");
			if (f2 != NULL) fclose(f2);

			fprintf(f1, "\n予想候補数字統計\n");
			fprintf(f1, "数: %d\n\n[合計]\n", o);
			for (i = 0; i < 37; i++)
				if (sum_cnt[i] > 0) fprintf(f1, "%2d %3d %9f%%\n", i, sum_cnt[i], (float)sum_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[大小]\n");
			for (i = 0; i < 5; i++)
				if (big_cnt[i] > 0) fprintf(f1, "%d:%d %4d %9f%%\n", i, 4 - i, big_cnt[i], (float)big_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[奇数偶数]\n");
			for (i = 0; i < 5; i++)
				if (odd_cnt[i] > 0) fprintf(f1, "%d:%d %4d %9f%%\n", i, 4 - i, odd_cnt[i], (float)odd_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[隣接数字差]\n");
			for (i = 0; i < 5; i++)
				if (sub_cnt[i] > 0) fprintf(f1, "%d %4d %9f%%\n", i, sub_cnt[i], (float)sub_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[連番]\n");
			for (i = 0; i < 5; i++)
				if (seq_cnt[i] > 0) fprintf(f1, "%d %4d %9f%%\n", i, seq_cnt[i], (float)seq_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[ヒキ]\n");
			for (i = 0; i < 5; i++)
				if (samepast_cnt[i] > 0) fprintf(f1, "%d %4d %9f%%\n", i, samepast_cnt[i], (float)samepast_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[平均出現回数]\n");
			for (i = 0; i < 5; i++)
				if (avg_cnt[i] > 0) fprintf(f1, "%d %4d %9f%%\n", i, avg_cnt[i], (float)avg_cnt[i] / (float)o * 100);
			fprintf(f1, "\n[過去当選]\n");
			fprintf(f1, "B %4d %9f%%\n", hb_cnt, (float)hb_cnt / (float)o * 100);
			fprintf(f1, "S %4d %9f%%\n", hs_cnt, (float)hs_cnt / (float)o * 100);
			fprintf(f1, "\n[数字]\n");
			for (i = 0; i < 40; i++)
				if (num_cnt[i] > 0) fprintf(f1, "%s %4d %9f%%\n", numstr[i], num_cnt[i], (float)num_cnt[i] / (float)o * 100);
			fprintf(f1, SEPAL);

			if (g_arf && (f2 = fopen("ana_mm.txt", "w")) != NULL) {
				write_data(hitbmax, f2, NULL, -1);
				write_data(hitsmax, f2, NULL, -1);
				fclose(f2);
			}
		}
	}

	actwncnt = wncnt;
	chtp1 = chtp2 = chtp3 = chtp4 = chtp5 = chtp6 = chtp7 = chtmp1 = chtmp3 = chtmp4 = chtmp7 = chta1 = chta2 = chta3 = chta4 = chta5 = chta6 = chta7 = chtma1 = chtma3 = chtma4 = chtma7 = 0;
	for (i = 0; i < 5; i++) chth1[i] = chth2[i] = chth3[i] = chth4[i] = chth5[i] = chth6[i] = chth7[i] = chtmh1[i] = chtmh3[i] = chtmh4[i] = chtmh7[i] = 0;
	r = 0;
	for (wncnt = anacnt < 0 ? 50 : actwncnt - anacnt; wncnt <= actwncnt - (anatype > 0); wncnt++) {
		printf("\b\b\b\b\b%d", wncnt);
		wnumavg = (float)wncnt / 10;
		if (wncnt < actwncnt) r++;
		for (i = 0; i < 40; i++) wnum_cnt1[i] = 0;
		for (i = 0; i < wncnt; i++)
			for (j = 0; j < 4; j++) wnum_cnt1[wn[i][j] + j * 10]++;

		if (wncnt == actwncnt) fprintf(f1, "統計一覧\n");
		cp = cht1;
		cmp = chtm1;
		for (i = 0; i < 40; i++) {
			j = k = l = m = n = o = 0;
			for (p = 0; p < wncnt; p++) {
				for (q = 0; q < 4 && wn[p][q] + q * 10 != i; q++);
				if (q < 4) {
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

			if (wncnt == actwncnt) fprintf(f1, "%s: %2d %f |%3d %2d %9f %9f | %f %4d %10f%%\n", numstr[i],
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
				fprintf(f1, " %s", numstr[*cp]);
			else
				for (j = 0; j < 4; j++) {
					if (wn[wncnt][j] + j * 10 != *cp) continue;
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
				fprintf(f1, " %s", numstr[*cmp]);
			else
				for (j = 0; j < 4; j++) {
					if (wn[wncnt][j] + j * 10 != *cmp) continue;
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
			for (i = 0; i < 40; i++) fprintf(f1, "%s|", numstr[i]);
			fprintf(f1, "\n");
		}
		for (i = wncnt - 40; i < wncnt; i++) {
			if (wncnt == actwncnt) fprintf(f1, "%4d|", i + 1);
			for (j = 0; j < 40; j++) {
				k = -1;
				for (l = i; l >= 0; l--) {
					for (m = 0; m < 4; m++) {
						if (wn[l][m] + m * 10 != j) continue;
						k = i - l;
						break;
					}
					if (k >= 0) break;
				}

				if (wncnt == actwncnt)
					if (k) fprintf(f1, "%4d|", k < 0 ? i + 1 : k);
					else fprintf(f1, "####|");
			}
			if (wncnt == actwncnt) fprintf(f1, "\n");
		}

		cp = cht2;
		for (i = 0; i < 40; i++) {
			/* 当選間隔長期間で当選して短期間にまた当選して現在のハズレ短期間。*/
			j = k = 0;
			l = wncnt;
			for (m = 0; m < 3; m++) {
				for (l--; l >= 0; l--) {
					for (n = 0; n < 4 && wn[l][n] + n * 10 != i; n++);
					if (n < 4) {
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
					for (n = 0; n < 4 && wn[l][n] + n * 10 != i; n++);
					if (n < 4) {
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
					for (n = 0; n < 4 && wn[l][n] + n * 10 != i; n++);
					if (n < 4) {
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
					for (n = 0; n < 4 && wn[l][n] + n * 10 != i; n++);
					if (n < 4) {
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
					for (n = 0; n < 4 && wn[l][n] + n * 10 != i; n++);
					if (n < 4) {
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
				fprintf(f1, " %s", numstr[*cp]);
			else
				for (j = 0; j < 4; j++) {
					if (wn[wncnt][j] + j * 10 != *cp) continue;
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
		for (i = 0; i < 40; i++) wnum_cnt2[i] = wnum_cnt3[i] = wnum_cnt4[i] = 0;
		cp = cht3;
		cmp = chtm3;
		for (i = 0; i < 4; i++) {
			for (j = wncnt - 50; j < wncnt; j++) wnum_cnt2[wn[j][i] + i * 10]++;
			for (j = wncnt - 25; j < wncnt; j++) wnum_cnt3[wn[j][i] + i * 10]++;
			for (j = wncnt - 10; j < wncnt; j++) wnum_cnt4[wn[j][i] + i * 10]++;
		}
		for (i = 0; i < 40; i++) {
			j = -1;
			for (k = wncnt - 1; k >= 0; k--) {
				for (l = 0; l < 4; l++) {
					if (wn[k][l] + l * 10 != i) continue;
					j = wncnt - k - 1;
					break;
				}
				if (j >= 0) break;
			}

			if (wncnt == actwncnt) fprintf(f1, "%s: %d %f%% | %2d %f%% | %d %f%% | %d %9f%% | %2d\n", numstr[i],
				wnum_cnt1[i], (float)wnum_cnt1[i] / ((float)wncnt * 4) * 100,	/* 全ての回数。*/
				wnum_cnt2[i], (float)wnum_cnt2[i] / (50 * 4) * 100,	/* 直前50回数。*/
				wnum_cnt3[i], (float)wnum_cnt3[i] / (25 * 4) * 100,	/* 直前25回数。*/
				wnum_cnt4[i], (float)wnum_cnt4[i] / (10 * 4) * 100,	/* 直前10回数。*/
				j);	/* ハズレ。*/

			if ((float)wnum_cnt1[i] / ((float)wncnt * 4) * 100 > (float)wnum_cnt4[i] / (10 * 4) * 100 && j < 12)
				*cp++ = i;
			else if ((float)wnum_cnt1[i] / ((float)wncnt * 4) * 100 < (float)wnum_cnt2[i] / (50 * 4) * 100)
				*cmp++ = i;
		}

		*cp = 0;
		cp = cht3;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n予:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %s", numstr[*cp]);
			else
				for (j = 0; j < 4; j++) {
					if (wn[wncnt][j] + j * 10 != *cp) continue;
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
				fprintf(f1, " %s", numstr[*cmp]);
			else
				for (j = 0; j < 4; j++) {
					if (wn[wncnt][j] + j * 10 != *cmp) continue;
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

		if (wncnt == actwncnt) fprintf(f1, "飛後当選\n        ");
		for (i = 0; i < 37; i++) {
			a2[i] = 0;
			if (wncnt == actwncnt) fprintf(f1, "|%2d", i);
		}
		if (wncnt == actwncnt) fprintf(f1, "以上\n");
		for (i = 0; i < 40; i++) {
			for (j = 0; j < 37; j++) a1[j] = 0;
			if (wncnt == actwncnt) fprintf(f1, "%s: ", numstr[i]);

			j = 0;
			for (k = 0; k < wncnt; k++) {
				for (l = 0; l < 4 && wn[k][l] + l * 10 != i; l++);
				if (l < 4) {
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
			fprintf(f1, "AVG :   ", i);
			for (i = 0; i < 37; i++) fprintf(f1, "|%2d", (int)((float)a2[i] / 40 + 0.5));
			fprintf(f1, "\n");
		}

		cp = cht4;
		cmp = chtm4;
		for (i = 0; i < 40; i++) {
			for (j = 0; j < 37; j++) a1[j] = 0;
			j = 0;
			for (k = 0; k < wncnt; k++) {
				for (l = 0; l < 4 && wn[k][l] + l * 10 != i; l++);
				if (l < 4) {
					a1[j < 37 ? j : 36]++;
					j = 0;
				} else
					j++;
			}
			if (j > 36) j = 36;

			k = (float)a2[j] / 40 + 0.5;
			if (k > 0 && a1[j] < k - 2) *cp++ = i;
			else if (a1[j] >= k + 2) *cmp++ = i;
		}

		*cp = 0;
		cp = cht4;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n予:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %s", numstr[*cp]);
			else
				for (j = 0; j < 4; j++) {
					if (wn[wncnt][j] + j * 10 != *cp) continue;
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
				fprintf(f1, " %s", numstr[*cmp]);
			else
				for (j = 0; j < 4; j++) {
					if (wn[wncnt][j] + j * 10 != *cmp) continue;
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

		if (wncnt == actwncnt) fprintf(f1, "多重当選\n ");
		for (i = 1; i < 8; i++) {
			a2[i] = 0;
			if (wncnt == actwncnt) fprintf(f1, "|  %d", i);
		}
		if (wncnt == actwncnt) fprintf(f1, "|多当比率\n");
		i = j = 0;
		for (k = 0; k < 10; k++) {
			for (l = 1; l < 8; l++) a1[l] = 0;
			if (wncnt == actwncnt) fprintf(f1, "%d", k);

			l = m = n = 0;
			for (o = 0; o < wncnt; o++) {
				for (p = 0; p < 4 && wn[o][p] != k; p++);
				if (p < 4) {
					if (l) {	/* 多重当選？ */
						a1[l < 8 ? l : 7]++;
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
				for (l = 1; l < 8; l++)
					if (a1[l]) fprintf(f1, "|%3d", a1[l]);
					else fprintf(f1, "|   ");
				fprintf(f1, "|%9f%%\n", (float)m / (float)n * 100);
			}
		}
		if (wncnt == actwncnt) fprintf(f1, "平均                         |%9f%%\n", (float)i / (float)j * 100);

		cp = cht5;
		for (k = 0; k < 4; k++) {
			for (l = 1; l < 8; l++) a1[l] = 0;

			l = m = n = 0;
			for (o = 0; o < wncnt; o++) {
				for (p = 0; p < 4 && wn[o][p] != wn[wncnt - 1][k]; p++);
				if (p < 4) {
					if (l) {	/* 多重当選？ */
						a1[l < 8 ? l : 7]++;
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
		for (i = 0; cht5[i]; i++)
			for (j = i + 1; cht5[j]; j++) {
				if (cht5[i] <= cht5[j]) continue;
				k = cht5[i];
				cht5[i]= cht5[j];
				cht5[j] = k;
			}
		cp = cht5;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n予:");
		k = -1;
		l = 0;
		for (;;) {
			if (wncnt == actwncnt) {
				if (k != *cp) {
					if (k != -1) fprintf(f1, ",");
					fprintf(f1, " %d", *cp);
					k = *cp;
				}
			} else
				if (k != *cp) {
					for (j = 0; j < 4; j++) {
						if (wn[wncnt][j] != *cp) continue;
						i++;
						break;
					}
					k = *cp;
					l++;
				}
			if (!*++cp) break;
		}
		if (wncnt < actwncnt) {
			chtp5 += (float)i / (float)l;
			chth5[i]++;
			chta5 += (float)l;

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				cht5hh->cnt = wncnt + 1;
				cht5hh->wn = wn[wncnt];
				cht5hh->gap = cht5gap;
				cht5hh->hit = i;
				cht5hh->encnt = l;
				cp = cht5;
				for (j = 0; ; ) {
					if (j == 0 || k != *cp) {
						k = *cp;
						cht5hh->en[j] = k;
						j++;
					}
					if (!*++cp) break;
				}
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
			for (j = 0; j < 4; j++) {
				k = 0;
				for (l = i - 1; l >= 0; l--) {
					for (m = 0; m < 4 && wn[i][j] + j * 10 != wn[l][m] + m * 10; m++);
					if (m < 4) break;
					k++;
				}
				a1[k < 36 ? k : 36]++;
			}
		if (wncnt == actwncnt) {
			for (i = 0; i < 37; i++) {
				fprintf(f1, "%2d: %d |", i, a1[i]);

				j = 0;
				for (k = 0; k < 40; k++) {
					l = 0;
					for (m = wncnt - 1; m >= 0 && l < 36; m--) {
						for (o = 0; o < 4 && wn[m][o] + o * 10 != k; o++);
						if (o < 4) break;
						l++;
					}
					if (i != l) continue;
					if (j) fprintf(f1, ",");
					fprintf(f1, " %s", numstr[k]);
					j = 1;
				}
				fprintf(f1, "\n");
			}
			fprintf(f1, SEPAL);
		}

		/* ------------------------------------------------------------ */

		if (wncnt == actwncnt) fprintf(f1, "前後数当\n");
		i = j = 0;
		for (k = 0; k < 40; k++) {
			l = m = 0;
			for (n = 1; n < wncnt; n++)
				for (o = 0; o < 4; o++) {
					if (wn[n][o] + o * 10 != k) continue;
					for (p = 0; p < 4; p++) {
						q = k / 4;
						if (wn[n - 1][p] == (q > 0 ? q - 1 : 9)) l++;	/* 数字-1 */
						else if (wn[n - 1][p] == (q < 9 ? q + 1 : 0)) m++;	/* 数字+1 */
					}
					break;
				}
			i += l;
			j += m;

			if (wncnt == actwncnt) fprintf(f1, "%s: %4d | %3d %9f%% | %3d %9f%%\n", numstr[k], wnum_cnt1[k],
				l, (float)l / (float)wnum_cnt1[k] * 100,
				m, (float)m / (float)wnum_cnt1[k] * 100);
		}
		i = (float)i / 40 + 0.5;
		j = (float)j / 40 + 0.5;

		if (wncnt == actwncnt) fprintf(f1, "AVG : %4d | %3d %9f%% | %3d %9f%%\n", (int)(wnumavg + 0.5),
			i, (float)i / wnumavg * 100, j, (float)j / wnumavg * 100);

		cp = cht6;
		for (k = 0; k < 40; k++) {
			l = m = 0;

			for (n = 1; n < wncnt; n++)
				for (o = 0; o < 4; o++) {
					if (wn[n][o] + o * 10 != k) continue;
					for (p = 0; p < 4; p++) {
						q = k / 10;
						if (wn[n - 1][p] == (q > 0 ? q - 1 : 9)) l++;	/* 数字-1 */
						else if (wn[n - 1][p] == (q < 9 ? q + 1 : 0)) m++;	/* 数字+1 */
					}
					break;
				}

			n = k / 10;
			if ((float)l / (float)wnum_cnt1[k] * 100 < (float)i / wnumavg * 100)
				*cp++ = k / 10 * 10 + (n > 0 ? n - 1 : 9);
			if ((float)m / (float)wnum_cnt1[k] * 100 < (float)j / wnumavg * 100)
				*cp++ = k / 10 * 10 + (n < 9 ? n + 1 : 0);
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
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n予:");
		k = -1;
		l = 0;
		for (;;) {
			if (wncnt == actwncnt) {
				if (k != *cp) {
					if (k != -1) fprintf(f1, ",");
					fprintf(f1, " %s", numstr[*cp]);
					k = *cp;
				}
			} else
				if (k != *cp) {
					for (j = 0; j < 4; j++) {
						if (wn[wncnt][j] + j * 10 != *cp) continue;
						i++;
						break;
					}
					k = *cp;
					l++;
				}
			if (!*++cp) break;
		}
		if (wncnt < actwncnt) {
			chtp6 += (float)i / (float)l;
			chth6[i]++;
			chta6 += (float)l;

			if (anatype == 3 && (i == anahit || wncnt >= actwncnt - 5)) {
				cht6hh->cnt = wncnt + 1;
				cht6hh->wn = wn[wncnt];
				cht6hh->gap = cht6gap;
				cht6hh->hit = i;
				cht6hh->encnt = l;
				cp = cht6;
				for (j = 0; ; ) {
					if (j == 0 || k != *cp) {
						k = *cp;
						cht6hh->en[j] = k;
						j++;
					}
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

		if (wncnt == actwncnt) fprintf(f1, "後追い数\n    ");
		for (i = 0; i < 4; i++) {
			if (wncnt == actwncnt) fprintf(f1, "|%2d", wn[wncnt - 1][i]);
			for (j = 0; j < 40; j++) numcnt[i][j] = 0;
			a1[i] = 0;

			for (j = 0; j < wncnt - 1; j++)
				for (k = 0; k < 4; k++) {
					if (wn[wncnt - 1][i] + i * 10 != wn[j][k] + k * 10) continue;
					for (l = 0; l < 4; l++) numcnt[i][wn[j + 1][l] + l * 10]++;
					break;
				}
		}
		if (wncnt == actwncnt) fprintf(f1, "|計\n");
		for (i = 0; i < 40; i++) {
			if (wncnt == actwncnt) fprintf(f1, "%s", numstr[i]);
			j = 0;
			for (k = 0; k < 4; k++) {
				if (wncnt == actwncnt) fprintf(f1, "|%2d", numcnt[k][i]);
				j += numcnt[k][i];
				a1[k] += numcnt[k][i];
			}
			if (wncnt == actwncnt) fprintf(f1, "|%d\n", j);
		}
		if (wncnt == actwncnt) fprintf(f1, "AVG ");
		i = 0;
		for (j = 0; j < 4; j++) {
			if (wncnt == actwncnt) fprintf(f1, "|%2d", (int)((float)a1[j] / 40 + 0.5));
			i += a1[j];
		}
		i = (float)i / 40 + 0.5;
		if (wncnt == actwncnt) fprintf(f1, "|%2d\n", i);

		cp = cht7;
		cmp = chtm7;
		for (j = 0; j < 40; j++) {
			k = 0;
			for (l = 0; l < 4; l++) k += numcnt[l][j];

			if (k > i + 5) *cp++ = j;
			else if (k < i - 5) *cmp++ = j;
		}

		*cp = 0;
		cp = cht7;
		i = 0;
		if (wncnt == actwncnt) fprintf(f1, "\n予:");
		for (;;) {
			if (wncnt == actwncnt)
				fprintf(f1, " %s", numstr[*cp]);
			else
				for (j = 0; j < 4; j++) {
					if (wn[wncnt][j] + j * 10 != *cp) continue;
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
				fprintf(f1, " %s", numstr[*cmp]);
			else
				for (j = 0; j < 4; j++) {
					if (wn[wncnt][j] + j * 10 != *cmp) continue;
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

		cp = cht1;
		fprintf(f1, "\n統計一覧 予 %4d %4d %4d %3d %3d %9f %9f%%",
			chth1[0], chth1[1], chth1[2], chth1[3], chth1[4], chta1, chtp1);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %s", numstr[*cp++]);
				if (*cp) fprintf(f1, ",");
				else break;
			}
		}
		cmp = chtm1;
		fprintf(f1, "\n         削 %4d %4d %4d %3d %3d %9f %9f%%",
			chtmh1[0], chtmh1[1], chtmh1[2], chtmh1[3], chtmh1[4], chtma1, chtmp1);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %s", numstr[*cmp++]);
				if (*cmp) fprintf(f1, ",");
				else break;
			}
		}

		cp = cht2;
		fprintf(f1, "\n最後当選 予 %4d %4d %4d %3d %3d %9f %9f%%",
			chth2[0], chth2[1], chth2[2], chth2[3], chth2[4], chta2, chtp2);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %s", numstr[*cp++]);
				if (*cp) fprintf(f1, ",");
				else break;
			}
		}

		cp = cht3;
		fprintf(f1, "\n当選期間 予 %4d %4d %4d %3d %3d %9f %9f%%",
			chth3[0], chth3[1], chth3[2], chth3[3], chth3[4], chta3, chtp3);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %s", numstr[*cp++]);
				if (*cp) fprintf(f1, ",");
				else break;
			}
		}
		cmp = chtm3;
		fprintf(f1, "\n         削 %4d %4d %4d %3d %3d %9f %9f%%",
			chtmh3[0], chtmh3[1], chtmh3[2], chtmh3[3], chtmh3[4], chtma3, chtmp3);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %s", numstr[*cmp++]);
				if (*cmp) fprintf(f1, ",");
				else break;
			}
		}

		cp = cht4;
		fprintf(f1, "\n飛後当選 予 %4d %4d %4d %3d %3d %9f %9f%%",
			chth4[0], chth4[1], chth4[2], chth4[3], chth4[4], chta4, chtp4);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %s", numstr[*cp++]);
				if (*cp) fprintf(f1, ",");
				else break;
			}
		}
		cmp = chtm4;
		fprintf(f1, "\n         削 %4d %4d %4d %3d %3d %9f %9f%%",
			chtmh4[0], chtmh4[1], chtmh4[2], chtmh4[3], chtmh4[4], chtma4, chtmp4);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %s", numstr[*cmp++]);
				if (*cmp) fprintf(f1, ",");
				else break;
			}
		}

		cp = cht5;
		fprintf(f1, "\n多重当選 予 %4d %4d %4d %3d %3d %9f %9f%%",
			chth5[0], chth5[1], chth5[2], chth5[3], chth5[4], chta5, chtp5);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				for (i = 0; &cht5[i] < cp && cht5[i] != *cp; i++);
				if (&cht5[i] < cp)
					*cp++;
				else {
					if (i) fprintf(f1, ",");
					fprintf(f1, " %d", *cp++);
				}
				if (!*cp) break;
			}
		}

		cp = cht6;
		fprintf(f1, "\n前後数当 予 %4d %4d %4d %3d %3d %9f %9f%%",
			chth6[0], chth6[1], chth6[2], chth6[3], chth6[4], chta6, chtp6);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				for (i = 0; &cht6[i] < cp && cht6[i] != *cp; i++);
				if (&cht6[i] < cp)
					*cp++;
				else {
					if (i) fprintf(f1, ",");
					fprintf(f1, " %s", numstr[*cp++]);
				}
				if (!*cp) break;
			}
		}

		cp = cht7;
		fprintf(f1, "\n後追い数 予 %4d %4d %4d %3d %3d %9f %9f%%",
			chth7[0], chth7[1], chth7[2], chth7[3], chth7[4], chta7, chtp7);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %s", numstr[*cp++]);
				if (*cp) fprintf(f1, ",");
				else break;
			}
		}
		cmp = chtm7;
		fprintf(f1, "\n         削 %4d %4d %4d %3d %3d %9f %9f%%",
			chtmh7[0], chtmh7[1], chtmh7[2], chtmh7[3], chtmh7[4], chtma7, chtmp7);
		if (anatype <= 0) {
			fprintf(f1, ":");
			for (;;) {
				fprintf(f1, " %s", numstr[*cmp++]);
				if (*cmp) fprintf(f1, ",");
				else break;
			}
		}

		if (anatype > 0) fprintf(f1, "\n" SEPAL);
		else fprintf(f1, "\n\n");
	}

	if (anatype <= 0) {
		wnumavg = 0;
		for (i = 0; i < 40; i++) {
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
				if (i != *cp && i + 10 != *cp && i + 20 != *cp && i + 30 != *cp) continue;
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
			chtp[i] = f;
			wnumavg += f;
		}
		f = wnumavg / 40 + 6;
		for (i = 0; i < 40; i++)
			if (f < chtp[i]) fprintf(f1, "%s: %9f\n", numstr[i], chtp[i]);
	}

	if (anatype == 2) {
		if (anacnt < 0) fprintf(f1, "50回～");
		else fprintf(f1, "過去%d回 ", anacnt);
		fprintf(f1, "%d回 %dhit", wncnt, anahit);
		if (anahit < 4) fprintf(f1, "以上");
		fprintf(f1, "\n");

		f = (float)cht1avg / (float)cht1cnt;
		fprintf(f1, "統計一覧 予 %4d %4d %4d %11f %11f\n", cht1gap, cht1min,  cht1max, f, sqrt((float)cht1sd / (float)cht1cnt - f * f));
		f = (float)chtm1avg / (float)chtm1cnt;
		fprintf(f1, "         削 %4d %4d %4d %11f %11f\n", chtm1gap, chtm1min, chtm1max,f, sqrt((float)chtm1sd / (float)chtm1cnt - f * f));
		f = (float)cht2avg / (float)cht2cnt;
		fprintf(f1, "最後当選 予 %4d %4d %4d %11f %11f\n", cht2gap, cht2min,  cht2max, f, sqrt((float)cht2sd / (float)cht2cnt - f * f));
		f = (float)cht3avg / (float)cht3cnt;
		fprintf(f1, "当選期間 予 %4d %4d %4d %11f %11f\n", cht3gap, cht3min,  cht3max, f, sqrt((float)cht3sd / (float)cht3cnt - f * f));
		f = (float)chtm3avg / (float)chtm3cnt;
		fprintf(f1, "         削 %4d %4d %4d %11f %11f\n", chtm3gap, chtm3min, chtm3max,f, sqrt((float)chtm3sd / (float)chtm3cnt - f * f));
		f = (float)cht4avg / (float)cht4cnt;
		fprintf(f1, "飛後当選 予 %4d %4d %4d %11f %11f\n", cht4gap, cht4min,  cht4max, f, sqrt((float)cht4sd / (float)cht4cnt - f * f));
		f = (float)chtm4avg / (float)chtm4cnt;
		fprintf(f1, "         削 %4d %4d %4d %11f %11f\n", chtm4gap, chtm4min, chtm4max,f, sqrt((float)chtm4sd / (float)chtm4cnt - f * f));
		f = (float)cht5avg / (float)cht5cnt;
		fprintf(f1, "多重当選 予 %4d %4d %4d %11f %11f\n", cht5gap, cht5min,  cht5max, f, sqrt((float)cht5sd / (float)cht5cnt - f * f));
		f = (float)cht6avg / (float)cht6cnt;
		fprintf(f1, "前後数当 予 %4d %4d %4d %11f %11f\n", cht6gap, cht6min,  cht6max, f, sqrt((float)cht6sd / (float)cht6cnt - f * f));
		f = (float)cht7avg / (float)cht7cnt;
		fprintf(f1, "後追い数 予 %4d %4d %4d %11f %11f\n", cht7gap, cht7min,  cht7max, f, sqrt((float)cht7sd / (float)cht7cnt - f * f));
		f = (float)chtm7avg / (float)chtm7cnt;
		fprintf(f1, "         削 %4d %4d %4d %11f %11f\n\n", chtm7gap, chtm7min, chtm7max,f, sqrt((float)chtm7sd / (float)chtm7cnt - f * f));
	} else if (anatype == 3)
		for (i = 0; i < 11; i++) {
			fprintf(f1, "%s\n", chtname[i]);
			for (cht1hh = chthh[i]; cht1hh->cnt > 0; cht1hh++) {
				fprintf(f1, "%4d [%d|%d|%d|%d] %3d %dh %2d:", cht1hh->cnt, cht1hh->wn[0], cht1hh->wn[1],
				cht1hh->wn[2], cht1hh->wn[3], cht1hh->gap, cht1hh->hit, cht1hh->encnt);
				for (j = 0; j < cht1hh->encnt; j++) {
					if (j > 0) fprintf(f1, ",");
					if (i == 7) fprintf(f1, " %d", cht1hh->en[j]);
					else fprintf(f1, " %s", numstr[cht1hh->en[j]]);
				}
				fprintf(f1, "\n");
			}
			fprintf(f1, "\n");
		}

	fclose(f1);
}
