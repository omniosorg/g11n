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

#include <string.h>
#include <glib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include "fsexam-debug.h"
#include "fsexam-header.h"

void        cb_text_buffer_changed (GtkTextBuffer *buffer, gpointer data);

static void fsexam_dryrun_base_init (gpointer g_class);

static void dryrun_file_instance_init (GTypeInstance *instance, 
        gpointer g_class);
static void dryrun_file_interface_init (gpointer g_iface, 
        gpointer iface_data);
static void dryrun_file_class_init (GObjectClass *klass);

static void dryrun_buffer_instance_init (GTypeInstance *instance,
        gpointer g_class);
static void dryrun_buffer_interface_init (gpointer g_iface,
        gpointer iface_data);
static void dryrun_buffer_class_init (GObjectClass *klass);

static gboolean fsexam_dryrun_write_path (FsexamDryrun *, const gchar *);

static gboolean fsexam_dryrun_write_candidate (FsexamDryrun *info,
        const gchar *encoding_name,
        const gchar *sample);

/* FsexamDryrun Interface Implementation */
static void
fsexam_dryrun_base_init (gpointer g_class)
{
    static gboolean initialized = FALSE;

    if (!initialized) {
        initialized = TRUE;
    }

    return;
}

GType
fsexam_dryrun_get_type (void)
{
    static GType type = 0;

    if (type == 0) {
        static const GTypeInfo info = {
            sizeof (FsexamDryrunInterface),
            fsexam_dryrun_base_init,    /* base_init */
            NULL,                       /* bsae_finalize */
            NULL,                       /* class_init */
            NULL,                       /* class_finalize */
            NULL,                       /* class_data */
            0,                          /* sizeof (Instance) */
            0,                          /* n_preallocs */
            NULL,                       /* instance_init */
        };

        type = g_type_register_static (G_TYPE_INTERFACE, 
                "FsexamDryrun", 
                &info, 
                0);
    }

    return type;
}

gboolean
fsexam_dryrun_write (FsexamDryrun *self, const gchar *string)
{
    return FSEXAM_DRYRUN_GET_INTERFACE (self)->write (self, string);
}

gboolean
fsexam_dryrun_read (FsexamDryrun *self, gchar *buf, guint size)
{
    return FSEXAM_DRYRUN_GET_INTERFACE (self)->read (self, buf, size);
}

/* --- Dryrun implementation on file --- */
static void
dryrun_file_finalize (GObject *object)
{
    FsexamDryrunFile *dryrun_file = FSEXAM_DRYRUN_FILE (object);

    if (dryrun_file && dryrun_file->fp) {
        fclose (dryrun_file->fp);
        dryrun_file->fp = NULL;
    }

    return;
}

GType
fsexam_dryrun_file_get_type (void)
{
    static GType type = 0;

    if (type == 0) {
        static const GTypeInfo info = {
            sizeof (FsexamDryrunFileClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) dryrun_file_class_init, 
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof (FsexamDryrunFile),
            0,      /* n_preallocs */
            (GInstanceInitFunc) dryrun_file_instance_init, 
        };

        static const GInterfaceInfo dryrun_info = {
            (GInterfaceInitFunc) dryrun_file_interface_init,/* interface_init */
            NULL,   /* interface_finalize */
            NULL,   /* interface_data */
        };

        type = g_type_register_static (G_TYPE_OBJECT, "FsexamDryrunFile", &info, 0);
        g_type_add_interface_static (type, FSEXAM_TYPE_DRYRUN, &dryrun_info);
    }

    return type;
}

/* write callback implementation of DryrunFile */
static gboolean 
dryrun_file_write (FsexamDryrunFile *self, const gchar *string)
{
    gboolean ret = TRUE;

    if (fputs (string, self->fp) < 0)
        ret = FALSE;

    return ret;
}

static gboolean
dryrun_file_read (FsexamDryrunFile *self, gchar *buf, guint size)
{
    gboolean ret = TRUE;

    if (fgets (buf, size, self->fp) == NULL)
        ret = FALSE;

    return ret;
}

static void
dryrun_file_interface_init (gpointer g_iface, gpointer iface_data)
{
    FsexamDryrunInterface *iface = (FsexamDryrunInterface *)g_iface;

    iface->write = (gboolean (*) (FsexamDryrun *, const gchar *))dryrun_file_write;
    iface->read = (gboolean (*) (FsexamDryrun *, gchar *, guint)) dryrun_file_read;

    return;
}

static void
dryrun_file_instance_init (GTypeInstance *instance, gpointer g_class)
{
    FsexamDryrunFile *self = FSEXAM_DRYRUN_FILE (instance);

    self->fp = NULL;

    return;
}

static void
dryrun_file_class_init (GObjectClass *klass)
{
    klass->finalize = dryrun_file_finalize;

    return;
}

FsexamDryrun *
fsexam_dryrun_file_new (const gchar *filename, gboolean readonly)
{
    FsexamDryrunFile *dryrun = NULL;

    if (filename == NULL)
        return NULL;

    dryrun = g_object_new (FSEXAM_TYPE_DRYRUN_FILE, NULL);
    dryrun->fp = fopen (filename, readonly ? "r" : "w");

    if (dryrun->fp == NULL) {
        g_object_unref (dryrun);
        return NULL;
    }

    return FSEXAM_DRYRUN (dryrun);
}

/* --- Widget implementation for dryrun --- */
GType
fsexam_dryrun_buffer_get_type (void)
{
    static GType type = 0;

    if (type == 0) {
        static const GTypeInfo info = {
            sizeof (FsexamDryrunBufferClass),
            NULL,   /* base_init */
            NULL,   /* base_finalize */
            (GClassInitFunc) dryrun_buffer_class_init, 
            NULL,   /* class_finalize */
            NULL,   /* class_data */
            sizeof (FsexamDryrunBuffer),
            0,      /* n_preallocs */
            (GInstanceInitFunc) dryrun_buffer_instance_init, 
        };

        static const GInterfaceInfo dryrun_info = {
            (GInterfaceInitFunc) dryrun_buffer_interface_init,
            NULL,   /* interface_finalize */
            NULL,   /* interface_data */
        };

        type = g_type_register_static (G_TYPE_OBJECT, "FsexamDryrunBuffer", &info, 0);
        g_type_add_interface_static (type, FSEXAM_TYPE_DRYRUN, &dryrun_info);
    }

    return type;
}

/*
 * Create mark and connect signal handler for GtkTextBuffer 
 */
static void
setup_text_buffer (GtkTextBuffer *buffer)
{
    GtkTextIter iter;

    if (buffer == NULL)
        return;

    gtk_text_buffer_get_end_iter (buffer, &iter);
    gtk_text_buffer_create_mark (buffer, "end_mark", &iter, FALSE);

    g_signal_connect (G_OBJECT (buffer), "changed", 
                      G_CALLBACK (cb_text_buffer_changed), NULL);

    return;
}

static gboolean 
dryrun_buffer_write (FsexamDryrunBuffer *self, const gchar *string)
{
    GtkTextIter     iter;

    if (self->buffer == NULL)
        return FALSE;

    gtk_text_buffer_get_end_iter (self->buffer, &iter);
    gtk_text_buffer_insert (self->buffer, &iter, string, -1);

    return TRUE;
}

/*
 * read one line from text buffer and increase line number
 */
static gboolean
dryrun_buffer_read (FsexamDryrunBuffer *self, gchar *buf, guint size)
{
    gboolean    ret = TRUE;
    guint       line_count;
    GtkTextIter start, end;
    gchar       *text = NULL;

    line_count = (guint) gtk_text_buffer_get_line_count (self->buffer);
    line_count --;  /* The last line is empty line */

    if (self->current_line >= line_count) /* EOF */
        return FALSE;

    gtk_text_buffer_get_iter_at_line (self->buffer, 
            &start, 
            self->current_line);
    gtk_text_buffer_get_iter_at_line (self->buffer, 
            &end, 
            self->current_line + 1);

    text = gtk_text_buffer_get_text (self->buffer, &start, &end, FALSE);

    strlcpy (buf, text, size);

    g_free (text);
    self->current_line++;

    return ret;
}

static void
dryrun_buffer_interface_init (gpointer g_iface, gpointer iface_data)
{
    FsexamDryrunInterface *iface = (FsexamDryrunInterface *)g_iface;

    iface->write = (gboolean (*) (FsexamDryrun *, const gchar *))dryrun_buffer_write;
    iface->read = (gboolean (*) (FsexamDryrun *, gchar *, guint)) dryrun_buffer_read;

    return;
}

static void
dryrun_buffer_instance_init (GTypeInstance *instance, gpointer g_class)
{
    FsexamDryrunBuffer *self = FSEXAM_DRYRUN_BUFFER (instance);

    self->buffer = NULL;
    self->current_line = 0;

    return;
}

static void 
dryrun_buffer_finalize (GObject *object)
{
    return;
}

static void
dryrun_buffer_class_init (GObjectClass *klass)
{
    klass->finalize = dryrun_buffer_finalize; 

    return;
}

FsexamDryrun *
fsexam_dryrun_buffer_new (void)
{
    FsexamDryrunBuffer *dryrun = NULL;

    dryrun = g_object_new (FSEXAM_TYPE_DRYRUN_BUFFER, NULL);

    return FSEXAM_DRYRUN (dryrun);
}

FsexamDryrun *
fsexam_dryrun_buffer_new_with_buffer (GtkTextBuffer *buffer)
{
    FsexamDryrunBuffer *dryrun = NULL;

    dryrun = g_object_new (FSEXAM_TYPE_DRYRUN_BUFFER, NULL);

    if (buffer != NULL) {
        dryrun->buffer = buffer;
        setup_text_buffer (buffer);
    }

    return FSEXAM_DRYRUN (dryrun);
}

void
fsexam_dryrun_buffer_clear_buffer (FsexamDryrunBuffer *buffer)
{
    GtkTextIter start, end;

    gtk_text_buffer_get_start_iter (buffer->buffer, &start);
    gtk_text_buffer_get_end_iter (buffer->buffer, &end);
    gtk_text_buffer_delete (buffer->buffer, &start, &end);

    buffer->current_line = 0;

    return;
}

void 
fsexam_dryrun_buffer_set_buffer (FsexamDryrunBuffer *buffer, 
        GtkTextBuffer *buf)
{
    if (buffer == NULL)
        return;

    buffer->buffer = buf;
    setup_text_buffer (buffer->buffer);
    
    return;
}

void 
fsexam_dryrun_buffer_set_current_line (FsexamDryrunBuffer *buffer, guint line_no)
{
    if (buffer == NULL)
        return;

    buffer->current_line = line_no;

    return;
}

/*
 * Write the full file path into dryrun file
 */
static gboolean 
fsexam_dryrun_write_path (FsexamDryrun *info, const gchar *path)
{
    if ((NULL == path) || (NULL == info))
        return FALSE;

    if (g_utf8_validate (path, -1, NULL)){
        if (! fsexam_dryrun_write (info, path)){
            fsexam_errno = ERR_CANNOT_WRITE_DRYRUN;
            goto fail;
        }
    }else{
        gchar *uri = NULL;
        uri = g_filename_to_uri (path, NULL, NULL);
        if (NULL == uri){
            fsexam_errno = ERR_CANNOT_CONVERT_TO_URI;
            goto fail;
        }
        if (! fsexam_dryrun_write (info, uri) < 0){
            fsexam_errno = ERR_CANNOT_WRITE_DRYRUN;
            g_free (uri);
            goto fail;
        }
        g_free (uri);
    }

    fsexam_dryrun_write (info, "\n");
    
    return TRUE;

fail:
    return FALSE;
}

static gboolean
fsexam_dryrun_write_candidate (FsexamDryrun *info, 
                               const gchar *encoding_name, 
                               const gchar *sample)
{
    if ((NULL == info) || (NULL == sample) || (NULL == encoding_name))
        return FALSE;

    if (! fsexam_dryrun_write (info, "\t")) {
        fsexam_errno = ERR_CANNOT_WRITE_DRYRUN;
        return FALSE;
    }

    fsexam_dryrun_write (info, encoding_name);
    fsexam_dryrun_write (info, "\t");
    fsexam_dryrun_write (info, sample);
    fsexam_dryrun_write (info, "\n");

    return TRUE;
}

gboolean
fsexam_dryrun_write_msg (FsexamDryrun *info, const gchar *msg)
{
    if ((NULL == info) || (NULL == msg))
        return FALSE;

    if (! fsexam_dryrun_write (info, "\t")) {
        fsexam_errno = ERR_CANNOT_WRITE_DRYRUN;
        return FALSE;
    }

    fsexam_dryrun_write (info, msg);
    fsexam_dryrun_write (info, "\n");

    return TRUE;
}

gboolean 
fsexam_dryrun_write_convtype (FsexamDryrun *info, ConvType type)
{
    g_return_val_if_fail ((info != NULL), FALSE);

    if (type == ConvName)
        fsexam_dryrun_write (info, "Name conversion\n");
    else
        fsexam_dryrun_write (info, "Content conversion\n");

    return TRUE;
}

/*
 * Get the conversion type, and set fsexam_errno if invalid
 */
gboolean
fsexam_dryrun_get_convtype (FsexamDryrun *info, ConvType *type)
{
    const gint TYPE_LENGTH = 80;
    gchar convtype[TYPE_LENGTH];
    gchar *strip = NULL;

    if (! fsexam_dryrun_read (info, convtype, sizeof (convtype))){
        fsexam_errno = ERR_DRYRUN_FILE_INVALID;
        return FALSE;
    }

    strip = g_strstrip (convtype);

    if (strcmp (strip, "Name conversion") == 0)
        *type = ConvName;
    else if (strcmp (strip, "Content conversion") == 0)
        *type = ConvContent;
    else
        fsexam_errno = ERR_DRYRUN_FILE_INVALID;

    return TRUE;
}

/* 
 *  Analyze dryrun result file, add Dryrun_item which represent
 *  user selected encoding into GSList result.
 */
gboolean
fsexam_dryrun_process (FsexamDryrun *info, GSList **result)
{
    g_return_val_if_fail (info != NULL, FALSE);

    gchar    linebuf [PATH_MAX + 20];
    gchar    *path = NULL, *encoding = NULL, *sample = NULL, *ptr = NULL;
    GSList   *slist = NULL;
    gboolean metpath = FALSE, metcode = FALSE, tab = FALSE;
    gboolean ret = FALSE;

    while (fsexam_dryrun_read (info, linebuf, sizeof (linebuf))) {
        if (linebuf[strlen (linebuf) - 1] == '\n')
            linebuf[strlen (linebuf) - 1] = '\0';

        /* remove leading and trailing white space */
        if ((ptr = str_compress (linebuf, &tab)) == NULL) {
            continue;                       /* Empty line */
        }

        /* 
         * Some files may not need convert at all, or have no 
         * proper encoding. bypass it and continue.
         */
        if ((strcmp (ptr, FSEXAM_DRYRUN_NO_NEED_CONVERT) == 0)
                || (strcmp (ptr, FSEXAM_DRYRUN_NO_PROPER_ENCODING) == 0)) {
            if (!metpath) {
                fsexam_errno = ERR_DRYRUN_FILE_INVALID;
                goto done;
            }

            g_free (path);
            path = NULL;
            metpath = FALSE;
            continue;
        }
        
        if (tab) {
            Dryrun_item *item = NULL;

            if (!metpath) {
                if (metcode) {      /* metpath will be false after first candidate */
                    continue;       /* ignore non-first encoding candidate */
                }else{
                    fsexam_errno = ERR_DRYRUN_FILE_INVALID;
                    goto done;
                }
            }

            metcode = TRUE;

            str_split (ptr, &encoding, &sample);
            if ((encoding2id (encoding) == -1) || (sample == NULL)) {
                fsexam_errno = ERR_DRYRUN_FILE_INVALID;
                goto done;
            }

            /*
             * Found one valid path/encoding pair.
             */
            if ((item = g_new0 (Dryrun_item, 1)) == NULL) {
                fsexam_errno = ERR_NO_MEMORY;
                goto done;
            }

            item->path = path;
            item->encoding = encoding;

            slist = g_slist_prepend (slist, item);

            g_free (sample);
            path = NULL;
            encoding = NULL;
            sample = NULL;

            metpath = FALSE;
        }else{
            if (metpath) {
                fsexam_errno = ERR_DRYRUN_FILE_INVALID;
                goto done;
            }

            if ('/' == *ptr) {
                path = g_strdup (ptr);
            }else{
                path = g_filename_from_uri (ptr, NULL, NULL);
            }

            if (NULL == path) {
                fsexam_errno = ERR_DRYRUN_FILE_INVALID;
                goto done;
            }

            metpath = TRUE;
            metcode = FALSE;
        }
    }

    if (NULL == slist)
        fsexam_errno = ERR_DRYRUN_FILE_INVALID;
    else
        slist = g_slist_reverse (slist);

    *result = slist;        /* caller must pass in this param */
    ret = TRUE;

done:
    g_free (path);
    g_free (encoding);
    g_free (sample);

    return ret;
}

void
fsexam_dryrun_item_slist_free (GSList *slist)
{
    if (NULL == slist)
        return;

    while (slist != NULL) {
        Dryrun_item *item = slist->data;

        g_free (item->path);
        g_free (item->encoding);
        g_free (item);

        slist = slist->next;
    }

    g_slist_free (slist);

    return;
}

/* 
 * Write one dryrun item into file
 */
gboolean
fsexam_dryrun_puts (FsexamDryrun *info, 
                    const gchar *fullpath, 
                    Score score,            /* total score */
                    GList *encoding_list, 
                    ConvType convtype)
{
    gboolean    ret = TRUE;

    if ((NULL == info) || (NULL == fullpath) || (NULL == encoding_list))
        return FALSE;

    if (! fsexam_dryrun_write_path (info, fullpath)) {
        return FALSE;
    }

    if (score == FAIL){
        if (! fsexam_dryrun_write_msg (info, 
                                      _(FSEXAM_DRYRUN_NO_PROPER_ENCODING))){
            fsexam_errno = ERR_CANNOT_WRITE_DRYRUN;
            ret = FALSE;
        }
    }else if (score == ORIGINAL){
        if (! fsexam_dryrun_write_msg (info, 
                                       _(FSEXAM_DRYRUN_NO_NEED_CONVERT))) {
            fsexam_errno = ERR_CANNOT_WRITE_DRYRUN;
            ret = FALSE;
        }
    }else{
        while (encoding_list) {
            Encoding  *encoding = NULL;
            gchar     *sample = NULL;
            
            encoding = (Encoding *)encoding_list->data; 
            encoding_list = g_list_next (encoding_list);

            if ((encoding->score == FAIL) || (encoding->score == ORIGINAL)){
                continue;
            }

            if (convtype == ConvName) {
                sample = encoding->u.converted_text;
            }else{
                sample = encoding->u.contents;
            }

            if (! fsexam_dryrun_write_candidate (info, 
                                    id2encoding (encoding->encodingID),
                                    sample)) {
                fsexam_errno = ERR_CANNOT_WRITE_DRYRUN;
                ret = FALSE;
            }
/*      BIG bug: double free
            if (convtype != ConvName)
                g_free (sample);
*/
        }
    }

    return ret;
}
