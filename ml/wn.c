#define UNICODE
#define _UNICODE
#include <windows.h>
#include "../common.c"

void _tmain(int argc, TCHAR **argv) {
	char ***list;
	size_t len, i, j;
	FILE *f;
	TCHAR dirpath[MAX_PATH], *tmp;

	if (argc != 2) return;
	GetModuleFileName(NULL, dirpath, MAX_PATH);
	tmp = _tcsrchr(dirpath, TEXT('\\'));
	if (tmp != NULL) *tmp = TEXT('\0');
	_tchdir(dirpath);

	if (!get_csv_list(&list, argv[1])) return;
	f = fopen("wn.txt", "w");
	if (f != NULL) {
		len = _msize(list) / sizeof(list);
		for (i = len - 1; i > 1; i--) {
			if (_msize(list[i]) / sizeof(*list) < 10) {
				printf("error");
				getch();
				break;
			}
			fprintf(f, "%2s|%2s|%2s|%2s|%2s (%2s) %10s\n", list[i][2], list[i][3], list[i][4], list[i][5], list[i][6], list[i][7], list[i][9]);
		}
		fclose(f);
	}
	free(list);
}
