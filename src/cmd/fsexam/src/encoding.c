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
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */


#include <glib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include "encode.h"

GList *
init_encode (GSList *list)
{
  int      n = 0;
  GList    *encode_list = NULL;

  while (list)
    {
      GIConv icd;
      Encoding *encode;
      GSList *tmp_list = list;

      list = g_slist_next (list);

      if ((icd = g_iconv_open ("UTF-8", (char *)tmp_list->data)) == (GIConv)-1)
	continue;

      encode = g_new0 (Encoding, 1);
      encode->icd = icd;
      strcpy (encode->codename, (char *)tmp_list->data);
      encode->score = FAIL;
      encode->convtype = ConvName;

      encode_list = g_list_insert (encode_list, encode, n);

      ++n;
    }

  return encode_list;
}

void
destroy_encode (GList *encode_list)
{
  while (encode_list)
    {
      Encoding *encode = (Encoding *)encode_list->data;

      if (encode->convtype == ConvContent && encode->u.contents)
	g_free (encode->u.contents);
	
      g_iconv_close (encode->icd);
      g_free (encode);

      encode_list = g_list_next (encode_list);
    }

  g_list_free (encode_list);
}

/*
 * Only work for ConvName and ConvContent
 */
Score
decode_analyzer (GList *encode_list,
		 ConvType convtype,
		 gchar *text, 
		 size_t textlen)
{
  gboolean success = FAIL;

  if (g_utf8_validate (text, textlen, NULL)) return ORIGINAL;

  while (encode_list)
    {
      Encoding *encode = (Encoding *)encode_list->data;
      gchar    *inbuf = text;
      size_t   inbytes_left = textlen;
      gchar    *outbuf;
      size_t   outbytes_left;
      size_t   num_uconv = 0;

      encode->convtype = convtype;
      if (convtype == ConvName)
	{
	  outbuf = encode->u.converted_text;
	  outbytes_left = 256;
	}
      else /* ConvContent */
	{
	  outbytes_left = 3 * inbytes_left;
	  outbuf = encode->u.contents = g_new0 (char, outbytes_left);
	}

      memset (outbuf, 0, outbytes_left);
      num_uconv = g_iconv (encode->icd, &inbuf, &inbytes_left,
			   &outbuf, &outbytes_left);
      switch (num_uconv)
	{
	case 0:
	  encode->score = HIGH;
	  success = TRUE;
	  break;
	case (size_t)-1:
	  encode->score = FAIL;
	  break;
	default:
	  encode->score = LOW;
	  success = TRUE;
	  break;
	}

      encode_list = g_list_next (encode_list);
    }

  if (success) return HIGH;
  else return FAIL;
}

gboolean
get_encode_elements (Encoding *encode,
		     gint index,
		     va_list args)
{
  gint *n_elements;

  n_elements = va_arg (args, int *);
  (*n_elements)++;

  return TRUE;
}

void
iterate_encode_with_func (GList *encode_list,
			  EncodeFunc func, ...)
{
  va_list args;

  va_start (args, func);

  int encode_idx = -1;
  while (encode_list)
    {
      Encoding *encode = (Encoding *)encode_list->data;

      encode_list = g_list_next (encode_list);
      ++encode_idx;

      if (encode->score == FAIL) continue;

      if (!(*func)(encode, encode_idx, args))
	break;
    }

  va_end (args);
}

gboolean
translate_encode_index (Encoding *encode, 
			gint index,
			va_list args)
{
  gint *old_index = va_arg (args, int *);
  gint *new_index = va_arg (args, int *);

  if (*old_index)
    {
      --*old_index;
      return TRUE;
    }
  else
    {
      *new_index = index;
      return FALSE;
    }
}

int
get_first_encode_index (GList *encode_list)
{
  int encode_idx = -1, best_idx = -1;

  while (encode_list)
    {
      Encoding *encode = (Encoding *)encode_list->data;

      encode_list = g_list_next (encode_list);
      ++encode_idx;

      if (encode->score == HIGH) 
	{
	  // found it, the first encoding with score as HIGH
	  best_idx = encode_idx;
	  break;
	}
      else if (encode->score == LOW)
	{
	  // record the first encoding with score as LOW
	  if (best_idx == -1) 
	    best_idx = encode_idx;
	}
    }

  return best_idx;
}

void
cleanup_encode (GList *encode_list)
{
  while (encode_list)
    {
      Encoding *encode = (Encoding *)encode_list->data;

      if (encode->convtype == ConvContent)
	{
	  encode->convtype = ConvName;
	  if (encode->u.contents)
	    g_free (encode->u.contents);
	}

      encode_list = g_list_next (encode_list);
    }
}
