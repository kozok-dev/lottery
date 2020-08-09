#include "../common.c"
#define NUMMAX 9999
#define CNT_HIT(num, start) hit = 0; \
	for (i = start; i < wncnt; i++) { \
		j = 0; \
		for (k = 0; k < 4; k++) { \
			for (l = 0; l < 3 && wn[i][k] != num[k * 3 + l]; l++); \
			if (l < 3) j++; \
		} \
		if (j == 4) hit++; \
	}

char g_flag = 1;
struct numcnt {
	short num, cnt;
} g_numcnt[10000];

void set_flag(void *dmy) {
	getch();
	g_flag = 0;
	_endthread();
}

int cmp(const void *val1, const void *val2)
{
	return ((struct numcnt *)val2)->cnt - ((struct numcnt *)val1)->cnt;
}

void main()
{
	char wn[NUMMAX][4], num[12], rnd[NUMMAX * 3], rnd_n[NUMMAX * 3], cncnt, maxidx;
	short hit, wncnt, rndcnt, rndcnt_n, i, j, k, l, m, n;
	FILE *f;
	struct {
		char num[12];
		short hit;
	} cn[30], cn_n[30], *pcn1, *pcn2;

	if ((f = fopen("wn.txt", "r")) == NULL) return;
	for (wncnt = 0; wncnt < NUMMAX && get_number(f, wn[wncnt], 4, 4, 0, 9, 1, 0, 0); wncnt++);
	fclose(f);

	/* 初期データ生成。*/
	maxidx = rndcnt = 0;
	for (cncnt = 0; cncnt < 30; cncnt++) {
		for (i = 0; i < 4; i++) {
			j = i * 3;
			for (k = 0; k < 3; k++) {
				do {
					l = next_int_ex(10);
					for (m = 0; m < k && num[j + m] != l; m++);
				} while (m < k);
				num[j + k] = l;
				for (l = k; l > 0 && num[j + l] < num[j + l - 1]; l--) {
					m = num[j + l];
					num[j + l] = num[j + l - 1];
					num[j + l - 1] = m;
				}
			}
		}

		memcpy(cn[cncnt].num, num, 12);
		CNT_HIT(cn[cncnt].num, 0);
		cn[cncnt].hit = hit;
		if (hit > cn[maxidx].hit) maxidx = cncnt;
		memset(rnd + rndcnt, cncnt, hit);
		rndcnt += hit;
	}

	_beginthread(set_flag, 0, NULL);
	while (g_flag) {
		printf("\b\b\b\b\b%2d   ", cn[maxidx].hit);
		pcn1 = cn_n;
		cncnt = 1;
		n = 0;

		/* 最も良いデータを保存 ----------------------------- */
		*pcn1 = cn[maxidx];
		maxidx = 0;
		memset(rnd_n, 0, pcn1->hit);
		rndcnt_n = pcn1->hit;
		pcn1++;

		while (cncnt < 30 && g_flag) {
			if ((n & 1023) == 0) {
				printf("\b\b%2d", cncnt);
				n = 1;
			} else
				n++;

			/* データを2つ選択後、交叉して保存 ------------------ */
			if (cncnt < 29) {
				i = rnd[next_int_ex(rndcnt)];
				do j = rnd[next_int_ex(rndcnt)]; while (i == j);
				*pcn1 = cn[i];
				pcn2 = &pcn1[1];
				*pcn2 = cn[j];

				for (i = 0; i < 4; i++) {
					j = next_int_ex(4) * 3;
					for (k = j; k < j + 3; k++) {
						m = pcn1->num[k];
						pcn1->num[k] = pcn2->num[k];
						pcn2->num[k] = m;
					}
				}

				for (i = 0; i < cncnt && memcmp(cn_n[i].num, pcn1->num, 12); i++);
				if (i < cncnt) {
					*pcn1 = *pcn2;
					pcn2 = pcn1;
				} else {
					CNT_HIT(pcn1->num, 0);
					if (hit >= cn_n[0].hit - 3) {
						pcn1->hit = hit;
						if (hit > cn_n[maxidx].hit) maxidx = cncnt;
						memset(rnd_n + rndcnt_n, cncnt, pcn1->hit);
						rndcnt_n += pcn1->hit;

						pcn1++;
						cncnt++;
					} else {
						*pcn1 = *pcn2;
						pcn2 = pcn1;
					}
				}

				for (i = 0; i < cncnt && memcmp(cn_n[i].num, pcn2->num, 12); i++);
				if (i == cncnt) {
					CNT_HIT(pcn2->num, 0);
					if (hit >= cn_n[0].hit - 3) {
						pcn2->hit = hit;
						if (hit > cn_n[maxidx].hit) maxidx = cncnt;
						memset(rnd_n + rndcnt_n, cncnt, pcn2->hit);
						rndcnt_n += pcn2->hit;

						pcn1++;
						cncnt++;
					}
				}
			}

			/* 突然変異 ----------------------------------------- */
			if (cncnt < 30 && next_int_ex(4) == 0 || cncnt == 29) {
				*pcn1 = cn[rnd[next_int_ex(rndcnt)]];
				for (i = 0; i < 4; i++) {
					if (next_int_ex(2) > 0) continue;

					j = i * 3;
					k = next_int_ex(3);
					do {
						l = next_int_ex(10);
						for (m = 0; m < 3 && pcn1->num[j + k] != l; m++);
					} while (m < 3);
					pcn1->num[j + k] = l;
					for (k = 0; k < 2; k++)
						for (l = k + 1; l < 3; l++) {
							if (pcn1->num[j + k] <= pcn1->num[j + l]) continue;
							m = pcn1->num[j + k];
							pcn1->num[j + k] = pcn1->num[j + l];
							pcn1->num[j + l] = m;
						}
				}

				for (i = 0; i < cncnt && memcmp(cn_n[i].num, pcn1->num, 12); i++);
				if (i == cncnt) {
					CNT_HIT(pcn1->num, 0);
					if (hit >= cn_n[0].hit - 3) {
						pcn1->hit = hit;
						if (hit > cn_n[maxidx].hit) maxidx = cncnt;
						memset(rnd_n + rndcnt_n, cncnt, pcn1->hit);
						rndcnt_n += pcn1->hit;

						pcn1++;
						cncnt++;
					}
				}
			}
		}

		/* データ更新。*/
		memcpy(cn, cn_n, sizeof(cn_n[0]) * cncnt);
		memcpy(rnd, rnd_n, sizeof(rnd_n));
		rndcnt = rndcnt_n;
	}

	/* データ並べ替え。*/
	for (i = 0; i < 29; i++)
		for (j = i + 1; j < 30; j++) {
			if (cn[i].hit >= cn[j].hit) continue;
			cn_n[0] = cn[i];
			cn[i] = cn[j];
			cn[j] = cn_n[0];
		}
	for (i = 0; i < 29; i++)
		for (j = i + 1; j < 30 && cn[i].hit == cn[j].hit; j++) {
			if (memcmp(cn[i].num, cn[j].num, 12) <= 0) continue;
			cn_n[0] = cn[i];
			cn[i] = cn[j];
			cn[j] = cn_n[0];
		}

	if ((f = fopen("cn.txt", "w")) == NULL) return;
	fprintf(f, "%d\n", wncnt);
	for (cncnt = 0; cncnt < 30; cncnt++) {
		fprintf(f, "%2d ", cncnt + 1);
		for (i = 0; i < 4; i++) {
			fprintf(f, "{");
			for (j = 0; j < 3; j++) {
				if (j > 0) fprintf(f, ", ");
				fprintf(f, "%d", cn[cncnt].num[i * 3 + j]);
			}
			fprintf(f, "} ");
		}
		fprintf(f, "| %3d:%.2f%% ", cn[cncnt].hit, (float)cn[cncnt].hit / 2089 * 100);
		CNT_HIT(cn[cncnt].num, wncnt - 50);
		fprintf(f, "| %d:%5.2f%% ", hit, (float)hit / 50 * 100);
		CNT_HIT(cn[cncnt].num, wncnt - 25);
		fprintf(f, "| %d:%5.2f%% ", hit, (float)hit / 25 * 100);
		CNT_HIT(cn[cncnt].num, wncnt - 10);
		fprintf(f, "| %d:%5.2f%%\n", hit, (float)hit / 10 * 100);

		for (i = 0; i < 3; i++)
			for (j = 0; j < 3; j++)
				for (k = 0; k < 3; k++)
					for (l = 0; l < 3; l++) {
						m = cn[cncnt].num[i] * 1000 + cn[cncnt].num[j + 3] * 100 + cn[cncnt].num[k + 3 * 2] * 10 + cn[cncnt].num[l + 3 * 3];
						g_numcnt[m].num = m;
						g_numcnt[m].cnt++;
					}
	}
	qsort(g_numcnt, 10000, sizeof(g_numcnt[0]), cmp);
	for (i = 0; i < 10000; i++)
		if (g_numcnt[i].cnt > 0) fprintf(f, "\n%04d: %d", g_numcnt[i].num, g_numcnt[i].cnt);
	fclose(f);
}
