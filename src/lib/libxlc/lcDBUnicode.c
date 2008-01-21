/* $XConsortium: lcDB.c /main/9 1995/12/01 11:53:25 kaleb $ */
/*
 * Copyright (c) 1996, 1998, 1999, 2001 by Sun Microsystems, Inc.
 * All rights reserved.
 */

#pragma	ident	"@(#)lcDBUnicode.c	1.7 01/07/16 SMI"

/*
 *
 * Copyright IBM Corporation 1993
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

#ifndef	NOT_X_ENV

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xlibint.h>
#include <X11/XlcPubI.h>

#else	/* NOT_X_ENV */

#define	Xmalloc	malloc
#define	Xrealloc	realloc
#define	Xfree	free

#endif	/* NOT_X_ENV */

/* specifying NOT_X_ENV allows users to just use
   the database parsing routine. */

#ifndef	BUFSIZE
#define	BUFSIZE	2048
#endif

#include <X11/Xlocale.h>
#define WC_TO_CS_CONV_TABLE_ENTRY	"wc_conversion_table"
#define CS_TO_WC_CONV_FILE_ENTRY	"cs_conversion_file"
#define CS_TO_WC_CONV_TABLE_ENTRY	"cs_conversion_table"
#define	XLOCALE_USER_DIR	".Xlocale"
#define	XLOCALE_USER_NAME	"required.charsets"
#define	XLOCALE_USER_CATEGORY	"XLC_XLOCALE_USER"

#include <stdio.h>
#include <unistd.h>

typedef struct _DatabaseRec {
    char *category;
    char *name;
    char **value;
    int value_num;
    struct _DatabaseRec *next;
} DatabaseRec, *Database;

typedef enum {
    S_NULL,	/* outside category */
    S_CATEGORY,	/* inside category */
    S_NAME,	/* has name, expecting values */
    S_VALUE
} ParseState;

typedef enum {
    T_NEWLINE,
    T_COMMENT,
    T_SEMICOLON,
    T_DOUBLE_QUOTE,
    T_LEFT_BRACE,
    T_RIGHT_BRACE,
    T_SPACE,
    T_TAB,
    T_BACKSLASH,
    T_NUMERIC_HEX,
    T_NUMERIC_DEC,
    T_NUMERIC_OCT,
    T_DEFAULT
} Token;

typedef struct {
    Token token;	/* token id */
    char *name;		/* token sequence */
    int len;		/* length of token sequence */
    int (*parse_proc)(); /* parsing procedure */
} TokenTable;

static int f_newline();
static int f_comment();
static int f_semicolon();
static int f_double_quote();
static int f_left_brace();
static int f_right_brace();
static int f_white();
static int f_backslash();
static int f_numeric();
static int f_default();

static TokenTable token_tbl[] = {
    { T_NEWLINE,	"\n",	1,	f_newline },
    { T_COMMENT,	"#",	1,	f_comment },
    { T_SEMICOLON,	";",	1,	f_semicolon },
    { T_DOUBLE_QUOTE,	"\"",	1,	f_double_quote },
    { T_LEFT_BRACE,	"{",	1,	f_left_brace },
    { T_RIGHT_BRACE,	"}",	1,	f_right_brace },
    { T_SPACE,		" ",	1,	f_white },
    { T_TAB,		"\t",	1,	f_white },
    { T_BACKSLASH,	"\\",	1,	f_backslash },
    { T_NUMERIC_HEX,	"\\x",	2,	f_numeric },
    { T_NUMERIC_DEC,	"\\d",	2,	f_numeric },
    { T_NUMERIC_OCT,	"\\o",	2,	f_numeric },
    { T_DEFAULT,	" ",	1,	f_default },	/* any character */
    0 
};

#define	SYM_NEWLINE	'\n'
#define	SYM_COMMENT	'#'
#define	SYM_SEMICOLON	';'
#define	SYM_DOUBLE_QUOTE	'"'
#define	SYM_LEFT_BRACE	'{'
#define	SYM_RIGHT_BRACE	'}'
#define	SYM_SPACE	' '
#define	SYM_TAB		'\t'
#define	SYM_BACKSLASH	'\\'

/************************************************************************/

#define MAX_NAME_NEST	64

typedef struct {
    ParseState pre_state;
    char *category;
    char *name[MAX_NAME_NEST];
    int nest_depth;
    char **value;
    int value_len;
    int value_num;
    char buf[BUFSIZE];
    int bufsize;
} DBParseInfo;

static DBParseInfo parse_info;

static void
clear_parse_info()
{
    int i;
    parse_info.pre_state = S_NULL;
    if(parse_info.category != NULL){
	Xfree(parse_info.category);
    }
    for(i = 0; i <= parse_info.nest_depth; ++i){
	if(parse_info.name[i]){
	    Xfree(parse_info.name[i]);
	}
    }
    if(parse_info.value){
	if(*parse_info.value){
	    Xfree(*parse_info.value);
	}
	Xfree((char *)parse_info.value);
    }
    memset((void *)&parse_info, 0, sizeof(DBParseInfo));
}

/************************************************************************/
typedef struct _Line {
    char *str;
    int cursize;
    int maxsize;
    int seq;
} Line;

static void
free_line(line)
    Line *line;
{
    if(line->str != NULL){
	Xfree(line->str);
    }
    memset((void *)line, 0, sizeof(Line));
}

static int
realloc_line(line, size)
    Line *line;
    int size;
{
    char *str = line->str;

    if(str != NULL){
	str = (char *)Xrealloc(str, size);
    }else{
	str = (char *)Xmalloc(size);
    }
    if(str == NULL){
	/* malloc error */
	memset((void *)line, 0, sizeof(Line));
	return 0;
    }
    line->str = str;
    line->maxsize = size;
    return 1;
}

#define	ISWHITE(ch)	((ch) == SYM_SPACE   || (ch) == SYM_TAB)

static void
zap_comment(str, quoted)
    char *str;
    int *quoted;
{
    char *p = str;
#ifdef	never
    *quoted = 0;
    if(*p == SYM_COMMENT){
	int len = strlen(str);
	if(p[len - 1] == SYM_NEWLINE){
	    *p++ = SYM_NEWLINE;
	}
	*p = '\0';
    }
#else
    while(*p){
	if(*p == SYM_DOUBLE_QUOTE){
	    if(p == str || p[-1] != SYM_BACKSLASH){
		/* unescaped double quote changes quoted state. */
		*quoted = *quoted ? 0 : 1;
	    }
	}
	if(*p == SYM_COMMENT && !*quoted){
	    int pos = p - str;
	    if(pos == 0 ||
	       ISWHITE(p[-1]) && (pos == 1 || p[-2] != SYM_BACKSLASH)){
		int len = strlen(p);
		if(len > 0 && p[len - 1] == SYM_NEWLINE){
		    /* newline is the identifier for finding end of value.
		       therefore, it should not be removed. */
		    *p++ = SYM_NEWLINE;
		}
		*p = '\0';
		break;
	    }
	}
	++p;
    }
#endif
}

static int
read_line(fd, line)
    FILE *fd;
    Line *line;
{
    char buf[BUFSIZE], *p;
    int len;
    int quoted = 0;	/* quoted by double quote? */
    char *str;
    int cur;

    str = line->str;
    cur = line->cursize = 0;

    while((p = fgets(buf, BUFSIZE, fd)) != NULL){
	++line->seq;
	zap_comment(p, &quoted);	/* remove comment line */
	len = strlen(p);
	if(len == 0){
	    if(cur > 0){
		break;
	    }
	    continue;
	}
	if(cur + len + 1 > line->maxsize){
	    /* need to reallocate buffer. */
	    if(! realloc_line(line, line->maxsize + BUFSIZE)){
		goto err;	/* realloc error. */
	    }
	    str = line->str;
	}
	strncpy(str + cur, p, len);
	cur += len;
	str[cur] = '\0';
	if(!quoted){
	    if(cur > 1 && str[cur - 2] == SYM_BACKSLASH &&
	       str[cur - 1] == SYM_NEWLINE){
		/* the line is ended backslash followed by newline.
		   need to concatinate the next line. */
		cur -= 2;
		str[cur] = '\0';
	    }else{
		break;
	    }
	}
    }
    if(quoted){
	/* error.  still in quoted state. */
	goto err;
    }
    return line->cursize = cur;

err:
    return -1;
}

/************************************************************************/

static Token
get_token(str)
    char *str;
{
    switch(*str){
    case SYM_NEWLINE:	return T_NEWLINE;
    case SYM_COMMENT:	return T_COMMENT;
    case SYM_SEMICOLON:	return T_SEMICOLON;
    case SYM_DOUBLE_QUOTE:	return T_DOUBLE_QUOTE;
    case SYM_LEFT_BRACE:	return T_LEFT_BRACE;
    case SYM_RIGHT_BRACE:	return T_RIGHT_BRACE;
    case SYM_SPACE:	return T_SPACE;
    case SYM_TAB:	return T_TAB;
    case SYM_BACKSLASH:
	switch(str[1]){
	case 'x': return T_NUMERIC_HEX;
	case 'd': return T_NUMERIC_DEC;
	case 'o': return T_NUMERIC_OCT;
	}
	return T_BACKSLASH;
    default:
	return T_DEFAULT;
    }
}

static int
get_word(str, word)
    char *str;
    char *word;
{
    char *p = str, *w = word;
    Token token;
    int token_len;

    while(*p != '\0'){
	token = get_token(p);
	token_len = token_tbl[token].len;
	if(token == T_BACKSLASH){
	    p += token_len;
	    if(*p == '\0'){
		break;
	    }
	    token = get_token(p);
	    token_len = token_tbl[token].len;
	}else if(token != T_COMMENT &&
		 token != T_DEFAULT){
	    break;
	}
	strncpy(w, p, token_len);
	p += token_len; w += token_len;
    }
    *w = '\0';
    return p - str;	/* return number of scanned chars */
}

static int
get_quoted_word(str, word)
    char *str;
    char *word;
{
    char *p = str, *w = word;
    Token token;
    int token_len;

    if(*p == SYM_DOUBLE_QUOTE){
	++p;
    }
    while(*p != '\0'){
	token = get_token(p);
	token_len = token_tbl[token].len;
	if(token == T_DOUBLE_QUOTE){
	    p += token_len;
	    goto found;
	}
	if(token == T_BACKSLASH){
	    p += token_len;
	    if(*p == '\0'){
		break;
	    }
	    token = get_token(p);
	    token_len = token_tbl[token].len;
	}
	strncpy(w, p, token_len);
	p += token_len; w += token_len;
    }
    /* error. cannot detect next double quote */
    return 0;

 found:;
    *w = '\0';
    return p - str;
}

/************************************************************************/

static int
append_value_list()
{
    char **value_list = parse_info.value;
    char *value;
    int value_num = parse_info.value_num;
    int value_len = parse_info.value_len;
    char *str = parse_info.buf;
    int len = parse_info.bufsize;
    char *p;

    if(len < 1){
	return 1; /* return with no error */
    }

    if(value_list == (char **)NULL){
	value_list = (char **)Xmalloc(sizeof(char *) * 2);
	*value_list = NULL;
    }else{
	value_list = (char **)
	    Xrealloc(value_list, sizeof(char *) * (value_num + 2));
    }
    if(value_list == (char **)NULL){
	goto err;
    }

    value = *value_list;
    if(value == NULL){
	value = (char *)Xmalloc(value_len + len + 1);
    }else{
	value = (char *)Xrealloc(value, value_len + len + 1);
    }
    if(value == NULL){
	goto err;
    }
    if(value != *value_list){
	int delta, i;
	delta = value - *value_list;
	*value_list = value;
	for(i = 1; i < value_num; ++i){
	    value_list[i] += delta;
	}
    }

    value_list[value_num] = p = &value[value_len];
    value_list[value_num + 1] = NULL;
    strncpy(p, str, len);
    p[len] = 0;

    parse_info.value = value_list;
    parse_info.value_num = value_num + 1;
    parse_info.value_len = value_len + len + 1;
    parse_info.bufsize = 0;
    return 1;

err:
    if(value_list){
	Xfree((char **)value_list);
    }
    if(value){
	Xfree(value);
    }
    parse_info.value = (char **)NULL;
    parse_info.value_num = 0;
    parse_info.value_len = 0;
    parse_info.bufsize = 0;
    return 0;
}

static int 
construct_name(name)
    char *name;
{
    int i, len = 0;
    char *p = name;

    for(i = 0; i <= parse_info.nest_depth; ++i){
	len += strlen(parse_info.name[i]) + 1;
    }

    strcpy(p, parse_info.name[0]);
    p += strlen(parse_info.name[0]);
    for(i = 1; i <= parse_info.nest_depth; ++i){
	*p++ = '.';
	strcpy(p, parse_info.name[i]);
	p += strlen(parse_info.name[i]);
    }
    return *name != '\0';
}

static int
store_to_database(db)
    Database *db;
{
    Database new = (Database)NULL;
    char name[BUFSIZE];

    if(parse_info.pre_state == S_VALUE){
	if(! append_value_list()){
	    goto err;
	}
    }

    if(parse_info.name[parse_info.nest_depth] == NULL){
	goto err;
    }

    new = (Database)Xmalloc(sizeof(DatabaseRec));
    if(new == (Database)NULL){
	goto err;
    }
    memset((void *)new, 0, sizeof(DatabaseRec));

    new->category = (char *)Xmalloc(strlen(parse_info.category) + 1);
    if(new->category == NULL){
	goto err;
    }
    strcpy(new->category, parse_info.category);

    if(! construct_name(name)){
	goto err;
    }
    new->name = (char *)Xmalloc(strlen(name) + 1);
    if(new->name == NULL){
	goto err;
    }
    strcpy(new->name, name);
    new->next = *db;
    new->value = parse_info.value;
    new->value_num = parse_info.value_num;
    *db = new;

    Xfree(parse_info.name[parse_info.nest_depth]);
    parse_info.name[parse_info.nest_depth] = NULL;

    parse_info.value = (char **)NULL;
    parse_info.value_num = 0;
    parse_info.value_len = 0;

    return 1;

err:
    if(new){
	if(new->category){
	    Xfree(new->category);
	}
	if(new->name){
	    Xfree(new->name);
	}
    }
    if(parse_info.value){
	if(*parse_info.value){
	    Xfree(*parse_info.value);
	}
	Xfree((char **)parse_info.value);
	parse_info.value = (char **)NULL;
	parse_info.value_num = 0;
	parse_info.value_len = 0;
    }
    return 0;
}

#define END_MARK	"END"
#define	END_MARK_LEN	3 /*strlen(END_MARK)*/

static int
check_category_end(str)
    char *str;
{
    char *p;
    int len;

    p = str;
    if(strncmp(p, END_MARK, END_MARK_LEN)){
	return 0;
    }
    p += END_MARK_LEN;

    while(ISWHITE(*p)){
	++p;
    }
    len = strlen(parse_info.category);
    if(strncmp(p, parse_info.category, len)){
	return 0;
    }
    p += len;
    return p - str;
}

/************************************************************************/

static int
f_newline(str, token, db)
    char *str;
    Token token;
    Database *db;
{
    switch(parse_info.pre_state){
    case S_NULL:
    case S_CATEGORY:
	break;
    case S_NAME:
	goto err; /* no value */
    case S_VALUE:
	if(!store_to_database(db)){
	    goto err;
	}
	parse_info.pre_state = S_CATEGORY;
	break;
    default:
	goto err;
    }
    return token_tbl[token].len;

err:
    return 0;
}

static int
f_comment(str, token, db)
    char *str;
    Token token;
    Database *db;
{
    /* NOTE: comment is already handled in read_line(),
       so this function is not necessary. */

    char *p = str;

    while(*p != SYM_NEWLINE && *p != '\0'){
	++p;	/* zap to the end of line */
    }
    return p - str;
}

static int
f_white(str, token, db)
    char *str;
    Token token;
    Database *db;
{
    char *p = str;

    while(ISWHITE(*p)){
	++p;
    }
    return p - str;
}

static int
f_semicolon(str, token, db)
    char *str;
    Token token;
    Database *db;
{
    switch(parse_info.pre_state){
    case S_NULL:
    case S_CATEGORY:
    case S_NAME:
	goto err;
    case S_VALUE:
	if(! append_value_list()){
	    goto err;
	}
	parse_info.pre_state = S_VALUE;
	break;
    default:
	goto err;
    }
    return token_tbl[token].len;

err:
    return 0;
}

static int
f_left_brace(str, token, db)
    char *str;
    Token token;
    Database *db;
{
    switch(parse_info.pre_state){
    case S_NULL:
    case S_CATEGORY:
	goto err;
    case S_NAME:
	if(parse_info.name[parse_info.nest_depth] == NULL ||
	   parse_info.nest_depth + 1 > MAX_NAME_NEST){
	    goto err;
	}
	++parse_info.nest_depth;
	parse_info.pre_state = S_CATEGORY;
	break;
    case S_VALUE:
    default:
	goto err;
    }
    return token_tbl[token].len;

err:
    return 0;
}

static int
f_right_brace(str, token, db)
    char *str;
    Token token;
    Database *db;
{
    if(parse_info.nest_depth < 1){
	goto err;
    }

    switch(parse_info.pre_state){
    case S_NULL:
    case S_NAME:
	goto err;
    case S_VALUE:
	if(! store_to_database(db)){
	    goto err;
	}
	/* fall into next case */
    case S_CATEGORY:
	if(parse_info.name[parse_info.nest_depth] != NULL){
	    Xfree(parse_info.name[parse_info.nest_depth]);
	    parse_info.name[parse_info.nest_depth] = NULL;
	}
	--parse_info.nest_depth;
	parse_info.pre_state = S_CATEGORY;
	break;
    default:
	goto err;
    }
    return token_tbl[token].len;

err:
    return 0;
}

static int
f_double_quote(str, token, db)
    char *str;
    Token token;
    Database *db;
{
    char word[BUFSIZE];
    int len = 0;

    switch(parse_info.pre_state){
    case S_NULL:
    case S_CATEGORY:
	goto err;
    case S_NAME:
    case S_VALUE:
	len = get_quoted_word(str, word);
	if(len < 1){
	    goto err;
	}
	strcpy(&parse_info.buf[parse_info.bufsize], word);
	parse_info.bufsize += strlen(word);
	parse_info.pre_state = S_VALUE;
	break;
    default:
	goto err;
    }
    return len;	/* including length of token */

err:
    return 0;
}

static int
f_backslash(str, token, db)
    char *str;
    Token token;
    Database *db;
{
    return f_default(str, token, db);
}

static int
f_numeric(str, token, db)
    char *str;
    Token token;
    Database *db;
{
    char word[BUFSIZE], *p;
    int len;
    int token_len;

    switch(parse_info.pre_state){
    case S_NULL:
    case S_CATEGORY:
	goto err;
    case S_NAME:
    case S_VALUE:
	token_len = token_tbl[token].len;
	p = str + token_len;
	len = get_word(p, word);
	if(len < 1){
	    goto err;
	}
	strncpy(&parse_info.buf[parse_info.bufsize], str, token_len);
	strcpy(&parse_info.buf[parse_info.bufsize + token_len], word);
	parse_info.bufsize += token_len + strlen(word);
	parse_info.pre_state = S_VALUE;
	break;
    default:
	goto err;
    }
    return len + token_len;

err:
    return 0;
}

static int
f_default(str, token, db)
    char *str;
    Token token;
    Database *db;
{
    char word[BUFSIZE], *p;
    int len;

    len = get_word(str, word);
    if(len < 1){
	goto err;
    }

    switch(parse_info.pre_state){
    case S_NULL:
	if(parse_info.category != NULL){
	    goto err;
	}
	p = (char *)Xmalloc(strlen(word) + 1);
	if(p == NULL){
	    goto err;
	}
	strcpy(p, word);
	parse_info.category = p;
	parse_info.pre_state = S_CATEGORY;
	break;
    case S_CATEGORY:
	if(parse_info.nest_depth == 0){
	    if(check_category_end(str)){
		/* end of category is detected.
		   clear context and zap to end of this line */
		clear_parse_info();
		len = strlen(str);
		break;
	    }
	}
	p = (char *)Xmalloc(strlen(word) + 1);
	if(p == NULL){
	    goto err;
	}
	strcpy(p, word);
	if(parse_info.name[parse_info.nest_depth] != NULL){
	    Xfree(parse_info.name[parse_info.nest_depth]);
	}
	parse_info.name[parse_info.nest_depth] = p;
	parse_info.pre_state = S_NAME;
	break;
    case S_NAME:
    case S_VALUE:
	strcpy(&parse_info.buf[parse_info.bufsize], word);
	parse_info.bufsize += strlen(word);
	parse_info.pre_state = S_VALUE;
	break;
    default:
	goto err;
    }
    return len;

err:
    return 0;
}

/************************************************************************/

#ifdef	DEBUG
static void
PrintDatabase(db)
    Database db;
{
    Database p = db;
    int i = 0, j;

    printf("***\n*** BEGIN Database\n***\n");
    while(p){
	printf("%3d: ", i++);
	printf("%s, %s, ", p->category, p->name);
	printf("\t[%d: ", p->value_num);
	for(j = 0; j < p->value_num; ++j){
	    printf("%s, ", p->value[j]);
	}
	printf("]\n");
	p = p->next;
    }
    printf("***\n*** END   Database\n***\n");
}
#endif

static void
DestroyDatabase(db)
    Database db;
{
    Database p = db;

    while(p){
	if(p->category != NULL){
	    Xfree(p->category);
	}
	if(p->name != NULL){
	    Xfree(p->name);
	}
	if(p->value != (char **)NULL){
	    if(*p->value != NULL){
		Xfree(*p->value);
	    }
	    Xfree((char *)p->value);
	}
	db = p->next;
	Xfree((char *)p);
	p = db;
    }
}

static int
CountDatabase(db)
    Database db;
{
    Database p = db;
    int cnt = 0;

    while(p){
	++cnt;
	p = p->next;
    }
    return cnt;
}

static Database
CreateDatabase(dbfile)
    char *dbfile;
{
    Database db = (Database)NULL;
    FILE *fd;
    Line line;
    char *p;
    Token token;
    int len;
    int error = 0;

    fd = fopen(dbfile, "r");
    if(fd == (FILE *)NULL){
	return NULL;
    }

    memset((void *)&line, 0, sizeof(Line));
    memset((void *)&parse_info, 0, sizeof(DBParseInfo));

    do {
	int rc = read_line(fd, &line);
	if(rc < 0){
	    error = 1;
	    break;
	}else if(rc == 0){
	    break;
	}
	p = line.str;
	while(*p){
	    token = get_token(p);
	    len = (*token_tbl[token].parse_proc)(p, token, &db);
	    if(len < 1){
		error = 1;
		break;
	    }
	    p += len;
	}
    } while (!error);

    if(parse_info.pre_state != S_NULL){
	clear_parse_info();
	error = 1;
    }
    if(error){
#ifdef	DEBUG
	fprintf(stderr, "database format error at line %d.\n", line.seq);
#endif
	DestroyDatabase(db);
	db = (Database)NULL;
    }

    fclose(fd);
    free_line(&line);

#ifdef	DEBUG
    PrintDatabase(db);
#endif

    return db;
}

void	FreeDatabase(Database db)
{
	if(db->category)
		Xfree(db->category);
	if(db->name)
		Xfree(db->name);
	if(db->value){
		if(*db->value)
			Xfree(*db->value);
		Xfree(db->value);
	}
	Xfree(db);
}

#ifdef READ_USER_ENV
int
CheckUserFilename(char *name, char **path)
{
	char	*home_dir, *locale_dir, *ptr;
	char	locale_name[256];
	char	*new_path;

	if(strcmp(name, WC_TO_CS_CONV_TABLE_ENTRY)){
		if(strcmp(name, CS_TO_WC_CONV_TABLE_ENTRY)){
			if(strcmp(name, CS_TO_WC_CONV_FILE_ENTRY)){
				return 0;
			}
		}
	}

	home_dir = getenv("HOME");
	if(home_dir == (char *)NULL)
		return -1;

	locale_dir = _XlcLocaleDirName(setlocale(LC_CTYPE, (char *)NULL));
	ptr = strrchr(locale_dir, '/');
	strcpy(locale_name, ptr + 1);
	Xfree(locale_dir);

	new_path = (char *)malloc(strlen(home_dir) + 1 + strlen("XLOCALE_USER_DIR") + 1 + strlen(locale_name) + 1 + strlen(*path) + 1);
	if(new_path == (char *)NULL)
		return -1;

	sprintf(new_path, "%s/%s/%s/%s", home_dir, XLOCALE_USER_DIR, locale_name, *path);
	Xfree(*path);
	*path = new_path;

	return 0;
}

static Database
MergeXLocaleDatabase(Database usr, Database sys)
{
	Database	db_head, db_cur;
	Database	sys_head, sys_cur;
	Database	tmp_head, tmp_cur;
	Database	usr_head, usr_cur;
	Database	fs_head, fs_cur;
	Database	ret_db = (Database)NULL;
	int	fs_count, i;

	db_head = (Database)Xmalloc(sizeof(DatabaseRec));
	if(db_head == (Database)NULL){
		goto err_return;
	}
	db_head->next = (Database)NULL;
	db_cur = db_head;

	tmp_head = (Database)Xmalloc(sizeof(DatabaseRec));
	if(tmp_head == (Database)NULL){
		goto err_return;
	}
	tmp_head->next = (Database)NULL;
	tmp_cur = tmp_head;

	sys_head = (Database)Xmalloc(sizeof(DatabaseRec));
	if(sys_head == (Database)NULL){
		goto err_return;
	}
	sys_head->next = sys;
	sys_cur = sys_head;

	usr_head = (Database)Xmalloc(sizeof(DatabaseRec));
	if(usr_head == (Database)NULL){
		goto err_return;
	}
	usr_head->next = usr;
	usr_cur = usr_head;

	fs_head = (Database)Xmalloc(sizeof(DatabaseRec));
	if(fs_head == (Database)NULL){
		goto err_return;
	}
	fs_head->next = (Database)NULL;
	fs_cur = fs_head;


	/* remove non XLC_XLOCALE Category */
	while(sys_cur->next){
		if (strcmp(sys_cur->next->category, "XLC_XLOCALE")){
			tmp_cur->next = sys_cur->next;
			tmp_cur = tmp_cur->next;
			sys_cur->next = tmp_cur->next;
			tmp_cur->next = (Database)NULL;
		}
		else{
			sys_cur = sys_cur->next;
		}
	}

	/* XLC_XLOCALE not csN in system */
	sys_cur = sys_head;
	while(sys_cur->next){
		if(strncmp(sys_cur->next->name, "cs", 2)){
			usr_cur = usr_head;
			while(usr_cur->next){
				if(!strcmp(usr_cur->next->name, sys_cur->next->name)){
					Database unused;
					if (CheckUserFilename(usr_cur->next->name, &(usr_cur->next->value[0])))
						goto err_return;
					db_cur->next = usr_cur->next;
					db_cur = db_cur->next;
					usr_cur->next = db_cur->next;
					db_cur->next = (Database)NULL;

					unused = sys_cur->next;
					sys_cur->next = unused->next;
					FreeDatabase(unused);

					goto found1;
					break;
				}
				else{
					usr_cur = usr_cur->next;
				}
			}
			db_cur->next = sys_cur->next;
			db_cur = db_cur->next;
			sys_cur->next = db_cur->next;
			db_cur->next = (Database)NULL;
found1:
			;
		}
		else{
			sys_cur = sys_cur->next;
		}
	}

	/* XLC_XLOCALE not csN in user */
	usr_cur = usr_head;
	while(usr_cur->next){
		if(strncmp(usr_cur->next->name, "cs", 2)){
			if (CheckUserFilename(usr_cur->next->name, &(usr_cur->next->value[0])))
				goto err_return;
			db_cur->next = usr_cur->next;
			db_cur = db_cur->next;
			usr_cur->next = db_cur->next;
			db_cur->next = (Database)NULL;
		}
		else{
			usr_cur = usr_cur->next;
		}
	}

	/* XLC_XLOCALE csN in user */
	usr_cur = usr_head;
	while(usr_cur->next){
		char	*dot_ptr;
		int	csN, ref_csN;
		char	csN_name[6];
		char	ref_csN_name[6];
		int	csN_len, ref_csN_len;
		char	*new_name;
		Database	ptr, cur_first, cur_last;

		/* csN entry */
		if((dot_ptr = strchr(usr_cur->next->name, '.')) != (char *)NULL){
			csN_len = dot_ptr - usr_cur->next->name;
			strncpy(csN_name, usr_cur->next->name, csN_len);
			csN_name[csN_len] = (char)NULL;
			if(!strncmp(csN_name, "cs", 2)){
				csN = atoi(csN_name + 2);
				ref_csN = -1;
				cur_first = usr_cur->next;
				cur_last = (Database)NULL;
				/* first pass in user */
				while(usr_cur->next){
					if(strncmp(usr_cur->next->name, csN_name, csN_len)){
						cur_last = usr_cur;
						break;
					}
					if(!strcmp(usr_cur->next->name + csN_len + 1, "base_csN")){
						ref_csN = atoi(usr_cur->next->value[0]);
					}
					usr_cur = usr_cur->next;
				}
				if(cur_last == (Database)NULL)
					cur_last = usr_cur;

				/* add system */
				if(ref_csN >= 0){
					sys_cur = sys_head;
					sprintf(ref_csN_name, "cs%d", ref_csN);
					ref_csN_len = strlen(ref_csN_name);
					while(sys_cur->next){
						if(!strncmp(sys_cur->next->name, ref_csN_name, ref_csN_len)){
							ptr = cur_first;
							do{
								if(!strcmp(sys_cur->next->name + ref_csN_len + 1, ptr->name + csN_len + 1)){
									goto found2;
								}
							}while(ptr != cur_last);
							new_name = (char *)malloc(strlen(sys_cur->next->name) - ref_csN_len + csN_len + 1);
							if(new_name == (char *)NULL)
								goto err_return;
							sprintf(new_name, "%s.%s", csN_name, sys_cur->next->name + ref_csN_len + 1);
							Xfree(sys_cur->next->name);
							sys_cur->next->name = new_name;
							db_cur->next = sys_cur->next;
							db_cur = db_cur->next;
							sys_cur->next = db_cur->next;
							db_cur->next = (Database)NULL;
						}
						else{
found2:
							sys_cur = sys_cur->next;
						}
					}
				}

				/* add user */
				ptr = cur_first;
				do{
					if(strcmp(ptr->name + csN_len + 1, "base_csN")){
						if (CheckUserFilename(ptr->name + csN_len + 1, &(usr_cur->next->value[0])))
							goto err_return;
						db_cur->next = ptr;
						db_cur = db_cur->next;
						db_cur->next = (Database)NULL;
					}
				}
				while(ptr != cur_last);
				usr_cur = cur_last;
			}
			else
				usr_cur = usr_cur->next;
		}
		else
			usr_cur = usr_cur->next;
	}

	/* select XLC_FONTSET */
	db_cur = db_head;
	fs_cur = fs_head;
	fs_count = 0;
	while(db_cur->next){
		char	*dot_ptr;
		if((dot_ptr = strchr(db_cur->next->name, '.')) == (char *)NULL){
			goto nextloop1;
		}
		if(strcmp(dot_ptr + 1, "ct_encoding")){ 
			goto nextloop1;
		}
		for(i = 0; i < db_cur->next->value_num; i++){
			tmp_cur = tmp_head;
			while(tmp_cur->next){
				if(strcmp(tmp_cur->next->category, "XLC_FONTSET"))
					goto nextloop2;
				if((dot_ptr = strchr(tmp_cur->next->name, '.')) == (char *)NULL)
					goto nextloop2;
				if(strcmp(dot_ptr + 1, "charset"))
					goto nextloop2;
				if(tmp_cur->next->value_num < 1)
					goto nextloop2;
				if(!strcmp(tmp_cur->next->value[0], db_cur->next->value[0])){
					fs_cur->next = tmp_cur->next;
					fs_cur = fs_cur->next;
					tmp_cur->next = fs_cur->next;
					fs_cur->next = (Database)NULL;
					fs_count++;
					continue;
				}
nextloop2:
				tmp_cur = tmp_cur->next;
			}
		}
nextloop1:
		db_cur = db_cur->next;
	}
	fs_cur = fs_head;
	while(fs_cur->next){
		char	fsN_name[6], ref_fsN_name[6];
		int	fsN_len, ref_fsN_len;
		char	*dot_ptr;
		char	*new_name;
		if((dot_ptr = strchr(fs_cur->next->name, '.')) != (char *)NULL){
			ref_fsN_len = dot_ptr - fs_cur->next->name;
			strncpy(ref_fsN_name, fs_cur->next->name, ref_fsN_len);
			ref_fsN_name[ref_fsN_len] = (char)NULL;

			sprintf(fsN_name, "fs%d", --fs_count);
			fsN_len = strlen(fsN_name);

			new_name = (char *)malloc(fsN_len + 1 + strlen(dot_ptr + 1) + 1);
			if(new_name == (char *)NULL)
				goto err_return;
			sprintf(new_name, "%s.%s", fsN_name, dot_ptr + 1);
			Xfree(fs_cur->next->name);
			fs_cur->next->name = new_name;

			tmp_cur = tmp_head;
			while(tmp_cur->next){
				if(!strncmp(tmp_cur->next->name, ref_fsN_name, ref_fsN_len)){
					if((dot_ptr = strchr(tmp_cur->next->name, '.')) != (char *)NULL){
						new_name = (char *)malloc(fsN_len + 1 + strlen(dot_ptr + 1) + 1);
						if(new_name == (char *)NULL)
							goto err_return;
						sprintf(new_name, "%s.%s", fsN_name, dot_ptr + 1);
						Xfree(tmp_cur->next->name);
						tmp_cur->next->name = new_name;

						db_cur->next = tmp_cur->next;
						db_cur = db_cur->next;
						tmp_cur->next = db_cur->next;
						db_cur->next = (Database)NULL;
						continue;
					}
				}
				tmp_cur = tmp_cur->next;
			}
			db_cur->next = fs_cur->next;
			db_cur = db_cur->next;
			fs_cur->next = db_cur->next;
			db_cur->next = (Database)NULL;
		}
		else
			fs_cur = fs_cur->next;
	}

	ret_db = db_head->next;
#ifdef DEBUG
	PrintDatabase(ret_db);
#endif

err_return:
	DestroyDatabase(fs_head->next);
	DestroyDatabase(usr_head->next);
	DestroyDatabase(tmp_head->next);
	DestroyDatabase(sys_head->next);
	Xfree(fs_head);
	Xfree(usr_head);
	Xfree(tmp_head);
	Xfree(sys_head);
	Xfree(db_head);

	return ret_db;
}
#endif


/************************************************************************/

#ifndef	NOT_X_ENV

/* locale framework functions */

typedef struct _XlcDatabaseRec {
    XrmQuark category_q;
    XrmQuark name_q;
    Database db;
    struct _XlcDatabaseRec *next;
} XlcDatabaseRec, *XlcDatabase;

typedef	struct _XlcDatabaseListRec {
    XrmQuark name_q;
    XlcDatabase lc_db;
    Database database;
    int ref_count;
    struct _XlcDatabaseListRec *next;
} XlcDatabaseListRec, *XlcDatabaseList;

/* database cache list (per file) */
static XlcDatabaseList _db_list = (XlcDatabaseList)NULL;


/************************************************************************/
/*	_XlcDestroyLocaleDataBase(lcd)					*/
/*----------------------------------------------------------------------*/
/*	This function destroy the XLocale Database that bound to the 	*/
/*	specified lcd.  If the XLocale Database is refered from some 	*/
/*	other lcd, this function just decreases reference count of 	*/
/*	the database.  If no locale refers the database, this function	*/
/*	remove it from the cache list and free work area.		*/
/************************************************************************/
void
_XlcDestroyLocaleDataBaseUnicode(lcd)
    XLCd lcd;
{
    XlcDatabase lc_db = (XlcDatabase)XLC_PUBLIC(lcd, xlocale_db);
    XlcDatabaseList p, prev;

    for(p = _db_list, prev = (XlcDatabaseList)NULL; p;
	prev = p, p = p->next){
	if(p->lc_db == lc_db){
	    if((-- p->ref_count) < 1){
		if(p->lc_db != (XlcDatabase)NULL){
		    Xfree((char *)p->lc_db);
		}
		DestroyDatabase(p->database);
		if(prev == (XlcDatabaseList)NULL){
		    _db_list = p->next;
		}else{
		    prev->next = p->next;
		}
		Xfree((char*)p);
	    }
	    break;
	}
    }

    XLC_PUBLIC(lcd, xlocale_db) = (XPointer)NULL;
}

/************************************************************************/
/*	_XlcCreateLocaleDataBase(lcd)					*/
/*----------------------------------------------------------------------*/
/*	This function create an XLocale database which correspond to	*/
/*	the specified XLCd.						*/
/************************************************************************/
XPointer
_XlcCreateLocaleDataBaseUnicode(lcd)
    XLCd lcd;
{
    XlcDatabaseList list, new;
    Database p, database = (Database)NULL;
    XlcDatabase lc_db = (XlcDatabase)NULL;
    XrmQuark name_q;
    char pathname[256], *name;
    int i, n;

    Database database_sys = (Database)NULL;
    Database database_usr = (Database)NULL;
    char	*home_dir, *locale_dir, *ptr;
    char	locale_name[256];

    name = _XlcFileName(lcd, "locale");
    if(name == NULL){
	return (XPointer)NULL;
    }
    strcpy(pathname, name);
    Xfree(name);

    name_q = XrmStringToQuark(pathname);
    for(list = _db_list; list; list = list->next){
	if(name_q == list->name_q){
	    list->ref_count++;
	    return XLC_PUBLIC(lcd, xlocale_db) = (XPointer)list->lc_db;
	}
    }

    /* read /usr/openwin/lib/locale/<locale>.UTF-8/XLC_LOCALE */
    database_sys = CreateDatabase(pathname);
    if(database_sys == (Database)NULL){
	return (XPointer)NULL;
    }

/* READ_USER_ENV - read usr environment and merge */
#ifdef READ_USER_ENV
    /* read ~/.Xlocale/*.UTF-8/XLC_LOCALE */
    home_dir = getenv("HOME");
    if((locale_dir = _XlcLocaleDirName(setlocale(LC_CTYPE, (char *)NULL))) != (char *)NULL){
    	if((ptr = strrchr(locale_dir, '/')) != (char *)NULL)
		strcpy(locale_name, ptr + 1);
    	Xfree(locale_dir);
    }
    sprintf(pathname, "%s/%s/%s/%s", home_dir, XLOCALE_USER_DIR, locale_name, XLOCALE_USER_NAME);
    database_usr = CreateDatabase(pathname);

    /* merge two databases */
    if(database_usr != (Database)NULL){
	database = MergeXLocaleDatabase(database_usr, database_sys);
    }
    else{
	database = database_sys;
    }
#else
    database = database_sys;
#endif

    n = CountDatabase(database);
    lc_db = (XlcDatabase)Xmalloc(sizeof(XlcDatabaseRec) * (n + 1));
    if(lc_db == (XlcDatabase)NULL){
	goto err;
    }
    memset((void *)lc_db, 0, sizeof(XlcDatabaseRec) * (n + 1));
    for(p = database, i = 0; p && i < n; p = p->next, ++i){
	lc_db[i].category_q = XrmStringToQuark(p->category);
	lc_db[i].name_q = XrmStringToQuark(p->name);
	lc_db[i].db = p;
    }

    new = (XlcDatabaseList)Xmalloc(sizeof(XlcDatabaseListRec));
    if(new == (XlcDatabaseList)NULL){
	goto err;
    }
    new->name_q = name_q;
    new->lc_db = lc_db;
    new->database = database;
    new->ref_count = 1;
    new->next = _db_list;
    _db_list = new;

    return XLC_PUBLIC(lcd, xlocale_db) = (XPointer)lc_db;

err:
    DestroyDatabase(database);
    if(lc_db != (XlcDatabase)NULL){
	Xfree((char *)lc_db);
    }
    return (XPointer)NULL;
}

#endif	/* NOT_X_ENV */
