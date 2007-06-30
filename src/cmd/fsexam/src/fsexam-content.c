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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gtk/gtk.h>
#include <gconf/gconf-client.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

#include <libgnome/gnome-i18n.h>

#include "encode.h"
#include "fsexam-history.h"
#include "fsexam-preference.h"
#include "fsexam-conversion.h"
#include "fsexam-content.h"
#include "fsexam.h"

static gboolean
fsexam_content_match_suffix (GSList *suffixlist, char *suffix)
{
  // unmatched if no suffix
  if (!suffix) return FALSE;

  while (suffixlist)
    {
      if (!g_ascii_strcasecmp ((gchar *)suffixlist->data, suffix))
	return TRUE;
      suffixlist = g_slist_next (suffixlist);
    }

  return FALSE;
}

static gboolean
is_plain_text_by_locale (char *name, char *locale)
{
	gchar *cmd, *p, buf[256];
	gboolean retval = FALSE;
	FILE *ptr = NULL;

	memset (buf, 0, 256);

	// Since file command in solairs outputs localized file type,
	// we just temporarily set LC_MESSAGES=C for the child process -- file
	setenv ("LC_MESSAGES", "C", 1);

	if (locale)
		setenv ("LC_CTYPE", locale, 1);

	// in case there includes any whitespace within the filename
	cmd = g_strdup_printf ("file \"%s\"", name);

	if ((ptr = popen (cmd, "r")) != NULL)
	{
		fgets (buf, 256, ptr);
		pclose (ptr);
        }

	g_free (cmd);

	if (*buf != 0x0)
	{
		p = buf;
		while (*p != ':') ++p;
		++p;
		while (isspace (*p)) ++p;

		if (strstr (p, "text"))
			retval = TRUE;
	}

	// unset the LC_CTYPE, LC_MESSAGES variable.
	unsetenv ("LC_CTYPE");
	unsetenv ("LC_MESSAGES");

	return retval;
}

static char*
encoding_to_locale (char *encoding)
{
	int i = 0;
	static char *mapping[][2] = {
		{"EUC-JP",	"ja_JP.eucJP"},
		{"SJIS", 	"ja_JP.PCK"},
		{"ISO-2022-JP",	"ja"},
		{"EUC-KR",	"ko_KR.EUC"},
		{"ISO-2022-KR",	"ko"},
		{"JOHAB",	"ko"},
		{"GB18030",	"zh_CN.GB18030"},
		{"ISO-2022-CN",	"zh_CN"},
		{"BIG5",	"zh_TW.BIG5"},
		{"EUC-TW",	"zh_TW.EUC"},
		{"BIG5-HKSCS",	"zh_HK.BIG5HK"},
	};

	for (i=0; i < sizeof(mapping)/sizeof(*mapping); i++)
	{
		if (!strcmp (encoding, mapping[i][0]))
			return strdup (mapping[i][1]);
	}

	return NULL;
}

static gboolean
fsexam_content_check_filetype (char *name)
{
  gboolean retval = FALSE;

  GList *p = NULL;
  GList *encode_list = view->pref->encode_list;
  
  /* fast check in current locale */
  if (is_plain_text_by_locale (name, NULL))
	  return TRUE;

  /* iterate encode_list, and get the corresponding locale, 
   * then tell whether it's plain text file */
  for (p = encode_list; p && retval != TRUE; p = p->next)
  {
	  char *locale = encoding_to_locale ( ((Encoding*) p->data)->codename );

	  /* some times, encoding has no corresponding locale, since we already 
	   * do the fast check, so just continue here */
	  if (!locale)
		  continue;

	  if (is_plain_text_by_locale (name, locale))
		  retval = TRUE;

	  g_free (locale);
  }
  
  return retval;
}

static gboolean
fsexam_content_check_plain_text (char *name)
{
  gchar *suffix = g_strrstr (name, ".");

  if (!fsexam_content_match_suffix (view->pref->suffix_list, suffix) &&
      !fsexam_content_check_filetype (name))
    {
      gchar *statusmsg;

      statusmsg = g_strdup_printf ("%s: %s", _("Not plain text"), name);
      fsexam_statusbar_update (statusmsg);

      g_free (statusmsg);

      return FALSE;
    }

  return TRUE;
}

GMainLoop *loop = NULL;
int encode_index = -1;

static void
widget_destroy (GtkWidget *widget,
		gpointer user_data)
{
  GMainLoop *loop =*(GMainLoop **)user_data;

  if (!loop) return;

  if (g_main_loop_is_running (loop))
    g_main_loop_quit (loop);

  encode_index = -1;
}

static void
encode_button_press (GtkWidget *widget,
		     gpointer user_data)
{
  if (g_main_loop_is_running (loop))
    g_main_loop_quit (loop);

  encode_index = (int) user_data;
}

static gboolean
fsexam_content_construct_ui (Encoding *encode,
			     gint index,
			     va_list args)
{
  GtkWidget *table;
  GtkWidget *sw;
  GtkTextBuffer *buffer;
  GtkWidget *contents;
  GtkWidget *label;
  GtkWidget *button;
  char *markup;

  table = va_arg (args, GtkWidget *);

  label = gtk_label_new (NULL);

  markup = g_strdup_printf ("<b>%s</b>:", encode->codename);
  gtk_label_set_markup (GTK_LABEL (label), markup);
  g_free (markup);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

  gtk_table_attach (GTK_TABLE (table),
		    label,
		    /* X direction */ /* Y direction */
		    index, index+1,           0, 1,
		    GTK_EXPAND| GTK_FILL, 0,
		    0, 0);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
				       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
				  GTK_POLICY_AUTOMATIC,
				  GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request (sw, 50, -1);

  gtk_table_attach (GTK_TABLE (table),
		    sw,
		    /* X direction */ /* Y direction */
		    index, index+1,   1, 2,
		    GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL,
		    0, 0);

  contents = gtk_text_view_new ();

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (contents));
  gtk_text_buffer_set_text (buffer, encode->u.contents, -1);

  gtk_container_add (GTK_CONTAINER (sw), contents);

  button = gtk_button_new_from_stock (GTK_STOCK_OK);
  gtk_table_attach (GTK_TABLE (table),
		    button,
		    /* X direction */ /* Y direction */
		    index, index+1,           2, 3,
		    GTK_EXPAND | GTK_FILL, 0,
		    0,  0);

  g_signal_connect (G_OBJECT (button),
		    "pressed",
		    G_CALLBACK (encode_button_press),
		    (gpointer)index);

  return TRUE;
}

static int
fsexam_content_show_candidates (GList *encode_list)
{
  int n_candidates = 0;
  GtkWidget *table;
  GtkWidget *win;
  int x, y, width, height;

  iterate_encode_with_func (encode_list, (void *)get_encode_elements, &n_candidates);

  win = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (win), _("File System Examiner content conversion"));
  g_signal_connect (G_OBJECT (win),
		    "destroy",
		    G_CALLBACK (widget_destroy),
		    &loop);

  gtk_window_set_modal (GTK_WINDOW (win), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (win), GTK_WINDOW (view->mainwin));

  table = gtk_table_new (3, n_candidates, FALSE);
  gtk_container_set_border_width (GTK_CONTAINER (table), 1);
  gtk_container_add (GTK_CONTAINER (win), table);

  iterate_encode_with_func (encode_list, (void *)fsexam_content_construct_ui, table);

  gtk_window_get_position (GTK_WINDOW (view->mainwin), &x, &y);
  gtk_window_get_size (GTK_WINDOW (view->mainwin), &height, &width);

  gtk_window_move (GTK_WINDOW (win), x+width/2, y+height/2);
  gtk_window_set_default_size (GTK_WINDOW (win), 450, 300);			     
  gtk_widget_show_all (win);

  loop = g_main_new (FALSE);
  GDK_THREADS_LEAVE ();
  g_main_loop_run (loop);
  GDK_THREADS_ENTER ();

  g_main_loop_unref (loop);
  loop = NULL;

  if (encode_index != -1)
    gtk_widget_destroy (win);

  return encode_index;
}

// FIXME the default carriage return conversion should be 
// specified by the preference setting?
static gboolean
exclude_windows_carriage_return (char *s)
{
  char *p = s;
  gboolean crlf_flag = FALSE;

  while (*s)
    {
      if (*s == '\r' && *(s+1) == '\n')
	{
	  crlf_flag = TRUE;
	  ++s;
	}

      *p++ = *s++;
    }

  *p = 0x0;

  return crlf_flag;
}

static char *
include_windows_carriage_return (char *s)
{
  char *contents, *p;
  gint num = 0;

  p = s;
  while (*p) { if (*p++ == '\n') ++num; }

  p = contents = g_new0 (char, strlen (s) + num + 1);
  while (*s)
    {
      if (*s == '\n')
	*p++ = '\r';

      *p++ = *s++;
    }

  return contents;
}

static void
fsexam_content_convert_file (char *path,
			     char *filename,
			     gboolean same_serial)
{
  gchar  name[256];
  gchar  *contents = NULL;
  FSEXAM_pref *pref = view->pref;
  gsize  length;
  GError *g_error = NULL;
  gboolean err;
  Score score;
  gboolean crlf_flag;

  memset (name, 0, 256);
  sprintf (name, "%s/%s", path, filename);

  if (!fsexam_content_check_plain_text (name)) return;

  err = g_file_get_contents (name, &contents, &length, &g_error);
  if (err && !contents)
    {
      fsexam_statusbar_update (g_error->message);
      g_free (g_error);
      return;
    }

  if (crlf_flag = exclude_windows_carriage_return (contents))
    length = strlen (contents);

  score = decode_analyzer (pref->encode_list, ConvContent, contents, length);
  if (score == FAIL)
    {
      fsexam_statusbar_update (_("Byte sequence not recognized yet"));
      goto done;
    }

  if (score == ORIGINAL && !crlf_flag)
    fsexam_statusbar_update (_("File content - UTF8 already"));
  else
    {
      unsigned int serial;
      ConvType convtype = ConvContent;
      gchar *converted_text;
      gchar *codeinfo = "dummy_codeinfo";
      int index = -1;
      int len;
      FILE *fp = NULL;
      gchar *new_path, *new_filename;

      if (score == ORIGINAL)
	{
	  convtype = ConvContentCRLFOnly;
	  converted_text = contents;
	}
      else
	{
	  GList *element;
	  Encoding *encode;

	  if ((index = fsexam_content_show_candidates (pref->encode_list)) == -1)
	    goto done;

	  element = g_list_nth (pref->encode_list, index);
	  encode = (Encoding *)element->data;

	  converted_text = encode->u.contents;

	  if (crlf_flag) convtype = ConvContentWithCRLF;

	  codeinfo = encode->codename;
	}

      if ((fp = fopen (name, "wb")) == NULL)
	{
	  fsexam_statusbar_update (_("File content - open failure"));
	  goto done;
	}

      length = strlen (converted_text);
      if ((len = fwrite (converted_text, 1, length, fp)) != length)
	fsexam_statusbar_update (_("File content - write error"));
      else
	{
	  char *message = NULL;

	  switch (convtype)
	    {
	    case ConvContent:
	      message = g_strdup_printf (_("File content - convert from %s"), codeinfo);
	      break;
	    case ConvContentWithCRLF:
	      message = g_strdup_printf (_("File content - convert from %s and delete Carriage Return"), codeinfo);
	      break;
	    case ConvContentCRLFOnly:
	      message = g_strdup (_("File content - delete Carriage Return only"));
	      break;
	    }

	  fsexam_statusbar_update (message);

	  if (message) g_free (message);
	}

      serial = fsexam_history_put (view->histinfo,
				   convtype,
				   codeinfo,
				   filename,
				   path,
				   same_serial);

      if (!same_serial)
	fsexam_undo_insert (serial);

      new_path = fsexam_validate_with_newline (path, TRUE);
      new_filename = fsexam_validate_with_newline (filename, FALSE);

      write_to_report_pane (pref, convtype, index, new_path, new_filename, view->lineoffset);

      g_free (new_path);
      g_free (new_filename);
      fclose (fp);
    }

done:
  // Free encode->u.contents to avoid memory leak
  cleanup_encode (pref->encode_list);

  g_free (contents);
}

static void
fsexam_content_convert_single_selection (GtkTreeModel *model,
					 GtkTreePath  *path,
					 GtkTreeIter  *iter,
					 gpointer     user_data)
{
  GtkTreeIter subiter;
  gchar *filename;
  gint *indicator = (gint *)user_data;
  GString *dir;

  if (gtk_tree_model_iter_children (model, &subiter, iter))
    {
      fsexam_statusbar_update (_("Cannot convert contents of folder"));

      return;
    }

  gtk_tree_model_get (model, iter,
		      FILENAME_COLUMN, &filename,
		      -1);

  dir = fsexam_filename_get_path (model, *iter, view->rootdir);

  fsexam_content_convert_file (dir->str, filename, *indicator != 0);

  ++*indicator;

  g_free (filename);
  g_string_free (dir, TRUE);
}

/*
 * Convert the file encoding from UTF8 to native encoding
 */
void fsexam_content_convert ()
{
  GtkTreeSelection *selection;
  gint same_serial_indicator = 0;

  // Do nothing if view->rootdir isn't set yet
  if (!view->rootdir) return;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view->treeview));

  gtk_tree_selection_selected_foreach (selection,
				       fsexam_content_convert_single_selection,
				       (gpointer) &same_serial_indicator);
}

/*
 * Convert the file content between UTF8 and native encoding 'native_encode'.
 * If crlf_flag is set, it depends on the 'delete_carriage_return' to either 
 * remove or add the carriage return code in the stream. And 'direction' 
 * controls in which direction the conversion would be performed:
 *
 *            ToDIR:        UTF8     --> native_encode
 *            FromDIR: native_encode --> UTF8
 *            NoneDIR:      Don't need conversion
 */
gboolean
fsexam_content_undo (char *filename,
		     char *native_encode,
		     ConvType convtype)
{
  gboolean crlf_flag, delete_carriage_return;
  Direction direction;
  GError *g_error = NULL;
  gsize  length, len, outbytes_left, outbytes;
  FILE   *fp = NULL;
  gboolean success;
  GIConv icd;
  gint   num_uconv;
  gchar  *inbuf, *contents = NULL;
  gchar  *outbuf, *outbuf2 = NULL;
  gchar  *converted_text;
  gboolean retval = FALSE;

  switch (convtype)
    {
    case ConvContent:
      crlf_flag = FALSE;
      delete_carriage_return = FALSE;
      direction = ToDIR;
      break;
    case ConvContentWithCRLF:
      crlf_flag = TRUE;
      delete_carriage_return = FALSE;
      direction = ToDIR;
      break;
    case ConvContentCRLFOnly:
      crlf_flag = TRUE;
      delete_carriage_return = FALSE;
      direction = NoneDIR;
      break;
    case ConvContentReverse:
      crlf_flag = FALSE;
      delete_carriage_return = FALSE;
      direction = FromDIR;
      break;
    case ConvContentWithCRLFReverse:
      crlf_flag = TRUE;
      delete_carriage_return = TRUE;
      direction = FromDIR;
      break;
    case ConvContentCRLFOnlyReverse:
      crlf_flag = TRUE;
      delete_carriage_return = TRUE;
      direction = NoneDIR;
      break;
    }

  if (!g_file_test (filename, G_FILE_TEST_EXISTS))
    {
      char *message;

      message = g_strdup_printf (_("%s doesn't exist yet"), filename);
      fsexam_statusbar_update (message);
      g_free (message);

      return retval;
    }

  success = g_file_get_contents (filename, &contents, &length, &g_error);
  if (!success && !contents)
    {
      fsexam_statusbar_update (g_error->message);
      g_free (g_error);
      return retval;
    }

  if (!length) 
    {
      fsexam_statusbar_update (_("File has been emptied, can not be restored!"));
      return retval;
    }

  if (crlf_flag)
    {
      if (delete_carriage_return)
	exclude_windows_carriage_return (contents);
      else
	{
	  char *p = include_windows_carriage_return (contents);

	  g_free (contents);
	  contents = p;
	}

      length = strlen (contents);
    }

  if (direction == NoneDIR)
    converted_text = contents;
  else
    {
      if (direction == ToDIR)
	{
	  // don't try to convert if it is legacy encoding already
	  if (!g_utf8_validate (contents, length, NULL))
	      goto _Err;

	  icd = g_iconv_open (native_encode, "UTF-8");
	}
      else if (direction == FromDIR)
	icd = g_iconv_open ("UTF-8", native_encode);

      outbuf2 = outbuf = g_new0 (char, length * 3);
      outbytes = outbytes_left = length * 3;
      inbuf = contents;
      if ((num_uconv = g_iconv (icd, &inbuf, &length,
				&outbuf, &outbytes_left)) == -1)
	{
	  fsexam_statusbar_update (_("File content - conversion failure"));
	  goto _Err;
	}

      length = outbytes - outbytes_left;
      converted_text = outbuf2;

      g_free (icd);
    }

  if ((fp = fopen (filename, "wb")) == NULL)
    {
      fsexam_statusbar_update (_("File content - open failure"));
      goto _Err;
    }
  
  if ((len = fwrite (converted_text, 1, length, fp)) != length)
    fsexam_statusbar_update (_("File content - write error"));
  else
    {
      char *message = NULL;

      switch (convtype)
	{
	case ConvContent:
	  message = g_strdup_printf (_("Restore %s"), native_encode);
	  break;
	case ConvContentWithCRLF:
	  message = g_strdup_printf (_("Restore %s and add Carriage Return"), native_encode);
	  break;
	case ConvContentCRLFOnly:
	  message = g_strdup (_("Restore by adding Carriage Return"));
	  break;
	case ConvContentReverse:
	  message = g_strdup (_("Restore UTF8"));
	  break;
	case ConvContentWithCRLFReverse:
	  message = g_strdup (_("Restore UTF8 and delete Carriage Return"));
	  break;
	case ConvContentCRLFOnlyReverse:
	  message = g_strdup (_("Restore by deleting Carriage Return"));
	  break;
	}
      
      fsexam_statusbar_update (message);
      if (message) g_free (message);
    }

  retval = TRUE;

  fclose (fp);
 _Err:
  if (outbuf2) g_free (outbuf2);
  g_free (contents);

  return retval;
}

GdkPixmap *content_pixmap;

static GdkPixmap *
create_content_pixmap (GtkWidget *peekwin, char *content)
{
  PangoLayout *pango_layout;
  PangoRectangle rect;
  gint pixmap_width, pixmap_height;
  GtkStyle *style;
  GdkPixmap *pixmap;
  PangoFontDescription *font_desc;
  static PangoLayout *main_pango_layout = NULL;

  enum { PADDING = 8 };

  main_pango_layout = gtk_widget_create_pango_layout (view->mainwin, NULL);
  font_desc = pango_font_description_copy (
	  gtk_widget_get_style (view->mainwin)->font_desc);

  pango_layout = pango_layout_new (pango_layout_get_context (main_pango_layout));

  pango_layout_set_font_description (pango_layout, font_desc);
  
  pango_layout_set_text (pango_layout, content, -1);
  pango_font_description_free (font_desc);

  pango_layout_get_pixel_extents (pango_layout, &rect, NULL);

  pixmap_width = rect.width + 2 * PADDING;
  pixmap_height = rect.height + 2 * PADDING;

  style = gtk_widget_get_style (view->mainwin);
  
  pixmap = gdk_pixmap_new (view->mainwin->window,
			   pixmap_width, pixmap_height, -1);

  gdk_draw_rectangle (pixmap, style->base_gc[GTK_STATE_NORMAL],
		     TRUE, 0, 0, pixmap_width, pixmap_height);

  gdk_draw_rectangle (pixmap, style->fg_gc[GTK_STATE_INSENSITIVE],
		      FALSE, 1, 1, pixmap_width - 3, pixmap_height -3);

  gdk_draw_layout (pixmap, style->text_gc[GTK_STATE_NORMAL],
		   -rect.x + PADDING, -rect.y + PADDING,
		   pango_layout);
  g_object_unref (pango_layout);

  return pixmap;
}

static void
set_window_background (GtkWidget *window,
		       GdkPixmap *pixmap)
{
  gdk_window_set_back_pixmap (window->window, pixmap, FALSE);
}

static void
peek_window_realize (GtkWidget *peekwin,
		     gpointer user_data)
{
  gint width, height;

  set_window_background (peekwin, content_pixmap);
  gdk_window_clear (peekwin->window);

  gdk_drawable_get_size (GDK_DRAWABLE (content_pixmap),
			 &width, &height);

  gtk_widget_set_size_request (peekwin, width, height);
  gtk_window_resize (GTK_WINDOW (peekwin), width, height);
}

static void
update_peek_window (GtkWidget *peekwin, char *content)
{
  gint width, height;

  g_return_if_fail (peekwin != NULL);

  if (content_pixmap != NULL)
    g_object_unref (content_pixmap);

  content_pixmap = create_content_pixmap (peekwin, content);

  if (GTK_WIDGET_REALIZED (peekwin))
    {
      set_window_background (peekwin, content_pixmap);
      gdk_window_clear (peekwin->window);
    }

  gdk_drawable_get_size (GDK_DRAWABLE (content_pixmap),
			 &width, &height);

  gtk_widget_set_size_request (peekwin, width, height);
  gtk_window_resize (GTK_WINDOW (peekwin), width, height);
}

static GtkWidget *
make_peek_window ()
{
  GtkWidget *peekwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (peekwin, "realize",
		    G_CALLBACK (peek_window_realize),
		    NULL);

  gtk_window_set_type_hint (GTK_WINDOW (peekwin),
			    GDK_WINDOW_TYPE_HINT_UTILITY);
  gtk_window_set_decorated (GTK_WINDOW (peekwin), FALSE);
  gtk_window_set_screen (GTK_WINDOW (peekwin),
			 gtk_widget_get_screen (view->mainwin));
  gtk_widget_set_app_paintable (peekwin, TRUE);

  return peekwin;
}

static void
get_upper_left_xy (GtkWidget *peekwin,
		   gint width, gint height,
		   gint x_root, gint y_root,
		   gint *x, gint *y)
{
  *x = x_root;
  *y = y_root;

  *x -= width;
  *y -= height;
}

static void
place_peek_window (GtkWidget *peekwin,
		   gint x_root,
		   gint y_root)
{
  gint width, height;
  gint x, y;

  g_return_if_fail (peekwin != NULL);

  gtk_widget_get_size_request (peekwin,
			       &width,
			       &height);

  get_upper_left_xy (peekwin, width, height,
		     x_root, y_root, &x, &y);

  gtk_window_move (GTK_WINDOW (peekwin), x, y);
}

#define MAX_NUM_MULTIBYTE_CHARACTERS 400
#define MAX_NUM_CHARACTERS 2000

static gchar *
fsexam_content_get_sample (char *file)
{
  char *p, *sample;
  gsize length;
  gint multi_num = 0, char_num = 0;
  GError *err = NULL; 

  if (!g_file_get_contents (file, &p, &length, &err))
    {
      fsexam_statusbar_update (err->message);
      g_error_free (err);

      return NULL;
    }

  sample = p;

  while (1)
    {
      gunichar wc;

      wc = g_utf8_get_char_validated (p, -1);
      if (wc == 0x0 || multi_num == MAX_NUM_MULTIBYTE_CHARACTERS ||
	  char_num == MAX_NUM_CHARACTERS) 
	{
	  *p = 0x0;
	  break;
	}
      if (wc == (gunichar)-1 || wc == (gunichar)-2)
	{
	  ++multi_num;
	  ++p;
	}
      else
	{
	  if (wc >= 0x7e)
	    ++multi_num;

	  p = g_utf8_next_char (p);
	}
      ++char_num;
    }

  return sample;
}

static void
peekwin_destroy (GtkWidget *widget,
		 gpointer user_data)
{
  GtkWidget *peekwin = view->peekwin;

  view->peekwin = NULL;

  gtk_object_destroy (GTK_OBJECT (peekwin));
}

static gboolean
fsexam_content_get_selection (char **path,
			     char **filename)
{
  GtkTreeSelection *selection;
  GtkTreeIter iter, subiter;
  GtkTreeModel *model;
  GList *row;
  GString *dir;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view->treeview));
  row = gtk_tree_selection_get_selected_rows (selection, &model);

  if ( !row )
      return FALSE;

  if (g_list_length (row) > 1)
    {
      g_list_foreach (row, gtk_tree_path_free, NULL);
      g_list_free (row);

      fsexam_statusbar_update (_("Cannot preview multiple selections"));

      return FALSE;
    }

  gtk_tree_model_get_iter (model, &iter, row->data);

  g_list_foreach (row, gtk_tree_path_free, NULL);
  g_list_free (row);

  if (gtk_tree_model_iter_children (model, &subiter, &iter))
    {
      fsexam_statusbar_update (_("Cannot convert contents of folder"));

      return FALSE;
    }

  gtk_tree_model_get (model, &iter,
		      FILENAME_COLUMN, filename,
		      -1);

  dir = fsexam_filename_get_path (model, iter, view->rootdir);

  *path = g_strdup (dir->str);

  g_string_free (dir, TRUE);

  return TRUE;
}

void
fsexam_content_peek (gint x_root, gint y_root)
{
  char *path, *filename, *name, *sample_text;
  int y_root2 = y_root;
  
  // Do nothing if view->rootdir isn't set yet
  if (!view->rootdir) return;

  if (!fsexam_content_get_selection (&path, &filename))
    return;

  if (!y_root)
    {
      int x, y, height, width;

      gtk_window_get_position (GTK_WINDOW (view->mainwin), &x, &y);
      gtk_window_get_size (GTK_WINDOW (view->mainwin), &height, &width);

      x_root = x+width/2;
      y_root = y+height/2;
    }

  name = g_strdup_printf ("%s/%s", path, filename);

  if (!fsexam_content_check_plain_text (name))
    goto ERR;

  if (!view->peekwin)
    view->peekwin = make_peek_window ();

  place_peek_window (view->peekwin, x_root, y_root);

  if ((sample_text = fsexam_content_get_sample (name)) == NULL)
    goto ERR;

  update_peek_window (view->peekwin, sample_text);

  gtk_widget_show (view->peekwin);

  set_window_background (view->peekwin, content_pixmap);
  gdk_window_clear (view->peekwin->window);

  if (y_root2 == 0)
    g_signal_connect (G_OBJECT (view->peekwin),
		      "focus-in-event",
		      G_CALLBACK (peekwin_destroy),
		      NULL);

  g_free (sample_text);

 ERR:
  g_free (name);
  g_free (filename);
  g_free (path);
}
