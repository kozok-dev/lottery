#include "../common.c"

void main()
{
	char en[5], ens[1000][5], cmp[2][10], hit, k;
	short cnt = 0, i, j;
	FILE *f;

	if ((f = fopen("exp.txt", "r")) == NULL) return;
	while (get_number(f, en, 5, 5, 1, 31, 0, 5, 0)) {
		for (i = 0; i < cnt; i++) {
			hit = 0;
			for (j = 0; j < 5; j++)
				for (k = 0; k < 5; k++)
					if (ens[i][j] == en[k]) {
						hit++;
						break;
					}
			if (hit > 4) break;
		}
		if (i < cnt) continue;
		for (j = 0; j < 5; j++) ens[cnt][j] = en[j];

		for (i = cnt; i > 0; i--) {
			for (j = 0; j < 2; j++)
				for (k = 0; k < 5; k++) {
					cmp[j][k * 2] = ens[i - j][k] / 10 + '0';
					cmp[j][k * 2 + 1] = ens[i - j][k] % 10 + '0';
				}
			if (memcmp(cmp[0], cmp[1], 10) >= 0) continue;
			memcpy(en, ens[i - 1], 5);
			memcpy(ens[i - 1], ens[i], 5);
			memcpy(ens[i], en, 5);
		}

		cnt++;
	}
	fclose(f);

	if ((f = fopen("exp.txt", "w")) == NULL) return;
	for (i = 0; i < cnt; i++) {
		if (i > 0) fprintf(f, "\n");
		fprintf(f, "%2d|%2d|%2d|%2d|%2d",
		ens[i][0], ens[i][1], ens[i][2], ens[i][3], ens[i][4], ens[i][5]);
	}
	fclose(f);
}
