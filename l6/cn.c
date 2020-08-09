#include "../common.c"
#define NUMMAX 6000
#define CNT_HIT(num, start) for (i = 0; i < 6; i++) hit[i] = 0;\
	for (i = start; i < wncnt; i++) { \
		j = 0; \
		for (k = 0; k < 6; k++) { \
			for (l = 0; l < 11 && wn[i][k] != num[l]; l++); \
			if (l < 11) j++; \
		} \
		if (j < 3) \
			hit[0]++; \
		else if (j == 5) { \
			for (k = 0; k < 11 && wn[i][6] != num[k]; k++); \
			hit[k < 11 ? 5 : 3]++; \
		} else \
			hit[j - 2]++; \
	}

char g_hit[44][44], g_flag = 1;

void set_flag(void *dmy) {
	getch();
	g_flag = 0;
	_endthread();
}

void main()
{
	char wn[NUMMAX][7], num[11], rnd[NUMMAX * 3], rnd_n[NUMMAX * 3], cncnt, maxidx, maxcnt;
	short hit[6], wncnt, rndcnt, rndcnt_n, i, j, k, l;
	FILE *f;
	struct {
		char num[11];
		short hit1, hit2;
	} cn[30], cn_n[30], *pcn1, *pcn2;

	if ((f = fopen("wn.txt", "r")) == NULL) return;
	for (wncnt = 0; wncnt < NUMMAX && get_number(f, wn[wncnt], 7, 7, 1, 43, 0, 6, 0); wncnt++);
	fclose(f);

	if ((f = fopen("cne.txt", "r")) == NULL) {
		maxcnt = 30;

		/* 初期データ生成。*/
		maxidx = rndcnt = 0;
		for (cncnt = 0; cncnt < 30; cncnt++) {
			if (cncnt > 0)
				for (i = 0; i < 11; i++) {
					do {
						j = next_int_ex(43) + 1;
						for (k = 0; k < i && num[k] != j; k++);
					} while (k < i);
					num[i] = j;
					for (j = i; j > 0 && num[j] < num[j - 1]; j--) {
						k = num[j];
						num[j] = num[j - 1];
						num[j - 1] = k;
					}
				}

			memcpy(cn[cncnt].num, num, 11);
			CNT_HIT(cn[cncnt].num, 0);
			cn[cncnt].hit1 = hit[0];
			cn[cncnt].hit2 = hit[4];
			if (hit[4] > cn[maxidx].hit2 || hit[4] >= cn[maxidx].hit2 && hit[0] < cn[maxidx].hit1)
				maxidx = cncnt;
			i = hit[0] / 10 + 0.5;
			memset(rnd + rndcnt, cncnt, i);
			rndcnt += i;
		}

		_beginthread(set_flag, 0, NULL);
		while (g_flag) {
			printf("\b\b\b\b\b\b%d-%d", cn[maxidx].hit2, cn[maxidx].hit1);
			pcn1 = cn_n;
			cncnt = 1;

			/* 最も良いデータを保存 ----------------------------- */
			*pcn1 = cn[maxidx];
			maxidx = 0;
			i = pcn1->hit1 / 10 + 0.5;
			memset(rnd_n, 0, i);
			rndcnt_n = i;
			pcn1++;

			while (cncnt < 30 && g_flag) {
				/* データを2つ選択後、交叉して保存 ------------------ */
				if (cncnt < 29) {
					i = rnd[next_int_ex(rndcnt)];
					do j = rnd[next_int_ex(rndcnt)]; while (i == j);
					*pcn1 = cn[i];
					pcn2 = &pcn1[1];
					*pcn2 = cn[j];

					for (i = 0; i < 11; i++) {
						if (next_int_ex(2) > 0) continue;
						for (j = 0; j < 11 && pcn1->num[i] != pcn2->num[j]; j++);
						if (j < 11) continue;
						for (j = 0; j < 11 && pcn1->num[j] != pcn2->num[i]; j++);
						if (j < 11) continue;

						j = pcn1->num[i];
						pcn1->num[i] = pcn2->num[i];
						pcn2->num[i] = j;
					}
					for (i = 0; i < 10; i++)
						for (j = i + 1; j < 11; j++) {
							if (pcn1->num[i] > pcn1->num[j]) {
								k = pcn1->num[i];
								pcn1->num[i] = pcn1->num[j];
								pcn1->num[j] = k;
							}
							if (pcn2->num[i] > pcn2->num[j]) {
								k = pcn2->num[i];
								pcn2->num[i] = pcn2->num[j];
								pcn2->num[j] = k;
							}
						}

					for (i = 0; i < cncnt && memcmp(cn_n[i].num, pcn1->num, 11); i++);
					if (i < cncnt) {
						*pcn1 = *pcn2;
						pcn2 = pcn1;
					} else {
						CNT_HIT(pcn1->num, 0);
						if (hit[4] >= cn_n[0].hit2 - 1 ||
						hit[4] >= cn_n[0].hit2 - 2 && hit[0] <= cn_n[0].hit1 + 5) {
							pcn1->hit1 = hit[0];
							pcn1->hit2 = hit[4];
							if (hit[4] > cn_n[maxidx].hit2 || hit[4] >= cn_n[maxidx].hit2 &&
							hit[0] < cn_n[maxidx].hit1) maxidx = cncnt;
							i = hit[0] / 10 + 0.5;
							memset(rnd_n + rndcnt_n, cncnt, i);
							rndcnt_n += i;

							pcn1++;
							cncnt++;
						} else {
							*pcn1 = *pcn2;
							pcn2 = pcn1;
						}
					}

					for (i = 0; i < cncnt && memcmp(cn_n[i].num, pcn2->num, 11); i++);
					if (i == cncnt) {
						CNT_HIT(pcn2->num, 0);
						if (hit[4] >= cn_n[0].hit2 - 1 ||
						hit[4] >= cn_n[0].hit2 - 2 && hit[0] <= cn_n[0].hit1 + 5) {
							pcn2->hit1 = hit[0];
							pcn2->hit2 = hit[4];
							if (hit[4] > cn_n[maxidx].hit2 || hit[4] >= cn_n[maxidx].hit2 &&
							hit[0] < cn_n[maxidx].hit1) maxidx = cncnt;
							i = hit[0] / 10 + 0.5;
							memset(rnd_n + rndcnt_n, cncnt, i);
							rndcnt_n += i;

							pcn1++;
							cncnt++;
						}
					}
				}

				/* 突然変異 ----------------------------------------- */
				if (cncnt < 30 && next_int_ex(5) == 0 || cncnt == 29) {
					*pcn1 = cn[rnd[next_int_ex(rndcnt)]];

					for (i = 0; i < 10; i++) {
						if (next_int_ex(3) > 0) continue;

						j = next_int_ex(11);
						do {
							k = next_int_ex(43) + 1;
							l = 0;
							if (pcn1->num[j] == k) continue;
							for (; l < 11 && pcn1->num[l] != k; l++);
						} while (l < 11);
						pcn1->num[j] = k;
					}
					for (i = 0; i < 10; i++)
						for (j = i + 1; j < 11; j++) {
							if (pcn1->num[i] > pcn1->num[j]) {
								k = pcn1->num[i];
								pcn1->num[i] = pcn1->num[j];
								pcn1->num[j] = k;
							}
						}

					for (i = 0; i < cncnt && memcmp(cn_n[i].num, pcn1->num, 11); i++);
					if (i == cncnt) {
						CNT_HIT(pcn1->num, 0);
						if (hit[4] >= cn_n[0].hit2 - 1 ||
						hit[4] >= cn_n[0].hit2 - 2 && hit[0] <= cn_n[0].hit1 + 5) {
							pcn1->hit1 = hit[0];
							pcn1->hit2 = hit[4];
							if (hit[4] > cn_n[maxidx].hit2 || hit[4] >= cn_n[maxidx].hit2 &&
							hit[0] < cn_n[maxidx].hit1) maxidx = cncnt;
							i = hit[0] / 10 + 0.5;
							memset(rnd_n + rndcnt_n, cncnt, i);
							rndcnt_n += i;

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
				if (cn[i].hit2 >= cn[j].hit2) continue;
				cn_n[0] = cn[i];
				cn[i] = cn[j];
				cn[j] = cn_n[0];
			}
		for (i = 0; i < 29; i++)
			for (j = i + 1; j < 30 && cn[i].hit2 == cn[j].hit2; j++) {
				if (cn[i].hit1 <= cn[j].hit1) continue;
				cn_n[0] = cn[i];
				cn[i] = cn[j];
				cn[j] = cn_n[0];
			}
		for (i = 0; i < 29; i++)
			for (j = i + 1; j < 30 && cn[i].hit2 == cn[j].hit2 && cn[i].hit1 == cn[j].hit1; j++) {
				if (memcmp(cn[i].num, cn[j].num, 11) <= 0) continue;
				cn_n[0] = cn[i];
				cn[i] = cn[j];
				cn[j] = cn_n[0];
			}
	} else {
		for (maxcnt = 0; maxcnt < 30 && get_number(f, cn[maxcnt].num, 11, 11, 1, 43, 0, 11, 0); maxcnt++);
		fclose(f);
	}

	if ((f = fopen("cn.txt", "w")) == NULL) return;
	fprintf(f, "%d\n", wncnt);
	for (cncnt = 0; cncnt < maxcnt; cncnt++) {
		fprintf(f, "%2d {", cncnt + 1);
		for (i = 0; i < 11; i++) {
			if (i > 0) fprintf(f, ", ");
			fprintf(f, "%2d", cn[cncnt].num[i]);
		}
		CNT_HIT(cn[cncnt].num, 0);
		fprintf(f, "} %d:%.2f%% | ", hit[0], (float)hit[0] / (float)wncnt * 100);
		fprintf(f, "%3d:%5.2f%% | ", hit[1], (float)hit[1] / (float)wncnt * 100);
		fprintf(f, "%3d:%.2f%% | ", hit[2], (float)hit[2] / (float)wncnt * 100);
		fprintf(f, "%2d:%.2f%% | ", hit[3], (float)hit[3] / (float)wncnt * 100);
		fprintf(f, "%2d:%.2f%% | ", hit[5], (float)hit[5] / (float)wncnt * 100);
		fprintf(f, "%2d:%.2f%%", hit[4], (float)hit[4] / (float)wncnt * 100);
		CNT_HIT(cn[cncnt].num, wncnt - 50);
		fprintf(f, " [%d:%.2f%% | ", hit[4], (float)hit[4] / 50 * 100);
		CNT_HIT(cn[cncnt].num, wncnt - 25);
		fprintf(f, "%d:%.2f%% | ", hit[4], (float)hit[4] / 25 * 100);
		CNT_HIT(cn[cncnt].num, wncnt - 10);
		fprintf(f, "%d:%5.2f%%]\n", hit[4], (float)hit[4] / 10 * 100);

		for (i = 0; i < 11; i++)
			for (j = 0; j < 11; j++)
				if (i != j) g_hit[cn[cncnt].num[i]][cn[cncnt].num[j]]++;
	}
	fprintf(f, "\n  |");
	for (i = 1; i < 44; i++) fprintf(f, "%2d|", i);
	for (i = 1; i < 44; i++) {
		j = 0;
		fprintf(f, "\n%2d", i);
		for (k = 1; k < 44; k++) {
			if (g_hit[i][k] > 0) fprintf(f, "|%2d", g_hit[i][k]);
			else fprintf(f, "|  ");
			j += g_hit[i][k];
		}
		fprintf(f, "|%d", j);
	}
	fclose(f);
}
