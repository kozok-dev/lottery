#if SCRTYPE == 1
#define NUM_MIN 0
#define NUM_MAX 9
#define NUM_CNT 3
#define SUM_MIN 0
#define SUM_MAX 27
#define GROUP_MAX 0
#define SUB_MIN -9
#define SUB_MAX 9
#define PASTHIT_SIZE 8
#elif SCRTYPE == 2
#define NUM_MIN 0
#define NUM_MAX 9
#define NUM_CNT 4
#define SUM_MIN 0
#define SUM_MAX 36
#define GROUP_MAX 0
#define SUB_MIN -9
#define SUB_MAX 9
#define PASTHIT_SIZE 6
#elif SCRTYPE == 3
#define NUM_MIN 1
#define NUM_MAX 31
#define NUM_CNT 5
#define SUM_MIN 15
#define SUM_MAX 145
#define GROUP_MAX 'g'
#define SUB_MIN 0
#define SUB_MAX 27
#define PASTHIT_SIZE 14
#else
#define NUM_MIN 1
#define NUM_MAX 43
#define NUM_CNT 6
#define SUM_MIN 21
#define SUM_MAX 243
#define GROUP_MAX 'k'
#define SUB_MIN 0
#define SUB_MAX 38
#define PASTHIT_SIZE 16
#endif

/* 構文の各種区切り文字。*/
#define WORDCHR "+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"
#define SYMCHR "!\"$%&*,.:<=>?@[\\]^`{|}~"
#define BLANKCHR "\t\n\v\f\r "
/* 動的領域追加確保マクロ。*/
#define AALLOC(add) code = realloc(g_code, size + add); \
	if (code == NULL) return TOO_LONG_SCRIPT; \
	g_code = code; \
	code = g_code + size; \
	size = _msize(g_code);
/* push numマクロ。*/
#define COMPILE_PUSH_NUM(adr) if (radr + adr < 128) { \
		AALLOC(4); \
		code[1] = 0x74; \
		code[3] = radr + adr; \
	} else { \
		AALLOC(7); \
		code[1] = 0xb4; \
		*((int *)&code[3]) = radr + adr; \
	} \
	code[0] = 0xff; code[2] = 0x24;
/* lea edx, [result + adr]、push edxマクロ。*/
#define COMPILE_PUSH_RESULT_ADR(adr) if (g_spword[g_scrcondexp[i].exp1.data].result_radr + adr < 128) { \
		AALLOC(4); \
		code[1] = 0x54; code[3] = g_spword[g_scrcondexp[i].exp1.data].result_radr + adr; \
	} else { \
		AALLOC(7); \
		code[1] = 0x94; *((int *)&code[3]) = g_spword[g_scrcondexp[i].exp1.data].result_radr + adr; \
	} \
	code[0] = 0x8d; code[2] = 0x24; \
	AALLOC(2); \
	code[0] = 0xff; code[1] = 0xf2;	/* push edx */

/* 構文チェックコードと対応する文字列。*/
typedef enum {
	SUCCESS,
	CONTINUE,
	TOO_LONG_SCRIPT,
	TOO_MANY_BRACKET,
	OUT_RANGE_NUMBER,
	OUT_RANGE_EXP,
	OUT_RANGE_SUFFIX,
	REQUIRE_SUFFIX,
	INVALID_SYNTAX,
	INVALID_COND_EXP,
	INVALID_EXP,
	INVALID_COND_OP,
	INVALID_LOGIC_OP,
	INVALID_BRACKET,
	INVALID_CLOSE_BRACKET,
	RESERVED
} sscode;
char *g_ssmsg[] = {
	"エラーはありません",
	"構文チェックは完了していません",
	"構文が長すぎます",
	"開き括弧が多すぎます",
	"数値が許容範囲外です",
	"式が許容範囲外です",
	"添え字が許容範囲外です",
	"添え字が必要です",
	"構文エラーです",
	"条件式が不正です",
	"式が不正です",
	"条件演算子が不正です",
	"論理演算子が不正です",
	"括弧が不正です",
	"閉じ括弧が不正です",
	"予約済み"
};

typedef struct {	/* 式データ。*/
	enum {SPWORD, NUM, ALPHA} type;
	short data;
	char suffix[NUM_MAX - NUM_MIN + 2];	/* 特殊語句の場合のみ使用する添え字。*/
} SCREXP;
typedef struct {	/* 条件式データ。*/
	SCREXP exp1, exp2;
	enum {EQ, NE, LT, GT, LE, GE} cond;
	enum {NONE, AND, OR} logic;
	char bracket;
	short jcode, size;	/* 条件ジャンプを設定する位置、実行コードサイズ。*/
} SCRCONDEXP;
struct {	/* 特殊語句データ。*/
	short cnt;	/* 使用される特殊語句の数。エラー時の数は未定義。*/
	short flag_radr;	/* 計算済みフラグ先頭相対格納場所。*/
	short result_radr;	/* 計算結果先頭相対格納場所。*/
} g_spword[21];
char g_arf, g_ec, g_expnum[SCRTYPE <= 2 ? NUM_CNT : 1][NUM_MAX - NUM_MIN + 2], *g_code;
short g_scrcondexp_cnt, g_wnrange, g_rndcnt;
SCRCONDEXP *g_scrcondexp;

/* 式の組み合わせの構文チェック。
特殊語句に合った数値やアルファベットになっているか。exp1を特殊語句、exp2を数値やアルファベットにする。*/
sscode check_syntax_exp_cmb(SCRCONDEXP *scrcondexp)
{
	short i;
	SCREXP screxp;

	/* exp1を特殊語句、exp2を数値やアルファベットにする。*/
	if (scrcondexp->exp1.type != SPWORD) {
		screxp = scrcondexp->exp1;
		scrcondexp->exp1 = scrcondexp->exp2;
		scrcondexp->exp2 = screxp;

		switch (scrcondexp->cond) {
		case LT:
			scrcondexp->cond = GT; break;
		case GT:
			scrcondexp->cond = LT; break;
		case LE:
			scrcondexp->cond = GE; break;
		case GE:
			scrcondexp->cond = LE;
		}
	}

	/* 特殊語句に合った数値やアルファベットかチェック。-- */
	i = scrcondexp->exp2.data;
	switch (scrcondexp->exp1.data) {
	case 0:	/* NUM */
		if (scrcondexp->exp1.suffix[0] < 0) return REQUIRE_SUFFIX;
		if (i < NUM_MIN || i > NUM_MAX) return OUT_RANGE_EXP;
		break;
	case 1:	/* SUM */
		if (scrcondexp->exp2.type != NUM || i < SUM_MIN || i > SUM_MAX) return OUT_RANGE_EXP;
		break;
	case 2:	/* BIG */
	case 3:	/* SMALL */
	case 4:	/* ODD */
	case 5:	/* EVEN */
	case 7:	/* GROUP2 */
	case 9:	/* DIGIT1 */
	case 10:	/* DIGIT2 */
	case 12:	/* SEQ */
	case 13:	/* PASTSAME */
	case 16:	/* GAP2 */
	case 17:	/* HOT */
		if (i > NUM_CNT) return OUT_RANGE_EXP;
		break;
	case 6:	/* GROUP1 */
	case 8:	/* SET */
		if (scrcondexp->exp2.type != ALPHA) return OUT_RANGE_EXP;
		i = tolower(i);
		if (scrcondexp->exp1.suffix[0] < 0) {
			if (i < 'a' || i > GROUP_MAX) return OUT_RANGE_EXP;
		} else
			if (i < 'a' || i > 'f') return OUT_RANGE_EXP;
		break;
	case 11:	/* SUB */
		if (i < SUB_MIN || i > SUB_MAX) return OUT_RANGE_EXP;
		break;
	case 14:	/* AVG */
		if (scrcondexp->exp1.suffix[0] < 0) {
			if (scrcondexp->exp2.type != NUM || i > NUM_CNT) return OUT_RANGE_EXP;
		} else {
			i = tolower(i);
			if (scrcondexp->exp2.type != ALPHA || i != 'w' && i != 'l') return OUT_RANGE_EXP;
		}
		break;
	case 15:	/* GAP1 */
		if (i > 99) return OUT_RANGE_EXP;
		break;
	case 18:	/* PASTHIT */
	case 19:	/* CNT */
		if (scrcondexp->exp1.suffix[0] < 0) return REQUIRE_SUFFIX;
		break;
	case 20:	/* MARK */
	case 21:	/* PASTSAMESEQ */
		if (i > 1) return OUT_RANGE_EXP;
		break;
	default:
		return RESERVED;
	}

	return CONTINUE;
}

/* 式の構文チェック。
特殊語句、数値、アルファベットが正しいかどうか。
screxp2が設定されていれば、特殊語句に合った数値やアルファベットになっているかもチェックします。
一部の特殊語句に添え字があるなら、正しい添え字で特殊語句に合った添え字になっているか。
※screxp1にチェックする式、screxp2に既にチェックした式を指定します。
  つまり、左辺の式をチェックする場合はscrexp2にNULL、
  右辺の式をチェックする場合はscrexp2に、左辺の式をチェックしたときのscrexpを指定します。*/
sscode check_syntax_exp(const char **scr, const char *len, SCREXP *screxp1, SCREXP *screxp2, char bracket)
{
	static char *spword[] = {
		"NUM",
		"SUM",
		"BIG", "SMALL",
		"ODD", "EVEN",
		"GROUP1", "GROUP2", "SET",
		"DIGIT1", "DIGIT2",
		"SUB",
		"SEQ",
		"PASTSAME",
		"AVG",
		"GAP1", "GAP2", "HOT",
		"PASTHIT",
		"CNT",
		"MARK",
		"PASTSAMESEQ"
	};
	char r1, r2, *chk;
	short i;
	int j;
	sscode ssc;
	SCREXP *screxp;

	i = strspn(*scr, WORDCHR);
	if (i < 1) return INVALID_SYNTAX;

	for (j = 0; j < sizeof(spword) / sizeof(spword[0]); j++) {
#if SCRTYPE <= 2
		if (j >= 6 && j <= 10 || j == 16 || j == 19 || j == 20) continue;
#endif
		if (i == strlen(spword[j]) && strnicmp(*scr, spword[j], i) == 0) break;
	}

	/* 式チェック。*/
	if (j < sizeof(spword) / sizeof(spword[0])) {	/* 特殊語句。*/
		if (screxp2 != NULL && screxp2->type == SPWORD) return INVALID_COND_EXP;
		screxp1->type = SPWORD;
		screxp1->data = j;

		/* NUMとPASTSAMEとCNTとPASTSAMESEQは計算済みフラグを使用しない。*/
		if (j != 0 && j != 13 && j != 19 && j != 21 || g_spword[j].cnt < 1) {
			g_spword[j].cnt++;

			switch (j) {	/* 相互作用。*/
			/* BIG, SMALL */
			case 2: g_spword[3].cnt++; break;
			case 3: g_spword[2].cnt++; break;
			/* ODD, EVEN */
			case 4: g_spword[5].cnt++; break;
			case 5: g_spword[4].cnt++; break;
			/* GROUP1, GROUP2 */
			case 6: g_spword[7].cnt++; break;
			case 7: g_spword[6].cnt++; break;
			/* DIGIT1, DIGIT2 */
			case 9: g_spword[10].cnt++; break;
			case 10: g_spword[9].cnt++; break;
			/* GAP1, GAP2, HOT */
			case 15: g_spword[16].cnt++; g_spword[17].cnt++; break;
			case 16: g_spword[15].cnt++; g_spword[17].cnt++; break;
			case 17: g_spword[15].cnt++; g_spword[16].cnt++; break;
			}
		}
	} else if (isdigit(**scr) || **scr == '+' || **scr == '-') {	/* 数値。*/
		j = strtol(*scr, &chk, 10);
		if (*scr + i != chk) return INVALID_EXP;
		if (screxp2 != NULL && screxp2->type != SPWORD) return INVALID_COND_EXP;
		if (abs(j) >= 10000) return OUT_RANGE_NUMBER;
		screxp1->type = NUM;
		screxp1->data = j;
	} else if (isalpha(**scr) && i == 1) {	/* アルファベット1文字。*/
		if (screxp2 != NULL && screxp2->type != SPWORD) return INVALID_COND_EXP;
		screxp1->type = ALPHA;
		screxp1->data = **scr;
	} else	/* 何も当てはまらなかった。*/
		return INVALID_EXP;

	/* コンパイルの為の設定。*/
	screxp = screxp2 != NULL ? &g_scrcondexp[g_scrcondexp_cnt - 1].exp2 : &g_scrcondexp[g_scrcondexp_cnt - 1].exp1;
	*screxp = *screxp1;
	screxp->suffix[0] = -1;

	*scr += i;
	if (*scr >= len) return screxp2 != NULL && bracket == 0 ? SUCCESS : INVALID_SYNTAX;

	/* 添え字可能な特殊語句なら添え字チェック。コンパイルの為の設定もする。*/
	if (**scr == '[' && screxp1->type == SPWORD) {
		switch (j) {	/* 添え字の許容範囲設定。*/
		case 0:	/* NUM */
		case 6:	/* GROUP1 */
		case 8:	/* SET */
		case 14:	/* AVG */
		case 15:	/* GAP1 */
			r1 = 1; r2 = NUM_CNT; break;
		case 7:	/* GROUP2 */
			r1 = 1; r2 = 9; break;
		case 9:	/* DIGIT1 */
			r1 = 0; r2 = 9; break;
		case 10:	/* DIGIT2 */
			r1 = 0; r2 = NUM_MAX / 10; break;
		case 11:	/* SUB */
			r1 = 1; r2 = NUM_CNT - 1; break;
		case 13:	/* PASTSAME */
		case 21:	/* PASTSAMESEQ */
			r1 = 0; r2 = 99; break;
		case 16:	/* GAP2 */
			r1 = 0; r2 = 36; break;
		case 18:	/* PASTHIT */
			r1 = 0;
#if SCRTYPE == 1
			r2 = 3;
#elif SCRTYPE == 2
			r2 = 2;
#else
			r2 = NUM_CNT;
#endif
			break;
		case 19:	/* CNT */
			r1 = NUM_MIN; r2 = NUM_MAX; break;
		default:
			return INVALID_SYNTAX;
		}

		for (i = 0; ; ) {
			/* 添え字の始まりまで。*/
			(*scr)++;
			*scr += strspn(*scr, "\t ");
			if (*scr >= len) return INVALID_SYNTAX;

			/* 添え字。*/
			j = strtol(*scr, &chk, 10);
			if (chk == NULL || *scr == chk) return INVALID_SYNTAX;
			*scr = chk;
#if SCRTYPE >= 3
			if (screxp1->data == 18 && j == NUM_CNT - 1 &&
			tolower(**scr) == 'b') {
				screxp->suffix[i++] = NUM_CNT + 1;
				(*scr)++;
			}
#endif

			/* 添え字終了文字や添え字区切りの始まりまで。*/
			*scr += strspn(*scr, "\t ");
			if (*scr >= len) return INVALID_SYNTAX;

			/* 添え字終了文字や添え字区切り。*/
			if (**scr == ']') {
#if SCRTYPE >= 3
				if (screxp1->data != 18 || j != NUM_CNT - 1 || tolower(**scr) != 'b') {
#endif
					if (i >= NUM_MAX - NUM_MIN + 1 || j < r1 || j > r2) return OUT_RANGE_SUFFIX;
					screxp->suffix[i++] = j;
#if SCRTYPE >= 3
				}
#endif
				(*scr)++;
				break;
			} else if (screxp1->data != 19 || **scr != ',')	/* 複数添え字はCNTだけ。*/
				return INVALID_SYNTAX;
			if (i >= NUM_MAX - NUM_MIN + 1 || j < r1 || j > r2) return OUT_RANGE_SUFFIX;
			screxp->suffix[i++] = j;
		}
		screxp->suffix[i] = -1;
	}

	return CONTINUE;
}

/* 構文チェック。
左辺の式 条件演算子 右辺の式 [論理演算子 ...] の形式になっているかどうか。
開き括弧は左辺の式の前、閉じ括弧は右辺の式の後になっていて、正しい括弧かどうか。
コメントは次行まで構文チェックしないようにする。*/
sscode check_syntax(const char *scr)
{
	static char *condop[] = {"==", "=", "!=", "<>", "<", ">", "<=", ">="};
	char bracket, flag, j;
	const char *len;
	short i;
	sscode ssc, ssc2;
	SCREXP screxp1, screxp2;
	SCRCONDEXP *scrcondexp;

	if (scr == NULL) return SUCCESS;
	if ((i = strlen(scr)) > 2048) return TOO_LONG_SCRIPT;
	len = scr + i;
	bracket = 0;

	for (flag = 1; ; flag = 0) {
		/* 左辺の式の始まりまで。---------------------------- */
		for (;;) {	/* 開き括弧チェックとコメント。*/
			scr += strspn(scr, BLANKCHR);
			if (scr >= len) return flag && bracket == 0 ? SUCCESS : INVALID_SYNTAX;
			if (*scr == '(') {
				if (bracket >= 10) return TOO_MANY_BRACKET;	/* 開き括弧が多すぎる。*/
				bracket++;
				scr++;
			} else if (*scr == '#' || *scr == 39 || *scr == ';' ||
			*scr == '/' && scr + 1 < len && scr[1] == '/') {	/* コメントは次行まで飛ばす。*/
				scr += strcspn(scr, "\n\r");
				if (scr >= len) return flag && bracket == 0 ? SUCCESS : INVALID_SYNTAX;
			} else
				break;
		}

		/* コンパイルの為の設定。*/
		g_scrcondexp_cnt++;
		scrcondexp = realloc(g_scrcondexp, sizeof(SCRCONDEXP) * g_scrcondexp_cnt);
		if (scrcondexp == NULL) return TOO_LONG_SCRIPT;
		g_scrcondexp = scrcondexp;
		g_scrcondexp[g_scrcondexp_cnt - 1].logic = NONE;
		g_scrcondexp[g_scrcondexp_cnt - 1].bracket = bracket;

		/* 左辺の式。---------------------------------------- */
		if ((ssc = check_syntax_exp(&scr, len, &screxp1, NULL, bracket)) != CONTINUE) return ssc;

		/* 条件演算子の始まりまで。-------------------------- */
		scr += strspn(scr, BLANKCHR);
		if (scr >= len) return INVALID_SYNTAX;
		if (*scr == '(' || *scr == ')') return INVALID_BRACKET;	/* 不正な括弧。*/
		if (*scr == '#' || *scr == 39 || *scr == ';' ||
		*scr == '/' && scr + 1 < len && scr[1] == '/')
			return INVALID_SYNTAX;	/* コメントは不正。*/

		/* 条件演算子。-------------------------------------- */
		i = strspn(scr, SYMCHR);
		if (i < 1) return INVALID_SYNTAX;
		for (j = 0; j < sizeof(condop) / sizeof(condop[0]); j++)
			if (i == strlen(condop[j]) && strncmp(scr, condop[j], i) == 0) break;
		switch (j) {	/* コンパイルの為の設定。*/
		case 0:	/* == */
		case 1:	/* = */
			g_scrcondexp[g_scrcondexp_cnt - 1].cond = EQ; break;
		case 2:	/* != */
		case 3:	/* <> */
			g_scrcondexp[g_scrcondexp_cnt - 1].cond = NE; break;
		case 4:	/* < */
			g_scrcondexp[g_scrcondexp_cnt - 1].cond = LT; break;
		case 5:	/* > */
			g_scrcondexp[g_scrcondexp_cnt - 1].cond = GT; break;
		case 6:	/* <= */
			g_scrcondexp[g_scrcondexp_cnt - 1].cond = LE; break;
		case 7:	/* >= */
			g_scrcondexp[g_scrcondexp_cnt - 1].cond = GE; break;
		default:
			return INVALID_COND_OP;	/* 不正な値。*/
		}
		scr += i;
		if (scr >= len) return INVALID_SYNTAX;

		/* 右辺の式の始まりまで。---------------------------- */
		scr += strspn(scr, BLANKCHR);
		if (scr >= len) return INVALID_SYNTAX;
		if (*scr == '(' || *scr == ')') return INVALID_BRACKET;	/* 不正な括弧。*/
		if (*scr == '#' || *scr == 39 || *scr == ';' ||
		*scr == '/' && scr + 1 < len && scr[1] == '/')
			return INVALID_SYNTAX;	/* コメントは不正。*/

		/* 右辺の式。---------------------------------------- */
		ssc = check_syntax_exp(&scr, len, &screxp2, &screxp1, bracket);
		if (ssc == SUCCESS || ssc == CONTINUE) {
			if ((ssc2 = check_syntax_exp_cmb(&g_scrcondexp[g_scrcondexp_cnt - 1])) != CONTINUE)
				return ssc2;
			if (ssc == SUCCESS) return ssc;
		} else
			return ssc;

		/* 論理演算子の始まりまで。-------------------------- */
		for (;;) {	/* 閉じ括弧チェックとコメント。*/
			scr += strspn(scr, BLANKCHR);
			if (scr >= len) return bracket == 0 ? SUCCESS : INVALID_SYNTAX;
			if (*scr == ')') {
				if (bracket <= 0) return INVALID_CLOSE_BRACKET;	/* 不正な閉じ括弧。*/
				bracket--;
				scr++;
			} else if (*scr == '#' || *scr == 39 || *scr == ';' ||
			*scr == '/' && scr + 1 < len && scr[1] == '/') {	/* コメントは次行まで飛ばす。*/
				scr += strcspn(scr, "\n\r");
				if (scr >= len) return bracket == 0 ? SUCCESS : INVALID_SYNTAX;
			} else
				break;
		}
		g_scrcondexp[g_scrcondexp_cnt - 1].bracket = bracket;	/* コンパイルの為の括弧再設定。*/

		/* 論理演算子。-------------------------------------- */
		i = strspn(scr, SYMCHR);
		if (i != 2) return INVALID_SYNTAX;
		/* コンパイルの為の設定。*/
		if (strncmp(scr, "&&", i) == 0)
			g_scrcondexp[g_scrcondexp_cnt - 1].logic = AND;
		else if (strncmp(scr, "||", i) == 0)
			g_scrcondexp[g_scrcondexp_cnt - 1].logic = OR;
		else
			return INVALID_LOGIC_OP;	/* 不正な値。*/
		scr += i;
		if (scr >= len) return INVALID_SYNTAX;
	}
}

/* スクリプトの条件処理の部分をコンパイルする。*/
sscode compile_script_cond(SCRCONDEXP *scrcondexp, short *psize, short swpcode, char type, char index)
{
	char *code;
	short size = *psize, i;

	if (g_spword[scrcondexp->exp1.data].cnt < 2)
		switch (type) {
		case 0:	/* BYTE型。*/
			AALLOC(2);
			code[0] = 0x3c; code[1] = scrcondexp->exp2.data;	/* cmp al, data */
			break;
		case 1:	/* WORD型。*/
			AALLOC(4);
			code[0] = 0x66; code[1] = 0x3d;
			*((short *)&code[2]) = scrcondexp->exp2.data;	/* cmp ax, data */
			break;
		}
	else {
		switch (type) {
		case 0: /* BYTE型。*/
			/* mov result, al */
			if (g_spword[scrcondexp->exp1.data].result_radr < 128) {
				AALLOC(4);
				code[1] = 0x44; code[3] = g_spword[scrcondexp->exp1.data].result_radr;
			} else {
				AALLOC(7);
				code[1] = 0x84; *((int *)&code[3]) = g_spword[scrcondexp->exp1.data].result_radr;
			}
			code[0] = 0x88; code[2] = 0x24;
			break;
		case 1:	/* WORD型。*/
			/* mov result, ax */
			if (g_spword[scrcondexp->exp1.data].result_radr < 128) {
				AALLOC(5);
				code[2] = 0x44; code[4] = g_spword[scrcondexp->exp1.data].result_radr;
			} else {
				AALLOC(8);
				code[2] = 0x84; *((int *)&code[4]) = g_spword[scrcondexp->exp1.data].result_radr;
			}
			code[0] = 0x66; code[1] = 0x89; code[3] = 0x24;
		}

		/* mov flag, 1 */
		if (g_spword[scrcondexp->exp1.data].flag_radr < 128) {
			AALLOC(5);
			code[1] = 0x44; code[3] = g_spword[scrcondexp->exp1.data].flag_radr;
			code[4] = 0x01;
		} else {
			AALLOC(8);
			code[1] = 0x84; *((int *)&code[3]) = g_spword[scrcondexp->exp1.data].flag_radr;
			code[7] = 0x01;
		}
		code[0] = 0xc6; code[2] = 0x24;

		/* 計算済みフラグによるジャンプ処理のコンパイル。jne (cmp result, data) */
		i = size - swpcode;
		if (i < 128) {
			AALLOC(2);
			memmove(g_code + swpcode + 2, g_code + swpcode, i);
			g_code[swpcode] = 0x75; g_code[swpcode + 1] = i;
		} else {
			AALLOC(6);
			memmove(g_code + swpcode + 6, g_code + swpcode, i);
			g_code[swpcode] = 0x0f; g_code[swpcode + 1] = 0x85; *((int *)&g_code[swpcode + 2]) = i;
		}
	}

	switch (type) {
	case 0:	/* BYTE型。*/
		if (g_spword[scrcondexp->exp1.data].cnt < 2) break;
	case 2:	/* BYTE配列型。*/
		/* cmp [result + index], data */
		i = g_spword[scrcondexp->exp1.data].result_radr + index;
		if (i < 128) {
			AALLOC(5);
			code[1] = 0x7c; code[3] = i;
			code[4] = scrcondexp->exp2.data;
		} else {
			AALLOC(8);
			code[1] = 0xbc; *((int *)&code[3]) = i;
			code[7] = scrcondexp->exp2.data;
		}
		code[0] = 0x80; code[2] = 0x24;
		break;
	case 1:	/* WORD型。*/
		if (g_spword[scrcondexp->exp1.data].cnt < 2) break;
	case 3:	/* WORD配列型。*/
		/* cmp [result + index], data */
		i = g_spword[scrcondexp->exp1.data].result_radr + index;
		if (i < 128) {
			AALLOC(7);
			code[2] = 0x7c; code[4] = i;
			*((short *)&code[5]) = scrcondexp->exp2.data;
		} else {
			AALLOC(10);
			code[2] = 0xbc; *((int *)&code[4]) = i;
			*((short *)&code[8]) = scrcondexp->exp2.data;
		}
		code[0] = 0x66; code[1] = 0x81; code[3] = 0x24;
		break;
	default:
		return RESERVED;
	}

	*psize = size;
	return CONTINUE;
}

/* 構文チェック後、スクリプトをコンパイルする。正しく構文チェックできていない場合の動作は未定義。*/
sscode compile_script(const char *scr, short opt)
{
	char *code;
	short radr, size, cecode, swpcode, jsize, i, j;
	sscode ssc;

	g_scrcondexp = NULL;
	g_scrcondexp_cnt = 0;
	memset(g_spword, 0, sizeof(g_spword));

	/* 構文チェック。*/
	if ((ssc = check_syntax(scr)) != SUCCESS) goto procend;

	/* 格納場所の相対アドレスを設定。*/
	for (radr = i = 0; i < sizeof(g_spword) / sizeof(g_spword[0]); i++) {
		if (g_spword[i].cnt < 1) continue;

		switch (i) {
		case 0:	/* NUM */
		case 13:	/* PASTSAME */
		case 19:	/* CNT */
		case 21:	/* PASTSAMESEQ */
			break;
		case 1:	/* SUM */
			if (g_spword[1].cnt < 2) break;
			g_spword[1].result_radr = radr;
			radr += 2;
			g_spword[1].flag_radr = radr++;
			break;
		case 2:	/* BIG */
		case 4:	/* ODD */
			g_spword[i].result_radr = radr;
			radr += 4;
			if (g_spword[i].cnt >= 2) g_spword[i].flag_radr = radr++;
			break;
		case 3:	/* SMALL */
			g_spword[3].result_radr = g_spword[2].result_radr;
			g_spword[3].flag_radr = g_spword[2].flag_radr;
			break;
		case 5:	/* EVEN */
			g_spword[5].result_radr = g_spword[4].result_radr;
			g_spword[5].flag_radr = g_spword[4].flag_radr;
			break;
		case 6:	/* GROUP1 */
		case 8:	/* SET */
		case 9:	/* DIGIT1 */
			g_spword[i].result_radr = radr;
			radr += NUM_CNT + 11;
			if (g_spword[i].cnt >= 2) g_spword[i].flag_radr = radr++;
			break;
		case 7:	/* GROUP2 */
			g_spword[7].result_radr = g_spword[6].result_radr;
			g_spword[7].flag_radr = g_spword[6].flag_radr;
			break;
		case 10:	/* DIGIT2 */
			g_spword[10].result_radr = g_spword[9].result_radr;
			g_spword[10].flag_radr = g_spword[9].flag_radr;
			break;
		case 11:	/* SUB */
			g_spword[11].result_radr = radr;
			radr += NUM_CNT;
			if (g_spword[11].cnt >= 2) g_spword[11].flag_radr = radr++;
			break;
		case 12:	/* SEQ */
		case 20:	/* MARK */
			if (g_spword[i].cnt < 2) break;
			g_spword[i].result_radr = radr++;
			g_spword[i].flag_radr = radr++;
			break;
		case 14:	/* AVG */
			g_spword[14].result_radr = radr;
			radr += NUM_CNT + 1;
			if (g_spword[14].cnt >= 2) g_spword[14].flag_radr = radr++;
			break;
		case 15:	/* GAP1 */
			g_spword[15].result_radr = radr;
			radr += NUM_CNT + 42;
			if (g_spword[15].cnt >= 2) g_spword[15].flag_radr = radr++;
			break;
		case 16:	/* GAP2 */
		case 17:	/* HOT */
			g_spword[i].result_radr = g_spword[15].result_radr;
			g_spword[i].flag_radr = g_spword[15].flag_radr;
			break;
		case 18:	/* PASTHIT */
			g_spword[18].result_radr = radr;
			radr += PASTHIT_SIZE;
			if (g_spword[18].cnt >= 2) g_spword[18].flag_radr = radr++;
			break;
		default:
			ssc = RESERVED;
			goto procend;
		}
	}

	/* コンパイル。-------------------------------------- */

	size = 0;

	/* 自動変数確保処理のコンパイル。*/
	if (radr > 0) {
		/* sub esp, radr */
		if (radr < 128) {
			AALLOC(3);
			code[0] = 0x83; code[2] = radr;
		} else {
			AALLOC(6);
			code[0] = 0x81; *((int *)&code[2]) = radr;
		}
		code[1] = 0xec;
	}

	/* 計算済みフラグ初期化処理のコンパイル。*/
	for (i = 0; i < sizeof(g_spword) / sizeof(g_spword[0]); i++) {
		if (g_spword[i].cnt < 2 || i == 3 || i == 5 || i == 7 || i == 10 || i == 16 || i == 17) continue;

		/* mov flag, 0 */
		if (g_spword[i].flag_radr < 128) {
			AALLOC(5);
			code[1] = 0x44; code[3] = g_spword[i].flag_radr;
			code[4] = 0x00;
		} else {
			AALLOC(8);
			code[1] = 0x84; *((int *)&code[3]) = g_spword[i].flag_radr;
			code[7] = 0x00;
		}
		code[0] = 0xc6; code[2] = 0x24;
	}

	/* 条件式処理のコンパイル。次の条件式のジャンプ処理の為の設定も行う。*/
	for (i = 0; i < g_scrcondexp_cnt; i++) {
		if (g_scrcondexp[i].exp1.data == 3 || g_scrcondexp[i].exp1.data == 5) {
			/* SMALLはBIGの反対、EVENはODDの反対とする。*/
			g_scrcondexp[i].exp2.data = NUM_CNT - g_scrcondexp[i].exp2.data;
			switch (g_scrcondexp[i].cond) {
			case LT:
				g_scrcondexp[i].cond = GT; break;
			case GT:
				g_scrcondexp[i].cond = LT; break;
			case LE:
				g_scrcondexp[i].cond = GE; break;
			case GE:
				g_scrcondexp[i].cond = LE;
			}
		}

		cecode = size;	/* 条件式コードの開始位置設定。*/

		if (g_spword[g_scrcondexp[i].exp1.data].cnt >= 2) {
			/* cmp flag, 0 */
			if (g_spword[g_scrcondexp[i].exp1.data].flag_radr < 128) {
				AALLOC(5);
				code[1] = 0x7c; code[3] = g_spword[g_scrcondexp[i].exp1.data].flag_radr; code[4] = 0x00;
			} else {
				AALLOC(8);
				code[1] = 0xbc; *((int *)&code[3]) = g_spword[g_scrcondexp[i].exp1.data].flag_radr;
				code[7] = 0x00;
			}
			code[0] = 0x80; code[2] = 0x24;

			swpcode = size;	/* 特殊語句処理の開始位置設定。*/
		} else
			swpcode = 0;

		/* mov eax, numfunc */
		if (g_scrcondexp[i].exp1.data > 0) {	/* NUM以外。*/
			if (radr < 120) {
				AALLOC(4);
				code[1] = 0x44; code[3] = radr + 4;
			} else {
				AALLOC(7);
				code[1] = 0x84; *((int *)&code[3]) = radr + 4;
			}
			code[0] = 0x8b; code[2] = 0x24;
		}

		switch (g_scrcondexp[i].exp1.data) {
		case 0:	/* NUM */
			/* mov eax, num */
			if (radr + 8 < 128) {
				AALLOC(4);
				code[1] = 0x44;
				code[3] = radr + 8;
			} else {
				AALLOC(7);
				code[1] = 0x84;
				*((int *)&code[3]) = radr + 8;
			}
			code[0] = 0x8b; code[2] = 0x24;

			AALLOC(4);
			/* cmp [eax + index], data */
			code[0] = 0x80; code[1] = 0x78; code[2] = g_scrcondexp[i].exp1.suffix[0] - 1;
			code[3] = g_scrcondexp[i].exp2.data;
			break;
		case 1:	/* SUM */
			COMPILE_PUSH_NUM(8);
			AALLOC(5);
			code[0] = 0xff; code[1] = 0x10;	/* call [eax] */
			code[2] = 0x83; code[3] = 0xc4; code[4] = 0x04;	/* add esp, 4 */

			if ((ssc = compile_script_cond(&g_scrcondexp[i], &size, swpcode, 1, 0)) != CONTINUE)
				goto procend;
			break;
		case 2:	/* BIG */
		case 3:	/* SMALL */
		case 4:	/* ODD */
		case 5:	/* EVEN */
			AALLOC(5);
			code[0] = 0x68; *((int *)&code[1]) = opt;	/* push opt */
			COMPILE_PUSH_RESULT_ADR(2 + 4 * 1);
			COMPILE_PUSH_RESULT_ADR(0 + 4 * 2);
			COMPILE_PUSH_NUM(8 + 4 * 3);
			AALLOC(6);
			code[0] = 0xff; code[1] = 0x50;
			code[2] = g_scrcondexp[i].exp1.data < 4 ? 4 : 8;	/* call [eax + 4/8] */
			code[3] = 0x83; code[4] = 0xc4; code[5] = 0x10;	/* add esp, 16 */

			if ((ssc = compile_script_cond(&g_scrcondexp[i], &size, swpcode, 3, 0)) != CONTINUE)
				goto procend;
			break;
		case 6:	/* GROUP1 */
		case 7:	/* GROUP2 */
		case 8:	/* SET */
			AALLOC(2);
			code[0] = 0x6a; code[1] = g_scrcondexp[i].exp1.data < 8 ? 0 : 1;	/* push 0/1 */
			COMPILE_PUSH_RESULT_ADR(NUM_CNT + 1 + 4 * 1);
			COMPILE_PUSH_RESULT_ADR(0 + 4 * 2);
			COMPILE_PUSH_NUM(8 + 4 * 3);
			AALLOC(6);
			code[0] = 0xff; code[1] = 0x50; code[2] = 0x0c;	/* call [eax + 12] */
			code[3] = 0x83; code[4] = 0xc4; code[5] = 0x10;	/* add esp, 16 */

			if (g_scrcondexp[i].exp1.data == 7)
				j = NUM_CNT + 1 + (g_scrcondexp[i].exp1.suffix[0] < 0 ? 9 : g_scrcondexp[i].exp1.suffix[0] - 1);
			else
				if (g_scrcondexp[i].exp1.suffix[0] < 0) {
					j = NUM_CNT;
					g_scrcondexp[i].exp2.data = tolower(g_scrcondexp[i].exp2.data);
				} else {
					j = g_scrcondexp[i].exp1.suffix[0] - 1;
					g_scrcondexp[i].exp2.data = toupper(g_scrcondexp[i].exp2.data);
				}
			if ((ssc = compile_script_cond(&g_scrcondexp[i], &size, swpcode, 2, j)) != CONTINUE)
				goto procend;
			break;
		case 9:	/* DIGIT1 */
		case 10:	/* DIGIT2 */
			COMPILE_PUSH_RESULT_ADR(11);
			COMPILE_PUSH_RESULT_ADR(0 + 4 * 1);
			COMPILE_PUSH_NUM(8 + 4 * 2);
			AALLOC(6);
			code[0] = 0xff; code[1] = 0x50; code[2] = 0x10;	/* call [eax + 16] */
			code[3] = 0x83; code[4] = 0xc4; code[5] = 0x0c;	/* add esp, 12 */

			if (g_scrcondexp[i].exp1.data < 10)
				j = g_scrcondexp[i].exp1.suffix[0] < 0 ? 10 : g_scrcondexp[i].exp1.suffix[0];
			else
				j = 11 + (g_scrcondexp[i].exp1.suffix[0] < 0 ? NUM_MAX / 10 + 1 : g_scrcondexp[i].exp1.suffix[0]);
			if ((ssc = compile_script_cond(&g_scrcondexp[i], &size, swpcode, 2, j)) != CONTINUE)
				goto procend;
			break;
		case 11:	/* SUB */
			COMPILE_PUSH_RESULT_ADR(0);
			COMPILE_PUSH_NUM(8 + 4 * 1);
			AALLOC(6);
			code[0] = 0xff; code[1] = 0x50; code[2] = 0x14;	/* call [eax + 20] */
			code[3] = 0x83; code[4] = 0xc4; code[5] = 0x08;	/* add esp, 8 */

			if ((ssc = compile_script_cond(&g_scrcondexp[i], &size, swpcode, 2,
			g_scrcondexp[i].exp1.suffix[0] < 0 ? NUM_CNT - 1 : g_scrcondexp[i].exp1.suffix[0] - 1)) != CONTINUE)
				goto procend;
			break;
		case 12:	/* SEQ */
			COMPILE_PUSH_NUM(8);
			AALLOC(6);
			code[0] = 0xff; code[1] = 0x50; code[2] = 0x18;	/* call [eax + 24] */
			code[3] = 0x83; code[4] = 0xc4; code[5] = 0x04;	/* add esp, 4 */

			if ((ssc = compile_script_cond(&g_scrcondexp[i], &size, swpcode, 0, 0)) != CONTINUE)
				goto procend;
			break;
		case 13:	/* PASTSAME */
			j = opt;
			if (g_scrcondexp[i].exp1.suffix[0] > 0) j -= g_scrcondexp[i].exp1.suffix[0];
			AALLOC(5);
			code[0] = 0x68; *((int *)&code[1]) = j > 0 ? j : 0;	/* push j */
			COMPILE_PUSH_NUM(8 + 4 * 1);
			AALLOC(6);
			code[0] = 0xff; code[1] = 0x50; code[2] = 0x1c;	/* call [eax + 28] */
			code[3] = 0x83; code[4] = 0xc4; code[5] = 0x08;	/* add esp, 8 */

			if ((ssc = compile_script_cond(&g_scrcondexp[i], &size, swpcode, 0, 0)) != CONTINUE)
				goto procend;
			break;
		case 14:	/* AVG */
			AALLOC(5);
			code[0] = 0x68; *((int *)&code[1]) = opt;	/* push opt */
			COMPILE_PUSH_RESULT_ADR(0 + 4 * 1);
			COMPILE_PUSH_NUM(8 + 4 * 2);
			AALLOC(6);
			code[0] = 0xff; code[1] = 0x50; code[2] = 0x20;	/* call [eax + 32] */
			code[3] = 0x83; code[4] = 0xc4; code[5] = 0x0c;	/* add esp, 12 */

			if (g_scrcondexp[i].exp1.suffix[0] < 0)
				j = NUM_CNT;
			else {
				j = g_scrcondexp[i].exp1.suffix[0] - 1;
				g_scrcondexp[i].exp2.data = tolower(g_scrcondexp[i].exp2.data);
			}
			if ((ssc = compile_script_cond(&g_scrcondexp[i], &size, swpcode, 2, j)) != CONTINUE)
				goto procend;
			break;
		case 15:	/* GAP1 */
		case 16:	/* GAP2 */
		case 17:	/* HOT */
			AALLOC(5);
			code[0] = 0x68; *((int *)&code[1]) = opt;	/* push opt */
			COMPILE_PUSH_RESULT_ADR(NUM_CNT + 40 + 4 * 1);
			COMPILE_PUSH_RESULT_ADR(NUM_CNT + 38 + 4 * 2);
			COMPILE_PUSH_RESULT_ADR(NUM_CNT + 1 + 4 * 3);
			COMPILE_PUSH_RESULT_ADR(0 + 4 * 4);
			COMPILE_PUSH_NUM(8 + 4 * 5);
			AALLOC(6);
			code[0] = 0xff; code[1] = 0x50; code[2] = 0x24;	/* call [eax + 36] */
			code[3] = 0x83; code[4] = 0xc4; code[5] = 0x18;	/* add esp, 24 */

			if (g_scrcondexp[i].exp1.data == 15)
				if (g_scrcondexp[i].exp1.suffix[0] < 0) {
					jsize = 3;
					j = NUM_CNT + 38;
				} else {
					jsize = 2;
					j = g_scrcondexp[i].exp1.suffix[0] - 1;
				}
			else if (g_scrcondexp[i].exp1.data == 16)
				if (g_scrcondexp[i].exp1.suffix[0] < 0) {
					jsize = 3;
					j = NUM_CNT + 40;
				} else {
					jsize = 2;
					j = NUM_CNT + 1 + g_scrcondexp[i].exp1.suffix[0];
				}
			else {
				jsize = 2;
				j = NUM_CNT;
			}
			if ((ssc = compile_script_cond(&g_scrcondexp[i], &size, swpcode, jsize, j)) != CONTINUE)
				goto procend;
			break;
		case 18:	/* PASTHIT */
			AALLOC(5);
			code[0] = 0x68; *((int *)&code[1]) = opt;	/* push opt */
			COMPILE_PUSH_RESULT_ADR(0 + 4 * 1);
			COMPILE_PUSH_NUM(8 + 4 * 2);
			AALLOC(6);
			code[0] = 0xff; code[1] = 0x50; code[2] = 0x28;	/* call [eax + 40] */
			code[3] = 0x83; code[4] = 0xc4; code[5] = 0x0c;	/* add esp, 12 */

			if ((ssc = compile_script_cond(&g_scrcondexp[i], &size, swpcode, 3,
			g_scrcondexp[i].exp1.suffix[0] * 2)) != CONTINUE)
				goto procend;
			break;
		case 19:	/* CNT */
			AALLOC(2);
			code[0] = 0x6a; code[1] = 0x00;	/* push 0 */
			for (j = 0; g_scrcondexp[i].exp1.suffix[j] >= 0; j++) {
				AALLOC(2);
				code[0] = 0x6a; code[1] = g_scrcondexp[i].exp1.suffix[j];	/* push suffix */
			}
			COMPILE_PUSH_NUM(8 + 4 * (j + 1));

			AALLOC(3);
			code[0] = 0xff; code[1] = 0x50; code[2] = 0x2c;	/* call [eax + 44] */
			/* add esp, j */
			j = (j + 2) * 4;
			if (j < 128) {
				AALLOC(3);
				code[0] = 0x83; code[2] = j;
			} else {
				AALLOC(3);
				code[0] = 0x81; *((int *)&code[2]) = j;
			}
			code[1] = 0xc4;

			if ((ssc = compile_script_cond(&g_scrcondexp[i], &size, swpcode, 0, 0)) != CONTINUE)
				goto procend;
			break;
		case 20:	/* MARK */
			COMPILE_PUSH_NUM(8);
			AALLOC(6);
			code[0] = 0xff; code[1] = 0x50; code[2] = 0x30;	/* call [eax + 48] */
			code[3] = 0x83; code[4] = 0xc4; code[5] = 0x04;	/* add esp, 4 */

			if ((ssc = compile_script_cond(&g_scrcondexp[i], &size, swpcode, 0, 0)) != CONTINUE)
				goto procend;
			break;
		case 21:	/* PASTSAMESEQ */
			AALLOC(5);
			/* push opt/suffix */
			code[0] = 0x68; *((int *)&code[1]) = g_scrcondexp[i].exp1.suffix[0] > 0 ? g_scrcondexp[i].exp1.suffix[0] : opt;

			COMPILE_PUSH_NUM(8 + 4 * 1);
			AALLOC(6);
			code[0] = 0xff; code[1] = 0x50; code[2] = 0x34;	/* call [eax + 52] */
			code[3] = 0x83; code[4] = 0xc4; code[5] = 0x08;	/* add esp, 8 */

			if ((ssc = compile_script_cond(&g_scrcondexp[i], &size, swpcode, 0, 0)) != CONTINUE)
				goto procend;
			break;
		default:
			ssc = RESERVED;
			goto procend;
		}

		g_scrcondexp[i].jcode = size;	/* 条件ジャンプを設定する位置を設定。*/
		g_scrcondexp[i].size = size - cecode;	/* 条件式のコードサイズ設定。*/
	}

	/* 次の条件式のジャンプ処理のコンパイル。*/
	for (i = g_scrcondexp_cnt - 1; i >= 0; i--)
		if (i == g_scrcondexp_cnt - 1 || g_scrcondexp[i].logic == AND) {
			jsize = 0;
			for (j = i + 1; j < g_scrcondexp_cnt; j++) {
				jsize += g_scrcondexp[j].size;
				if (g_scrcondexp[j].logic == OR &&
				g_scrcondexp[j].bracket < g_scrcondexp[j - 1].bracket) break;
			}
			if (j >= g_scrcondexp_cnt) jsize += 4;

			/* j?? (次の条件式) */
			if (jsize < 128) {
				AALLOC(2);
				memmove(g_code + g_scrcondexp[i].jcode + 2, g_code + g_scrcondexp[i].jcode,
				size - 2 - g_scrcondexp[i].jcode);
				switch (g_scrcondexp[i].cond) {
				case EQ:	/* ne */
					g_code[g_scrcondexp[i].jcode] = 0x75; break;
				case NE:	/* e */
					g_code[g_scrcondexp[i].jcode] = 0x74; break;
				case LT:	/* ge */
					g_code[g_scrcondexp[i].jcode] = 0x7d; break;
				case GT:	/* le */
					g_code[g_scrcondexp[i].jcode] = 0x7e; break;
				case LE:	/* g */
					g_code[g_scrcondexp[i].jcode] = 0x7f; break;
				case GE:	/* l */
					g_code[g_scrcondexp[i].jcode] = 0x7c; break;
				default:
					ssc = RESERVED;
					goto procend;
				}
				g_code[g_scrcondexp[i].jcode + 1] = jsize;
				g_scrcondexp[i].size += 2;
			} else {
				AALLOC(6);
				memmove(g_code + g_scrcondexp[i].jcode + 6, g_code + g_scrcondexp[i].jcode,
				size - 6 - g_scrcondexp[i].jcode);
				switch (g_scrcondexp[i].cond) {
				case EQ:	/* ne */
					g_code[g_scrcondexp[i].jcode + 1] = 0x85; break;
				case NE:	/* e */
					g_code[g_scrcondexp[i].jcode + 1] = 0x84; break;
				case LT:	/* ge */
					g_code[g_scrcondexp[i].jcode + 1] = 0x8d; break;
				case GT:	/* le */
					g_code[g_scrcondexp[i].jcode + 1] = 0x8e; break;
				case LE:	/* g */
					g_code[g_scrcondexp[i].jcode + 1] = 0x8f; break;
				case GE:	/* l */
					g_code[g_scrcondexp[i].jcode + 1] = 0x8c; break;
				default:
					ssc = RESERVED;
					goto procend;
				}
				g_code[g_scrcondexp[i].jcode] = 0x0f;
				*((int *)&g_code[g_scrcondexp[i].jcode + 2]) = jsize;
				g_scrcondexp[i].size += 6;
			}
		} else if (g_scrcondexp[i].logic == OR) {
			jsize = 0;
			for (j = i + 1; j < g_scrcondexp_cnt; j++) {
				jsize += g_scrcondexp[j].size;
				if (g_scrcondexp[j].logic == AND &&
				g_scrcondexp[j].bracket < g_scrcondexp[j - 1].bracket) break;
			}

			/* j?? (次の条件式) */
			if (jsize < 128) {
				AALLOC(2);
				memmove(g_code + g_scrcondexp[i].jcode + 2, g_code + g_scrcondexp[i].jcode,
				size - 2 - g_scrcondexp[i].jcode);
				switch (g_scrcondexp[i].cond) {
				case EQ:	/* e */
					g_code[g_scrcondexp[i].jcode] = 0x74; break;
				case NE:	/* ne */
					g_code[g_scrcondexp[i].jcode] = 0x75; break;
				case LT:	/* l */
					g_code[g_scrcondexp[i].jcode] = 0x7c; break;
				case GT:	/* g */
					g_code[g_scrcondexp[i].jcode] = 0x7f; break;
				case LE:	/* le */
					g_code[g_scrcondexp[i].jcode] = 0x7e; break;
				case GE:	/* ge */
					g_code[g_scrcondexp[i].jcode] = 0x7d; break;
				default:
					ssc = RESERVED;
					goto procend;
				}
				g_code[g_scrcondexp[i].jcode + 1] = jsize;
				g_scrcondexp[i].size += 2;
			} else {
				AALLOC(6);
				memmove(g_code + g_scrcondexp[i].jcode + 6, g_code + g_scrcondexp[i].jcode,
				size - 6 - g_scrcondexp[i].jcode);
				switch (g_scrcondexp[i].cond) {
				case EQ:	/* e */
					g_code[g_scrcondexp[i].jcode + 1] = 0x84; break;
				case NE:	/* ne */
					g_code[g_scrcondexp[i].jcode + 1] = 0x85; break;
				case LT:	/* l */
					g_code[g_scrcondexp[i].jcode + 1] = 0x8c; break;
				case GT:	/* g */
					g_code[g_scrcondexp[i].jcode + 1] = 0x8f; break;
				case LE:	/* le */
					g_code[g_scrcondexp[i].jcode + 1] = 0x8e; break;
				case GE:	/* ge */
					g_code[g_scrcondexp[i].jcode + 1] = 0x8d; break;
				default:
					ssc = RESERVED;
					goto procend;
				}
				g_code[g_scrcondexp[i].jcode] = 0x0f;
				*((int *)&g_code[g_scrcondexp[i].jcode + 2]) = jsize;
				g_scrcondexp[i].size += 6;
			}
		}

	AALLOC(6);
	code[0] = 0xb0; code[1] = 0x01;	/* mov al, 1 */
	code[2] = 0xeb; code[3] = 0x02;	/* jmp 2 */
	code[4] = 0xb0; code[5] = 0x00;	/* mov al, 0 */

	if (radr > 0) {
		/* add esp, radr */
		if (radr < 128) {
			AALLOC(3);
			code[0] = 0x83; code[2] = radr;
		} else {
			AALLOC(6);
			code[0] = 0x81; *((int *)&code[2]) = radr;
		}
		code[1] = 0xc4;
	}

	AALLOC(1);
	code[0] = 0xc3;	/* ret */

	ssc = SUCCESS;

procend:
	free(g_scrcondexp);
	return ssc;
}

/* 各種設定読み込み。*/
char init_script(short opt)
{
	char buf[1024], flag, j, *scr, *sp;
	int i;
	FILE *f;
	sscode ssc;

	g_code = NULL;

	if ((f = fopen("scr.txt", "r")) == NULL) return 0;

	scr = NULL;
	flag = 0;

	/* 過去何回の当選数字を出力するかの値。*/
	if (fgets(buf, 1024, f) == NULL) goto procend;
	g_wnrange = atoi(buf);
	if (g_wnrange < 0 || opt - g_wnrange < 0) g_wnrange = opt;

	/* ランダム数字を何回出力するかの値。*/
	if (fgets(buf, 1024, f) == NULL) goto procend;
	g_rndcnt = atoi(buf);
	if (g_rndcnt < 0 || g_rndcnt > 30000) g_rndcnt = 0;

	/* フォーマットされた分析結果を出力するかの値。*/
	if (fgets(buf, 1024, f) == NULL) goto procend;
	g_arf = atoi(buf) != 0;

	/* 予想候補数字の出力方法の値。0:出力しない、1:全件出力、2:指定数字のみ全件出力、3:指定数字と特定条件一致のみ出力。*/
	if (fgets(buf, 1024, f) == NULL) goto procend;
	g_ec = atoi(buf);
	if (g_ec < 0 || g_ec > 3) goto procend;

	if (g_ec == 1)
		for (i = 0; i < (SCRTYPE <= 2 ? NUM_CNT : 1); i++) {
			for (j = NUM_MIN; j <= NUM_MAX; j++) g_expnum[i][j - NUM_MIN] = j;
			g_expnum[i][NUM_MAX - NUM_MIN + 1] = -1;
		}
	else if (g_ec >= 2) {
		for (i = 0; i < (SCRTYPE <= 2 ? NUM_CNT : 1); i++)
			if (!get_number(f, g_expnum[i], SCRTYPE <= 2 ? 1 : NUM_CNT, NUM_MAX - NUM_MIN + 1, NUM_MIN, NUM_MAX, 0, NUM_MAX - NUM_MIN + 1, 0))
				goto procend;

		if (g_ec == 3)
			/* 特定条件スクリプト取得。*/
			for (i = 0; fgets(buf, 1024, f) != NULL; ) {
				sp = realloc(scr, i + strlen(buf) + 1);
				if (sp == NULL) break;
				scr = sp;
				sp = scr + i;
				strcpy(sp, buf);
				if ((i = _msize(scr) - 1) >= 2048) break;
			}
	}

	flag = 1;

procend:
	fclose(f);

	if (flag && (ssc = compile_script(scr, opt)) != SUCCESS) {
		free(g_code);
		g_code = NULL;
		printf("%s\n", g_ssmsg[ssc]);
		g_ec = 2;
	}
	free(scr);
	return flag;
}

/* コンパイルされたスクリプトを実行する。*/
char exec_script(const char *num)
{
	static void *numfunc[] = {
#if SCRTYPE <= 2
		sum_number, big_number, odd_number, NULL, NULL, sub_number, seq_number, same_past_number,
		avg_number, hot_number, past_winning_number, NULL, NULL, same_past_seq_winning_number
#else
		sum_number, big_number, odd_number, group_number, same_number, sub_number, seq_number,
		same_past_number, avg_number, hot_number, past_winning_number, cnt_number, mark_number,
		same_past_seq_winning_number
#endif
	};

	_asm {
		sub esp, 8

		; num
		mov eax, num
		mov [esp + 4], eax
		; numfunc
		lea eax, numfunc
		mov [esp], eax

		call g_code

		add esp, 8
	}
}

/* 各種設定による条件に一致した数字を書き込む。*/
int write_ec(short opt, FILE *f1, FILE *f2)
{
	char num[NUM_CNT], cnt, i, j, k, l, m, n;
	int ncnt;

	if (g_ec == 0) return 0;

	ncnt = 0;
#if SCRTYPE == 1
	for (i = 0; g_expnum[0][i] >= 0; i++) {
		num[0] = g_expnum[0][i];
		printf("\b\b\b\b\b%d    ", num[0]);
		for (j = 0; g_expnum[1][j] >= 0; j++) {
			num[1] = g_expnum[1][j];
			printf("\b\b\b%d  ", num[1]);
			for (k = 0; g_expnum[2][k] >= 0; k++) {
				num[2] = g_expnum[2][k];
				printf("\b%d", num[2]);
				if (g_ec == 3 && !exec_script(num)) continue;
				write_data(num, f1, f2, -2);
				ncnt++;
			}
		}
	}
#elif SCRTYPE == 2
	for (i = 0; g_expnum[0][i] >= 0; i++) {
		num[0] = g_expnum[0][i];
		printf("\b\b\b\b\b\b\b%d      ", num[0]);
		for (j = 0; g_expnum[1][j] >= 0; j++) {
			num[1] = g_expnum[1][j];
			printf("\b\b\b\b\b%d    ", num[1]);
			for (k = 0; g_expnum[2][k] >= 0; k++) {
				num[2] = g_expnum[2][k];
				printf("\b\b\b%d  ", num[2]);
				for (l = 0; g_expnum[3][l] >= 0; l++) {
					num[3] = g_expnum[3][l];
					printf("\b%d", num[3]);
					if (g_ec == 3 && !exec_script(num)) continue;
					write_data(num, f1, f2, -2);
					ncnt++;
				}
			}
		}
	}
#elif SCRTYPE == 3
	for (cnt = 0; g_expnum[0][cnt] >= 0; cnt++);
	for (i = 0; i < cnt - 4; i++) {
		num[0] = g_expnum[0][i];
		printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b%2d            ", num[0]);
		for (j = i + 1; j < cnt - 3; j++) {
			num[1] = g_expnum[0][j];
			printf("\b\b\b\b\b\b\b\b\b\b\b%2d         ", num[1]);
			for (k = j + 1; k < cnt - 2; k++) {
				num[2] = g_expnum[0][k];
				printf("\b\b\b\b\b\b\b\b%2d      ", num[2]);
				for (l = k + 1; l < cnt - 1; l++) {
					num[3] = g_expnum[0][l];
					printf("\b\b\b\b\b%2d   ", num[3]);
					for (m = l + 1; m < cnt; m++) {
						num[4] = g_expnum[0][m];
						printf("\b\b%2d", num[4]);
						if (g_ec == 3 && !exec_script(num)) continue;
						write_data(num, f1, f2, -2);
						ncnt++;
					}
				}
			}
		}
	}
#else
	for (cnt = 0; g_expnum[0][cnt] >= 0; cnt++);
	for (i = 0; i < cnt - 5; i++) {
		num[0] = g_expnum[0][i];
		printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%2d               ", num[0]);
		for (j = i + 1; j < cnt - 4; j++) {
			num[1] = g_expnum[0][j];
			printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b%2d            ", num[1]);
			for (k = j + 1; k < cnt - 3; k++) {
				num[2] = g_expnum[0][k];
				printf("\b\b\b\b\b\b\b\b\b\b\b%2d         ", num[2]);
				for (l = k + 1; l < cnt - 2; l++) {
					num[3] = g_expnum[0][l];
					printf("\b\b\b\b\b\b\b\b%2d      ", num[3]);
					for (m = l + 1; m < cnt - 1; m++) {
						num[4] = g_expnum[0][m];
						printf("\b\b\b\b\b%2d   ", num[4]);
						for (n = m + 1; n < cnt; n++) {
							num[5] = g_expnum[0][n];
							printf("\b\b%2d", num[5]);
							if (g_ec == 3 && !exec_script(num)) continue;
							write_data(num, f1, f2, -2);
							ncnt++;
						}
					}
				}
			}
		}
	}
#endif

	free(g_code);
	return ncnt;
}
