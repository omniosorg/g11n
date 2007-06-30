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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <encode.h>
#include "fsexam-history.h"

#define MAGICNUM 0xCDEF

#define VALID_CONVTYPE(v) (((v) == ConvName) || ((v) & ConvContent))
#define MATCH_CONVTYPE(v, w) ((v) == ConvName ? (w) == ConvName : \
                  ((w) == ConvContent || (w) == ConvContentWithCRLF || (w) == ConvContentCRLFOnly))

History_info *histinfo;

typedef struct _History_item
{
  guint32  serial;    // used by undo action for multi-selection, not persisted in disk
  ConvType convtype;

  guchar   *text;     // the old name
  guchar   *value;    // the new name
  guchar   *path;     // its directory name
}History_item;

/* defined to History_info in header file */
struct _History_info
{
  FILE  *fp;		// FILE stream pointer for ~/.fsexam.history
  GList *histlist;	// the list of History_item
};

static void 
release_each_item (gpointer data_ptr, gpointer p)
{
	History_item *h = (History_item *) data_ptr;

	g_free (h->text);
	g_free (h->value);
	g_free (h->path);

	g_free (h);
}

static void 
serialize_history_item (gpointer data_ptr, gpointer fp)
{
	FILE *stream = (FILE *) fp;
	History_item *h = (History_item *) data_ptr;

	if (VALID_CONVTYPE (h->convtype))
		fprintf (stream, "%d\27%s\27%s\27%s\n", h->convtype, h->text, h->value, h->path);
}

static GList*
deserialize_history_data (FILE* fp)
{
	GList *histlist = NULL;
	History_item *tmp = NULL;

	char linebuf[2048];
	
	if (!fp)
		return NULL;

	memset (linebuf, 0, sizeof(linebuf));

	while ( fgets (linebuf, sizeof(linebuf), fp) ) 
	{
		char *conv = NULL;
		char *text = NULL;
		char *value = NULL;
		char *path = NULL;

		char *last_char = NULL;

		if ( linebuf [sizeof(linebuf)-1] != '\0' &&
		     linebuf [sizeof(linebuf)-1] != '\n')
		{
			memset (linebuf, 0, sizeof(linebuf));
			continue;
		}

		linebuf [strlen(linebuf)-1] = '\0';

		conv = strtok_r (linebuf, "\27", &last_char);
		text = strtok_r (NULL, "\27", &last_char);
		value = strtok_r (NULL, "\27", &last_char);
		path = last_char;

		if (!conv || !text || !value || !path)
			continue;

		tmp = g_new0 (History_item, 1);

		tmp->serial = 0;
		tmp->convtype = atoi (conv);

		tmp->text = strdup (text);
		tmp->value = strdup (value);
		tmp->path = strdup (path);

		histlist = g_list_append (histlist, (gpointer) tmp);

	}

	return histlist;
}

static gboolean
serialize_history_data (GList *histlist, FILE* fp)
{
	if (!histlist || !fp)
		return FALSE;

	g_list_foreach (histlist, serialize_history_item, fp);

	return TRUE;
}

static unsigned int
get_serial (History_item *h,
            gboolean same_serial)
{
  unsigned int serial;

  if (!h) return 0;
  
  serial = h->serial;

  if (!same_serial)
    {
      if (serial == G_MAXUINT) serial = 0;
      else ++serial;
    } 

  return serial;
}

static History_item*
locate_with_serial (History_info *info, 
		    guint32 serial)
{
	GList *ptr = NULL;
	History_item *h = NULL;

	ptr = g_list_last (info->histlist);

	if (ptr)
		h = (History_item *) (ptr->data);
	else
		return NULL;

	while (1) {
		if (h->serial == serial ) return h;

		if (h->serial < serial) break;

		ptr = g_list_previous (ptr);

		if (ptr)
			h = (History_item *) (ptr->data);
		else
			break;
	}

	return NULL;
}


/*
 * put a history item in the info->histlist, and return info->serial
 */
unsigned int
fsexam_history_put (History_info *info, 
		    ConvType convtype,
		    char *text, 
		    char *value, 
		    char *path,
		    gboolean same_serial)
{
	g_assert (text && value && path);

	GList *ptr = NULL;
	History_item *prev = NULL;
	History_item *h = g_new0 (History_item, 1);
	
	h->convtype = convtype;

	h->text = g_strdup (text);
	h->value = g_strdup (value);
	h->path = g_strdup (path);

	ptr = g_list_last (info->histlist);
	if (ptr) prev = (History_item*) (ptr->data);

	h->serial = get_serial (prev, same_serial);

	info->histlist = g_list_append (info->histlist, (gpointer)h);
	
	return h->serial;
}

/*
 * Search backward with 'text' and 'path' to check whether any history
 * item can match them and return its 'value' if found.
 * Otherwise, return NULL.
 */
char *
fsexam_history_get_reverse (History_info *info, 
			    ConvType *convtype, 
			    char *text, 
			    char *path)
{
	GList *ptr = NULL;
	History_item *h = NULL;
	char *value = NULL;

	ptr = g_list_last (info->histlist);

	if (ptr) 
		h = (History_item*) (ptr->data);
	else
		return NULL;

	while (1) {
		if (VALID_CONVTYPE (h->convtype) &&
		    !strcmp (h->text, text) &&
		    !strcmp (h->path, path) ) 
		{
			*convtype = h->convtype;
			value = g_strdup (h->value);
			break;
		}

		ptr = g_list_previous (ptr);

		if (ptr)
			h = (History_item*) (ptr->data);
		else
			break;
	}

	return value;
}

/*
 * Search backward with 'value' and 'path' to check whether any history
 * item can match them with the same converion type and return its 
 * 'text' if found.
 * Otherwise, return NULL.
 */
char *
fsexam_history_get_reverse_by_value2 (History_info *info, 
				      ConvType *convtype, 
				      char *value, 
				      char *path)
{
	GList *ptr = NULL;
	History_item *h = NULL;
	char *text = NULL;

	ptr = g_list_last (info->histlist);

	if (ptr)
		h = (History_item*) (ptr->data);
	else
		return NULL;

	while (1) {
		if (MATCH_CONVTYPE (*convtype, h->convtype) &&
		    !strcmp (h->value, value) &&
		    !strcmp (h->path, path) ) 
		{
			*convtype = h->convtype;
			text = g_strdup (h->text);
			break;
		}

		ptr = g_list_previous (ptr);

		if (ptr)
			h = (History_item*) (ptr->data);
		else
			break;
	}

	return text;
}

/*
 * Search backward with 'value' and 'path' to check whether any history
 * item can match them and return its 'text' and the conversion type
 * if found.
 * Otherwise, return NULL.
 */
char *
fsexam_history_get_reverse_by_value (History_info *info, 
				     ConvType *convtype, 
				     char *value, 
				     char *path)
{
	GList *ptr = NULL;
	History_item *h = NULL;
	char *text = NULL;

	ptr = g_list_last (info->histlist);

	if (ptr)
		h = (History_item*) (ptr->data);
	else
		return NULL;

	while (1) {
		if (VALID_CONVTYPE(h->convtype) &&
		    !strcmp (h->value, value) &&
		    !strcmp (h->path, path) ) 
		{
			*convtype = h->convtype;
			text = g_strdup (h->text);
			break;
		}

		ptr = g_list_previous (ptr);

		if (ptr)
			h = (History_item*) (ptr->data);
		else
			break;
	}

	return text;
}

gboolean
fsexam_history_undo (History_info *info,
		     unsigned int serial,
		     ConvType *convtype, 
		     char **text, 
		     char **value, 
		     char **path)
{
	History_item *h = NULL;

	h = locate_with_serial (info, serial);

	if (!h) return FALSE;

	*convtype = h->convtype;
	*text = g_strdup (h->text);
	*value = g_strdup (h->value);
	*path = g_strdup (h->path);

	info->histlist = g_list_remove (info->histlist, (gconstpointer)h);

	return TRUE;
}

/*
 * If the file specified by filename exist and open properly, 
 * deserialize the data from file to list of History_items.
 */
History_info *
fsexam_history_open (char *filename)
{
	struct stat stat_buf;
	History_info *info = g_new0 (History_info, 1);

	if (stat (filename, &stat_buf) == -1)
		info->fp = fopen (filename, "w+b");
	else
		info->fp = fopen (filename, "r+b");

	info->histlist = deserialize_history_data (info->fp);
	
	return info;
}

/* Serialize list of History_items, and save to info->fp */
gboolean
fsexam_history_update (History_info *info)
{
	fseek (info->fp, SEEK_SET, 0);
	serialize_history_data (info->histlist, info->fp);
	return TRUE;
}

/* 
 * Serialize list of History_items, and save to info->fp.
 * At last, close info->fp, and release info->histlist.
 */
gboolean
fsexam_history_close (History_info *info)
{
	fseek (info->fp, SEEK_SET, 0);
	serialize_history_data (info->histlist, info->fp);
	fclose (info->fp);

	g_list_foreach (info->histlist, release_each_item, NULL);
	g_list_free (info->histlist);

	g_free (info);

	return TRUE;
}
