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


#ifndef _FSEXAM_DRYRUN_H
#define _FSEXAM_DRYRUN_H

#include <glib-object.h>

/* --- Interface for handling dry run result --- */
#define FSEXAM_TYPE_DRYRUN                  (fsexam_dryrun_get_type ())
#define FSEXAM_DRYRUN(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), FSEXAM_TYPE_DRYRUN, FsexamDryrun))
#define FSEXAM_IS_DRYRUN(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FSEXAM_TYPE_DRYRUN))
#define FSEXAM_DRYRUN_GET_INTERFACE(inst)   (G_TYPE_INSTANCE_GET_INTERFACE ((inst), FSEXAM_TYPE_DRYRUN, FsexamDryrunInterface)) 

typedef struct _FsexamDryrun FsexamDryrun;  /* dummy object */
typedef struct _FsexamDryrunInterface FsexamDryrunInterface;

struct _FsexamDryrunInterface {
    GTypeInterface parent;

    /* write one string, may contain '\n' or not */
    gboolean (*write) (FsexamDryrun *self, const gchar *string);
    /* read one line */
    gboolean (*read) (FsexamDryrun *self, gchar *buf, guint size);
};

GType    fsexam_dryrun_get_type (void);
gboolean fsexam_dryrun_write (FsexamDryrun *self, const gchar *string);
gboolean fsexam_dryrun_read (FsexamDryrun *self, gchar *buf, guint size);

/* --- File implementation of the Interface --- */
#define FSEXAM_TYPE_DRYRUN_FILE             (fsexam_dryrun_file_get_type ())
#define FSEXAM_DRYRUN_FILE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), FSEXAM_TYPE_DRYRUN_FILE, FsexamDryrunFile))
#define FSEXAM_DRYRUN_FILE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), FSEXAM_TYPE_DRYRUN_FILE, FsexamDryrunFileClass))
#define FSEXAM_IS_DRYRUN_FILE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FSEXAM_TYPE_DRYRUN_FILE))
#define FSEXAM_IS_DRYRUN_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FSEXAM_TYPE_DRYRUN_FILE))
#define FSEXAM_DRYRUN_FILE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FSEXAM_TYPE_DRYRUN_FILE, FsexamDryrunFileClass))

typedef struct _FsexamDryrunFile FsexamDryrunFile;
typedef struct _FsexamDryrunFileClass FsexamDryrunFileClass;

struct _FsexamDryrunFile {
    GObject parent;

    FILE *fp;
};

struct _FsexamDryrunFileClass {
    GObjectClass parent;
};

GType fsexam_dryrun_file_get_type (void);
FsexamDryrun *fsexam_dryrun_file_new (const gchar *file, gboolean);

/* --- Widget implementation for Dryrun Interface --- */
#define FSEXAM_TYPE_DRYRUN_BUFFER           (fsexam_dryrun_buffer_get_type ())
#define FSEXAM_DRYRUN_BUFFER(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), FSEXAM_TYPE_DRYRUN_BUFFER, FsexamDryrunBuffer))
#define FSEXAM_DRYRUN_BUFFER_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST ((klass), FSEXAM_TYPE_DRYRUN_BUFFER, FsexamDryrunBufferClass))
#define FSEXAM_IS_DRYRUN_BUFFER(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FSEXAM_TYPE_DRYRUN_BUFFER))
#define FSEXAM_IS_DRYRUN_BUFFER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FSEXAM_TYPE_DRYRUN_BUFFER))
#define FSEXAM_DRYRUN_BUFFER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), FSEXAM_TYPE_DRYRUN_BUFFER, FsexamDryrunBufferClass))

typedef struct _FsexamDryrunBuffer FsexamDryrunBuffer;
typedef struct _FsexamDryrunBufferClass FsexamDryrunBufferClass;

struct _FsexamDryrunBuffer {
    GObject parent;

    GtkTextBuffer *buffer;
    guint current_line;  /* used only for reading text from buffer */
};

struct _FsexamDryrunBufferClass {
    GObjectClass parent;
};

GType        fsexam_dryrun_buffer_get_type (void);
FsexamDryrun *fsexam_dryrun_buffer_new (void);
FsexamDryrun *fsexam_dryrun_buffer_new_with_buffer (GtkTextBuffer *buffer);
void         fsexam_dryrun_buffer_set_buffer (FsexamDryrunBuffer *buffer, GtkTextBuffer *buf);
void         fsexam_dryrun_buffer_clear_buffer (FsexamDryrunBuffer *buffer);
void         fsexam_dryrun_buffer_set_current_line (FsexamDryrunBuffer *buffer, guint line_no);


/* --- Dryrun result content processing API --- */
#define FSEXAM_DRYRUN_NO_PROPER_ENCODING    "No-proper-encoding"
#define FSEXAM_DRYRUN_NO_NEED_CONVERT       "No-need-convert"

typedef struct _Dryrun_item Dryrun_item;
struct _Dryrun_item {
    gchar *path;
    gchar *encoding;
};

/*
 * Write conversion type into dryrun result file 
 */
gboolean fsexam_dryrun_write_convtype (FsexamDryrun *info, ConvType convtype);
gboolean fsexam_dryrun_get_convtype (FsexamDryrun *info, ConvType *type);

/*
 *  Write one dryrun item including all candidates
 */
gboolean fsexam_dryrun_puts (FsexamDryrun *info, 
                             const gchar *fullpath, 
                             Score score, 
                             GList *encoding_list, 
                             ConvType convtype);
gboolean fsexam_dryrun_write_msg (FsexamDryrun *, const gchar *);

/*
 *  Read dryrun file and proces each item using given handler
 */
gboolean fsexam_dryrun_process (FsexamDryrun *info, GSList  **slist);
void     fsexam_dryrun_item_slist_free (GSList *slist);

#endif //_FSEXAM_DRYRUN_H
