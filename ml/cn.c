#include "../common.c"
#define NUMMAX 6000
#define CNT_HIT(start) for (i = 0; i < 7; i++) hit[i] = 0;\
	for (i = start; i < wncnt; i++) { \
		j = 0; \
		for (k = 0; k < 5; k++) { \
			for (l = 0; l < 8 && wn[i][k] != cn[cncnt].num[l]; l++); \
			if (l < 8) j++; \
		} \
		if (j == 4) { \
			for (k = 0; k < 8 && wn[i][5] != cn[cncnt].num[k]; k++); \
			hit[k < 8 ? 6 : 4]++; \
		} else \
			hit[j]++; \
	}

char g_hit[32][32];

void main()
{
	char wn[NUMMAX][6], num[8], maxcnt;
	short hit[7], wncnt, i, j, k, l;
	int cncnt;
	FILE *f;
	struct {
		char num[8];
		short hit1, hit2;
	} cn[31], tmp, *pcn;

	if ((f = fopen("wn.txt", "r")) == NULL) return;
	for (wncnt = 0; wncnt < NUMMAX && get_number(f, wn[wncnt], 6, 6, 1, 31, 0, 5, 0); wncnt++);
	fclose(f);

	if ((f = fopen("cne.txt", "r")) == NULL) {
		maxcnt = 30;
		for (i = 0; i < 30; i++) {
			cn[i].hit1 = 999;
			cn[i].hit2 = 0;
		}
		cncnt = 0;
		for (num[0] = 1; num[0] <= 24; num[0]++)
		for (num[1] = num[0] + 1; num[1] <= 25; num[1]++)
		for (num[2] = num[1] + 1; num[2] <= 26; num[2]++)
		for (num[3] = num[2] + 1; num[3] <= 27; num[3]++)
		for (num[4] = num[3] + 1; num[4] <= 28; num[4]++)
		for (num[5] = num[4] + 1; num[5] <= 29; num[5]++)
		for (num[6] = num[5] + 1; num[6] <= 30; num[6]++)
		for (num[7] = num[6] + 1; num[7] <= 31; num[7]++) {
			if ((cncnt++ & 1023) == 0) printf("\b\b\b\b\b\b\b%d", cncnt);

			pcn = &cn[30];
			memcpy(pcn->num, num, 8);
			pcn->hit1 = pcn->hit2 = 0;
			for (i = 0; i < wncnt; i++) {
				j = 0;
				for (k = 0; k < 5; k++) {
					l = wn[i][k];
					if (l == num[0] || l == num[1] || l == num[2] || l == num[3] || l == num[4] ||
					l == num[5] || l == num[6] || l == num[7]) j++;
				}
				if (j < 3) pcn->hit1++;
				else if (j == 5) pcn->hit2++;
			}

			for (; pcn > cn && pcn->hit2 > pcn[-1].hit2; pcn--) {
				tmp = *pcn;
				*pcn = pcn[-1];
				pcn[-1] = tmp;
			}
			for (; pcn > cn && pcn->hit2 == pcn[-1].hit2 && pcn->hit1 < pcn[-1].hit1; pcn--) {
				tmp = *pcn;
				*pcn = pcn[-1];
				pcn[-1] = tmp;
			}
		}
	} else {
		for (maxcnt = 0; maxcnt < 30 && get_number(f, wn[wncnt], 8, 8, 1, 31, 0, 8, 0); maxcnt++);
		fclose(f);
	}

	if ((f = fopen("cn.txt", "w")) == NULL) return;
	fprintf(f, "%d\n", wncnt);
	for (cncnt = 0; cncnt < maxcnt; cncnt++) {
		fprintf(f, "%2d {", cncnt + 1);
		for (i = 0; i < 8; i++) {
			if (i > 0) fprintf(f, ", ");
			fprintf(f, "%2d", cn[cncnt].num[i]);
		}
		CNT_HIT(0);
		fprintf(f, "} %4d:%.2f%% | ", hit[0] + hit[1] + hit[2],
		(float)(hit[0] + hit[1] + hit[2]) / (float)wncnt * 100);
		fprintf(f, "%3d:%5.2f%% | ", hit[3], (float)hit[3] / (float)wncnt * 100);
		fprintf(f, "%2d:%.2f%% | ", hit[4], (float)hit[4] / (float)wncnt * 100);
		fprintf(f, "%2d:%.2f%% | ", hit[6], (float)hit[6] / (float)wncnt * 100);
		fprintf(f, "%2d:%.2f%%", hit[5], (float)hit[5] / (float)wncnt * 100);
		CNT_HIT(wncnt - 50);
		fprintf(f, " [%d:%.2f%% | ", hit[5], (float)hit[5] / 50 * 100);
		CNT_HIT(wncnt - 25);
		fprintf(f, "%d:%.2f%% | ", hit[5], (float)hit[5] / 25 * 100);
		CNT_HIT(wncnt - 10);
		fprintf(f, "%d:%5.2f%%]\n", hit[5], (float)hit[5] / 10 * 100);

		for (j = 0; j < 8; j++)
			for (k = 0; k < 8; k++)
				if (j != k) g_hit[cn[cncnt].num[j]][cn[cncnt].num[k]]++;
	}
	fprintf(f, "\n  |");
	for (i = 1; i < 32; i++) fprintf(f, "%2d|", i);
	for (i = 1; i < 32; i++) {
		j = 0;
		fprintf(f, "\n%2d", i);
		for (k = 1; k < 32; k++) {
			if (g_hit[i][k] > 0) fprintf(f, "|%2d", g_hit[i][k]);
			else fprintf(f, "|  ");
			j += g_hit[i][k];
		}
		fprintf(f, "|%d", j);
	}
	fclose(f);
}
