#include "../common.c"
#define NUMMAX 9999
#define CNT_HIT(range) hit = 0; \
	for (i = wncnt - range; i < wncnt; i++) { \
		j = 0; \
		for (k = 0; k < 3; k++) { \
			for (l = 0; l < 3 && wn[i][k] != cn[cncnt].num[k * 3 + l]; l++); \
			if (l < 3) j++; \
		} \
		if (j == 3) hit++; \
	}

struct numcnt {
	short num, cnt;
} g_numcnt[1000];

int cmp(const void *val1, const void *val2)
{
	return ((struct numcnt *)val2)->cnt - ((struct numcnt *)val1)->cnt;
}

void main()
{
	char wn[NUMMAX][3], num[9];
	short hit, wncnt, i, j, k, l;
	int cncnt;
	FILE *f;
	struct {
		char num[9];
		short hit;
	} cn[31], tmp, *pcn;

	if ((f = fopen("wn.txt", "r")) == NULL) return;
	for (wncnt = 0; wncnt < NUMMAX && get_number(f, wn[wncnt], 3, 3, 0, 9, 1, 0, 0); wncnt++);
	fclose(f);

	for (i = 0; i < 30; i++) cn[i].hit = 0;
	cncnt = 0;
	for (num[0] = 0; num[0] <= 7; num[0]++)
	for (num[1] = num[0] + 1; num[1] <= 8; num[1]++)
	for (num[2] = num[1] + 1; num[2] <= 9; num[2]++)
	for (num[3] = 0; num[3] <= 7; num[3]++)
	for (num[4] = num[3] + 1; num[4] <= 8; num[4]++)
	for (num[5] = num[4] + 1; num[5] <= 9; num[5]++)
	for (num[6] = 0; num[6] <= 7; num[6]++)
	for (num[7] = num[6] + 1; num[7] <= 8; num[7]++)
	for (num[8] = num[7] + 1; num[8] <= 9; num[8]++) {
		if ((cncnt++ & 1023) == 0) printf("\b\b\b\b\b\b\b%d", cncnt);

		pcn = &cn[30];
		memcpy(pcn->num, num, 9);
		pcn->hit = 0;
		for (i = 0; i < wncnt; i++) {
			j = 0;

			k = wn[i][0];
			if (k == num[0] || k == num[1] || k == num[2]) j = 1;
			k = wn[i][1];
			if (k == num[3] || k == num[4] || k == num[5]) j++;
			k = wn[i][2];
			if (k == num[6] || k == num[7] || k == num[8]) j++;

			if (j == 3) pcn->hit++;
		}

		for (; pcn > cn && pcn->hit > pcn[-1].hit; pcn--) {
			tmp = *pcn;
			*pcn = pcn[-1];
			pcn[-1] = tmp;
		}
	}

	if ((f = fopen("cn.txt", "w")) == NULL) return;
	fprintf(f, "%d\n", wncnt);
	for (cncnt = 0; cncnt < 30; cncnt++) {
		fprintf(f, "%2d ", cncnt + 1);
		for (i = 0; i < 3; i++) {
			fprintf(f, "{");
			for (j = 0; j < 3; j++) {
				if (j > 0) fprintf(f, ", ");
				fprintf(f, "%d", cn[cncnt].num[i * 3 + j]);
			}
			fprintf(f, "} ");
		}
		fprintf(f, "| %d:%.2f%% ", cn[cncnt].hit, (float)cn[cncnt].hit / (float)wncnt * 100);
		CNT_HIT(50);
		fprintf(f, "| %d:%5.2f%% ", hit, (float)hit / 50 * 100);
		CNT_HIT(25);
		fprintf(f, "| %d:%5.2f%% ", hit, (float)hit / 25 * 100);
		CNT_HIT(10);
		fprintf(f, "| %d:%5.2f%%\n", hit, (float)hit / 10 * 100);

		for (i = 0; i < 3; i++)
			for (j = 0; j < 3; j++)
				for (k = 0; k < 3; k++) {
					l = cn[cncnt].num[i] * 100 + cn[cncnt].num[j + 3] * 10 + cn[cncnt].num[k + 3 * 2];
					g_numcnt[l].num = l;
					g_numcnt[l].cnt++;
				}
	}
	qsort(g_numcnt, 1000, sizeof(g_numcnt[0]), cmp);
	for (i = 0; i < 1000; i++)
		if (g_numcnt[i].cnt > 0) fprintf(f, "\n%03d: %d", g_numcnt[i].num, g_numcnt[i].cnt);
	fclose(f);
}
