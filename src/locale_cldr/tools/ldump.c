/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright (c) 2009, by Sun Microsystems, Inc.
 * All rights reserved.
 */

#include <locale.h>
#include <sys/localedef.h>
#include <langinfo.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <link.h>
#include <errno.h>



struct cmap_char {
	wchar_t wc;
	const unsigned char *native;
	const unsigned char *name;
};

struct cmap {
	const char *codeset;
	unsigned int len;
	const struct cmap_char *chars;
};

#include "cmap.dat"



static void
usage(void)
{
	printf("Usage: ldump [ lc_charmap | lc_collate | lc_monetary | lc_numeric | lc_messages | lc_time | lc_ctype ] <lib1>\n");
	exit(1);
}

static void
die(const char *format, ...)
{
	va_list args;

	fprintf(stderr, "ldump: ");
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, ": %s\n", strerror(errno));

	exit(1);
}



static void
die2(const char *format, ...)
{
	va_list args;

	fprintf(stderr, "ldump: ");
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");

	exit(1);
}



static _LC_locale_t*
get_lp(const char *path)
{
	void *handle;
        _LC_locale_t *(*fn)(void);

	if ((handle = dlopen(path, RTLD_LAZY)) == NULL)
		die2("dlopen(%s): %s", path, dlerror());

	if ((fn = (_LC_locale_t *(*)(void))dlsym(handle, "instantiate")) == NULL)
		die2("%s: not locale object");

	return fn();
}

#define LDUMP_REPORT(arg,type,fmt,name) printf("%s%s%s%s\t" fmt "\n", #arg, (name) == NULL ? "":" /* ",(name) == NULL ? "":name, (name) == NULL ? "":" */", ((type)p->arg))

#define LDUMP_STR(arg,name) LDUMP_REPORT(arg,const char*,"'%s'",name)
#define LDUMP_SIZET(arg,name) LDUMP_REPORT(arg,size_t,"%d",name)
#define LDUMP_UCHAR(arg,name) LDUMP_REPORT(arg,unsigned char,"%c",name)
#define LDUMP_HEX(arg,name) LDUMP_REPORT(arg,unsigned int,"0x%X",name)
#define LDUMP_INT(arg,name) LDUMP_REPORT(arg,int,"%d",name)
#define LDUMP_BIT(arg,n,name) LDUMP_HEX(arg,name)		/* FIXME */
#define LDUMP_WCHAR(arg,name) LDUMP_REPORT(arg,wchat_t,"%lc",name)

static void
lc_charmap(_LC_charmap_t *p)
{
	LDUMP_STR(cm_csname, "codeset name");
	/*	_LC_fc_type_t	cm_fc_type;	 file code type */
	/*	_LC_pc_type_t	cm_pc_type;	 process code type */
	LDUMP_SIZET(cm_mb_cur_max, "max encoding length for this codeset");
	LDUMP_SIZET(cm_mb_cur_min, "min encoding length for this codeset");
	LDUMP_UCHAR(cm_reserved, "reserved");
	LDUMP_UCHAR(cm_def_width, "default width");
	LDUMP_UCHAR(cm_base_max, "base area size: 0, 127, or 255");
	LDUMP_UCHAR(cm_tbl_ent, "# of extended table entries");
}

	
static void
lc_collate(const _LC_collate_t *p)
{
	LDUMP_HEX(co_nord, "number of collation orders supported in this locale");
	LDUMP_HEX(co_r_order, "relative weight flag");
	LDUMP_HEX(co_ext, "extinfo flag");

	/* LDUMP_BIT(co_sort, sizeof(wchar_t), "sort order processing flags"); */
	LDUMP_HEX(co_wc_min, "min process code");
	LDUMP_HEX(co_wc_max, "max process code");
	LDUMP_HEX(co_hbound, "max process code with");
	LDUMP_HEX(co_col_min, "min unique coll weight");
	LDUMP_HEX(co_col_max, "max unique coll weight");

	/*
	_const _LC_weight_t	*co_coltbl; array of collation weights
	_const _LC_collel_t	**co_cetbl; array of collating elements
	*/
	LDUMP_HEX(co_nsubs, "number of sub strs");
	/* _const _LC_subs_t	*co_subs;	substitution strs   */
	/* _const void	*co_extinfo;	extended info */
}


static void
lc_ctype(_LC_ctype_t *p)
{
	char *type[] = { "alnum", "alpha", "cntrl", "digit", "graph", "lower", "print", "punct", "space", "upper", "xdigit", "blank" };
	wctype_t wt[12];
	const struct cmap *cm = cmaps;
	unsigned int i, j;

	for (i = 0; i < 12; i++)
		wt[i] = METHOD(p,wctype)(p, type[i]);

	while (cm->codeset)
		if (strcmp(cm->codeset, p->cmapp->cm_csname) == 0)
			break;
		else
			cm++;

	assert(cm->codeset != NULL);


	for (i = 0; i < cm->len; i++) {
		printf("U+%04X (%s)\t", (unsigned int)cm->chars[i].wc, cm->chars[i].name);

		for (j = 0; j < 12; j++)
			if (METHOD_NATIVE(p, iswctype)(p, cm->chars[i].wc, wt[j]))
				printf("is%s ", type[j]);

		printf("\n");
	}
}



static void
lc_monetary(const _LC_monetary_t *p)
{
	LDUMP_STR(int_curr_symbol, "international currency symbol");
	LDUMP_STR(currency_symbol, "national currency symbol");
	LDUMP_STR(mon_decimal_point, "currency decimal point");
	LDUMP_STR(mon_thousands_sep, "currency thousands separator");
	LDUMP_STR(mon_grouping, "currency digits grouping");
	LDUMP_STR(positive_sign, "currency plus sign");
	LDUMP_STR(negative_sign, "currency minus sign");

	LDUMP_INT(int_frac_digits, "internat currency fract digits");
	LDUMP_INT(frac_digits, "currency fractional digits");
	LDUMP_INT(p_cs_precedes, "currency plus location");
	LDUMP_INT(p_sep_by_space, "currency plus space ind.");
	LDUMP_INT(n_cs_precedes, "currency minus location");
	LDUMP_INT(n_sep_by_space, "currency minus space ind.");
	LDUMP_INT(p_sign_posn, "currency plus position");
	LDUMP_INT(n_sign_posn, "currency minus position");

	LDUMP_INT(int_p_cs_precedes, "int'l currency plus location");
	LDUMP_INT(int_p_sep_by_space, "int'l currency plus space ind.");
	LDUMP_INT(int_n_cs_precedes, "int'l currency minus location");
	LDUMP_INT(int_n_sep_by_space, "int'l currency minus space ind.");
	LDUMP_INT(int_p_sign_posn, "int'l currency plus position");
	LDUMP_INT(int_n_sign_posn, "int'l currency minus position");
}

static void
lc_numeric(const _LC_numeric_t *p)
{
	LDUMP_STR(decimal_point, NULL);
	LDUMP_STR(thousands_sep, NULL);
	LDUMP_STR(grouping, NULL);
}

static void
lc_messages(const _LC_messages_t *p)
{
	LDUMP_STR(yesexpr, "POSIX: Expression for affirmative.");
	LDUMP_STR(noexpr, "POSIX: Expression for negative.");
	LDUMP_STR(yesstr, "X/OPEN: colon sep str for affirmative. ");
	LDUMP_STR(nostr, "X/OPEN: colon sep str for negative. ");
}

static void
lc_time(const _LC_time_t *p)
{
	LDUMP_STR(d_fmt, NULL);
	LDUMP_STR(t_fmt, NULL);
	LDUMP_STR(d_t_fmt, NULL);
	LDUMP_STR(t_fmt_ampm, NULL);
	LDUMP_STR(abday[0], NULL);
	LDUMP_STR(abday[1], NULL);
	LDUMP_STR(abday[2], NULL);
	LDUMP_STR(abday[3], NULL);
	LDUMP_STR(abday[4], NULL);
	LDUMP_STR(abday[5], NULL);
	LDUMP_STR(abday[6], NULL);
	LDUMP_STR(day[0], NULL);
	LDUMP_STR(day[1], NULL);
	LDUMP_STR(day[2], NULL);
	LDUMP_STR(day[3], NULL);
	LDUMP_STR(day[4], NULL);
	LDUMP_STR(day[5], NULL);
	LDUMP_STR(day[6], NULL);
	LDUMP_STR(abmon[ 0], NULL);
	LDUMP_STR(abmon[ 1], NULL);
	LDUMP_STR(abmon[ 2], NULL);
	LDUMP_STR(abmon[ 3], NULL);
	LDUMP_STR(abmon[ 4], NULL);
	LDUMP_STR(abmon[ 5], NULL);
	LDUMP_STR(abmon[ 6], NULL);
	LDUMP_STR(abmon[ 7], NULL);
	LDUMP_STR(abmon[ 8], NULL);
	LDUMP_STR(abmon[ 9], NULL);
	LDUMP_STR(abmon[10], NULL);
	LDUMP_STR(abmon[11], NULL);
	LDUMP_STR(mon[ 0], NULL);
	LDUMP_STR(mon[ 1], NULL);
	LDUMP_STR(mon[ 2], NULL);
	LDUMP_STR(mon[ 3], NULL);
	LDUMP_STR(mon[ 4], NULL);
	LDUMP_STR(mon[ 5], NULL);
	LDUMP_STR(mon[ 6], NULL);
	LDUMP_STR(mon[ 7], NULL);
	LDUMP_STR(mon[ 8], NULL);
	LDUMP_STR(mon[ 9], NULL);
	LDUMP_STR(mon[10], NULL);
	LDUMP_STR(mon[11], NULL);
	LDUMP_STR(am_pm[0], NULL);
	LDUMP_STR(am_pm[1], NULL);
	/* LDUMP_STR(*era;	NULL terminated array of strings */
	LDUMP_STR(era_d_fmt, NULL);
	LDUMP_STR(alt_digits, NULL);
	LDUMP_STR(era_d_t_fmt, NULL);
	LDUMP_STR(era_t_fmt, NULL);
	LDUMP_STR(date_fmt, NULL);
}


/*
#define LDUMP_STR2(arg,name) if (strcmp(p1[arg], p2[arg]) != 0) LDUMP_REPORT(p1[arg],p2[arg],"%s",name)

static void
lc_nl_info(int n, char *p1[], char *p2[])
{
	LDUMP_STR2(DAY_1, "sunday");
	LDUMP_STR2(DAY_2, "monday");
	LDUMP_STR2(DAY_3, "tuesday");
	LDUMP_STR2(DAY_4, "wednesday");
	LDUMP_STR2(DAY_5, "thursday");
	LDUMP_STR2(DAY_6, "friday");
	LDUMP_STR2(DAY_7, "saturday");

	LDUMP_STR2(ABDAY_1, "sunday (abbrev.)");
	LDUMP_STR2(ABDAY_2, "monday (abbrev.)");
	LDUMP_STR2(ABDAY_3, "tuesday (abbrev.)");
	LDUMP_STR2(ABDAY_4, "wednesday (abbrev.)");
	LDUMP_STR2(ABDAY_5, "thursday (abbrev.)");
	LDUMP_STR2(ABDAY_6, "friday (abbrev.)");
	LDUMP_STR2(ABDAY_7, "saturday (abbrev.)");

	LDUMP_STR2(MON_1, "january");
	LDUMP_STR2(MON_2, "february");
	LDUMP_STR2(MON_3, "march");
	LDUMP_STR2(MON_4, "april");
	LDUMP_STR2(MON_5, "may");
	LDUMP_STR2(MON_6, "june");
	LDUMP_STR2(MON_7, "july");
	LDUMP_STR2(MON_8, "august");
	LDUMP_STR2(MON_9, "september");
	LDUMP_STR2(MON_10, "october");
	LDUMP_STR2(MON_11, "november");
	LDUMP_STR2(MON_12, "december");

	LDUMP_STR2(ABMON_1, "january (abbrev.)");
	LDUMP_STR2(ABMON_2, "february (abbrev.)");
	LDUMP_STR2(ABMON_3, "march (abbrev.)");
	LDUMP_STR2(ABMON_4, "april (abbrev.)");
	LDUMP_STR2(ABMON_5, "may (abbrev.)");
	LDUMP_STR2(ABMON_6, "june (abbrev.)");
	LDUMP_STR2(ABMON_7, "july (abbrev.)");
	LDUMP_STR2(ABMON_8, "august (abbrev.)");
	LDUMP_STR2(ABMON_9, "september (abbrev.)");
	LDUMP_STR2(ABMON_1, "october (abbrev.)");
	LDUMP_STR2(ABMON_1, "november (abbrev.)");
	LDUMP_STR2(ABMON_1, "december (abbrev.)");

	LDUMP_STR2(RADIXCHAR, "separator for thousand");
	LDUMP_STR2(YESSTR, "affirmative response for yes/no queries");
	LDUMP_STR2(NOSTR, "negative response for yes/no queries");
	LDUMP_STR2(CRNCYSTR, "currency symbol");

	LDUMP_STR2(D_T_FMT, "string for formatting date and time");
	LDUMP_STR2(D_FMT, "date format");
	LDUMP_STR2(T_FMT, "time format");
	LDUMP_STR2(AM_STR, "am string");
	LDUMP_STR2(PM_STR, "pm string");

	LDUMP_STR2(CODESET, "am or pm time format string");
	LDUMP_STR2(ERA, "era date format string");
	LDUMP_STR2(ERA_D_T_FMT, "era time format string");
	LDUMP_STR2(ALT_DIGITS, "affirmative response expression");
	LDUMP_STR2(NOEXPR, "strftime format for date(1)");
}
*/


int
main(int argc, char* argv[])
{
	_LC_locale_t *lp;

	if (argc != 3)
		usage();

	lp = get_lp(argv[2]);


	if (strcmp("lc_charmap", argv[1]) == 0)
		lc_charmap(lp->lc_charmap);
	else if (strcmp("lc_collate", argv[1]) == 0)
		lc_collate(lp->lc_collate);
	else if (strcmp("lc_monetary", argv[1]) == 0)
		lc_monetary(lp->lc_monetary);
	else if (strcmp("lc_numeric", argv[1]) == 0)
		lc_numeric(lp->lc_numeric);
	else if (strcmp("lc_messages", argv[1]) == 0)
		lc_messages(lp->lc_messages);
	else if (strcmp("lc_time", argv[1]) == 0)
		lc_time(lp->lc_time);
	else if (strcmp("lc_ctype", argv[1]) == 0)
		lc_ctype(lp->lc_ctype);
	else
		assert(0);

/*        lc_nl_info(lp1->no_of_items, lp1->nl_info, lp2->nl_info); */

	return 0;
}
