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

/* �\���̊e���؂蕶���B*/
#define WORDCHR "+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"
#define SYMCHR "!\"$%&*,.:<=>?@[\\]^`{|}~"
#define BLANKCHR "\t\n\v\f\r "
/* ���I�̈�ǉ��m�ۃ}�N���B*/
#define AALLOC(add) code = realloc(g_code, size + add); \
	if (code == NULL) return TOO_LONG_SCRIPT; \
	g_code = code; \
	code = g_code + size; \
	size = _msize(g_code);
/* push num�}�N���B*/
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
/* lea edx, [result + adr]�Apush edx�}�N���B*/
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

/* �\���`�F�b�N�R�[�h�ƑΉ����镶����B*/
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
	"�G���[�͂���܂���",
	"�\���`�F�b�N�͊������Ă��܂���",
	"�\�����������܂�",
	"�J�����ʂ��������܂�",
	"���l�����e�͈͊O�ł�",
	"�������e�͈͊O�ł�",
	"�Y���������e�͈͊O�ł�",
	"�Y�������K�v�ł�",
	"�\���G���[�ł�",
	"���������s���ł�",
	"�����s���ł�",
	"�������Z�q���s���ł�",
	"�_�����Z�q���s���ł�",
	"���ʂ��s���ł�",
	"�����ʂ��s���ł�",
	"�\��ς�"
};

typedef struct {	/* ���f�[�^�B*/
	enum {SPWORD, NUM, ALPHA} type;
	short data;
	char suffix[NUM_MAX - NUM_MIN + 2];	/* ������̏ꍇ�̂ݎg�p����Y�����B*/
} SCREXP;
typedef struct {	/* �������f�[�^�B*/
	SCREXP exp1, exp2;
	enum {EQ, NE, LT, GT, LE, GE} cond;
	enum {NONE, AND, OR} logic;
	char bracket;
	short jcode, size;	/* �����W�����v��ݒ肷��ʒu�A���s�R�[�h�T�C�Y�B*/
} SCRCONDEXP;
struct {	/* ������f�[�^�B*/
	short cnt;	/* �g�p����������̐��B�G���[���̐��͖���`�B*/
	short flag_radr;	/* �v�Z�ς݃t���O�擪���Ίi�[�ꏊ�B*/
	short result_radr;	/* �v�Z���ʐ擪���Ίi�[�ꏊ�B*/
} g_spword[21];
char g_arf, g_ec, g_expnum[SCRTYPE <= 2 ? NUM_CNT : 1][NUM_MAX - NUM_MIN + 2], *g_code;
short g_scrcondexp_cnt, g_wnrange, g_rndcnt;
SCRCONDEXP *g_scrcondexp;

/* ���̑g�ݍ��킹�̍\���`�F�b�N�B
������ɍ��������l��A���t�@�x�b�g�ɂȂ��Ă��邩�Bexp1�������Aexp2�𐔒l��A���t�@�x�b�g�ɂ���B*/
sscode check_syntax_exp_cmb(SCRCONDEXP *scrcondexp)
{
	short i;
	SCREXP screxp;

	/* exp1�������Aexp2�𐔒l��A���t�@�x�b�g�ɂ���B*/
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

	/* ������ɍ��������l��A���t�@�x�b�g���`�F�b�N�B-- */
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

/* ���̍\���`�F�b�N�B
������A���l�A�A���t�@�x�b�g�����������ǂ����B
screxp2���ݒ肳��Ă���΁A������ɍ��������l��A���t�@�x�b�g�ɂȂ��Ă��邩���`�F�b�N���܂��B
�ꕔ�̓�����ɓY����������Ȃ�A�������Y�����œ�����ɍ������Y�����ɂȂ��Ă��邩�B
��screxp1�Ƀ`�F�b�N���鎮�Ascrexp2�Ɋ��Ƀ`�F�b�N���������w�肵�܂��B
  �܂�A���ӂ̎����`�F�b�N����ꍇ��screxp2��NULL�A
  �E�ӂ̎����`�F�b�N����ꍇ��screxp2�ɁA���ӂ̎����`�F�b�N�����Ƃ���screxp���w�肵�܂��B*/
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

	/* ���`�F�b�N�B*/
	if (j < sizeof(spword) / sizeof(spword[0])) {	/* ������B*/
		if (screxp2 != NULL && screxp2->type == SPWORD) return INVALID_COND_EXP;
		screxp1->type = SPWORD;
		screxp1->data = j;

		/* NUM��PASTSAME��CNT��PASTSAMESEQ�͌v�Z�ς݃t���O���g�p���Ȃ��B*/
		if (j != 0 && j != 13 && j != 19 && j != 21 || g_spword[j].cnt < 1) {
			g_spword[j].cnt++;

			switch (j) {	/* ���ݍ�p�B*/
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
	} else if (isdigit(**scr) || **scr == '+' || **scr == '-') {	/* ���l�B*/
		j = strtol(*scr, &chk, 10);
		if (*scr + i != chk) return INVALID_EXP;
		if (screxp2 != NULL && screxp2->type != SPWORD) return INVALID_COND_EXP;
		if (abs(j) >= 10000) return OUT_RANGE_NUMBER;
		screxp1->type = NUM;
		screxp1->data = j;
	} else if (isalpha(**scr) && i == 1) {	/* �A���t�@�x�b�g1�����B*/
		if (screxp2 != NULL && screxp2->type != SPWORD) return INVALID_COND_EXP;
		screxp1->type = ALPHA;
		screxp1->data = **scr;
	} else	/* �������Ă͂܂�Ȃ������B*/
		return INVALID_EXP;

	/* �R���p�C���ׂ̈̐ݒ�B*/
	screxp = screxp2 != NULL ? &g_scrcondexp[g_scrcondexp_cnt - 1].exp2 : &g_scrcondexp[g_scrcondexp_cnt - 1].exp1;
	*screxp = *screxp1;
	screxp->suffix[0] = -1;

	*scr += i;
	if (*scr >= len) return screxp2 != NULL && bracket == 0 ? SUCCESS : INVALID_SYNTAX;

	/* �Y�����\�ȓ�����Ȃ�Y�����`�F�b�N�B�R���p�C���ׂ̈̐ݒ������B*/
	if (**scr == '[' && screxp1->type == SPWORD) {
		switch (j) {	/* �Y�����̋��e�͈͐ݒ�B*/
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
			/* �Y�����̎n�܂�܂ŁB*/
			(*scr)++;
			*scr += strspn(*scr, "\t ");
			if (*scr >= len) return INVALID_SYNTAX;

			/* �Y�����B*/
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

			/* �Y�����I��������Y������؂�̎n�܂�܂ŁB*/
			*scr += strspn(*scr, "\t ");
			if (*scr >= len) return INVALID_SYNTAX;

			/* �Y�����I��������Y������؂�B*/
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
			} else if (screxp1->data != 19 || **scr != ',')	/* �����Y������CNT�����B*/
				return INVALID_SYNTAX;
			if (i >= NUM_MAX - NUM_MIN + 1 || j < r1 || j > r2) return OUT_RANGE_SUFFIX;
			screxp->suffix[i++] = j;
		}
		screxp->suffix[i] = -1;
	}

	return CONTINUE;
}

/* �\���`�F�b�N�B
���ӂ̎� �������Z�q �E�ӂ̎� [�_�����Z�q ...] �̌`���ɂȂ��Ă��邩�ǂ����B
�J�����ʂ͍��ӂ̎��̑O�A�����ʂ͉E�ӂ̎��̌�ɂȂ��Ă��āA���������ʂ��ǂ����B
�R�����g�͎��s�܂ō\���`�F�b�N���Ȃ��悤�ɂ���B*/
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
		/* ���ӂ̎��̎n�܂�܂ŁB---------------------------- */
		for (;;) {	/* �J�����ʃ`�F�b�N�ƃR�����g�B*/
			scr += strspn(scr, BLANKCHR);
			if (scr >= len) return flag && bracket == 0 ? SUCCESS : INVALID_SYNTAX;
			if (*scr == '(') {
				if (bracket >= 10) return TOO_MANY_BRACKET;	/* �J�����ʂ���������B*/
				bracket++;
				scr++;
			} else if (*scr == '#' || *scr == 39 || *scr == ';' ||
			*scr == '/' && scr + 1 < len && scr[1] == '/') {	/* �R�����g�͎��s�܂Ŕ�΂��B*/
				scr += strcspn(scr, "\n\r");
				if (scr >= len) return flag && bracket == 0 ? SUCCESS : INVALID_SYNTAX;
			} else
				break;
		}

		/* �R���p�C���ׂ̈̐ݒ�B*/
		g_scrcondexp_cnt++;
		scrcondexp = realloc(g_scrcondexp, sizeof(SCRCONDEXP) * g_scrcondexp_cnt);
		if (scrcondexp == NULL) return TOO_LONG_SCRIPT;
		g_scrcondexp = scrcondexp;
		g_scrcondexp[g_scrcondexp_cnt - 1].logic = NONE;
		g_scrcondexp[g_scrcondexp_cnt - 1].bracket = bracket;

		/* ���ӂ̎��B---------------------------------------- */
		if ((ssc = check_syntax_exp(&scr, len, &screxp1, NULL, bracket)) != CONTINUE) return ssc;

		/* �������Z�q�̎n�܂�܂ŁB-------------------------- */
		scr += strspn(scr, BLANKCHR);
		if (scr >= len) return INVALID_SYNTAX;
		if (*scr == '(' || *scr == ')') return INVALID_BRACKET;	/* �s���Ȋ��ʁB*/
		if (*scr == '#' || *scr == 39 || *scr == ';' ||
		*scr == '/' && scr + 1 < len && scr[1] == '/')
			return INVALID_SYNTAX;	/* �R�����g�͕s���B*/

		/* �������Z�q�B-------------------------------------- */
		i = strspn(scr, SYMCHR);
		if (i < 1) return INVALID_SYNTAX;
		for (j = 0; j < sizeof(condop) / sizeof(condop[0]); j++)
			if (i == strlen(condop[j]) && strncmp(scr, condop[j], i) == 0) break;
		switch (j) {	/* �R���p�C���ׂ̈̐ݒ�B*/
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
			return INVALID_COND_OP;	/* �s���Ȓl�B*/
		}
		scr += i;
		if (scr >= len) return INVALID_SYNTAX;

		/* �E�ӂ̎��̎n�܂�܂ŁB---------------------------- */
		scr += strspn(scr, BLANKCHR);
		if (scr >= len) return INVALID_SYNTAX;
		if (*scr == '(' || *scr == ')') return INVALID_BRACKET;	/* �s���Ȋ��ʁB*/
		if (*scr == '#' || *scr == 39 || *scr == ';' ||
		*scr == '/' && scr + 1 < len && scr[1] == '/')
			return INVALID_SYNTAX;	/* �R�����g�͕s���B*/

		/* �E�ӂ̎��B---------------------------------------- */
		ssc = check_syntax_exp(&scr, len, &screxp2, &screxp1, bracket);
		if (ssc == SUCCESS || ssc == CONTINUE) {
			if ((ssc2 = check_syntax_exp_cmb(&g_scrcondexp[g_scrcondexp_cnt - 1])) != CONTINUE)
				return ssc2;
			if (ssc == SUCCESS) return ssc;
		} else
			return ssc;

		/* �_�����Z�q�̎n�܂�܂ŁB-------------------------- */
		for (;;) {	/* �����ʃ`�F�b�N�ƃR�����g�B*/
			scr += strspn(scr, BLANKCHR);
			if (scr >= len) return bracket == 0 ? SUCCESS : INVALID_SYNTAX;
			if (*scr == ')') {
				if (bracket <= 0) return INVALID_CLOSE_BRACKET;	/* �s���ȕ����ʁB*/
				bracket--;
				scr++;
			} else if (*scr == '#' || *scr == 39 || *scr == ';' ||
			*scr == '/' && scr + 1 < len && scr[1] == '/') {	/* �R�����g�͎��s�܂Ŕ�΂��B*/
				scr += strcspn(scr, "\n\r");
				if (scr >= len) return bracket == 0 ? SUCCESS : INVALID_SYNTAX;
			} else
				break;
		}
		g_scrcondexp[g_scrcondexp_cnt - 1].bracket = bracket;	/* �R���p�C���ׂ̈̊��ʍĐݒ�B*/

		/* �_�����Z�q�B-------------------------------------- */
		i = strspn(scr, SYMCHR);
		if (i != 2) return INVALID_SYNTAX;
		/* �R���p�C���ׂ̈̐ݒ�B*/
		if (strncmp(scr, "&&", i) == 0)
			g_scrcondexp[g_scrcondexp_cnt - 1].logic = AND;
		else if (strncmp(scr, "||", i) == 0)
			g_scrcondexp[g_scrcondexp_cnt - 1].logic = OR;
		else
			return INVALID_LOGIC_OP;	/* �s���Ȓl�B*/
		scr += i;
		if (scr >= len) return INVALID_SYNTAX;
	}
}

/* �X�N���v�g�̏��������̕������R���p�C������B*/
sscode compile_script_cond(SCRCONDEXP *scrcondexp, short *psize, short swpcode, char type, char index)
{
	char *code;
	short size = *psize, i;

	if (g_spword[scrcondexp->exp1.data].cnt < 2)
		switch (type) {
		case 0:	/* BYTE�^�B*/
			AALLOC(2);
			code[0] = 0x3c; code[1] = scrcondexp->exp2.data;	/* cmp al, data */
			break;
		case 1:	/* WORD�^�B*/
			AALLOC(4);
			code[0] = 0x66; code[1] = 0x3d;
			*((short *)&code[2]) = scrcondexp->exp2.data;	/* cmp ax, data */
			break;
		}
	else {
		switch (type) {
		case 0: /* BYTE�^�B*/
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
		case 1:	/* WORD�^�B*/
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

		/* �v�Z�ς݃t���O�ɂ��W�����v�����̃R���p�C���Bjne (cmp result, data) */
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
	case 0:	/* BYTE�^�B*/
		if (g_spword[scrcondexp->exp1.data].cnt < 2) break;
	case 2:	/* BYTE�z��^�B*/
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
	case 1:	/* WORD�^�B*/
		if (g_spword[scrcondexp->exp1.data].cnt < 2) break;
	case 3:	/* WORD�z��^�B*/
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

/* �\���`�F�b�N��A�X�N���v�g���R���p�C������B�������\���`�F�b�N�ł��Ă��Ȃ��ꍇ�̓���͖���`�B*/
sscode compile_script(const char *scr, short opt)
{
	char *code;
	short radr, size, cecode, swpcode, jsize, i, j;
	sscode ssc;

	g_scrcondexp = NULL;
	g_scrcondexp_cnt = 0;
	memset(g_spword, 0, sizeof(g_spword));

	/* �\���`�F�b�N�B*/
	if ((ssc = check_syntax(scr)) != SUCCESS) goto procend;

	/* �i�[�ꏊ�̑��΃A�h���X��ݒ�B*/
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

	/* �R���p�C���B-------------------------------------- */

	size = 0;

	/* �����ϐ��m�ۏ����̃R���p�C���B*/
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

	/* �v�Z�ς݃t���O�����������̃R���p�C���B*/
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

	/* �����������̃R���p�C���B���̏������̃W�����v�����ׂ̈̐ݒ���s���B*/
	for (i = 0; i < g_scrcondexp_cnt; i++) {
		if (g_scrcondexp[i].exp1.data == 3 || g_scrcondexp[i].exp1.data == 5) {
			/* SMALL��BIG�̔��΁AEVEN��ODD�̔��΂Ƃ���B*/
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

		cecode = size;	/* �������R�[�h�̊J�n�ʒu�ݒ�B*/

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

			swpcode = size;	/* �����又���̊J�n�ʒu�ݒ�B*/
		} else
			swpcode = 0;

		/* mov eax, numfunc */
		if (g_scrcondexp[i].exp1.data > 0) {	/* NUM�ȊO�B*/
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

		g_scrcondexp[i].jcode = size;	/* �����W�����v��ݒ肷��ʒu��ݒ�B*/
		g_scrcondexp[i].size = size - cecode;	/* �������̃R�[�h�T�C�Y�ݒ�B*/
	}

	/* ���̏������̃W�����v�����̃R���p�C���B*/
	for (i = g_scrcondexp_cnt - 1; i >= 0; i--)
		if (i == g_scrcondexp_cnt - 1 || g_scrcondexp[i].logic == AND) {
			jsize = 0;
			for (j = i + 1; j < g_scrcondexp_cnt; j++) {
				jsize += g_scrcondexp[j].size;
				if (g_scrcondexp[j].logic == OR &&
				g_scrcondexp[j].bracket < g_scrcondexp[j - 1].bracket) break;
			}
			if (j >= g_scrcondexp_cnt) jsize += 4;

			/* j?? (���̏�����) */
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

			/* j?? (���̏�����) */
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

/* �e��ݒ�ǂݍ��݁B*/
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

	/* �ߋ�����̓��I�������o�͂��邩�̒l�B*/
	if (fgets(buf, 1024, f) == NULL) goto procend;
	g_wnrange = atoi(buf);
	if (g_wnrange < 0 || opt - g_wnrange < 0) g_wnrange = opt;

	/* �����_������������o�͂��邩�̒l�B*/
	if (fgets(buf, 1024, f) == NULL) goto procend;
	g_rndcnt = atoi(buf);
	if (g_rndcnt < 0 || g_rndcnt > 30000) g_rndcnt = 0;

	/* �t�H�[�}�b�g���ꂽ���͌��ʂ��o�͂��邩�̒l�B*/
	if (fgets(buf, 1024, f) == NULL) goto procend;
	g_arf = atoi(buf) != 0;

	/* �\�z��␔���̏o�͕��@�̒l�B0:�o�͂��Ȃ��A1:�S���o�́A2:�w�萔���̂ݑS���o�́A3:�w�萔���Ɠ��������v�̂ݏo�́B*/
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
			/* ��������X�N���v�g�擾�B*/
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

/* �R���p�C�����ꂽ�X�N���v�g�����s����B*/
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

/* �e��ݒ�ɂ������Ɉ�v�����������������ށB*/
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
