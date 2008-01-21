/*
 * Copyright 1996-1999, 2001-2002 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#pragma	ident	"@(#)lcUNICODE.c	1.30 02/02/02 SMI"

/****************/
/* Header files */
/****************/

/* C headers */
#include <stdio.h>
#include <limits.h>
#include <iconv.h>
#include <sys/isa_defs.h>

/* Xlib header */
#include <X11/Xlocale.h>

/* Xlib internal headers */
#include <X11/Xlibint.h>
#include <X11/XlcGeneric.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

/* UNICODE locale headers */
#include "XlcPublicUnicode.h"
#include "utf8_defs.h"


/***********/
/* Defines */
/***********/

#define CS_ENTRY			CARD8
#define PATH_MAX			1024

#define MB_ICONV_NAME			"mb_encoding_name"
#define WC_ICONV_NAME			"wc_encoding_name"

#define WC_VALID_LENGTH_ENTRY		"wc_valid_length"
#define WC_TO_CS_CONV_TABLE_ENTRY_PLANE	wc_conversion_tables

#define CS_TO_WC_CONV_TABLE_ENTRY	"cs_conversion_table"

#define LUT_CODESET_UNKNOWN		0xff

#define ICONVOPEN			1

#define	READ_TABLE_OFFSET		4

#define XlcNUtf8String "utf8String"

/*************************/
/* structure and typedef */
/*************************/

static const char *wc_conversion_tables[U8_MAX_PLANES] = {
	"wc_conversion_table_plane_0",
	"wc_conversion_table_plane_1",
	"wc_conversion_table_plane_2",
	"wc_conversion_table_plane_3",
	"wc_conversion_table_plane_4",
	"wc_conversion_table_plane_5",
	"wc_conversion_table_plane_6",
	"wc_conversion_table_plane_7",
	"wc_conversion_table_plane_8",
	"wc_conversion_table_plane_9",
	"wc_conversion_table_plane_10",
	"wc_conversion_table_plane_11",
	"wc_conversion_table_plane_12",
	"wc_conversion_table_plane_13",
	"wc_conversion_table_plane_14",
	"wc_conversion_table_plane_15",
	"wc_conversion_table_plane_16",
};

typedef int (*UnicodeWcharTableConverterProc)(
		CARD8 **tables,
		XPointer *from,
		int *from_left,
		XPointer *to,
		int *to_left,
		CS_ENTRY *cnN
	);
typedef int (*UnicodeTableConverterProc)(
		void *table,
		XPointer *from,
		int *from_left,
		XPointer *to,
		int *to_left,
		CS_ENTRY *cnN
	);

typedef struct _UnicodeMbstrInfo{
  char	*iconv_name;
#ifdef ICONVOPEN
  iconv_t	iconv_towc;
  iconv_t	iconv_tomb;
#endif
  UnicodeWcharTableConverterProc	cs_conv_func;
} UnicodeMbstrInfo;
static UnicodeMbstrInfo	*UnicodeMbstrInfo_create(XLCd lcd);
static void	UnicodeMbstrInfo_destroy(UnicodeMbstrInfo *info);

typedef struct _UnicodeWcharInfo{
  int	valid_len;
  char	*iconv_name;
  CARD8	*tables[U8_MAX_PLANES];
  UnicodeWcharTableConverterProc	cs_conv_func;
} UnicodeWcharInfo;
static UnicodeWcharInfo	*UnicodeWcharInfo_create(XLCd lcd);
static void	UnicodeWcharInfo_destroy(UnicodeWcharInfo *info);

typedef struct _UnicodeCs{
  CodeSet	codeset;
  XlcCharSet	charset;
  void	*table;
  UnicodeTableConverterProc	wc_conv_func;
  UnicodeTableConverterProc	mb_conv_func;
} UnicodeCs;

typedef struct _UnicodeCsInfo{
  UnicodeCs	*cs;
  int	cs_num;
  int	max_length;
  int	fallback_cs;
} UnicodeCsInfo;
static UnicodeCsInfo	*UnicodeCsInfo_create(XLCd lcd);
static void	UnicodeCsInfo_destroy(UnicodeCsInfo *info);


/*************/
/* Functions */
/*************/

/* registration functions */
static XlcConv open_ctstombs(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);
static XlcConv open_ctstowcs(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);
static XlcConv open_mbstocts(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);
static XlcConv open_wcstocts(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);
static XlcConv open_wcstocs(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);
static XlcConv open_cstowcs(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);
static XlcConv open_wcstombs(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);
static XlcConv open_mbstowcs(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);
static XlcConv open_cstombs(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);
static XlcConv open_mbstocs(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);

/* UTF8_STRING support*/
static XlcConv open_utf8stombs(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);
static XlcConv open_utf8stowcs(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);
static XlcConv open_mbstoutf8s(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);
static XlcConv open_wcstoutf8s(XLCd from_lcd, char *from_type, XLCd to_lcd,
			char *to_type);

/* converter functions */
static int uc_wcstocts(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_mbstocts(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_ctstombs(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_ctstowcs(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_wcstocs(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_cstowcs(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_cstowcs_for_ct(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_mbstocs(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_mbstocs_for_ct(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_cstombs(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_cstombs_for_ct(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_wcstombs(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_mbstowcs(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);

/*UTF8_STRING support*/
static int uc_wcstoutf8s(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_mbstoutf8s(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_utf8stombs(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);
static int uc_utf8stowcs(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args);

static void close_converter(XlcConv conv);

/* local converters */
#ifdef	NOT_BEING_USED
static int convert_wc16_to_cs(void *table, XPointer *from_wc, int *from_left,
			XPointer *to_cs, int *to_left, CS_ENTRY *csN);
static int convert_mb16_to_cs(void *table, XPointer *from_mb, int *from_left,
			XPointer *to_cs, int *to_left, CS_ENTRY *csN);
static int convert_cs8_to_wc16(void *table, XPointer *from_cs, int *from_left,
			XPointer *to_wc, int *to_left, CS_ENTRY *csN);
static int convert_cs8_to_mb16(void *table, XPointer *from_cs, int *from_left,
			XPointer *to_mb, int *to_left, CS_ENTRY *csN);
static int convert_cs16_to_wc16(void *table, XPointer *from_cs, int *from_left,
			XPointer *to_wc, int *to_left, CS_ENTRY *csN);
static int convert_cs16_to_mb16(void *table, XPointer *from_cs, int *from_left,
			XPointer *to_mb, int *to_left, CS_ENTRY *csN);
#endif	/* NOT_BEING_USED */

static int convert_wc32_to_cs_vector_table(CARD8 **tables, XPointer *from_wc,
			int *from_left, XPointer *to_cs, int *to_left,
			CS_ENTRY *csN);
static int convert_wc32_to_cs_vector_table_for_ct(CARD8 **tables,
			XPointer *from_wc, int *from_left, XPointer *to_cs,
			int *to_left, CS_ENTRY *csN);
static int convert_mb32_to_cs_vector_table(CARD8 **tables, XPointer *from_mb,
			int *from_left, XPointer *to_cs, int *to_left,
			CS_ENTRY *csN);

static int convert_cs8_to_wc32(void *table, XPointer *from_cs, int *from_left,
			XPointer *to_wc, int *to_left, CS_ENTRY *csN);
static int convert_cs8_to_mb32(void *table, XPointer *from_cs, int *from_left,
			XPointer *to_mb, int *to_left, CS_ENTRY *csN);

static int convert_cs16_to_wc32(void *table, XPointer *from_cs, int *from_left,
			XPointer *to_wc, int *to_left, CS_ENTRY *csN);
static int convert_cs16_to_mb32(void *table, XPointer *from_cs, int *from_left,
			XPointer *to_mb, int *to_left, CS_ENTRY *csN);

static int convert_wcstombs(XPointer *from, int *from_left, XPointer *to,
			int *to_left);
static int convert_mbstowcs(XPointer *from, int *from_left, XPointer *to,
			int *to_left);
static int convert_cstombs(void *table, XPointer *from, int *from_left,
			XPointer *to, int *to_left, CS_ENTRY *csN, Bool is_CT);
static int convert_mbstocs(void *table, XPointer *from, int *from_left,
			XPointer *to, int *to_left, CS_ENTRY *csN);
static int convert_mbstocs_for_ct(void *table, XPointer *from, int *from_left,
			XPointer *to, int *to_left, CS_ENTRY *csN);

/* miscellaneous functions */
static XlcConv create_conv(XLCd lcd, XlcConvMethods methods);
static int make_filename(char *str, char *filename, int name_len);
static int make_utf8_filename(XLCd lcd, char *str, char *filename, int namelen);
static void *read_table(XLCd lcd, char *str);


/*************/
/* Variables */
/*************/
static UnicodeMbstrInfo	*uc_mb_info;
static UnicodeWcharInfo	*uc_wc_info;
static UnicodeCsInfo	*uc_cs_info;

/*position in enum corresponds to position in conv_methods[]*/
enum { WCSTOCS, CSTOWCS, WCSTOMBS, MBSTOWCS, CSTOMBS, MBSTOCS,
       WCSTOCTS, MBSTOCTS, CTSTOMBS, CTSTOWCS, CSTOMBS_FOR_CT,
       CSTOWCS_FOR_CT,   
       WCSTOUTF8S, MBSTOUTF8S, UTF8STOMBS, UTF8STOWCS};

enum { TO_MBS, TO_WCS };
static XlcConvMethodsRec conv_methods[] = {
  { close_converter, uc_wcstocs, NULL },
  { close_converter, uc_cstowcs, NULL },
  { close_converter, uc_wcstombs, NULL },
  { close_converter, uc_mbstowcs, NULL },
  { close_converter, uc_cstombs, NULL },
  { close_converter, uc_mbstocs, NULL },
  { close_converter, uc_wcstocts, NULL },
  { close_converter, uc_mbstocts, NULL },
  { close_converter, uc_ctstombs, NULL },
  { close_converter, uc_ctstowcs, NULL },
  { close_converter, uc_cstombs_for_ct, NULL },
  { close_converter, uc_cstowcs_for_ct, NULL },
  { close_converter, uc_wcstoutf8s, NULL },
  { close_converter, uc_mbstoutf8s, NULL },
  { close_converter, uc_utf8stombs, NULL },
  { close_converter, uc_utf8stowcs, NULL },
};



/*****************/
/* Program Codes */
/*****************/


/* loader function */

XLCd
_XlcUnicodeLoader(char *name)
{
  XLCd	lcd = (XLCd)NULL;
  uc_mb_info = (UnicodeMbstrInfo *)NULL;
  uc_wc_info = (UnicodeWcharInfo *)NULL;
  uc_cs_info = (UnicodeCsInfo *)NULL;

  _XlcInitPublicMethodsUnicode();
  lcd = _XlcCreateLC(name, _XlcGenericMethods);
  if (lcd == (XLCd)NULL)
    goto ERR_RETURN;

  /* For ko.euc and ko.UTF-8 data exchanging in cut&paste */
  /* Following four converters were originally made for ko.UTF-8
   * locale.  But to support interoperability between ko.UTF-8 
   * and other UTF-8 locales, these convertes are necessary.
   * I rewrote ctstombs/ctstowcs converter to work on other
   * UTF-8 locale such as en_US.UTF-8 and ja_JP.UTF-8.  mbstocts
   * and wcstocts converters work well without modification
   * even on non ko.UTF-8 locale
   */
  _XlcSetConverter(lcd, XlcNCompoundText, lcd, XlcNMultiByte, open_ctstombs);
  _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNCompoundText, open_mbstocts);
  _XlcSetConverter(lcd, XlcNCompoundText, lcd, XlcNWideChar, open_ctstowcs);
  _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCompoundText, open_wcstocts);

  _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNCharSet, open_wcstocs);
  _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNWideChar, open_cstowcs);
  _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNMultiByte, open_wcstombs);
  _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNWideChar, open_mbstowcs);
  _XlcSetConverter(lcd, XlcNCharSet, lcd, XlcNMultiByte, open_cstombs);
  _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNCharSet, open_mbstocs);

  _XlcSetConverter(lcd, XlcNUtf8String, lcd, XlcNMultiByte, open_utf8stombs);
  _XlcSetConverter(lcd, XlcNUtf8String, lcd, XlcNWideChar, open_utf8stowcs);
  _XlcSetConverter(lcd, XlcNMultiByte, lcd, XlcNUtf8String, open_mbstoutf8s);
  _XlcSetConverter(lcd, XlcNWideChar, lcd, XlcNUtf8String, open_wcstoutf8s);

  return lcd;

ERR_RETURN:
  if(uc_cs_info)
    UnicodeCsInfo_destroy(uc_cs_info);
  if(uc_mb_info)
    UnicodeMbstrInfo_destroy(uc_mb_info);
  if(uc_wc_info)
    UnicodeWcharInfo_destroy(uc_wc_info);
  if(lcd)
    _XlcDestroyLC(lcd);

  return (XLCd)NULL;
}


/* register functions */

static XlcConv
open_ctstombs(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[CTSTOMBS]);
}

static XlcConv
open_ctstowcs(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[CTSTOWCS]);
}

static XlcConv
open_mbstocts(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[MBSTOCTS]);
}

static XlcConv
open_wcstocts(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[WCSTOCTS]);
}

static XlcConv
open_wcstocs(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[WCSTOCS]);
}

static XlcConv
open_cstowcs(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[CSTOWCS]);
}

static XlcConv
open_wcstombs(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[WCSTOMBS]);
}

static XlcConv
open_mbstowcs(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[MBSTOWCS]);
}

static XlcConv
open_cstombs(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[CSTOMBS]);
}

static XlcConv
open_mbstocs(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[MBSTOCS]);
}


/*UTF8_STRING support - next 4 functions*/
static XlcConv
open_utf8stombs(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[UTF8STOMBS]);
}

static XlcConv
open_utf8stowcs(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[UTF8STOWCS]);
}

static XlcConv
open_mbstoutf8s(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[MBSTOUTF8S]);
}

static XlcConv
open_wcstoutf8s(XLCd from_lcd, char *from_type, XLCd to_lcd, char *to_type)
{
  return create_conv(from_lcd, &conv_methods[WCSTOUTF8S]);
}

/* converter functions */

/*
   ctstostr:
   	converts Compound Text string to multibyte string in case that
	to_type is XlcNMultiByte, or to wide char string in case that
	to_type is XlcNWideChar.
*/
static int
ctstostr(XLCd lcd, char *to_type, XPointer *from, int *from_left,
	 XPointer *to, int *to_left, XPointer *args, int num_args)
{
  XlcCharSet charset;
  XPointer tmp_args[1];
  XlcConv ctos_conv = _XlcOpenConverter(NULL, XlcNCompoundText,
					NULL, XlcNCharSet);
  XlcConv stostr_conv = (XlcConv)0;
  char buf[BUFSIZ], *cs;
  int cs_left, ret, length, unconv_num = 0;
  int from_left_save = *from_left;

  if (! strcmp(to_type, XlcNMultiByte)) {
    stostr_conv = create_conv(lcd, &conv_methods[CSTOMBS_FOR_CT]);
  } else if (! strcmp(to_type, XlcNWideChar)) {
    stostr_conv = create_conv(lcd, &conv_methods[CSTOWCS_FOR_CT]);
  }

  if (ctos_conv == 0)
    return (*from_left);
  if (stostr_conv == 0) {
    _XlcCloseConverter(ctos_conv);
    return (*from_left);
  }

  cs = buf;
  cs_left = BUFSIZ;
  tmp_args[0] = (XPointer) &charset;
  ret = _XlcConvert(ctos_conv, (XPointer *)from, from_left,
		    &cs, &cs_left, tmp_args, 1);
  if (ret < 0) {
    /*
     * The cttocs() at libX11 called at above returns -1 iff there was
     * any illegal character in the given compound text; the cttocs()
     * will not update any of from, from_left, cs, or cs_left in that
     * case. Thus, to prevent infinite loop, we must move forward
     * from and from_left values.
     */
    unconv_num = *from_left;
    *from_left = 0;
    *from += unconv_num;

    _XlcCloseConverter(ctos_conv);
    Xfree(stostr_conv);

    return unconv_num;
  }

  length = cs_left = cs - buf;
  cs = buf;
  tmp_args[0] = (XPointer) charset;
  ret = (*stostr_conv->methods->convert)(stostr_conv, &cs, &cs_left, to,
		to_left, tmp_args, 1);
  if (ret < 0) {
    unconv_num += length / charset->char_size;
  }
ERR:
  _XlcCloseConverter(ctos_conv);
  Xfree(stostr_conv);
  if (cs_left > 0) {
    *from_left = cs_left;
    unconv_num = cs_left;
  }
  return unconv_num;
}

#ifdef	NOT_BEING_USED
/*
   check_ascii:
   	checks if the Compound Text string designates ascii charset
*/
static
check_ascii(XPointer *from, int *from_left)
{
  if (!strncmp((char*)*from, "\033(B", 3) ||
      !strncmp((char*)*from, "\033-A", 3)) {
    *from += 3;
    *from_left -= 3;
    return True;
  }
  return False;
}

/*
   check_koreanCT:
   	checks if the Compound Text string designates EUC charset
*/
static
check_koreanCT(XPointer *from, int *from_left)
{
  if (!strncmp((char*)*from, "\033$(C", 4) ||
      !strncmp((char*)*from, "\033$)C", 4)) {
    *from += 4;
    *from_left -= 4;
    return True;
  }
  return False;
}

/*
   ascii_to_uc:
   	converts ascii character string to UTF-8 or UCS-4 encoding
*/
static int
ascii_to_uc(int type, XPointer *from, int *from_left,
	    XPointer *to, int *to_left)
{
  register char *src, *dst;
  int length, i;

  length = *from_left;
  src = *((char **) from);

  if (type == TO_MBS) {
    dst = *((char **) to);
    for (i = 0; i < length; i++) {
      if (*src == 0x1b || *src == NULL) break;
      *dst++ = *src++ & 0x7f;
    }
    *from_left -= i;
    *((char **)from) += i;
    *((char **) to) += i;
    *to_left -= i;
  } else {
    wchar_t *bufptr;
    char *outbuf;
    int wc_conv_len;
    outbuf = Xmalloc(length + 1);
    if (outbuf == NULL) return(*from_left);
    dst = outbuf;
    for (i = 0; i < length; i++) {
      if (*src == 0x1b || *src == NULL) break;
      *dst++ = *src++ & 0x7f;
    }
    bufptr = *((wchar_t **)to);
    wc_conv_len = mbstowcs(NULL, outbuf, dst - outbuf);
    if (wc_conv_len > *to_left) {
      /* overflow */
      *to_left = 0;
      return(*from_left);
    }
    if (wc_conv_len != mbstowcs(bufptr, outbuf, dst - outbuf))
      return(*from_left);
    *to_left -= wc_conv_len;
    Xfree(outbuf);
    *from_left -= i;
    *((char **)from) += i;
    *((char **) to) += i;
    *to_left -= i;
  }
  return (*from_left);
}

/*
   koreanCT_to_uc:
   	converts EUC-based Compound Text to UTF-8 or UCS-4 encoding
 */
static int
koreanCT_to_uc(int type, XPointer *from, int *from_left,
	       XPointer *to, int *to_left)
{
  iconv_t i_conv;
  size_t ret;
  int from_left_save, length;
  char *src;
  char *buf, *dst;
  int i;

  from_left_save = length = *from_left;

  if ((i_conv = iconv_open("ko_KR-UTF-8", "ko_KR-euc")) == (iconv_t)-1)
    return(*from_left);

  buf = Xmalloc(length + 1);
  if (buf == NULL) goto ERR;
  dst = buf;

  /* copy from buffer and turn MSB on */
  src = *((char **) from);
  for (i = 0; i < length; i++) {
    if (*src == 0x1b || *src == NULL) break;
    *dst++ = *src++ | 0x80;
  }
  *dst = 0;
  length = dst - buf;
  *from_left -= length;
  *((char **)from) += length;

  dst = buf;
  if (type == TO_MBS) {
    ret = iconv(i_conv, (const char **)&dst, (size_t*)&length,
		to, (size_t*)to_left);
  } else {
    wchar_t *bufptr;
    char *outbuf = NULL, *out;
    size_t buf_len, outbytes;
    int mb_conv_len, wc_conv_len;

    buf_len = *from_left * MB_CUR_MAX /2;
    outbuf = Xmalloc(buf_len);
    if (outbuf == NULL) goto ERR;

    out = outbuf;
    outbytes = buf_len;
    ret = iconv(i_conv, (const char **)&dst, (size_t*)&length,
		(char**)&out, &outbytes);

    bufptr = *((wchar_t **)to);
    mb_conv_len = buf_len - outbytes;
    wc_conv_len = mbstowcs(NULL, outbuf, mb_conv_len);
    if (wc_conv_len > *to_left) {
      /* overflow */
      *from_left = from_left_save;
      *to_left = 0;
      goto ERR;
    }
    if (wc_conv_len != mbstowcs(bufptr, outbuf, mb_conv_len))
      goto ERR;
    *to_left -= wc_conv_len;
    bufptr += wc_conv_len;
    Xfree(outbuf);
  }
ERR:
  Xfree(buf);
  iconv_close(i_conv);
  return (*from_left);
}
#endif	/* NOT_BEING_USED */


static int
convertToCT(XPointer from, int from_len, XPointer *to, int *to_left,
	    XlcCharSet charset, int csn)
{
  char *ct_sequence = charset->ct_sequence;
  int ct_sequence_len = strlen(ct_sequence);
  char *ctptr;
  int ext_segment_len;

  ctptr = *((char **)to);

  memcpy(ctptr, ct_sequence, ct_sequence_len);

  /*
   * If this is an extended segment, we need to put the number of octets
   * in this segement denoted as so-called 'M' and 'L' and also Start of
   * Text (STX) octet.
   */
  if (ct_sequence[1] == '%' && ct_sequence[2] == '/') {
    ext_segment_len = ct_sequence_len + from_len - 5;
    ctptr[4] = (unsigned char)((ext_segment_len / 128) | 0x80);
    ctptr[5] = (unsigned char)((ext_segment_len % 128) | 0x80);
    ctptr += ct_sequence_len;
    *ctptr++ = '\002';  /* STX */
  } else {
    ctptr += ct_sequence_len;
  }

  memcpy(ctptr, from, from_len);
  ctptr += from_len;

  *to_left -= ctptr - *((char **)to);
  *to = (XPointer)ctptr;

  return 0;
}

/*
   uc_mbstocts:
   	converter function from UTF-8 encoding to Compound Text.
	At Solaris 2.6, it is used only for ko.UTF-8 locale.

	It appears this function and convertToCT() have never been checking
	whether the output buffer left, i.e., "to" and "to_left", is big
	enough to hold the converted compound text. We might will need to do
	some form of checking in future to prevent any possible segmentation
	fault/bus error. - 2001/10/25 is@eng
*/
static int
uc_mbstocts(XlcConv conv, XPointer *from, int *from_left, XPointer *to,
	    int *to_left, XPointer *args, int num_args)
{
  XlcCharSet charset = NULL;
  XPointer tmp_args[3];
  const int tmp_num = 2;
  int tmp_to_left;
  XPointer tmp_to, tmp_to_begin, tmp_to_save;
  int csn;
  int ret;
  int unconv_num = 0;

  tmp_to = (XPointer)Xmalloc(*to_left * sizeof(char));
  if (tmp_to == NULL) return *from_left;
  tmp_to_save = tmp_to;

  tmp_to_left = *to_left;

  while(*from_left > 0 && *to_left > 0) {
    tmp_args[0] = (XPointer) &charset;
    tmp_args[1] = (XPointer) &csn;
    tmp_to_begin = tmp_to;
    ret = uc_mbstocs_for_ct(conv, from, from_left, &tmp_to, &tmp_to_left,
		     tmp_args, tmp_num);
    if (ret == -1) break;
    if (convertToCT(tmp_to_begin, tmp_to - tmp_to_begin,
		    to, to_left, charset, csn) < 0) break;
  }
  Xfree(tmp_to_save);

  return unconv_num;
}

/*
   uc_wcstocts:
   	converter function from UCS-4 encoding to Compound Text.
	At Solaris 2.6, it is used only for ko.UTF-8 locale.
*/
static int
uc_wcstocts(XlcConv conv, XPointer *from, int *from_left, XPointer *to,
	    int *to_left, XPointer *args, int num_args)
{
  wchar_t *src;
  char *buf, *dst;
  int buflen, mb_len;
  int wc_length;
  int ret;

  src = *((wchar_t **)from);
  wc_length = *from_left;
  mb_len = wcstombs(NULL, src, wc_length);
  buf = Xmalloc(mb_len);
  if (buf == NULL) return(*from_left);
  if (mb_len != wcstombs(buf, src, mb_len)) goto err;

  dst = buf;
  ret = uc_mbstocts(conv, (char**)&dst, (int*)&mb_len, to, to_left, args,
		    num_args);
  if (ret != 0) goto err;

  *((wchar_t **)from) += wc_length;
  *from_left -= wc_length;
 err:
  Xfree(buf);
  return (*from_left);
}

/*
   uc_ctstombs:
   	converter function from Compound Text to UTF-8 encoding.
	At Solaris 2.6, it is used only for ko.UTF-8 locale.
*/
static int
uc_ctstombs(XlcConv conv, XPointer *from, int *from_left, XPointer *to,
	    int *to_left, XPointer *args, int num_args)
{
  XLCd lcd = (XLCd)conv->state;
  int unconv_num = 0;
  int left, tmp_left;

  while (*from_left && **from && *to_left) {
    if (*from_left >= 6 && **from == 0x1b && from[0][1] == '%' &&
	from[0][2] == '/') {
      /*
       * If this is an extended segment CT with a reasonable length,
       * we rely on the given M and L values for the length of the
       * this CT segment.
       */
      left = 6 + (((unsigned int)((unsigned)(from[0][4]) & 0x7f) << 8) |
		(unsigned int)((unsigned)(from[0][5]) & 0x7f));
      left = min(left, *from_left);
    } else {
      /*
       * Otherwise, we calculate the length of this CT segment by
       * going through the octets and hoping for the best. We don't care
       * about the "directionality" indication control sequences yet.
       *
       * All possible bogus CTs will also flow down to here.
       */
      for (left = (**from == 0x1b) ? 1 : 0; left < *from_left; left++) {
        if(from[0][left] == 0x1b || from[0][left] == 0)
	  break;
      }
    }

    tmp_left = left;

    unconv_num += ctstostr(lcd, XlcNMultiByte, from, &tmp_left, to, to_left,
				args, num_args);

    *from_left -= (left - tmp_left);
  }

  return unconv_num;
}

/*
   uc_ctstowcs:
   	converter function from Compound Text to UCS-4 encoding.
	At Solaris 2.6, it is used only for ko.UTF-8 locale.
*/
static int
uc_ctstowcs(XlcConv conv, XPointer *from, int *from_left, XPointer *to,
	    int *to_left, XPointer *args, int num_args)
{
  XLCd lcd = (XLCd)conv->state;
  int unconv_num = 0;
  int left, tmp_left;

  while (*from_left && **from && *to_left) {
    /* Calculate the length of this CT segment. */
    for (left = (**from == 0x1b) ? 1 : 0; left < *from_left; left++) {
      if(from[0][left] == 0x1b || from[0][left] == NULL)
	break;
    }
    tmp_left = left;

    unconv_num += ctstostr(lcd, XlcNWideChar, from, from_left, to, to_left,
				args, num_args);

    *from_left -= (left - tmp_left);
  }
  return unconv_num;
}

static int
uc_wcstocs(XlcConv conv, XPointer *from, int *from_left, XPointer *to, int *to_left, XPointer *args, int num_args)
{
  int	status;
  CS_ENTRY	csN;

  if (uc_wc_info == NULL)
    uc_wc_info = UnicodeWcharInfo_create((XLCd)conv->state);
  if (uc_cs_info == NULL)
    uc_cs_info = UnicodeCsInfo_create((XLCd)conv->state);
  if (uc_wc_info == (UnicodeWcharInfo *)NULL ||
      uc_cs_info == (UnicodeCsInfo *)NULL)
    return -1;

  if (uc_wc_info->tables == (void *)NULL)
    return -1;
  if(uc_wc_info->cs_conv_func == NULL)
    return -1;

  status = uc_wc_info->cs_conv_func(uc_wc_info->tables, from, from_left, to, to_left, &csN);
  if((status >= 0) && (num_args > 0)){
    if(csN != LUT_CODESET_UNKNOWN)
      *((XlcCharSet *)args[0]) = uc_cs_info->cs[csN].charset;
  }
  return status;
}

static int
uc_mbstocs(XlcConv conv, XPointer *from, int *from_left, XPointer *to, int *to_left, XPointer *args, int num_args)
{
  int	status;
  CS_ENTRY	csN;

  if (uc_wc_info == NULL)
    uc_wc_info = UnicodeWcharInfo_create((XLCd)conv->state);
  if (uc_mb_info == NULL)
    uc_mb_info = UnicodeMbstrInfo_create((XLCd)conv->state);
  if (uc_cs_info == NULL)
    uc_cs_info = UnicodeCsInfo_create((XLCd)conv->state);
  if (uc_wc_info == (UnicodeWcharInfo *)NULL ||
      uc_mb_info == (UnicodeMbstrInfo *)NULL ||
      uc_cs_info == (UnicodeCsInfo *)NULL)
    return -1;

  status = convert_mbstocs(uc_wc_info->tables, from, from_left, to, to_left, &csN);

  if((status >= 0) && (num_args > 0)){
    if(csN != LUT_CODESET_UNKNOWN)
      *((XlcCharSet *)args[0]) = uc_cs_info->cs[csN].charset;
  }
  if (status > 0) return -1; /* ignore unconv num */
  return status;
}

static int
uc_mbstocs_for_ct(XlcConv conv, XPointer *from, int *from_left, XPointer *to,
			int *to_left, XPointer *args, int num_args)
{
  int status;
  CS_ENTRY csN;

  if (uc_wc_info == NULL)
    uc_wc_info = UnicodeWcharInfo_create((XLCd)conv->state);
  if (uc_mb_info == NULL)
    uc_mb_info = UnicodeMbstrInfo_create((XLCd)conv->state);
  if (uc_cs_info == NULL)
    uc_cs_info = UnicodeCsInfo_create((XLCd)conv->state);
  if (uc_wc_info == (UnicodeWcharInfo *)NULL ||
      uc_mb_info == (UnicodeMbstrInfo *)NULL ||
      uc_cs_info == (UnicodeCsInfo *)NULL)
    return -1;

  status = convert_mbstocs_for_ct(uc_wc_info->tables, from, from_left,
					to, to_left, &csN);

  if (status >= 0 && num_args > 0 && csN != LUT_CODESET_UNKNOWN) {
    *((XlcCharSet *)args[0]) = uc_cs_info->cs[csN].charset;
    *((int *)args[1]) = (int)csN;
  }

  if (status > 0) return -1; /* ignore unconv num */

  return status;
}

static int
uc_cstowcs(XlcConv conv, XPointer *from, int *from_left, XPointer *to, int *to_left, XPointer *args, int num_args)
{
  int	i, j, k;
  void	*table = (void *)NULL;
  CS_ENTRY	dummy;

  if (num_args < 1)
    return -1;

  if (uc_cs_info == NULL)
    uc_cs_info = UnicodeCsInfo_create((XLCd)conv->state);
  if (uc_cs_info == (UnicodeCsInfo *)NULL)
    return -1;

  for(i = 0, k = 0; k < uc_cs_info->cs_num; i++){
    for(j = 0; j < uc_cs_info->cs[i].codeset->num_charsets; j++, k++){
      if(!strcmp(((XlcCharSet)(args[0]))->name, 
		 uc_cs_info->cs[i].codeset->charset_list[j]->name)){
	table = uc_cs_info->cs[i].table;
	goto finish_loop;
      }
    }
  }
 finish_loop:
  if(table == (void *)NULL)
    return -1;

  if(uc_cs_info->cs[i].wc_conv_func)
    return uc_cs_info->cs[i].wc_conv_func(table, from, from_left, to, to_left, &dummy);
  else
    return -1;
}

static int
uc_cstowcs_for_ct(XlcConv conv, XPointer *from, int *from_left, XPointer *to,
		int *to_left, XPointer *args, int num_args)
{
  int	i, j, k;
  void	*table = (void *)NULL;
  CS_ENTRY	dummy;

  if (num_args < 1)
    return -1;

  if (uc_cs_info == NULL)
    uc_cs_info = UnicodeCsInfo_create((XLCd)conv->state);
  if (uc_cs_info == (UnicodeCsInfo *)NULL)
    return -1;

  for (i = 0, k = 0; k < uc_cs_info->cs_num; i++) {
    for (j = 0; j < uc_cs_info->cs[i].codeset->num_charsets; j++, k++) {
      if (!strcmp(((XlcCharSet)(args[0]))->name, 
		 uc_cs_info->cs[i].codeset->charset_list[j]->name)) {
	table = uc_cs_info->cs[i].table;
	goto FINISH_LOOP;
      }
    }
  }

FINISH_LOOP:
  if (table == (void *)NULL)
    return -1;

  if (i == uc_cs_info->fallback_cs) {
    CARD8 *frombyte = *((CARD8 **)from);
    CARD8 *tobyte = *((CARD8 **)to);

    while ((*from_left > 3) && (*to_left > 0)) {
      *tobyte++ = *frombyte++;
      *tobyte++ = *frombyte++;
      *tobyte++ = *frombyte++;
      *tobyte++ = *frombyte++;
      (*from_left) -= 4;
      (*to_left)--;
    }

    *from = (XPointer)frombyte;
    *to = (XPointer)tobyte;

    return (0);
  } else if (uc_cs_info->cs[i].wc_conv_func)
    return (uc_cs_info->cs[i].wc_conv_func(table, from, from_left, to,
						to_left, &dummy));

  return (-1);
}

static int
uc_cstombs(XlcConv conv, XPointer *from, int *from_left, XPointer *to, int *to_left, XPointer *args, int num_args)
{
  CS_ENTRY	i, k, dummy;
  int		j;
  void	*table = (void *)NULL;

  if (uc_wc_info == NULL)
    uc_wc_info = UnicodeWcharInfo_create((XLCd)conv->state);
  if (uc_mb_info == NULL)
    uc_mb_info = UnicodeMbstrInfo_create((XLCd)conv->state);
  if (uc_cs_info == NULL)
    uc_cs_info = UnicodeCsInfo_create((XLCd)conv->state);
  if (uc_wc_info == (UnicodeWcharInfo *)NULL ||
      uc_mb_info == (UnicodeMbstrInfo *)NULL ||
      uc_cs_info == (UnicodeCsInfo *)NULL)
    return -1;

  for(i = 0, k = 0; k < uc_cs_info->cs_num; i++){
    for(j = 0; j < uc_cs_info->cs[i].codeset->num_charsets; j++, k++){
      if(!strcmp(((XlcCharSet)(args[0]))->name, 
		 uc_cs_info->cs[i].codeset->charset_list[j]->name)){
	table = uc_cs_info->cs[i].table;
	goto finish_loop;
      }
    }
  }
 finish_loop:
  if(table == (void *)NULL)
    return -1;

  return convert_cstombs(table, from, from_left, to, to_left, &i, False);
}

static int
uc_cstombs_for_ct(XlcConv conv, XPointer *from, int *from_left, XPointer *to,
			int *to_left, XPointer *args, int num_args)
{
  CS_ENTRY	i, k, dummy;
  int		j;
  void	*table = (void *)NULL;

  if (uc_wc_info == NULL)
    uc_wc_info = UnicodeWcharInfo_create((XLCd)conv->state);
  if (uc_mb_info == NULL)
    uc_mb_info = UnicodeMbstrInfo_create((XLCd)conv->state);
  if (uc_cs_info == NULL)
    uc_cs_info = UnicodeCsInfo_create((XLCd)conv->state);
  if (uc_wc_info == (UnicodeWcharInfo *)NULL ||
      uc_mb_info == (UnicodeMbstrInfo *)NULL ||
      uc_cs_info == (UnicodeCsInfo *)NULL)
    return -1;

  for (i = 0, k = 0; k < uc_cs_info->cs_num; i++) {
    for (j = 0; j < uc_cs_info->cs[i].codeset->num_charsets; j++, k++) {
      if (!strcmp(((XlcCharSet)(args[0]))->name, 
		 uc_cs_info->cs[i].codeset->charset_list[j]->name)) {
	table = uc_cs_info->cs[i].table;
	goto FINISH_LOOP;
      }
    }
  }

FINISH_LOOP:
  if (table == (void *)NULL)
    return -1;

  return convert_cstombs(table, from, from_left, to, to_left, &i, True);
}

static int
uc_wcstombs(XlcConv conv, XPointer *from, int *from_left, XPointer *to, int *to_left, XPointer *args, int num_args)
{
  if (uc_wc_info == NULL)
    uc_wc_info = UnicodeWcharInfo_create((XLCd)conv->state);
  if (uc_mb_info == NULL)
    uc_mb_info = UnicodeMbstrInfo_create((XLCd)conv->state);
  return convert_wcstombs(from, from_left, to, to_left);
}

static int
uc_mbstowcs(XlcConv conv, XPointer *from, int *from_left, XPointer *to, int *to_left, XPointer *args, int num_args)
{
  if (uc_wc_info == NULL)
    uc_wc_info = UnicodeWcharInfo_create((XLCd)conv->state);
  if (uc_mb_info == NULL)
    uc_mb_info = UnicodeMbstrInfo_create((XLCd)conv->state);
  return convert_mbstowcs(from, from_left, to, to_left);
}


/*UTF8_STRING support - next 5 functions*/
static int uc_wcstoutf8s(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args) 
{
	return uc_wcstombs(conv, from, from_left, to, to_left, args, num_args);
}

static int uc_utf8stowcs(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args) 
{
	return uc_mbstowcs(conv, from, from_left, to, to_left, args, num_args);
}

static int copy(XlcConv conv, XPointer *from, int *from_left, XPointer *to,
    int *to_left, XPointer *args, int num_args)
{
    unsigned char const *src;
    unsigned char const *srcend;
    unsigned char *dst;
    unsigned char *dstend;

    if (from == NULL || *from == NULL)
		return 0;

    src = (unsigned char const *) *from;
    srcend = src + *from_left;
    dst = (unsigned char *) *to;
    dstend = dst + *to_left;

    while (src < srcend && dst < dstend)
		*dst++ = *src++;

    *from = (XPointer) src;
    *from_left = srcend - src;
    *to = (XPointer) dst;
    *to_left = dstend - dst;

    return 0;
}

static int uc_mbstoutf8s(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args) 
{
	return copy (conv, from, from_left, to, to_left, args, num_args);
}

static int uc_utf8stombs(XlcConv conv, XPointer *from, int *from_left,
			XPointer *to, int *to_left, XPointer *args,
			int num_args) 
{
	return copy (conv, from, from_left, to, to_left, args, num_args);
}

static void
close_converter(XlcConv conv)
{
  Xfree(conv);
}


/* local converters */

#ifdef	NOT_BEING_USED
static int
convert_wc16_to_cs(void *table, XPointer *from_wc, int *from_left,
		   XPointer *to_cs, int *to_left, CS_ENTRY *csN)
{
  CARD32	offset;
  int	csn;
  int	cs_length, entry_length;
  int	ret_num;

  CARD8	*to = *((CARD8 **)to_cs);

  wchar_t	*from = *((wchar_t **)from_wc);
  ret_num = 0;

  if (*from_left <= 0)
    return ret_num;

  while (*from_left > 0) {
    offset = *((CARD16*)table + ((*from >> 8) & 0xff));
    *csN = (int)(*((CARD8 *)table + offset * 256 + (*from & 0xff)));
    if (*csN != LUT_CODESET_UNKNOWN)
      break;
    from++;
    (*from_left)--;
    ret_num++;
  }
  if (*csN == LUT_CODESET_UNKNOWN) {
    *from_wc = (XPointer)from;
    return ret_num;
  }

  cs_length = uc_cs_info->cs[*csN].codeset->length;
  entry_length = uc_cs_info->max_length;
  if ((entry_length == 2) && (cs_length == 2)){
LOOP_POINT22:
    if (*to_left > 1) {
      CARD16	*tmp16;
      tmp16 = (CARD16 *)((CARD8 *)table + offset * 256 + 256) + (*from & 0xff);
      *to++ = *tmp16 >> 8;
      *to++ = *tmp16 & 0xff;
      from++;
      (*from_left)--;
      (*to_left) -= 2;
      while (*from_left > 0) {
	offset = *((CARD16 *)table + ((*from >> 8) & 0xff));
	csn = (int)(*((CARD8 *)table + offset * 256 + (*from & 0xff)));
	if (csn == *csN)
	  goto LOOP_POINT22;
	if (csn == LUT_CODESET_UNKNOWN) {
	  from++;
	  (*from_left)--;
	  ret_num++;
	  break;
	} else
	  break;
      }
    }
  } else if ((entry_length == 2) && (cs_length == 1)) {
LOOP_POINT21:
    if (*to_left > 0) {
      CARD16	*tmp16;
      tmp16 = (CARD16 *)((CARD8 *)table + offset * 256 + 256) + (*from++ & 0xff);
      *to++ = *tmp16 & 0xff;
      (*from_left)--;
      (*to_left)--;
      while (*from_left > 0) {
	offset = *((CARD16 *)table + (*from >> 8));
	csn = *((CARD8 *)table + offset * 256 + (*from & 0xff));
	if (csn == *csN)
	  goto LOOP_POINT21;
	if (csn == LUT_CODESET_UNKNOWN) {
	  from++;
	  (*from_left)--;
	  ret_num++;
	  break;
	} else
	  break;
      }
    }
  } else if((entry_length == 1) && (cs_length == 1)) {
LOOP_POINT11:
    if (*to_left > 0){
      *to++ = *((CARD8 *)table + offset * 256 + 256 + (*from++ & 0xff));
      (*from_left)--;
      (*to_left)--;
      while (*from_left > 0) {
	offset = *((CARD16 *)table + (*from >> 8));
	csn = *((CARD8 *)table + offset * 256 + (*from & 0xff));
	if (csn == *csN)
	  goto LOOP_POINT11;
	if (csn == LUT_CODESET_UNKNOWN) {
	  from++;
	  (*from_left)--;
	  ret_num++;
	  break;
	} else
	  break;
      }
    }
  } else {
    return -1;
  }

  *from_wc = (XPointer)from;
  *to_cs = (XPointer)to;

  return ret_num;
}
#endif	/* NOT_BEING_USED */


static int
convert_wc32_to_cs_vector_table(CARD8 **tables, XPointer *from_wc,
		int *from_left, XPointer *to_cs, int *to_left, CS_ENTRY *csN)
{
  wchar_t *from;
  CARD8 *to;
  into_octets_t o;
  CARD8 *t;
  CARD8 csn;
  int cs_length;
  int ret_num;

  if (*from_left <= 0)
    return (0);

  from = *((wchar_t **)from_wc);
  to = *((CARD8 **)to_cs);
  ret_num = 0;
  *csN = LUT_CODESET_UNKNOWN;

  while (*from_left > 0) {
    /*
     * If the character given is out of valid range of UTF-32, or,
     * invalid characters, i.e., tables[o.plane] == NULL, then we
     * skip such characters.
     */
    if (*from <= U8_MAX_VALUE_IN_UTF32) {
      o = *(into_octets_t *)from;
      if (tables[o.plane] != (CARD8 *)NULL) {
        t = tables[o.plane] + o.rowcell * 3;
        *csN = *t++;
        if (*csN != LUT_CODESET_UNKNOWN)
	  break;
      }
    }

    from++;
    (*from_left)--;
    ret_num++;
  }

  if (*csN == LUT_CODESET_UNKNOWN) {
    *from_wc = (XPointer)from;
    return (ret_num);
  }

  cs_length = uc_cs_info->cs[*csN].codeset->length;
  if (cs_length == 2) {
LOOP_POINT2:
    if (*to_left > 1) {
      /*
       * We cannot do something like the following:
       *
       *	*(CARD16 *)to = *(CARD16 *)(t + 1);
       *	to += 2;
       *
       * since SPARC SysV ABI won't allow it due to rigid restriction on
       * alignment and we don't want to change default alignement scheme
       * during compile time either.
       */
#ifdef	_BIG_ENDIAN
      *to++ = *t++;
      *to++ = *t;
#else
      *to++ = *(t + 1);
      *to++ = *t;
#endif	/* _BIG_ENDIAN */
      from++;
      (*from_left)--;
      (*to_left) -= 2;

      if (*from_left > 0) {
	csn = LUT_CODESET_UNKNOWN;
	if (*from <= U8_MAX_VALUE_IN_UTF32) {
	  o = *(into_octets_t *)from;
	  if (tables[o.plane] != (CARD8 *)NULL) {
	    t = tables[o.plane] + o.rowcell * 3;
            csn = *t++;
            if (csn == *csN)
              goto LOOP_POINT2;
	  }
	}

	/* Again, here we also skip if there was an invalid character. */
	if (csn == LUT_CODESET_UNKNOWN) {
	  from++;
	  (*from_left)--;
	  ret_num++;
	}
      }
    }
  } else if (cs_length == 1) {
LOOP_POINT1:
    if (*to_left > 0) {
      *to++ = *t;
      from++;
      (*from_left)--;
      (*to_left)--;

      if (*from_left > 0) {
	csn = LUT_CODESET_UNKNOWN;
	if (*from <= U8_MAX_VALUE_IN_UTF32) {
	  o = *(into_octets_t *)from;
	  if (tables[o.plane] != (CARD8 *)NULL) {
	    t = tables[o.plane] + o.rowcell * 3;
	    csn = *t++;
	    if (csn == *csN)
	      goto LOOP_POINT1;
	  }
	}

	/* Again, here we also skip if there was an invalid character. */
	if (csn == LUT_CODESET_UNKNOWN) {
	  from++;
	  (*from_left)--;
	  ret_num++;
	}
      }
    }
  } else {
    return (-1);
  }

  *from_wc = (XPointer)from;
  *to_cs = (XPointer)to;

  return (ret_num);
}

static int
convert_wc32_to_cs_vector_table_for_ct(CARD8 **tables, XPointer *from_wc,
		int *from_left, XPointer *to_cs, int *to_left, CS_ENTRY *csN)
{
  wchar_t *from;
  CARD8 *to;
  into_octets_t o;
  CARD8 *t;
  CARD8 csn;
  int cs_length;
  int ret_num;

  if (*from_left <= 0)
    return (0);

  from = *((wchar_t **)from_wc);
  to = *((CARD8 **)to_cs);
  ret_num = 0;
  *csN = LUT_CODESET_UNKNOWN;

  while (*from_left > 0) {
    /*
     * If the character given is out of valid range of UTF-32, or,
     * invalid characters, i.e., tables[o.plane] == NULL, then we
     * skip such characters.
     */
    if (*from <= U8_MAX_VALUE_IN_UTF32) {
      o = *(into_octets_t *)from;
      if (tables[o.plane] != (CARD8 *)NULL) {
        t = tables[o.plane] + o.rowcell * 3;
        *csN = *t++;
        if (*csN != LUT_CODESET_UNKNOWN)
	  break;
      }
    }

    from++;
    (*from_left)--;
    ret_num++;
  }

  if (*csN == LUT_CODESET_UNKNOWN) {
    *from_wc = (XPointer)from;
    return (ret_num);
  }

  /*
   * Is this the last, fallback font/charset? If so, we preserve
   * the UTF-32 values so that compound text with extended segment will
   * also have the UTF-32 values.
   */
  if (*csN == uc_cs_info->fallback_cs) {
LOOP_POINT4:
    if (*to_left > 3) {
      /*
       * We cannot do something like the following:
       *
       *	*(CARD32 *)to = *from;
       *	to += 4;
       *
       * since SPARC SysV ABI won't allow it due to rigid restriction on
       * alignment and we don't want to change default alignement scheme
       * during compile time either.
       */
      t = (CARD8 *)from;
#ifdef	_BIG_ENDIAN
      *to++ = *t++;
      *to++ = *t++;
      *to++ = *t++;
      *to++ = *t;
#else
      *to++ = *(t + 3);
      *to++ = *(t + 2);
      *to++ = *(t + 1);
      *to++ = *t;
#endif	/* _BIG_ENDIAN */
      from++;
      (*from_left)--;
      (*to_left) -= 4;

      if (*from_left > 0) {
        csn = LUT_CODESET_UNKNOWN;
        if (*from <= U8_MAX_VALUE_IN_UTF32) {
          o = *(into_octets_t *)from;
          if (tables[o.plane] != (CARD8 *)NULL) {
	    csn = *(tables[o.plane] + o.rowcell * 3);
            if (csn == *csN)
              goto LOOP_POINT4;
	  }
	}

	/* Again, here we also skip if there was an invalid character. */
	if (csn == LUT_CODESET_UNKNOWN) {
	  from++;
	  (*from_left)--;
	  ret_num++;
	}
      }
    }
  } else {
    cs_length = uc_cs_info->cs[*csN].codeset->length;
    if (cs_length == 2) {
LOOP_POINT2:
      if (*to_left > 1) {
#ifdef	_BIG_ENDIAN
        *to++ = *t++;
        *to++ = *t;
#else
        *to++ = *(t + 1);
        *to++ = *t;
#endif	/* _BIG_ENDIAN */
        from++;
        (*from_left)--;
        (*to_left) -= 2;

        if (*from_left > 0) {
          csn = LUT_CODESET_UNKNOWN;
          if (*from <= U8_MAX_VALUE_IN_UTF32) {
            o = *(into_octets_t *)from;
            if (tables[o.plane] != (CARD8 *)NULL) {
	      t = tables[o.plane] + o.rowcell * 3;
	      csn = *t++;
              if (csn == *csN)
                goto LOOP_POINT2;
	    }
	  }

	  /* Again, here we also skip if there was an invalid character. */
	  if (csn == LUT_CODESET_UNKNOWN) {
	    from++;
	    (*from_left)--;
	    ret_num++;
	  }
        }
      }
    } else if (cs_length == 1) {
LOOP_POINT1:
      if (*to_left > 0) {
        *to++ = *t;
        from++;
        (*from_left)--;
        (*to_left)--;

        if (*from_left > 0) {
	  csn = LUT_CODESET_UNKNOWN;
	  if (*from <= U8_MAX_VALUE_IN_UTF32) {
	    o = *(into_octets_t *)from;
	    if (tables[o.plane] != (CARD8 *)NULL) {
	      t = tables[o.plane] + o.rowcell * 3;
	      csn = *t++;
	      if (csn == *csN)
	        goto LOOP_POINT1;
	    }
	  }

	  /* Again, here we also skip if there was an invalid character. */
	  if (csn == LUT_CODESET_UNKNOWN) {
	    from++;
	    (*from_left)--;
	    ret_num++;
	  }
        }
      }
    } else {
      return (-1);
    }
  }

  *from_wc = (XPointer)from;
  *to_cs = (XPointer)to;

  return (ret_num);
}

#ifdef	NOT_BEING_USED
static int
convert_mb16_to_cs(void *table, XPointer *from_mb, int *from_left, XPointer *to_cs, int *to_left, CS_ENTRY *csN)
{
  CARD32	offset;
  int	csn;
  int	cs_length, entry_length;
  int	ret_num;

  char	*from = *((char **)from_mb);
  CARD8	*to = *((CARD8 **)to_cs);
  wchar_t	wc;
  int	len;

  ret_num = 0;

  while (*from_left > 0) {
    len = mbtowc(&wc, from, *from_left);
    if (len > 0) {
      offset = *((CARD16 *)table + ((wc >> 8) & 0xff));
      *csN = (int)(*((CARD8 *)table + offset * 256 + (wc & 0xff)));
      if (*csN != LUT_CODESET_UNKNOWN)
	break;
      from += len;
      (*from_left) -= len;
      ret_num += len;
    } else
      return (ret_num);
  }
  if(*csN == LUT_CODESET_UNKNOWN) {
    *from_mb = (XPointer)from;
    return (ret_num);
  }

  cs_length = uc_cs_info->cs[*csN].codeset->length;
  entry_length = uc_cs_info->max_length;
  if ((entry_length == 2) && (cs_length == 2)) {
LOOP_POINT22:
    if (*to_left > 1) {
      CARD16	*tmp16;
      tmp16 = (CARD16 *)((CARD8 *)table + offset * 256 + 256) + (wc & 0xff);
      *to++ = *tmp16 >> 8;
      *to++ = *tmp16 & 0xff;
      from += len;
      (*from_left) -= len;
      *to_left -= 2;
      while (*from_left > 0) {
	len = mbtowc(&wc, from, *from_left);
	if (len > 0) {
	  offset = *((CARD16 *)table + ((wc >> 8) & 0xff));
	  csn = (int)(*((CARD8 *)table + offset * 256 + (wc & 0xff)));
	  if(csn == *csN)
	    goto LOOP_POINT22;
	  if (csn == LUT_CODESET_UNKNOWN) {
	    from += len;
	    (*from_left) -= len;
	    ret_num += len;
	    break;
	  } else
	    break;
	} else
	  break;
      }
    }
  } else if ((entry_length == 2) && (cs_length == 1)) {
LOOP_POINT21:
    if (*to_left > 0) {
      CARD16	*tmp16;
      tmp16 = (CARD16 *)((CARD8 *)table + offset * 256 + 256) + (wc & 0xff);
      *to++ = *tmp16 & 0xff;
      from += len;
      (*from_left) -= len;
      (*to_left)--;
      while (*from_left > 0) {
	len = mbtowc(&wc, from, *from_left);
	if (len > 0) {
	  offset = *((CARD16 *)table + (wc >> 8));
	  csn = *((CARD8 *)table + offset * 256 + (wc & 0xff));
	  if(csn == *csN)
	    goto LOOP_POINT21;
	  if (csn == LUT_CODESET_UNKNOWN) {
	    from += len;
	    (*from_left) -= len;
	    ret_num += len;
	    break;
	  } else
	    break;
	} else
	  break;
      }
    }
  } else if ((entry_length == 1) && (cs_length == 1)) {
LOOP_POINT11:
    if (*to_left > 0) {
      *to++ = *((CARD8 *)table + offset * 256 + 256 + (wc & 0xff));
      from += len;
      (*from_left) -= len;
      (*to_left)--;
      while (*from_left > 0) {
	len = mbtowc(&wc, from, *from_left);
	if (len > 0) {
	  offset = *((CARD16 *)table + (wc >> 8));
	  csn = *((CARD8 *)table + offset * 256 + (wc & 0xff));
	  if(csn == *csN)
	    goto LOOP_POINT11;
	  if (csn == LUT_CODESET_UNKNOWN) {
	    from += len;
	    (*from_left) -= len;
	    ret_num += len;
	    break;
	  } else
	    break;
	} else
	  break;
      }
    }
  } else {
    return (-1);
  }

  *from_mb = (XPointer)from;
  *to_cs = (XPointer)to;

  return (0);
}
#endif	/* NOT_BEING_USED */


static int
convert_mb32_to_cs_vector_table(CARD8 **tables, XPointer *from_mb,
		int *from_left, XPointer *to_cs, int *to_left, CS_ENTRY *csN)
{
  char *from;
  CARD8 *to;
  into_octets_t o;
  CARD8 *t;
  CARD8 csn;
  int cs_length;
  int ret_num;
  wchar_t wc;
  int len;

  if (*from_left <= 0)
    return (0);

  from = *((char **)from_mb);
  to = *((CARD8 **)to_cs);
  ret_num = 0;
  *csN = LUT_CODESET_UNKNOWN;

  while (*from_left > 0) {
    /*
     * We skip all invalid characters as much as possible. If we do not
     * know the length of such invalid character(s), i.e., mbtowc()
     * returned -1, we skip byte by byte until a valid character.
     */
    len = mbtowc(&wc, from, *from_left);
    if (len > 0) {
      o = *(into_octets_t *)wc;
      if (tables[o.plane] != (CARD8 *)NULL) {
        t = tables[o.plane] + o.rowcell * 3;
        *csN = *t++;
        if (*csN != LUT_CODESET_UNKNOWN)
	  break;
      }
    } else
      len = 1;  /* Skip this byte with a hope the next one will be valid. */

    from += len;
    (*from_left) -= len;
    ret_num += len;
  }

  if (*csN == LUT_CODESET_UNKNOWN) {
    *from_mb = (XPointer)from;
    return (ret_num);
  }

  cs_length = uc_cs_info->cs[*csN].codeset->length;
  if (cs_length == 2) {
LOOP_POINT2:
    if (*to_left > 1) {
#ifdef	_BIG_ENDIAN
      *to++ = *t++;
      *to++ = *t;
#else
      *to++ = *(t + 1);
      *to++ = *t;
#endif	/* _BIG_ENDIAN */
      from += len;
      (*from_left) -= len;
      *to_left -= 2;

      if (*from_left > 0) {
	csn = LUT_CODESET_UNKNOWN;

	len = mbtowc(&wc, from, *from_left);
	if (len > 0) {
	  o = *(into_octets_t *)wc;
	  if (tables[o.plane] != (CARD8 *)NULL) {
	    t = tables[o.plane] + o.rowcell * 3;
	    csn = *t++;
	    if (csn == *csN)
	      goto LOOP_POINT2;
	  }
	} else
	  len = 1;

	/* Again, here we also skip if there was an invalid character. */
        if (csn == LUT_CODESET_UNKNOWN) {
          from += len;
          (*from_left) -= len;
          ret_num += len;
        }
      }
    }
  } else if (cs_length == 1) {
LOOP_POINT1:
    if (*to_left > 0) {
      *to++ = *t;
      from += len;
      (*from_left) -= len;
      (*to_left)--;

      if (*from_left > 0) {
	csn = LUT_CODESET_UNKNOWN;

	len = mbtowc(&wc, from, *from_left);
	if (len > 0) {
	  o = *(into_octets_t *)wc;
	  if (tables[o.plane] != (CARD8 *)NULL) {
	    t = tables[o.plane] + o.rowcell * 3;
	    csn = *t++;
	    if (csn == *csN)
	      goto LOOP_POINT1;
	  }
	} else
	  len = 1;

	/* Again, here we also skip if there was an invalid character. */
	if (csn == LUT_CODESET_UNKNOWN) {
	  from += len;
	  (*from_left) -= len;
	  ret_num += len;
	}
      }
    }
  } else {
    return (-1);
  }

  *from_mb = (XPointer)from;
  *to_cs = (XPointer)to;

  return (0);
}

#ifdef	NOT_BEING_USED
static int
convert_cs8_to_wc16(void *table, XPointer *from_cs, int *from_left, XPointer *to_wc, int *to_left, CS_ENTRY *csN)
{
  CARD8	*from = *((CARD8 **)from_cs);
  wchar_t	*to = *((wchar_t **)to_wc);

  while((*from_left > 0) && (*to_left > 0)){
    *((wchar_t *)to++) = (wchar_t)*((CARD16 *)table + *((CARD8 *)from++));
    (*from_left)--;
    (*to_left)--;
  }
	
  *from_cs = (XPointer)from;
  *to_wc = (XPointer)to;

  return 0;
}
#endif	/* NOT_BEING_USED */

static int
convert_cs8_to_wc32(void *table, XPointer *from_cs, int *from_left,
			XPointer *to_wc, int *to_left, CS_ENTRY *csN)
{
  CARD8 *from;
  wchar_t *to;

  from = *((CARD8 **)from_cs);
  to = *((wchar_t **)to_wc);

  while ((*from_left > 0) && (*to_left > 0)) {
    *((wchar_t *)to++) = (wchar_t)*((CARD32 *)table + *((CARD8 *)from++));
    (*from_left)--;
    (*to_left)--;
  }
	
  *from_cs = (XPointer)from;
  *to_wc = (XPointer)to;

  return (0);
}

static int
convert_wcstombs(XPointer *from, int *from_left, XPointer *to, int *to_left)
{
#ifdef USE_ICONV
  size_t ret, inbytes, outbytes;

#ifdef ICONVOPEN
  inbytes = *from_left * sizeof(wchar_t), outbytes = *to_left;
  ret = iconv(uc_mb_info->iconv_tomb,
	      (const char **)from, &inbytes, (char **)to, &outbytes);
  *from_left = inbytes / sizeof(wchar_t), *to_left = outbytes;
#else
  iconv_t i_conv;

  i_conv = iconv_open(uc_mb_info->iconv_name, uc_wc_info->iconv_name);

  if (i_conv == (iconv_t)-1)
    return -1;

  inbytes = *from_left * sizeof(wchar_t), outbytes = *to_left;
  ret = iconv(i_conv, (const char **)from, &inbytes, (char **)to, &outbytes);
  *from_left = inbytes / sizeof(wchar_t), *to_left = outbytes;

  iconv_close(i_conv);
#endif
  return (int)ret;

#else
  size_t	num;
  wchar_t	*buf;
	
  buf = (wchar_t *)malloc((*from_left + 1) * sizeof(wchar_t));
  if(buf == (wchar_t *)NULL)
    return -1;

  wcsncpy(buf, (wchar_t *)(*from), *from_left);
  buf[*from_left] = (wchar_t)0;

  num = wcstombs(*to, buf, *to_left);
  free(buf);

  if(num == (size_t)-1)
    return -1;
  else{
    int	c_num = 0;
    char	*ptr = (char *)*to;
    int	len = 0;
    while(len < num){
      int	c_len = mblen(ptr, num - len);
      if(c_len > 0){
	c_num++;
	ptr += c_len;
	len += c_len;
      }
      else
	return -1;
    }
    *from += c_num * sizeof(wchar_t);
    *from_left -= c_num;
    *to += num;
    *to_left -= num;
    return 0;
  }
#endif
}

static int
convert_mbstowcs(XPointer *from, int *from_left, XPointer *to, int *to_left)
{
#ifdef USE_ICONV
  iconv_t	i_conv;
  size_t	ret, inbytes, outbytes;

#ifdef ICONVOPEN
  inbytes = *from_left, outbytes = *to_left * sizeof(wchar_t);
  ret = iconv(uc_mb_info->iconv_towc,
	      (const char **)from, &inbytes, (char **)to, &outbytes);
  *from_left = inbytes, *to_left = outbytes / sizeof(wchar_t);
#else
  i_conv = iconv_open(uc_wc_info->iconv_name, uc_mb_info->iconv_name);
  if(i_conv == (iconv_t)-1)
    return -1;

  inbytes = *from_left, outbytes = *to_left * sizeof(wchar_t);
  ret = iconv(i_conv, (const char **)from, &inbytes, (char **)to, &outbytes);
  *from_left = inbytes, *to_left = outbytes / sizeof(wchar_t);

  iconv_close(i_conv);
#endif
  return (int)ret;
#else
  size_t	num;
  char	*buf;
	
  buf = (char *)malloc(*from_left + 1);
  if(buf == (char *)NULL)
    return -1;

  memcpy(buf, (char *)(*from), *from_left);
  buf[*from_left] = (char)0;

  num = mbstowcs((wchar_t *)(*to), buf, *to_left);
  free(buf);

  if(num == (size_t)-1)
    return -1;
  else{
    int	i;
    char	*ptr = (char *)(*from);
    int	len = 0;
    for(i = 0; i < num; i++){
      int	c_len = mblen(ptr, *from_left - len);
      if(c_len > 0){
	ptr += c_len;
	len += c_len;
      }
      else
	return -1;
    }
    *from += len; 
    *from_left -= len;
    *to += num * sizeof(wchar_t);
    *to_left -= num;
    return 0;
  }
#endif
}

static int
convert_cstombs(void *table, XPointer *from, int *from_left, XPointer *to,
			int *to_left, CS_ENTRY *csN, Bool is_CT)
{
  wchar_t	*wc_buf = (wchar_t *)NULL;
  int	wc_buf_size;
  XPointer	from1, from2, to1, to2;
  int		from_left1, from_left2, to_left1, to_left2;
  int		from_conv_size1, from_conv_size2, to_conv_size1;
  int		ret_num, ret;

  ret_num = 0;

  wc_buf_size = (*from_left) / (uc_cs_info->cs[*csN].codeset->length) *
			sizeof(wchar_t);
  wc_buf = (wchar_t *)Xmalloc(wc_buf_size);
  if (wc_buf == (wchar_t *)NULL)
    return (-1);

  from1 = *from, from_left1 = *from_left;
  to1 = (XPointer)(&(wc_buf[0])), to_left1 = wc_buf_size / sizeof(wchar_t); 
  if (is_CT && *csN == uc_cs_info->fallback_cs) {
    CARD8 *frombyte = (CARD8 *)from1;
    CARD8 *tobyte = (CARD8 *)to1;

    while (from_left1 > 3 && to_left1 > 0) {
      *tobyte++ = *frombyte++;
      *tobyte++ = *frombyte++;
      *tobyte++ = *frombyte++;
      *tobyte++ = *frombyte++;
      from_left1 -= 4;
      to_left1--;
    }

    from1 = (XPointer)frombyte;
    to1 = (XPointer)tobyte;

    ret = 0;
  } else if (uc_cs_info->cs[*csN].wc_conv_func) {
    ret = uc_cs_info->cs[*csN].wc_conv_func(table, &from1, &from_left1,
						&to1, &to_left1, csN);
  } else {
    Xfree(wc_buf);
    return (-1);
  }

  ret_num += ret;
  from_conv_size1 = *from_left - from_left1;
  to_conv_size1 = wc_buf_size / sizeof(wchar_t) - to_left1;

  from2 = (XPointer)(&(wc_buf[0])), from_left2 = to_conv_size1;
  to2 = *to, to_left2 = *to_left;
  ret = convert_wcstombs(&from2, &from_left2, &to2, &to_left2);
  if(ret > 0)
    ret_num += ret;

  if (from_left2 == 0) {
    *from = from1;
    *from_left = from_left1;
    *to = to2;
    *to_left = to_left2;
  } else if (from_conv_size1 / uc_cs_info->cs[*csN].codeset->length == to_conv_size1) {
    from_conv_size2 = (to_conv_size1 - from_left2) *
			uc_cs_info->cs[*csN].codeset->length;
    *from += from_conv_size2;
    *from_left -= from_conv_size2;
    *to = to2;
    *to_left = to_left2;
  } else {
    if (uc_cs_info->cs[*csN].mb_conv_func)
      ret_num = uc_cs_info->cs[*csN].mb_conv_func(table, from, from_left,
							to, to_left, csN);
    else
      ret_num = -1;
  }

  Xfree(wc_buf);
  return ret_num;
}

static int
convert_mbstocs(void *table, XPointer *from, int *from_left, XPointer *to,
			int *to_left, CS_ENTRY *csN)
{
  wchar_t	*wc_buf = (wchar_t *)NULL;
  int	wc_buf_size;
  XPointer	from1, from2, to1, to2;
  int		from_left1, from_left2, to_left1, to_left2;
  int		from_conv_size1, from_conv_size2, to_conv_size1;
  int		ret_num, ret1, ret2;

  ret_num = 0;

  wc_buf_size = (*to_left) * sizeof(wchar_t);
  wc_buf = (wchar_t *)Xmalloc(wc_buf_size);
  if (wc_buf == (wchar_t *)NULL)
    return (-1);

  from1 = *from, from_left1 = *from_left;
  to1 = (XPointer)(&(wc_buf[0])), to_left1 = wc_buf_size / sizeof(wchar_t); 
  ret1 = convert_mbstowcs(&from1, &from_left1, &to1, &to_left1);
  if (ret1 >= 0) {
    ret_num += ret1;
  } else {
    ret_num = -1;
    *csN = LUT_CODESET_UNKNOWN;
    goto TO_RETURN;
  }

  from_conv_size1 = *from_left - from_left1;
  to_conv_size1 = wc_buf_size / sizeof(wchar_t) - to_left1;

  from2 = (XPointer)(&(wc_buf[0])), from_left2 = to_conv_size1;
  to2 = *to, to_left2 = *to_left;
  if (uc_wc_info->cs_conv_func)
    ret2 = uc_wc_info->cs_conv_func(uc_wc_info->tables, &from2, &from_left2,
					&to2, &to_left2, csN);
  else {
    ret_num = -1;
    goto TO_RETURN;
  }

  if (ret2 > 0)
    ret_num += ret2;

  if (from_left2 == 0) {
    *from = from1;
    *from_left = from_left1;
    *to = to2;
    *to_left = to_left2;
    goto TO_RETURN;
  } else {
    ret_num = ret2;
    from1 = *from, from_left1 = *from_left;
    to1 = (XPointer)(&(wc_buf[0])), to_left1 = to_conv_size1 - from_left2; 
    ret1 = convert_mbstowcs(&from1, &from_left1, &to1, &to_left1);
    if (ret1 > 0)
      ret_num += ret1;
    if (to_left1 == 0) {
      *from = from1;
      *from_left = from_left1;
      *to = to2;
      *to_left = to_left2;
      goto TO_RETURN;
    }
  }

  if (uc_mb_info->cs_conv_func != NULL)
    ret_num = uc_mb_info->cs_conv_func(uc_wc_info->tables, from, from_left,
					to, to_left, csN);
  else
    ret_num = -1;

TO_RETURN:
  Xfree(wc_buf);
  return ret_num; 
}

static int
convert_mbstocs_for_ct(void *table, XPointer *from, int *from_left,
			XPointer *to, int *to_left, CS_ENTRY *csN)
{
  wchar_t	*wc_buf = (wchar_t *)NULL;
  int	wc_buf_size;
  XPointer	from1, from2, to1, to2;
  int		from_left1, from_left2, to_left1, to_left2;
  int		from_conv_size1, from_conv_size2, to_conv_size1;
  int		ret_num, ret1, ret2;

  ret_num = 0;

  wc_buf_size = (*to_left) * sizeof(wchar_t);
  wc_buf = (wchar_t *)Xmalloc(wc_buf_size);
  if (wc_buf == (wchar_t *)NULL)
    return (-1);

  from1 = *from, from_left1 = *from_left;
  to1 = (XPointer)(&(wc_buf[0])), to_left1 = wc_buf_size / sizeof(wchar_t); 
  ret1 = convert_mbstowcs(&from1, &from_left1, &to1, &to_left1);
  if (ret1 >= 0) {
    ret_num += ret1;
  } else {
    ret_num = -1;
    *csN = LUT_CODESET_UNKNOWN;
    goto TO_RETURN;
  }

  from_conv_size1 = *from_left - from_left1;
  to_conv_size1 = wc_buf_size / sizeof(wchar_t) - to_left1;

  from2 = (XPointer)(&(wc_buf[0])), from_left2 = to_conv_size1;
  to2 = *to, to_left2 = *to_left;
  ret2 = convert_wc32_to_cs_vector_table_for_ct(uc_wc_info->tables, &from2,
			&from_left2, &to2, &to_left2, csN);

  if (ret2 > 0)
    ret_num += ret2;

  if (from_left2 == 0) {
    *from = from1;
    *from_left = from_left1;
    *to = to2;
    *to_left = to_left2;
    goto TO_RETURN;
  } else {
    ret_num = ret2;
    from1 = *from, from_left1 = *from_left;
    to1 = (XPointer)(&(wc_buf[0])), to_left1 = to_conv_size1 - from_left2; 
    ret1 = convert_mbstowcs(&from1, &from_left1, &to1, &to_left1);
    if (ret1 > 0)
      ret_num += ret1;
    if (to_left1 == 0) {
      *from = from1;
      *from_left = from_left1;
      *to = to2;
      *to_left = to_left2;
      goto TO_RETURN;
    }
  }

  if (uc_mb_info->cs_conv_func != NULL)
    ret_num = uc_mb_info->cs_conv_func(uc_wc_info->tables, from, from_left,
					to, to_left, csN);
  else
    ret_num = -1;

TO_RETURN:
  Xfree(wc_buf);
  return ret_num; 
}

#ifdef	NOT_BEING_USED
static int
convert_cs8_to_mb16(void *table, XPointer *from_cs, int *from_left, XPointer *to_mb, int *to_left, CS_ENTRY *csN)
{
  CARD8	*from = *((CARD8 **)from_cs);
  CARD8	*to = *((CARD8 **)to_mb);
  wchar_t	wc;
  char	mb_buf[MB_LEN_MAX];
  int	len;
  int	ret_num;
  ret_num = 0;

  while((*from_left > 0) && (*to_left > 0)){
    wc = (wchar_t)*((CARD16 *)table + *((CARD8 *)from));
    len = wctomb(mb_buf, wc);
    if(*to_left >= len){
      from++;
      (*from_left)--;
      if(len > 0){
	strncpy((char *)to, mb_buf, len);
	to += len;
	(*to_left) -= len;
      }
      else{
	ret_num++;
      }
    }
    else
      break;
  }

  *from_cs = (XPointer)from;
  *to_mb = (XPointer)to;

  return ret_num;
}
#endif	/* NOT_BEING_USED */

static int
convert_cs8_to_mb32(void *table, XPointer *from_cs, int *from_left,
			XPointer *to_mb, int *to_left, CS_ENTRY *csN)
{
  CARD8 *from;
  CARD8 *to;
  wchar_t wc;
  char mb_buf[MB_LEN_MAX];
  char *mbp;
  int len;
  int ret_num;

  from = *((CARD8 **)from_cs);
  to = *((CARD8 **)to_mb);
  ret_num = 0;

  while ((*from_left > 0) && (*to_left > 0)) {
    wc = (wchar_t)*((CARD32 *)table + *((CARD8 *)from));
    len = wctomb(mb_buf, wc);
    if (*to_left >= len) {
      from++;
      (*from_left)--;
      if (len > 0) {
	(*to_left) -= len;
        mbp = mb_buf;
	do {
	  *to++ = *mbp++;
	} while (--len > 0);
      } else {
	ret_num++;
      }
    } else
      break;
  }

  *from_cs = (XPointer)from;
  *to_mb = (XPointer)to;

  return ret_num;
}

#ifdef	NOT_BEING_USED
static int
convert_cs16_to_wc16(void *table, XPointer *from_cs, int *from_left, XPointer *to_wc, int *to_left, CS_ENTRY *csN)
{
  CARD8	*from = *((CARD8 **)from_cs);
  wchar_t	*to = *((wchar_t **)to_wc);
  CARD32 offset;

  while((*from_left > 1) && (*to_left > 0)){
    offset = *((CARD16 *)table + *((CARD8 *)from++));
    *((wchar_t *)to++) = (wchar_t)*((CARD16 *)((CARD8 *)table + offset * 256 + *((CARD8 *)from++) * sizeof(CARD16)));
    (*from_left) = (*from_left) - 2;
    (*to_left)--;
  }

  *from_cs = (XPointer)from;
  *to_wc = (XPointer)to;

  return 0;
}
#endif	/* NOT_BEING_USED */

static int
convert_cs16_to_wc32(void *table, XPointer *from_cs, int *from_left,
		XPointer *to_wc, int *to_left, CS_ENTRY *csN)
{
  CARD8 *from;
  wchar_t *to;
  CARD32 offset;

  from = *((CARD8 **)from_cs);
  to = *((wchar_t **)to_wc);

  while ((*from_left > 1) && (*to_left > 0)) {
    offset = *((CARD16 *)table + *((CARD8 *)from++));
    *((wchar_t *)to++) = (wchar_t)*((CARD32 *)((CARD8 *)table + offset * 256 +
				*((CARD8 *)from++) * sizeof(CARD32)));
    (*from_left) = (*from_left) - 2;
    (*to_left)--;
  }

  *from_cs = (XPointer)from;
  *to_wc = (XPointer)to;

  return (0);
}

#ifdef	NOT_BEING_USED
static int
convert_cs16_to_mb16(void *table, XPointer *from_cs, int *from_left, XPointer *to_mb, int *to_left, CS_ENTRY *csN)
{
  CARD8	*from = *((CARD8 **)from_cs);
  CARD8	*to = *((CARD8 **)to_mb);
  CARD32 offset;
  wchar_t	wc;
  char	mb_buf[MB_LEN_MAX];
  int	len;
  int	ret_num;
 
  ret_num = 0;
  while((*from_left > 1) && (*to_left > 0)){
    offset = *((CARD16 *)table + *((CARD8 *)from));
    wc = (wchar_t)*((CARD16 *)((CARD8 *)table + offset * 256 + *((CARD8 *)from+1) * sizeof(CARD16)));
    len = wctomb(mb_buf, wc);
    if(*to_left >= len){
      (*from_left) -= 2;
      from += 2;
      if(len > 0){
	strncpy((char *)to, mb_buf, len);
	to += len;
	(*to_left) -= len;
      }
      else{
	ret_num++;
      }
    }
    else{
      break;
    }
  }

  *from_cs = (XPointer)from;
  *to_mb = (XPointer)to;

  return 0;
}
#endif	/* NOT_BEING_USED */

static int
convert_cs16_to_mb32(void *table, XPointer *from_cs, int *from_left,
		XPointer *to_mb, int *to_left, CS_ENTRY *csN)
{
  CARD8 *from;
  CARD8 *to;
  CARD32 offset;
  wchar_t wc;
  char mb_buf[MB_LEN_MAX];
  char *mbp;
  int len;
  int ret_num;
 
  from = *((CARD8 **)from_cs);
  to = *((CARD8 **)to_mb);
  ret_num = 0;

  while ((*from_left > 1) && (*to_left > 0)) {
    offset = *((CARD16 *)table + *((CARD8 *)from));
    wc = (wchar_t)*((CARD32 *)((CARD8 *)table + offset * 256 +
		*((CARD8 *)from + 1) * sizeof(CARD32)));
    len = wctomb(mb_buf, wc);
    if (*to_left >= len) {
      (*from_left) -= 2;
      from += 2;
      if (len > 0) {
	(*to_left) -= len;
        mbp = mb_buf;
	do {
	  *to++ = *mbp++;
	} while (--len > 0);
      } else {
	ret_num++;
      }
    } else {
      break;
    }
  }

  *from_cs = (XPointer)from;
  *to_mb = (XPointer)to;

  return 0;
}


/* constructors and destructors */

static UnicodeMbstrInfo *
UnicodeMbstrInfo_create(XLCd lcd)
{
  UnicodeMbstrInfo	*info;
  char	**str_list;
  int	num;

  info = (UnicodeMbstrInfo *)Xmalloc(sizeof(UnicodeMbstrInfo));
  if(info == (UnicodeMbstrInfo *)NULL)
    return (UnicodeMbstrInfo *)NULL;

  info->iconv_name = (char *)NULL;
  _XlcGetResource(lcd, "XLC_XLOCALE", MB_ICONV_NAME, &str_list, &num);
  if(num > 0){
    info->iconv_name = (char *)Xmalloc(strlen(str_list[0]) + 1);
    if(info->iconv_name)
      strcpy(info->iconv_name, str_list[0]);
  }

#if	defined(USE_ICONV) && defined(ICONVOPEN)
  /*
   * UTF-8 to UTF-32 conversion gives the BOM character at the very
   * beginning of the conversion to specify which byte ordering
   * the conversion output stream will be in; we need to skip that BOM
   * right now since we don't need it.
   *
   * If the iconv_open(3C) returns -1, however, we nullify the conversion
   * descriptor so that when the iconv(3C) at libc validates the conversion
   * descriptor, it will detect invalid descriptor and return -1.
   */
  info->iconv_towc = iconv_open(uc_wc_info->iconv_name, info->iconv_name);
  if (info->iconv_towc != (iconv_t)-1) {
    const char *ib = " ";
    size_t ibl = 1;
    wchar_t ob[2];
    wchar_t *obp = ob;
    size_t obl = 8;  /* 2 * sizeof(wchar_t) */

    (void)iconv(info->iconv_towc, &ib, &ibl, (char **)&obp, &obl);
  } else
    info->iconv_towc = (iconv_t)NULL;

  /*
   * However, we don't need to worry about the UTF-32 to UTF-8 conversion
   * case since, by default, if there is no BOM sequence, the iconv code
   * conversion will/should assume that the UTF-32 input stream is in
   * the current processor's byte ordering.
   */
  info->iconv_tomb = iconv_open(info->iconv_name, uc_wc_info->iconv_name);
  if (info->iconv_tomb == (iconv_t)-1)
    info->iconv_towc = (iconv_t)NULL;
#endif	/* defined(USE_ICONV) && defined(ICONVOPEN) */

  info->cs_conv_func = NULL;
  if(uc_wc_info->valid_len >= 3)
    info->cs_conv_func = convert_mb32_to_cs_vector_table;

  return info;
}

static void
UnicodeMbstrInfo_destroy(UnicodeMbstrInfo *info)
{
  if(info){
    if(info->iconv_name)
      Xfree(info->iconv_name);
    Xfree(info);
  }
}

static UnicodeWcharInfo *
UnicodeWcharInfo_create(XLCd lcd)
{
  UnicodeWcharInfo	*info;
  char	resource_name[256];
  char	filename[256];
  char	**str_list;
  int	num;
  int	i;

  info = (UnicodeWcharInfo *)Xmalloc(sizeof(UnicodeWcharInfo));
  if(info == (UnicodeWcharInfo *)NULL)
    return (UnicodeWcharInfo *)NULL;

  info->valid_len = 0;
  _XlcGetResource(lcd, "XLC_XLOCALE", WC_VALID_LENGTH_ENTRY, &str_list, &num);
  if(num > 0)
    info->valid_len = atoi(str_list[0]);

  info->iconv_name = (char *)NULL;
  _XlcGetResource(lcd, "XLC_XLOCALE", WC_ICONV_NAME, &str_list, &num);
  if(num > 0){
    info->iconv_name = (char *)Xmalloc(strlen(str_list[0]) + 1);
    if(info->iconv_name)
      strcpy(info->iconv_name, str_list[0]);
  }

  /*
   * We try to read all 17 planes of UTF-32 one by one. If there is no
   * conversion table provided, that means there is no character defined in
   * that particular plane. In that case, conversion functions, i.e.,
   * convert_wc32_to_cs_vector_table() and convert_mb32_to_cs_vector_table(),
   * will handle such cases accordingly.
   */
  for (i = 0; i < U8_MAX_PLANES; i++) {
    info->tables[i] = (CARD8 *)NULL;
    _XlcGetResource(lcd, "XLC_XLOCALE",
		(char *)WC_TO_CS_CONV_TABLE_ENTRY_PLANE[i], &str_list, &num);
    if (num > 0)
      info->tables[i] = read_table(lcd, str_list[0]);
  }

  info->cs_conv_func = NULL;
  if (info->valid_len >= 3)
    info->cs_conv_func = convert_wc32_to_cs_vector_table;

  return info;
}

static void
UnicodeWcharInfo_destroy(UnicodeWcharInfo *info)
{
  int i;

  if(info){
    if(info->iconv_name)
      Xfree(info->iconv_name);
    for (i = 0; i < U8_MAX_PLANES; i++)
      if (info->tables[i] != (CARD8 *)NULL)
	Xfree(info->tables[i]);
    Xfree(info);
  }
}

static UnicodeCsInfo *
UnicodeCsInfo_create(XLCd lcd)
{
  UnicodeCsInfo	*info;
  int	i, j;
  int	codeset_num;
  CodeSet	*codeset_list;
  int	count;
  char	resource_name[256];
  char	**str_list;
  int	num;
  char	filename[256];

  info = (UnicodeCsInfo *)Xmalloc(sizeof(UnicodeCsInfo));
  if (info == (UnicodeCsInfo *)NULL)
    return (UnicodeCsInfo *)NULL;

  info->cs_num = 0;
  codeset_num = XLC_GENERIC(lcd, codeset_num);
  codeset_list = XLC_GENERIC(lcd, codeset_list);
  for (i = 0; i < codeset_num; i++)
    info->cs_num += codeset_list[i]->num_charsets;

  info->cs = (UnicodeCs *)Xmalloc(sizeof(UnicodeCs) * info->cs_num);
  if (info->cs == (UnicodeCs *)NULL) {
    Xfree(info);
    return (UnicodeCsInfo *)NULL;
  }

  count = 0;
  info->max_length = 0;
  info->fallback_cs = -1;
  for (i = 0; i < codeset_num; i++) {
    sprintf(resource_name, "cs%d.%s", i, CS_TO_WC_CONV_TABLE_ENTRY);
    _XlcGetResource(lcd, "XLC_XLOCALE", resource_name, &str_list, &num);
    /* for multiple charset definitions in a csN
       for (j = 0; j < codeset_list[i]->num_charsets; j++){
    */
    for (j = 0; j < 1; j++){
      info->cs[count].codeset = codeset_list[i];
      info->cs[count].charset = codeset_list[i]->charset_list[j];

      if (info->max_length < info->cs[count].codeset->length)
	info->max_length = info->cs[count].codeset->length;

      info->cs[count].table = (void *)NULL;
      if (j < num)
	info->cs[count].table = read_table(lcd, str_list[j]);

      info->cs[count].wc_conv_func = NULL;
      info->cs[count].mb_conv_func = NULL;
      if (info->cs[count].table) {
	if (uc_wc_info == NULL) {
	  uc_wc_info = UnicodeWcharInfo_create((XLCd)lcd);
	  if (!uc_wc_info) {
	    return NULL; /* error */
	  }
	}
	if (uc_wc_info->valid_len >= 3) {
	  switch (codeset_list[i]->length) {
	  case 2:
	    info->cs[count].wc_conv_func = convert_cs16_to_wc32;
	    info->cs[count].mb_conv_func = convert_cs16_to_mb32;
	    break;
	  case 1:
	    info->cs[count].wc_conv_func = convert_cs8_to_wc32;
	    info->cs[count].mb_conv_func = convert_cs8_to_mb32;
	    break;
	  default:
	    break;
	  }
	}
      }

      if (info->cs[count].charset && info->cs[count].charset->encoding_name &&
          (! strcmp(info->cs[count].charset->encoding_name,
		U8_FALLBACK_ENCODING_NAME)))
	info->fallback_cs = count;

      count++;
    }
  }
  return info;
}

static void
UnicodeCsInfo_destroy(UnicodeCsInfo *info)
{
  int	i;

  if(info){
    if(info->cs){
      for(i = 0; i < info->cs_num; i++){
	if(info->cs[i].table)
	  Xfree(info->cs[i].table);
      }
      Xfree(info->cs);
    }
    Xfree(info);
  }
}


/* miscellaneous functions */

static XlcConv
create_conv(XLCd lcd, XlcConvMethods methods)
{
  XlcConv	conv;

  conv = (XlcConv)Xmalloc(sizeof(XlcConvRec));
  if(conv == (XlcConv)NULL)
    return (XlcConv)NULL;

  conv->methods = methods;
  conv->state = (XPointer) lcd;

  return conv;
}

static int
make_filename(char *str, char *filename, int name_len)
{
  char *dir_name;

  if (str == (char *)NULL)
    return -1;
  if (str[0] == '/'){
    if (strlen(str) >= PATH_MAX)
      return -1;
    strcpy(filename, str);
  } else {
    dir_name = _XlcLocaleDirName(setlocale(LC_CTYPE, (char *)NULL));
    if (dir_name == (char *)NULL)
      return -1;
    if (strlen(dir_name) + 1 + strlen(str) >= name_len)
      return -1;
    /* 
     * Possible buffer overflow has been checked at above; we can safely use
     * strcpy(3C) and strcat(3C).
     */
    strcpy(filename, dir_name);
    strcat(filename, "/");
    strcat(filename, str);
    Xfree(dir_name);
  }
  return 0;
}

/* Considering possible @modifier, 20 might be a bit too small; hence 64. */
#define NAME_LEN	64

static int
make_utf8_filename(XLCd lcd, char *str, char *filename, int name_len)
{
  char *language, *territory;
  char *codeset;
  char *dir_name;
  char new_locale[NAME_LEN];
  size_t len;

  _XGetLCValues(lcd, XlcNLanguage,  &language,
		XlcNTerritory, &territory,
		XlcNCodeset,   &codeset,
		(char*)NULL);

  /*
   * The strlcpy(3C) and strlcat(3C) consume more CPU power than
   * the following; furthemore, we need to check the return value from
   * strlcpy(3C) and strlcat(3C) functions to detect buffer overflow.
   */
  if (!language)
    return -1;
  /*
   * The "7" is for strlen(".UTF-8") + 1 and the "1" is for terminating
   * NULL character; see below for description on potential problem on
   * this assumption of "7" and ".UTF-8".
   */
  len = strlen(language) + 7;
  if (territory && *territory) {
    len += strlen(territory) + 1;  /* The "1" is for "_". */
    if (len >= NAME_LEN)  /* buffer overflow? */
      return -1;
    strcpy(new_locale, language);
    strcat(new_locale, "_");
    strcat(new_locale, territory);
  } else {
    if (len >= NAME_LEN)  /* buffer overflow? */
      return -1;
    strcpy(new_locale, language);
  }
  /*
   * The following could be a source of future error if the "codeset" name
   * contains @modifier like ".UTF-8@dictionaryorder". We will need to fix in
   * future if we create such locales.
   */
  strcat(new_locale, ".UTF-8");

  dir_name = _XlcLocaleDirName(new_locale);

  if (dir_name == (char *)NULL)
    return -1;
  if (strlen(dir_name) + 1 + strlen(str) >= name_len)
    return -1;
  /*
   * Possible buffer overflow error has been cleared by the above checking;
   * we can safely use now strcpy() and strcat() which are much faster than
   * strlcpy(3C) and strlcat(3C), respectively.
   */
  strcpy(filename, dir_name);
  strcat(filename, "/");
  strcat(filename, str);
  Xfree(dir_name);

  return 0;
}


static void *
read_table(XLCd lcd, char *str)
{
  char	filename[PATH_MAX];
  lookup_size_type_t size_type;
  void    *table;
  char    *src;
  int     fildes;

  /* try locale specific path 1st */
  if (make_filename(str, filename, PATH_MAX) == -1)
    return (void *)NULL;

  if ((fildes = open(filename, O_RDONLY)) == -1) {
    /* try default path */
    if (make_utf8_filename(lcd, str, filename, PATH_MAX) == -1)
      return (void *)NULL;

    if ((fildes = open(filename, O_RDONLY)) == -1) {
      /* try fallback path */
      /*
       * The following check will clear a possibility of buffer overflow
       * error; hence we can safely use strcpy(3C) and strcat(3C).
       * The "32" is strlen("/usr/openwin/lib/locale/common/") + 1.
       * The "1" is for terminating NULL byte.
       */
      if ((32 + strlen(str)) >= PATH_MAX)
	return (void *)NULL;
      strcpy(filename, "/usr/openwin/lib/locale/common/");
      strcat(filename, str);
      if ((fildes = open(filename, O_RDONLY)) == -1) {
	/*
	 * The following check will clear a possibility of buffer overflow
	 * error; hence we can safely use strcpy(3C) and strcat(3C).
         * The "37" is strlen("/usr/openwin/lib/locale/en_US.UTF-8/") + 1.
	 * The "1" is for terminating NULL byte.
	 */
        if ((37 + strlen(str)) >= PATH_MAX)
	  return (void *)NULL;
	strcpy(filename, "/usr/openwin/lib/locale/en_US.UTF-8/");
	strcat(filename, str);
	if ((fildes = open(filename, O_RDONLY)) == -1) {
	  return (void*)NULL;
	}
      }
    }
  }

  /*
   * Read size and type of the conv file. Unlike before, we no longer
   * need to multiply the size by 256 as the size what we are reading is
   * the actual size of the file minus sizeof(lookup_size_type_t).
   */
  if (read(fildes, &size_type, READ_TABLE_OFFSET) != READ_TABLE_OFFSET) {
    close(fildes);
    return (void*)NULL;
  }

  /*
   * The table type value in CARD8 must be less than or equal to
   * U8_TABLE_TYPE_MAX_VALUE; otherwise, it's an invalid table.
   */
  if (size_type.type > U8_TABLE_TYPE_MAX_VALUE) {
    close(fildes);
    return ((void *)NULL);
  }

  if ((table = (void *)Xmalloc(size_type.size)) == (void *)NULL) {
    close(fildes);
    return (void *)NULL;
  }
  if ((src = mmap(0, size_type.size + READ_TABLE_OFFSET, PROT_READ,
		  MAP_SHARED, fildes, 0)) == MAP_FAILED) {
    close(fildes);
    return (void *)NULL;
  }
  src += READ_TABLE_OFFSET;
  memcpy(table, src, size_type.size);
  munmap(src, size_type.size + READ_TABLE_OFFSET);
  close(fildes);

  return table;
}
