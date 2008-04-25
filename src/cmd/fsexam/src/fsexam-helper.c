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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <glib.h>
#include <unistd.h>
#include <libgen.h>
#include <limits.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <glib/gi18n.h>

#include "fsexam-helper.h"
#include "fsexam-error.h"
#include "fsexam.h"

short encoding2id (const gchar *encoding_name);

gboolean
str_isascii (const gchar *s, gint len)
{
    if (NULL == s)
        return FALSE;

    if (len <= 0)
        len = strlen (s);

    while ((len-- != 0 )&& (*s != '\0')) {
        if (isascii((gint)*s++) == 0)
            return FALSE;
    }

    return TRUE;
}

/*
 * This function will resolve symbolic link also because of getcwd;
 * Need remove any redundancy '/' in the full path
 *
 * Note:
 *      It is hard to get the symlink path for arbitratry input.
 *      pwd of bash can do this, but it do that througth 'PWD' env var
 */
gchar *
get_abs_path (const gchar *file)
{
    if (file == NULL)
        return NULL;

    gchar       result[PATH_MAX];
    gchar       old_cwd[PATH_MAX];
    struct stat statbuf;

    memset (old_cwd, 0, sizeof (old_cwd));

    if (lstat (file, &statbuf) < 0) {
        fsexam_errno = ERR_FILE_NONEXIST;
        return NULL;
    }

    if (getcwd(old_cwd, sizeof (old_cwd)) == NULL) {
        fsexam_errno = ERR_GET_CWD;
        return NULL;
    }

    memset (result, sizeof (result), 0);

    if (S_ISDIR (statbuf.st_mode)) {        /* is folder */
        if ((chdir(file)) < 0){
            fsexam_errno = ERR_CHDIR;
            goto fail;
        }
        
        if (getcwd(result, PATH_MAX) == NULL)
            goto fail;

        if (chdir (old_cwd) < 0) {
            fsexam_errno = ERR_CHDIR;
        }

        return g_strdup (result);
    } else {                                /* non folder */
        gchar *dir = NULL;
        gchar *base = NULL;

        dir = g_path_get_dirname (file);
        
        if (chdir(dir) < 0){
            fsexam_errno = ERR_CHDIR;
            g_free (dir);
            goto fail;
        }

        g_free (dir);

        if (getcwd(result, PATH_MAX) == NULL){
            fsexam_errno = ERR_GET_CWD;
            goto fail;
        }

        if (chdir (old_cwd) < 0) {
            fsexam_errno = ERR_CHDIR;
        }

        base = g_path_get_basename (file);

        if (strcmp (base, ".") != 0){
            strlcat (result, "/", PATH_MAX);
            strlcat (result, base, PATH_MAX);
        }

        g_free (base);

        return g_strdup (result);
    }

fail:
    chdir (old_cwd);

    return NULL;
}

/* 
 * file exist and is regular | symlink | directory
 */
gboolean
file_exist (gchar *file)
{
    struct stat buf;

    if (lstat(file, &buf) == -1){
        return FALSE;
    }

    if (!(S_ISREG(buf.st_mode)) && !(S_ISDIR(buf.st_mode)) && 
            !(S_ISLNK(buf.st_mode))){
        return FALSE;
    }

    return TRUE;
}

/*
 * Remove trailing space, '\n' and leading space
 *
 * Please take care if operate on dynamically allocated storage, 
 * pointer will change
 *
 * g_strstrip is for this
 */
/*
gchar * 
str_chomp (gchar *str)
{
    gint    len, i, j;
    gchar   *result = NULL;

   if (str == NULL)
       return NULL;

    len = strlen (str);
    result = str;

    if (result[len-1] == '\n'){
        result[len-1] = '\0';
        len --;
    }

    i = j = 0;
    for (; i < len; i++){
        if (g_ascii_isspace(*(str+i))){
            continue;
        }else{
            break;
        }
    }

    if (i == len-1){
        *result = '\0';
    }else{
        for (; i < len; i++){   
            *(result+j) = *(str+i);
            j++;
        }
        *(result+j) = '\0';
    }

    return result;
}
*/

/*
 * Remove leading and trailing space
 * tab return TRUE if there is '\t' before any non-space characters
 */
gchar *
str_compress (gchar *str, gboolean *tab)
{
    gchar *end = NULL;

    *tab = FALSE;
    if (str == NULL)
        return NULL;

    while (g_ascii_isspace (*str)){
        if ('\t' == *str)
            *tab = TRUE;

        str ++;
    }

    if ('\0' == *str)
        return NULL;

    end = str + strlen (str) - 1;
    
    while (g_ascii_isspace (*end)){
        *end -- = '\0';
    }

    return str;
}

/*
 * split str to 2 string according to space
 */
void 
str_split (gchar *str, gchar **str1, gchar **str2)
{
    gchar *end = str;

    if (str == NULL)
        return; 

    while ((*end != '\0') && (! g_ascii_isspace (*end))){
        end ++;
    }

    if (str1 != NULL)
        *str1 = g_strndup (str, end - str);

    if (str2 == NULL)
        return;

    if ('\0' == *end){
        *str2 = NULL;
        return;
    }

    while ((*end != '\0') && (g_ascii_isspace (*end))){
        end ++;
    }
    if ('\0' == *end){
        *str2 = NULL;
    }else{
        *str2 = g_strdup (end);
    }

    return;
}

void 
list_free (gpointer data, gpointer user_data)
{
    if (data)
        g_free ((gchar *)data);

    return;
}

void 
list_print (gpointer data, gpointer user_data)
{
    if (user_data)
        fputs (user_data, stdout);
    
    fputs ("\t", stdout);
    fputs (data, stdout);
    fputs ("\n", stdout);

    return;
}

/*
 * split encoding string according to ':' or ';'
 */
GSList *
encoding_string_parse (const gchar *en_text)
{
    GSList  *list = NULL;
    gchar   *cur = NULL;

    if ((en_text == NULL) || (*en_text == '\0'))
        return NULL;

    cur = (gchar *)en_text;
    while (TRUE) {
        if ((*cur == ':') || (*cur == ',')) {
            *cur = '\0';
            if (cur != en_text) {
                if (encoding2id (en_text) == -1) {
                    g_print (_("Encoding '%s' is not supported\n"),
                            (char *)en_text);
                } else {
                    list = g_slist_append (list, g_strdup (en_text));
                }
            }
            
            en_text = ++cur;
        } if (*cur == '\0') {   /* the last element */
            if (cur != en_text) {
                if (encoding2id (en_text) == -1) {
                    g_print (_("Encoding '%s' is not supported\n"),
                            (char *)en_text);
                } else {
                    list = g_slist_append (list, g_strdup (en_text));
                }
            }

            break;
        }else{
            cur++;
        }
    }

    return list;
}

/*
 * Get the abs path for symlink's target file
 */
gchar *
get_abs_path_for_symlink_target (const gchar *symlink)
{
    gchar   *target = NULL;
    gchar   *abs_path = NULL;
    
    if (symlink == NULL)
        return NULL;

    target = g_file_read_link (symlink, NULL);
    if (target == NULL) 
        return NULL;

    if (*target == '/') {
        abs_path = get_abs_path (target);
    } else {
        gchar   *first = NULL;
        gchar   *folder = g_path_get_dirname (symlink);    
       
        first = g_strdup_printf ("%s/%s", folder, target);
        abs_path = get_abs_path (first);

        g_free (folder);
        g_free (first);
    }

    g_free (target);

    return abs_path;
}

void 
hash_print (gpointer key, gpointer value, gpointer data)
{
    g_print ("key = %s\n", (gchar *)key ?  (gchar *)key : "NULL");
    
    return;
}

void 
fsexam_list_free (GList *list)
{
    if (NULL == list) 
        return;

    GList *tmp = list;

    while (tmp){
        g_free (tmp->data);

        tmp = g_list_next (tmp);
    }

    g_list_free (list);

    return;
}

void 
fsexam_slist_free (GSList *list)
{
    if (NULL == list) return;

    GSList *tmp = list;

    while (tmp){
        g_free (tmp->data);

        tmp = g_slist_next (tmp);
    }

    g_slist_free (list);

    return;
}

#define SAMPLE_MIN_LEN  30

/*
 *  Get one sample line which contain non-ascii characters from contents 
 *  which is UTF-8 already
 *  Get one sample line which contain non-ascii characters from contents. 
 *  contents maybe in various encoding:
 *      - ASCII: get first non_empty line which > MIN_LEN
 *      - UTF-8: get the first non_ASCII line > MIN_LEN
 *      - Other: get the first line which > MIN_LEN
 */

gchar *
get_sample_text (const gchar *contents, guint *length)
{
    const gchar *start = contents;
    const gchar *nonempty_line = NULL;
    const gchar *nonascii_line = NULL;
    const gchar *nonutf8_line = NULL;
    gint        nonempty_len = 0;
    gint        nonascii_len = 0;
    gint        nonutf8_len = 0; 
    gint        len;

    if (NULL == contents)
        return NULL;

    while (TRUE) {
        len = 0;
        for (; (*contents != '\0') && (*contents != '\n'); contents++)  
            len++;
       
        /* got one new line */
        if (len == 0) {
            if (*contents == '\0') {    /* EOF */
                if (nonutf8_line != NULL) {
                    if (length != NULL)
                        *length = nonutf8_len;

                    return g_strndup (nonutf8_line, nonutf8_len);
                }else if (nonascii_line != NULL) {
                    if (length != NULL)
                        *length = nonascii_len;

                    return g_strndup (nonascii_line, nonascii_len);
                }else if (nonempty_line != NULL) {
                    if (length != NULL)
                        *length = nonempty_len;

                    return g_strndup (nonempty_line, nonempty_len);
                }else{
                    if (length != NULL)
                        *length = 1;

                    return "";          /* all line is empty */
                }
            }else{                      /* empty line */
                start = ++contents;
            }

            continue;
        }

        if ((NULL == nonempty_line) 
                || ((nonempty_len < SAMPLE_MIN_LEN) && (len > nonempty_len))) {
            nonempty_line = start;
            nonempty_len = len;
        }

        if (str_isascii (start, len)) {                 /* pure ASCII line */
            start = ++contents;         
        }else if (g_utf8_validate (start, len, NULL)) { /* Non-ASCII UTF-8 */
            if ((NULL == nonascii_line)
                    || ((nonascii_len < SAMPLE_MIN_LEN) 
                        && (len > nonascii_len)) ) {
                nonascii_line = start;
                nonascii_len = len;
            } 

            start = ++contents;         
        }else{                                          /* Non-UTF8 line */ 
            if (len > SAMPLE_MIN_LEN) {
                break; 
            }else{
                if ((NULL == nonutf8_line) || (nonutf8_len < len)){
                    nonutf8_line = start;
                    nonutf8_len = len;
                }

                start = ++contents;     /* continue */
            }
        }
    }

    if (length != NULL)
        *length = len;

    return g_strndup (start, len);
}

/*
 * Remove the first element containing user's data
 */
GSList *
remove_string_from_slist (GSList *list, const gchar *str)
{
    GSList *tmp = list;

    if ((NULL == list) || (NULL == str))
        return list;

    while (tmp != NULL) {
        if (strcmp (tmp->data, str) == 0)
            break;

        tmp = tmp->next;
    }

    if (tmp != NULL) {
        g_free (tmp->data);
        list = g_slist_delete_link (list, tmp);
    }

    return list;
}

GList *
remove_string_from_list (GList *list, const gchar *str)
{
    GList *tmp = list;

    if ((NULL == list) || (NULL == str))
        return list;

    while (tmp != NULL) {
        if (strcmp (tmp->data, str) == 0)
            break;

        tmp = tmp->next;
    }

    if (tmp != NULL) {
        g_free (tmp->data);
        list = g_list_delete_link (list, tmp);
    }

    return list;
}

static gboolean
remove_hash_entry_or_not (gpointer key, gpointer value, gpointer data)
{
    return TRUE;
}

void
fsexam_hash_remove_all (GHashTable *hash)
{
    if (hash == NULL)
        return;

    g_hash_table_foreach_remove (hash, remove_hash_entry_or_not, NULL);

    return;
}

/*
 * is path2 is subpath or parent of path1
 */
gboolean
fsexam_is_subpath (const gchar *path1, const gchar *path2)
{
    gint        len;
    const gchar *tmp;

    if (! g_str_has_prefix (path1, path2))
        return FALSE;

    len = strlen (path2);
    tmp = path1 + len;

    if (*tmp == '\0' || *tmp == '/')
        return TRUE;

    return FALSE;
}

/*
 * Get the display name
 */
gchar *
fsexam_filename_display_name (const gchar *filename)
{
    gchar *end = NULL;
    gchar *start = NULL;
    gchar *result = NULL;

    start = result = g_strdup (filename);

    for (;;) {
        gboolean valid;
        valid = g_utf8_validate (start, -1, (const gchar **)&end);

        if (!*end)
            break;

        if (!valid)
            *end++ = '?';

        start = end;
    }

    return result;
}

/* 
 * Get the display basename 
 */
gchar *
fsexam_filename_display_basename (const gchar *filename)
{
    //gchar *result = g_strdup (filename);
    
    return fsexam_filename_display_name (filename);
    //return result;
}

/*
 * Escape string
 */
gchar *
fsexam_string_escape (const gchar *string)
{
    gint    len;
    gchar   *escape = NULL, *tmp = NULL;
    gchar   *result = NULL;

    if (string == NULL)
        return NULL;

    len = strlen (string) * 4 + 1;
    tmp = escape = g_new0 (gchar, len);

    while (*string != '\0') {
        if (*(guchar *)string >= 0x80) {
            gchar *t;
            t = g_strdup_printf ("\\x%X", *(guchar *)string);
            strlcpy (tmp, t, len);
            g_free (t);
            tmp += 4;
        }else{
            *tmp++ = *string;
        }

        string ++;
    }
    
    *tmp = '\0';
    result = g_strdup (escape);
    g_free (escape);

    return result;
}

/*
 * convert one string to hex format
 */
char *
fsexam_print_hex(const char *str)
{
    char *result = NULL;
    char *tmp;

    if (str == NULL)
        return "NULL";

    if (*str == '\0')
        return "";

    result = g_malloc(PATH_MAX);
    memset(result, 0, PATH_MAX);

    tmp = result;
    int len = PATH_MAX;

    while (*str) {
        int ch = (unsigned char)*str++;

        if (ch > 0x7f) {
            snprintf(tmp, len, "\\x%X", ch);
            tmp += 4;
            len -= 4;
        } else {
            snprintf(tmp, len, "\\x%02X", ch);
            tmp += 4;
            len -= 4;
        }
    }

    return result;
}


#ifndef HAVE_STRLCPY
/*
 * strlcpy implement copied from opensolaris.org
 */
size_t
strlcpy(char *dst, const char *src, size_t len)
{
    size_t slen = strlen(src);
    size_t copied;

    if (len == 0)
        return (slen);

    if (slen >= len)
        copied = len - 1;
    else
        copied = slen;

    (void) memcpy(dst, src, copied);
    dst[copied] = '\0';

    return (slen);
}
#endif

#ifndef HAVE_STRLCAT
/*
 * strlcat implement copied from opensolaris.org
 */
size_t
strlcat(char *dst, const char *src, size_t dstsize)
{
    char *df = dst;
    size_t left = dstsize;
    size_t l1;
    size_t l2 = strlen(src);
    size_t copied;

    while (left-- != 0 && *df != '\0')
        df++;

    l1 = df - dst;
    if (dstsize == l1)
        return (l1 + l2);

    copied = l1 + l2 > dstsize ? dstsize - l1 - 1 : l2;

    (void) memcpy(dst + l1, src, copied);
    dst[l1 + copied] = '\0';

    return (l1 + l2);
}
#endif

#ifdef HAVE_NO_GLIB_2_8
/*
 * Implement g_file_set_contents as it only exist in glib-2.8+
 */
gboolean
g_file_set_contents(const gchar *filename,
        const gchar *contents,
        gssize length,
        GError **error)
{
    FILE *fp = NULL;
    gsize num;


    fp = fopen (filename, "wb");
    if (fp == NULL) {
        return FALSE;
    }

    num = fwrite (contents, 1, length, fp);
    fclose (fp);

    if (num < length)
        return FALSE;

    return TRUE;
}
#endif
