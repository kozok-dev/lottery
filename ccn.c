#include <stdio.h>
#include <string.h>
#include "sqlite3.h"

char *g_name;
struct {
	char *name;
	int cnt;
} g_rank[5], g_tmp;
FILE *g_f;

char imp_data(sqlite3 *db, const char *name)
{
	char sql[1024], buf[1024], flag = 1, *bp, *tmp;
	int cnt = 0;
	FILE *f;

	printf("0");
	sprintf(sql, "ana_%s.txt", name);
	if ((f = fopen(sql, "r")) == NULL) goto procend;

	if (fgets(buf, 1024, f) == NULL) goto procend;
	flag = 0;

	sprintf(sql, "CREATE TABLE IF NOT EXISTS %s (", name);
	buf[strlen(buf) - 1] = '\0';
	bp = buf;
	while ((tmp = strchr(bp, '\t')) != NULL) {
		*tmp = '\0';
		strcat(sql, bp);
		strcat(sql, ", ");
		bp = tmp + 1;
	}
	strcat(sql, bp);
	strcat(sql, ")");
	if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) goto procend;

	sprintf(sql, "DELETE FROM %s", name);
	if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) goto procend;

	if (sqlite3_exec(db, "BEGIN", NULL, NULL, NULL) != SQLITE_OK) goto procend;
	while (fgets(buf, 1024, f) != NULL) {
		if ((cnt++ & 1023) == 0) printf("\b\b\b\b\b\b\b\b\b\b%d", cnt);

		sprintf(sql, "INSERT INTO %s VALUES(", name);
		buf[strlen(buf) - 1] = '\0';
		bp = buf;
		while ((tmp = strchr(bp, '\t')) != NULL) {
			*tmp = '\0';
			strcat(sql, "'");
			strcat(sql, bp);
			strcat(sql, "', ");
			bp = tmp + 1;
		}
		strcat(sql, "'");
		strcat(sql, bp);
		strcat(sql, "')");
		if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) goto procend;
	}
	if (sqlite3_exec(db, "COMMIT", NULL, NULL, NULL) != SQLITE_OK) goto procend;

	flag = 1;

procend:
	printf("\b\b\b\b\b\b\b\b\b\b%d", cnt);
	if (f != NULL) fclose(f);
	printf("\n");
	if (!flag) sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
	return flag;
}

int set_rank(void *dmy, int colcnt, char **val, char **col)
{
	static char cnt;
	char i;

	if (colcnt != 1) return 0;

	g_rank[cnt].name = g_name;
	g_rank[cnt].cnt = atoi(val[0]);

	for (i = cnt; i > 0 && g_rank[i].cnt > g_rank[i - 1].cnt; i++) {
		g_tmp = g_rank[i];
		g_rank[i] = g_rank[i - 1];
		g_rank[i - 1] = g_tmp;
	}
	cnt++;

	return 0;
}

int write_number(void *dmy, int colcnt, char **val, char **col)
{
	char i;

	for (i = 0; i < NUM; i++) {
		if (i > 0) fprintf(g_f, "|");
		fprintf(g_f, "%2s", val[i]);
	}
	fprintf(g_f, "\n");
	return 0;
}

char create_view(sqlite3 *db, char *name)
{
	char sql[1024];

	printf("-");

	sprintf(sql,
	"CREATE VIEW IF NOT EXISTS %s AS "
	"SELECT %s, COUNT(*) AS cnt FROM wn GROUP BY %s ORDER BY cnt DESC, %s",
	name, name, name, name);
	if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK) return 0;

	g_name = name;
	sprintf(sql, "SELECT cnt FROM %s LIMIT 1", name);
	return sqlite3_exec(db, sql, set_rank, NULL, NULL) == SQLITE_OK;
}

void main()
{
	char sql[1024], i;
	sqlite3 *db;

	if (sqlite3_open("exp.db", &db) != SQLITE_OK) return;

	if (!imp_data(db, "wn") || !imp_data(db, "ec") || !create_view(db, "集2T") ||
	!create_view(db, "下T") || !create_view(db, "差T") || !create_view(db, "ｾｯﾄT") ||
	!create_view(db, "平T") || (g_f = fopen("exp.txt", "w")) == NULL)
		goto err;

	strcpy(sql,
	"SELECT ec.* FROM ec, 集2T, 下T, 差T, ｾｯﾄT, 平T WHERE ec.集2T = 集2T.集2T AND ec.下T = 下T.下T AND "
	"ec.差T = 差T.差T AND ec.ｾｯﾄT = ｾｯﾄT.ｾｯﾄT AND ec.平T = 平T.平T ORDER BY ");
	for (i = 0; i < 5; i++) {
		if (i > 0) strcat(sql, ", ");
		strcat(sql, g_rank[i].name);
		strcat(sql, ".cnt DESC");
	}
	if (sqlite3_exec(db, sql, write_number, NULL, NULL) != SQLITE_OK) {
err:
		printf("\n%s", sqlite3_errmsg(db));
		getch();
	}

	if (g_f != NULL) fclose(g_f);
	sqlite3_close(db);
}
