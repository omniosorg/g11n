/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma ident	"@(#)idnconv.c	1.2	04/06/18 SMI"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <idn/api.h>
#include <locale.h>
#include <langinfo.h>
#include <limits.h>
#include <iconv.h>
#include "idnconv.h"


/* Function prototypes: */
void idnconv_usage(int);
void idnconv_version(void);
void doit(FILE *, int, char *, char *);
char *increase_sz(char *, size_t *, FILE *, iconv_t);
void really_doit(int, char *, char *, size_t, FILE *);
char *simpler_iconv(char *, size_t *, char *, char *, FILE *);
void cleanup_and_exit(int, FILE *, iconv_t);

/* File-scope global variables: */
static char *cs;
static char *name;
static size_t namesz;
static char *icvbuf;
static size_t icvbufsz;
static char *outbuf;
static size_t outbufsz;


int
main(int ac, char **av)
{
	int actions;
	char *fromcode = (char *)NULL;
	char *tocode = (char *)NULL;
	int i;
	char *p;
	FILE *fi;

	(void) setlocale(LC_ALL, "");

#if !defined(TEXT_DOMAIN)
#define	TEXT_DOMAIN	"TEST_SUNW_I18N_CMD"
#endif
	(void) textdomain(TEXT_DOMAIN);

	/* The default actions for the IDN conversions. */
	actions = IDN_DELIMMAP | IDN_NAMEPREP | IDN_UNASCHECK | \
			IDN_ASCCHECK | IDN_IDNCONV | IDN_LENCHECK;

	/*
	 * Let's process options and option arguments first; if there is
	 * anything that starts without '-', that means it's the start of
	 * operand(s).
	 */
	for (i = 1; i < ac; i++) {
		if (av[i][0] == '-' && av[i][1] == '-') {
			/*
			 * The "--" alone means there will be no more
			 * options but operand(s) from now on.
			 */
			if (av[i][2] == '\0') {
				++i;
				break;
			}

			if (OPTION("in") || OPTION("from")) {
				if (++i >= ac) {
					(void) fprintf(stderr, gettext(
						"idnconv: error: in-code option argument is missing.\n"));
					idnconv_usage(0);
					exit(IDNCONV_ILLEGAL_OPTION);
				}
				fromcode = av[i];
			} else if (OPTION("out") || OPTION("to")) {
				if (++i >= ac) {
					(void) fprintf(stderr, gettext(
						"idnconv: error: out-code option argument is missing.\n"));
					idnconv_usage(0);
					exit(IDNCONV_ILLEGAL_OPTION);
				}
				tocode = av[i];
			} else if (OPTION("asciicheck") ||
				   OPTION("ascii-check")) {
				actions |= IDN_ASCCHECK;
			} else if (OPTION("noasciicheck") ||
				   OPTION("no-ascii-check")) {
				actions &= (~(IDN_ASCCHECK));
			} else if (OPTION("bidicheck") ||
				   OPTION("bidi-check")) {
				actions |= IDN_BIDICHECK;
			} else if (OPTION("nobidicheck") ||
				   OPTION("no-bidi-check")) {
				actions &= (~(IDN_BIDICHECK));
			} else if (OPTION("lengthcheck") ||
				   OPTION("length-check")) {
				actions |= IDN_LENCHECK;
			} else if (OPTION("nolengthcheck") ||
				   OPTION("no-length-check")) {
				actions &= (~(IDN_LENCHECK));
			} else if (OPTION("nameprep")) {
				actions |= IDN_NAMEPREP;
			} else if (OPTION("nonameprep") ||
				   OPTION("no-nameprep")) {
				actions &= (~(IDN_NAMEPREP));
			} else if (OPTION("unassigncheck") ||
				   OPTION("unassign-check")) {
				actions |= IDN_UNASCHECK;
			} else if (OPTION("nounassigncheck") ||
				   OPTION("no-unassign-check")) {
				actions &= (~(IDN_UNASCHECK));
			} else if (OPTION("help")) {
				idnconv_usage(1);
				exit(IDNCONV_SUCCESS);
			} else if (OPTION("version")) {
				idnconv_version();
				exit(IDNCONV_SUCCESS);
			} else {
				(void) fprintf(stderr, gettext(
					"idnconv: error: unknown option \"%s\" specified.\n"), av[i]);
				idnconv_usage(0);
				exit(IDNCONV_ILLEGAL_OPTION);
			}
		} else if (av[i][0] == '-') {
			if (av[i][1] == '\0') {
				(void) fprintf(stderr, gettext(
					"idnconv: error: option letter missing.\n"));
				idnconv_usage(0);
				exit(IDNCONV_ILLEGAL_OPTION);
			}
			for (p = av[i] + 1; *p; p++) {
				switch (*p) {
				case 'i':
				case 'f':
					if (++i >= ac) {
						(void) fprintf(stderr, gettext(
							"idnconv: error: in-code option argument is missing.\n"));
						idnconv_usage(0);
						exit(IDNCONV_ILLEGAL_OPTION);
					}
					fromcode = av[i];
					break;
				case 'o':
				case 't':
					if (++i >= ac) {
						(void) fprintf(stderr, gettext(
							"idnconv: error: out-code option argument is missing.\n"));
						idnconv_usage(0);
						exit(IDNCONV_ILLEGAL_OPTION);
					}
					tocode = av[i];
					break;
				case 'a':
					actions |= IDN_ASCCHECK;
					break;
				case 'A':
					actions &= (~(IDN_ASCCHECK));
					break;
				case 'b':
					actions |= IDN_BIDICHECK;
					break;
				case 'B':
					actions &= (~(IDN_BIDICHECK));
					break;
				case 'l':
					actions |= IDN_LENCHECK;
					break;
				case 'L':
					actions &= (~(IDN_LENCHECK));
					break;
				case 'n':
					actions |= IDN_NAMEPREP;
					break;
				case 'N':
					actions &= (~(IDN_NAMEPREP));
					break;
				case 'u':
					actions |= IDN_UNASCHECK;
					break;
				case 'U':
					actions &= (~(IDN_UNASCHECK));
					break;
				case 'h':
					idnconv_usage(1);
					exit(IDNCONV_SUCCESS);
				case 'v':
					idnconv_version();
					exit(IDNCONV_SUCCESS);
				default:
					(void) fprintf(stderr, gettext(
						"idnconv: error: unknown option letter '%c' specified.\n"), *p);
					idnconv_usage(0);
					exit(IDNCONV_ILLEGAL_OPTION);
				}
			}
		} else
			break;
	}

	/*
	 * Allocate the first memory blocks for the buffers that will be used.
	 */
	namesz = icvbufsz = outbufsz = BUFSIZ;
	name = (char *)malloc(namesz);
	icvbuf = (char *)malloc(icvbufsz);
	outbuf = (char *)malloc(outbufsz);
	if (name == (char *)NULL || icvbuf == (char *)NULL ||
	    outbuf == (char *)NULL) {
		(void) fprintf(stderr, gettext(
			"idnconv: error: no more memory.\n"));
		exit(IDNCONV_NOT_ENOUGH_MEMORY);
	}

	/*
	 * Collect the codeset name of the current locale for later uses.
	 * In some old systems, the nl_langinfo(CODESET) will return NULL or
	 * an empty string. For that case, we fallback to C locale's "646" for
	 * the completeness and correctness.
	 */
	cs = nl_langinfo(CODESET);
	if (cs == (char *)NULL || *cs == '\0') {
		cs = "646";
	}

	/*
	 * If there is no fromcode or tocode specified by the user, use
	 * the current locale's codeset.
	 */
	if (fromcode == (char *)NULL)
		fromcode = cs;
	if (tocode == (char *)NULL)
		tocode = cs;

	/*
	 * Process any possible operand(s) and then do the actual conversion(s)
	 * for each operand.
	 */
	if (i >= ac) {
		doit(stdin, actions, fromcode, tocode);
	} else {
		for (; i < ac; i++) {
			fi = fopen(av[i], "r");
			if (fi == (FILE *)NULL) {
				(void) fprintf(stderr, gettext(
					"idnconv: error: cannot access the file \"%s\".\n"), av[i]);
				cleanup_and_exit(IDNCONV_INPUT_FILE_NOT_FOUND,
					fi, (iconv_t)-1);
			}

			doit(fi, actions, fromcode, tocode);

			(void) fclose(fi);
		}
	}

	cleanup_and_exit(IDNCONV_SUCCESS, (FILE *)NULL, (iconv_t)-1);
}


void
idnconv_usage(int help)
{
	(void) fprintf(stderr, gettext(
"Usage: idnconv [ -i in-code | --in in-code | -f in-code | --from in-code ]\n\
	[ -o out-code | --out out-code | -t out-code | --to out-code ]\n\
	[ -a | --asciicheck | --ascii-check ]\n\
	[ -A | --noasciicheck | --no-ascii-check ]\n\
	[ -b | --bidicheck | --bidi-check ]\n\
	[ -B | --nobidicheck | --no-bidi-check ]\n\
	[ -l | --lengthcheck | --length-check ]\n\
	[ -L | --nolengthcheck | --no-length-check ]\n\
	[ -n | --nameprep ]\n\
	[ -N | --nonameprep | --no-nameprep ]\n\
	[ -u | --unassigncheck | --unassign-check ]\n\
	[ -U | --nounassigncheck | --no-unassign-check ]\n\
	[ -h | --help ]\n\
	[ -v | --version ]\n\
	[ file ... ]\n"));

	if (help) {
		(void) fprintf(stderr, gettext(
"\nThe 'idnconv' Internationalized Domain Name encoding conversion utility\n\
reads from specified file(s) or stdin and writes converted result to stdout.\n\
Refer to idnconv(1) man page for more detail.\n"));
	}
}


void
idnconv_version(void)
{
	(void) fprintf(stderr, gettext(
"idnconv 1.0\n\n\
Copyright 2004 Sun Microsystems, Inc.  All rights reserved.\n\
Use is subject to license terms.\n"));
}


void
doit(FILE *fi, int actions, char *from, char *to)
{
	char ib[BUFSIZ];
	size_t len;
	size_t i, j;

	/*
	 * Initialize the indices and then continuously read a buffer amount
	 * and process name(s) in the input buffer; sometimes, a name could be
	 * splitted between two input buffer contents over two fread(3C) calls.
	 * For each end of the process, reset the 'i' index for the next
	 * new input buffer.
	 */
	for (i = j = 0; (len = fread((void *)ib, 1, BUFSIZ, fi)) > 0; i = 0) {
		for (; i < len; i++) {
			/*
			 * In each loop, this extracts and accumulates a byte
			 * as a part of a name until there is either
			 * a white-space character of the POSIX/C locale or
			 * a user-supplied NULL byte embedded in the input,
			 * i.e., a delimiter.
			 *
			 * Any non-printables such as control characters
			 * will be screened out by idn_encodename(3EXT) and
			 * idn_decodename(3EXT) functions so don't worry
			 * about such characters here.
			 */
			if (ib[i] == '\0' || WHITESPACE(ib[i])) {
				if (j > 0) {
					really_doit(actions, from, to, j, fi);
					j = 0;
				}
				/*
				 * We make sure to putchar whatever we've
				 * gotten at here so that the output will
				 * resemble as much as possible of the input
				 * except the IDN names.
				 */
				putchar(ib[i]);
			} else {
				if (j >= namesz)
					name = increase_sz(name, &namesz, fi,
						(iconv_t)-1);
				name[j++] = ib[i];
			}
		}
	}

	/* We might still have a name to process. */
	if (j > 0) {
		really_doit(actions, from, to, j, fi);
	}
}


char *
increase_sz(char *s, size_t *sz, FILE *fi, iconv_t cd)
{
	*sz += BUFSIZ;
	s = (char *)realloc((void *)s, *sz);
	if (s == (char *)NULL) {
		(void) fprintf(stderr, gettext(
			"idnconv: error: no more memory.\n"));

		cleanup_and_exit(IDNCONV_NOT_ENOUGH_MEMORY, fi, cd);
	}

	return (s);
}


void
really_doit(int actions, char *from, char *to, size_t len, FILE *fi)
{
	char *ib;
	char *tmpp;
	idn_result_t idnres;
	int ret;
	int i;
	int to_encode;

	/* First, we properly terminate the name. */
	if (len >= namesz)
		name = increase_sz(name, &namesz, fi, (iconv_t)-1);
	name[len] = '\0';

	/*
	 * The idn_encodename(3EXT) and the idn_decodename(3EXT) expect
	 * a UTF-8 input argument string; if the "from" isn't UTF-8, then,
	 * it need to be converted to a UTF-8 string via simpler_iconv().
	 */
	if (strcasecmp(from, "UTF-8") == 0 || strcasecmp(from, "UTF8") == 0 ||
	    strcasecmp(from, "ACE") == 0 || strcasecmp(from, "Punycode") == 0 ||
	    strcasecmp(from, "ASCII") == 0 || strcasecmp(from, "646") == 0 ||
	    strcasecmp(from, "US-ASCII") == 0) {
		ib = name;
	} else {
		ib = simpler_iconv(name, &len, "UTF-8", from, fi);
	}

	/*
	 * Let's find out on whether to use the idn_decodename(3EXT), i.e.,
	 * ACE name to non-ACE IDN name, or the idn_encodename(3EXT),
	 * non-ACE IDN name to ACE name.
	 */
	if (strcasecmp(from, "ACE") == 0 || strcasecmp(from, "Punycode") == 0)
		to_encode = 0;
	else {
		to_encode = 1;

		/*
		 * Need to also check actual contents of the name and
		 * decide if it should be a decode or an encode process.
		 */
		if (len > 4 && strncasecmp(ib, ACE_PREFIX, 4) == 0) {
			to_encode = 0;
			for (tmpp = ib + 4; *tmpp; tmpp++)
				if (! isascii(*tmpp)) {
					to_encode = 1;
					break;
				}
		}
	}

	/* And, the "to" value finally decides on what to do. */
	if (strcasecmp(to, "ACE") == 0 || strcasecmp(to, "Punycode") == 0)
		to_encode = 1;

	/*
	 * Do the IDN conversion.
	 *
	 * If output buffer size is too small, we increase the output buffer
	 * size and then do the IDN conversion again. We only repeat such for
	 * MAX_CONV_LOOP times since doing such for the same input buffer
	 * for more than the MAX_CONV_LOOP times wouldn't really possible.
	 */
	for (i = 0; i <= MAX_CONV_LOOP; i++) {
		if (to_encode) {
			/*
			 * For the encoding process, we also do local mappings.
			 */
			actions |= IDN_LOCALMAP;
			idnres = idn_encodename(actions, ib, outbuf, outbufsz);
		} else {
			/*
			 * For the decoding process, we also do round trip
			 * checking as specified in the RFC 3490. 
			 * We also clean IDN_LENCHECK bit since it is not
			 * a valid action for idn_decodename(3EXT).
			 */
			actions |= IDN_RTCHECK;
			actions &= (~(IDN_LENCHECK));
			idnres = idn_decodename(actions, ib, outbuf, outbufsz);
		}

		if (idnres != idn_buffer_overflow)
			break;

		outbuf = increase_sz(outbuf, &outbufsz, fi, (iconv_t)-1);
	}

	if (idnres == idn_success) {
		if (strcasecmp(to, "UTF-8") == 0 ||
		    strcasecmp(to, "UTF8") == 0 ||
		    strcasecmp(to, "ACE") == 0 ||
		    strcasecmp(to, "Punycode") == 0) {
			tmpp = outbuf;
		} else {
			len = strlen(outbuf);
			tmpp = simpler_iconv(outbuf, &len, to, "UTF-8", fi);
		}

		/*
		 * Since fprintf(3C) functions use locale methods, we better
		 * use fputc(3C) to avoid any possible collision with
		 * underlying locale methods.
		 */
		while (*tmpp)
			(void) putc(*tmpp++, stdout);

		return;
	} else  if (idnres == idn_invalid_encoding || idnres == idn_nomapping) {
		(void) fprintf(stderr, gettext(
			"idnconv: error: internal code conversion failed due to possible incomplete or illegal character(s).\n"));
		ret = IDNCONV_ICONV_INTERNAL_ERROR;
	} else if (idnres == idn_invalid_name) {
		(void) fprintf(stderr, gettext(
			"idnconv: error: not supported internal code conversion name.\n"));
		ret = IDNCONV_UNSUPPORTED_INCODE_OR_OUTCODE;
	} else if (idnres == idn_invalid_length) {
		(void) fprintf(stderr, gettext(
			"idnconv: error: label length checking has failed.\n"));
		ret = IDNCONV_LABEL_LENGTH_CHECK_FAILED;
	} else if (idnres == idn_nomemory) {
		(void) fprintf(stderr, gettext(
			"idnconv: error: no more memory.\n"));
		ret = IDNCONV_NOT_ENOUGH_MEMORY;
	} else if (idnres == idn_prohibited) {
		(void) fprintf(stderr, gettext(
			"idnconv: error: prohibited, not allowed, or unassigned character(s) found.\n"));
		ret = IDNCONV_UNASSIGNED_CHARACTER_FOUND;
	} else {
		(void) fprintf(stderr, gettext(
			"idnconv: error: unspecified failure happened.\n"));
		ret = IDNCONV_UNKNOWN_FAILURE;
	}

	cleanup_and_exit(ret, fi, (iconv_t)-1);
}


char *
simpler_iconv(char *s, size_t *len, char *tocode, char *fromcode, FILE *fi)
{
	iconv_t cd;
	char *ibp;
	char *obp;
	size_t ileft, oleft;
	size_t ret;
	int i, j;

	cd = iconv_open((const char *)tocode, (const char *)fromcode);
	if (cd == (iconv_t)-1) {
		(void) fprintf(stderr, gettext(
			"idnconv: error: iconv from \"%s\" to \"%s\" is not supported.\n"), fromcode, tocode);
		cleanup_and_exit(IDNCONV_UNSUPPORTED_INCODE_OR_OUTCODE, fi, cd);
	}

	ibp = s;
	ileft = *len;
	obp = icvbuf;
	oleft = icvbufsz;

	for (i = 0; i <= MAX_CONV_LOOP; i++) {
		ret = iconv(cd, (const char **)&ibp, &ileft, &obp, &oleft);
		if (ret == 0)
			break;
		else if (ret > 0) {
			/*
			 * We had non-identical code conversion and that ain't
			 * good to be used as host or domain name; as the man
			 * page specifies, exit with a failure code.
			 */
			(void) fprintf(stderr, gettext(
				"idnconv: error: internal iconv code conversion encountered character(s) that cannot be converted to \"%s\" codeset.\n"), tocode);

			cleanup_and_exit(IDNCONV_ICONV_NON_IDENTICAL_MAPPING,
				fi, cd);
		}
		
		/*
		 * The only valid errno that we expect is E2BIG; if that's
		 * the case indeed, we increase the output buffer size and
		 * try again. (We will do this only MAX_CONV_LOOP times;
		 * if it needs more than MAX_CONV_LOOP times of output buffer
		 * size increases, that means the underlying iconv code
		 * conversion is most likely bad or something like that;
		 * we abort in that case.)
		 *
		 * Any other errno means we have encountered a problem, e.g.,
		 * an illegal character in the input buffer, an incomplete
		 * character at the end of the input buffer, conversion
		 * descriptor 'cd' is somehow bad or corrupted, or something
		 * unknown.
		 */
		if (errno == E2BIG) {
			j = icvbufsz - oleft;

			icvbuf = increase_sz(icvbuf, &icvbufsz, fi, cd);

			obp = icvbuf + j;
			oleft += BUFSIZ;
		} else {
			(void) fprintf(stderr, gettext(
				"idnconv: error: internal iconv code conversion failed.\n"));
			cleanup_and_exit(IDNCONV_ICONV_INTERNAL_ERROR, fi, cd);
		}
	}

	if (i > MAX_CONV_LOOP) {
		(void) fprintf(stderr, gettext(
			"idnconv: error: internal iconv code conversion failed.\n"));
		cleanup_and_exit(IDNCONV_ICONV_INTERNAL_ERROR, fi, cd);
	}

	/*
	 * We now reset the conversion state and collect any state reset
	 * values or remnants in the conversion before closing the iconv.
	 * It is quite unlikely that we will get out more than 256 bytes
	 * for the reset operation; if so, it will be treated as an error...
	 */
	ibp = (char *)NULL;
	ileft = 0;
	if (oleft < 256) {
		j = icvbufsz - oleft;

		icvbuf = increase_sz(icvbuf, &icvbufsz, fi, cd);

		obp = icvbuf + j;
		oleft += BUFSIZ;
	}

	ret = iconv(cd, (const char **)&ibp, &ileft, &obp, &oleft);
	if (ret > 0) {
		(void) fprintf(stderr, gettext(
			"idnconv: error: internal iconv code conversion encountered character(s) that cannot be converted to \"%s\".\n"), tocode);

		cleanup_and_exit(IDNCONV_ICONV_NON_IDENTICAL_MAPPING, fi, cd);
	} else if (ret == (size_t)-1) {
		(void) fprintf(stderr, gettext(
			"idnconv: error: internal iconv code conversion failed.\n"));
		cleanup_and_exit(IDNCONV_ICONV_INTERNAL_ERROR, fi, cd);
	}

	(void) iconv_close(cd);

	/*
	 * Now calculate the length and then append '\0' at the end of
	 * the icvbuf to make it a string.
	 */
	*len = icvbufsz - oleft;
	if (*len >= icvbufsz)
		icvbuf = increase_sz(icvbuf, &icvbufsz, fi, (iconv_t)-1);
	icvbuf[*len] = '\0';

	return (icvbuf);
}


void
cleanup_and_exit(int ret, FILE *fp, iconv_t cd)
{
	free((void *)name);
	free((void *)icvbuf);
	free((void *)outbuf);

	(void) fflush(stdout);

	if (fp)
		(void) fclose(fp);

	if (cd != (iconv_t)-1)
		(void) iconv_close(cd);

	exit(ret);
}
