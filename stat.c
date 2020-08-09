#include <stdio.h>
#include <stdlib.h>

void main()
{
	char buf[64];
	double cnt = 0, sum = 0, sd = 0, max = 0, min = 0, avg, i, j = 0, k = 0, l = 0, m = 0, n = 0;
	FILE *f;

	if ((f = fopen("stat.txt", "r")) == NULL) return;
	while (fgets(buf, 64, f) != NULL) {
		i = atof(buf);
		if (i > max || cnt == 0) max = i;
		if (i < min || cnt == 0) min = i;
		cnt++;
		sum += i;
		sd += i * i;
	}
	fclose(f);
	if (cnt == 0) return;

	avg = sum / cnt;
	sd = sqrt(sd / cnt - avg * avg);
	printf("��      �F%.0f\n", cnt);
	printf("����    �F%f\n", avg);
	printf("�W���΍��F%f\n", sd);
	printf("���}1�� �F%f �` %f\n", avg - sd, avg + sd);
	printf("���}2�� �F%f �` %f\n", avg - sd * 2, avg + sd * 2);
	printf("���}3�� �F%f �` %f\n", avg - sd * 3, avg + sd * 3);
	printf("�ő�    �F%f\n", max);
	printf("�ŏ�    �F%f\n", min);

	max = avg + sd;
	min = avg - sd;
	max = floor(max < 0 ? max - 0.5 : max + 0.5);
	min = floor(min < 0 ? min - 0.5 : min + 0.5);
	avg = floor(avg < 0 ? avg - 0.5 : avg + 0.5);
	if ((f = fopen("stat.txt", "r")) == NULL) return;
	while (fgets(buf, 64, f) != NULL) {
		i = atof(buf);
		if (i < avg) j++;
		else if (i > avg) k++;
		else l++;

		if (i < min) m++;
		else if (i > max) n++;
	}
	fclose(f);

	printf("����=   �F%.0f / %.0f = %f%%\n", l, cnt, l / cnt * 100);
	printf("����>   �F%.0f / %.0f = %f%%\n", j, cnt, j / cnt * 100);
	printf("����<   �F%.0f / %.0f = %f%%\n", k, cnt, k / cnt * 100);
	printf("����1��>�F%.0f / %.0f = %f%%, / %.0f = %f%%\n", m, cnt, m / cnt * 100, j, m / j * 100);
	printf("����1��<�F%.0f / %.0f = %f%%, / %.0f = %f%%\n", n, cnt, n / cnt * 100, k, n / k * 100);

	getch();
}
