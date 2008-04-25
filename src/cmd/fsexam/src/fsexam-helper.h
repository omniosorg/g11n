/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/*
 * Copyright 2008 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms.
 */


#ifndef _FSEXAM_HELPER_H_
#define _FSEXAM_HELPER_H_

#define FREE(p) if (p) {        \
                    free (p); \
                    p = NULL;   \
                }

/*=========================================================================
 *  Function Name:  str_isascii
 *
 *  Parameters:
 *
 *  Desc:
 *      Determine whether the parameter str contains only ascii characters.
 *
 *  Return value:
 *      Return True if str contain only ASCII
 *      Otherwise False
 *
 *  Author:     Yandong Yao 2006/08/24
 ========================================================================*/
gboolean str_isascii (const gchar *str, gint len);

/*============================================================================
 *  Function Name:  get_abs_path
 *
 *  Parameters:
 *      const gchar *file: the path
 *
 *  Desc:
 *      Convert relative path to absolute path, eliminate all '.', '..' 
 *      in the path
 *
 *  Return value:
 *      If the file exist, then return its absolute path
 *      if not exist or error occur, then return NULL.
 *
 *      As we can't determine whether the path is abs path already unless 
 *      traverse the whole string, so we always return a new string which 
 *      you need free when don't use them any more.
 *
 *  Author:     Yandong Yao 2006/08/24
 ============================================================================*/
gchar * get_abs_path (const gchar *file);

/*============================================================================
 *  Function Name:  file_exist
 *
 *  Parameters:
 *
 *  Desc:
 *      Determine whether file exist and S_ISREG || s_ISDIR || S_ISLNK 
 *
 *  Return value:
 *      Return True if file exist and S_ISREG || s_ISDIR || S_ISLNK 
 *      Otherwise return False.
 *
 *  Author:     Yandong Yao 2006/08/24
 ===========================================================================*/
gboolean file_exist (gchar *file);

/*============================================================================
 *  Function Name:  str_chomp
 *
 *  Parameters:
 *
 *  Desc:   
 *      Remove trailing newline '\n'
 *      Remove heading space
 *
 *  Return value:
 *      Return the original string, won't allocate any new memory
 *
 *  Author:     Yandong Yao 2006/08/24
 ===========================================================================*/
gchar * str_chomp (gchar *str);

/*====================================================================
 *  Function Name:  str_compress
 *
 *  Parameters:
 *      gchar *string:  the original string, null terminated.
 *      gboolean *tab:  whether contain '\t' before the first non-space char
 *
 *  Desc:
 *      str_compress() function will remove any heading or tailing space.
 *      The modification is in-place, so it will change the original string.
 *
 *      if string contain '\t' before first non-space char, then *tab = TRUE;
 *
 *  Return value:
 *      The new start position in original string
 *
 *  Exception:
 *
 *  Author:     Yandong Yao 2006/08/30
 ========================================================================*/ 
gchar *str_compress (gchar *string, gboolean *tab);

/*====================================================================
 *  Function Name:  str_split
 *
 *  Parameters:
 *      gchar *str: The original string
 *      gchar **str1: pointer to the first string
 *      gchar **str2: pointer to the second string
 *
 *  Desc:
 *      Split the original string into two separate string according to space 
 *      chars. If there is no space,the *str2 = NULL;
 *
 *  Return value:
 *      N/A
 *
 *  Author:     Yandong Yao 2006/08/31
 ========================================================================*/ 
void str_split (gchar *str, gchar **str1, gchar **str2);

/*===================================================================
 *  Function Name:  list_free
 *
 *  Parameters:
 *      gpointer data:  the data stored in GList or GSList
 *      gpointer user_data: arbitrary user data
 *
 *  Desc:
 *      This function is just a wrap of g_free, so that it can be
 *      used as g_list_foreach to free the glist data memory
 *
 *  Return value:
 *
 *  Author:     Yandong Yao 2006/10/17
 ======================================================================*/
void list_free (gpointer data, gpointer user_data);

void list_print (gpointer data, gpointer user_data);

/*===================================================================
 *  Function Name:  encoding_string_parse
 *
 *  Parameters:
 *      const gchar *encoding_string: one string contain encoding name 
 *      separated by ',' or ':'
 *
 *  Desc:
 *      Split ',' or ':' separated encoding string into encoding list
 *
 *  Return value:
 *      GSList *, need free when don't use at more
 *
 *  Author:     Yandong Yao 2006/11/15
  ==========================================================================*/
GSList *encoding_string_parse (const gchar *encoding_string);

gchar *get_abs_path_for_symlink_target (const gchar *symlink);

void hash_print (gpointer key, gpointer value, gpointer data);


void fsexam_list_free (GList *list);
void fsexam_slist_free (GSList *list);
void fsexam_hash_remove_all (GHashTable *hash);

gchar   *get_sample_text (const gchar *contents, guint *length);

GList   *remove_string_from_list (GList *list, const gchar *str);
GSList  *remove_string_from_slist (GSList *list, const gchar *str);

gboolean fsexam_is_subpath (const gchar *path1, const gchar *path2);

/* 
 * get the display name
 */
gchar   *fsexam_filename_display_name (const gchar *filename);

/*
 * get the display name: this func will anaylize '/'
 */
gchar   *fsexam_filename_display_basename (const gchar *filename);

/*
 * escape string
 */
gchar   *fsexam_string_escape (const gchar *string);

gchar   *fsexam_print_hex (const gchar *str);

#ifndef HAVE_STRLCPY
size_t strlcpy (char *dst, const char *src, size_t len);
#endif

#ifndef HAVE_STRLCAT
size_t strlcat (char *dst, const char *src, size_t dstsize);
#endif

#ifdef HAVE_NO_GLIB_2_8
gboolean g_file_set_contents (const gchar *filename, const gchar *contents,
        gssize length, GError **error);
#endif

#endif //_FSEXAM_HELPER_H_
