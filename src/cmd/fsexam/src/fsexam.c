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

#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include <glib.h>
#include <glib/gi18n.h>
#include <gconf/gconf-client.h>
#include <gnome.h>
#include <glade/glade.h>
#include <bonobo/bonobo-object.h>
#include <bonobo-activation/bonobo-activation.h>

#include "fsexam-application-server.h"
#include "GNOME_Fsexam.h"

#include "file-filter.h"
#include "fsexam.h"
#include "fsexam-debug.h"
#include "encoding.h"
#include "fsexam-log.h"
#include "fsexam-dryrun.h"
#include "fsexam-history.h"
#include "fsexam-pref.h"
#include "fsexam-setting.h"
#include "file-validate.h"
#include "fsexam-ui.h"
#include "fsexam-helper.h"
#include "fsexam-convname.h"
#include "fsexam-convcontent.h"
#include "fsexam-specialfile.h"
#include "fsexam-encoding-dialog.h"

gboolean    cli_mode = FALSE;  /* global var, CLI or GUI */
gboolean    force_quit = FALSE; 
gboolean    stop_search = FALSE;

static FSEXAM_setting *setting = NULL;
static GnomeProgram   *program = NULL;
static BonoboObject   *fsexam_app_server = NULL;

/* Remember command line options */
static gboolean        prepend_encoding = FALSE;
static gboolean        append_encoding = FALSE;
static gboolean        save_encoding = FALSE;
static gboolean        restore = FALSE;

static gchar           *special = NULL;
static gchar           *encoding_list = NULL;
static gchar           *dryrun_file = NULL;
static GList           *files = NULL;

static void             signal_handler (int);
static void             show_usage (void);
static void             show_version (void);
static void             decode_options (gint argc, gchar **argv, 
                                        FSEXAM_pref *pref);
static gint             CLI_processing (gint argc, gchar **argv);
static gint             GUI_processing (gint argc, gchar **argv);
static gint             all_in_one (gint argc, gchar **argv);
static gboolean         create_config_dir (const gchar *dir);
static GList *          read_files_from_stdin (GList *files);
static FSEXAM_setting * fsexam_init (gint argc, gchar **argv, gboolean cli_mode);


#define OPT_STRING "+abd:E:e:Ff:g:HklL:npPRrSstwV?"
static struct option long_options[] =
{
    {"auto-detect",             no_argument,    NULL,   'a'},
    {"batch",                   no_argument,    NULL,   'b'},
    {"force-convert",           no_argument,    NULL,   'F'},
    {"hidden",                  no_argument,    NULL,   'H'},
    {"no-check-symlink-content",no_argument,    NULL,   'k'},
    {"list-encoding",           no_argument,    NULL,   'l'},
    {"dry-run",                 no_argument,    NULL,   'n'},
    {"append-encoding-list",    no_argument,    NULL,   'p'},
    {"prepend-encoding-list",   no_argument,    NULL,   'P'},
    {"recursive",               no_argument,    NULL,   'R'},
    {"remove",                  no_argument,    NULL,   'r'},
    {"save-encoding-list",      no_argument,    NULL,   'S'},
    {"restore",                 no_argument,    NULL,   's'},
    {"conv-content",            no_argument,    NULL,   't'},
    {"follow",                  no_argument,    NULL,   'w'},
    {"version",                 no_argument,    NULL,   'V'},
    {"help",                    no_argument,    NULL,   '?'},
    {NULL,                      0,              NULL,   0},
};


gint
main(gint argc, gchar **argv)
{
    gint  ret = FEEXIT_SUCCESS;
    
    setlocale (LC_ALL, "");

    bindtextdomain (GETTEXT_PACKAGE, FSEXAM_LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    if (strcmp (basename (argv[0]), FSEXAM_CLI_NAME) == 0)
        cli_mode = TRUE;

    ret = all_in_one (argc, argv);
    
    return ret;
}

static void
signal_handler (int sig)
{
    switch (sig) {
        case SIGQUIT:
        case SIGINT:
            break;

        case SIGALRM:
            g_print (_("Another fsexam is running, will quit soon\n"));
            break;
    }

    fsexam_cleanup_all ();

    exit (EXIT_SUCCESS);
}

void 
fsexam_cleanup_all ()
{
    if (cli_mode) {
        fsexam_setting_destroy (setting);
        fsexam_list_free (files);
    }else if (view != NULL) {
        if (view->pid > 0) {    /* kill subprocess if running */
            kill (view->pid, SIGKILL);
            wait (NULL);
        }

        fsexam_pref_save_to_gconf (view->setting->pref, 
                                   view->setting->pref->gconf_client, 
                                   FALSE);

        fsexam_view_destroy (view);
        view = NULL;
    }

    if (G_IS_OBJECT (fsexam_app_server))
	    bonobo_object_unref (fsexam_app_server);

    return;
}

static void
handle_multiple_instance ()
{
    CORBA_Environment env;
    GNOME_Fsexam_Application server;

    alarm (5);

    CORBA_exception_init (&env);

    server = bonobo_activation_activate_from_id (
            "OAFIID:GNOME_Fsexam_Application",
            0,
            NULL,
            &env);

    if (server == NULL) {
        gdk_notify_startup_complete ();
        return;
    }

    GNOME_Fsexam_Application_grabFocus (server, &env);
    g_print (_("Another fsexam is running, will quit soon\n"));

    bonobo_object_release_unref (server, &env);
    CORBA_exception_free (&env);

    gdk_notify_startup_complete ();

    return;
}

static void
decode_options (gint argc, gchar **argv, FSEXAM_pref *pref)
{
    gint     	optchar;
    const gchar	*find_expr = NULL;

    while ((optchar = getopt_long (
                    argc, argv, OPT_STRING, long_options, NULL)) != -1){
        switch (optchar){
            case 'a':
                    pref->auto_detect = TRUE;
                    break;

            case 'b':
                    pref->auto_conversion = TRUE;
                    break;

            case 'd':
                    dryrun_file = optarg;
                    break;

            case 'E':
                    special = optarg;
                    break;
                    
            case 'e':
                    encoding_list = optarg;
                    break;

            case 'F':
                    pref->force = TRUE;
                    break;

            case 'f':
                    find_expr = optarg;
                    break;

            case 'g':
                    pref->hist_len = atoi (optarg);

            case 'H':
                    pref->hidden = TRUE;
                    break;

            case 'k':
                    pref->no_check_symlink_content = TRUE;
                    break;

            case 'l':
                    show_avail_encoding ();    /* show supported encoding */
                    exit (FEEXIT_SUCCESS);

            case 'L':
                    pref->use_log = TRUE;
                    pref->log_file = g_strdup (optarg);
                    break;

            case 'n':
                    pref->dry_run = TRUE;
                    break;

            case 'P':
                    append_encoding = TRUE;
                    break;

            case 'p':
                    prepend_encoding = TRUE;
                    break;
                    
            case 'R':
                    pref->recursive = TRUE;
                    break;

            case 'r':
                    pref->remote = TRUE;
                    break;

            case 'S':
                    save_encoding = TRUE;
                    break;

            case 's':
                    restore = TRUE;
                    break;

            case 't':
                    pref->conv_content = TRUE;
                    break;

            case 'w':
                    pref->follow = TRUE;
                    break;

            case 'V':
                    show_version ();   /* show version info and exit */
                    exit (FEEXIT_SUCCESS);

            case '?':
                    show_usage ();
                    exit (FEEXIT_SUCCESS);

            default:
                    fprintf (stderr, 
                         _("Run 'fsexam --help' for usage information.\n"));
                    exit (FEEXIT_FAILURE);
        }
    }

    if (pref->hist_len < 30) {
        g_print (_("Warning: The given history length is too short, will reset to default value %d\n"), HISTORY_LENGTH);
        pref->hist_len = HISTORY_LENGTH;
    }

    /* file list */
    if (find_expr != NULL) {        /* find(1) will suppress other operands */
        files = filter_cmd_run (find_expr);
    }else if (cli_mode) {           /* CLI operands */
        if  (optind < argc){
            for (; optind < argc; optind ++){
                if ((*argv[optind] == '-') && (*(argv[optind] + 1) == '\0')){
                    files = read_files_from_stdin (files);
                    break;
                }

                files = g_list_prepend (files, g_strdup (argv[optind]));
            }
        }else{
            if ((dryrun_file == NULL) || 
                    ((dryrun_file != NULL) && pref->dry_run)) {
                files = read_files_from_stdin (files);
            }
        }
    }else {                         /* GUI operands */
        if (optind < argc){
            for (; optind < argc; optind ++){
                files = g_list_append (files, g_strdup (argv[optind]));
                //files = g_list_prepend (files, g_strdup (argv[optind]));
            }
        }
    }

    return;
}

/* Read file list from stdin */
static GList *
read_files_from_stdin (GList *files)
{
    gchar *p = NULL;
    gchar path[PATH_MAX];

    fprintf (stdout, _("Please enter the file name:\n"));

    while ((fgets (path, PATH_MAX, stdin)) != NULL){
        p = g_strstrip (path);

        if ('\0' != *p){
            files = g_list_prepend (files, g_strdup (p));
        }
    }

    return files;
}

static gint 
CLI_processing (gint argc, gchar **argv)
{
    gint            ret = FEEXIT_SUCCESS;
    CORBA_Object factory;

    program = gnome_program_init ("fsexam", VERSION,
                        LIBGNOMEUI_MODULE, argc, argv,
                        GNOME_PARAM_HUMAN_READABLE_NAME,
                        _("File Encoding Examiner"),
                        GNOME_PROGRAM_STANDARD_PROPERTIES,
                        GNOME_PARAM_APP_DATADIR, DATADIR,
                        NULL);

    if (! bonobo_activation_is_initialized ()) {
        bonobo_activation_init (argc, argv);
    }

    factory = bonobo_activation_activate_from_id (
            "OAFIID:GNOME_Fsexam_Factory",
            Bonobo_ACTIVATION_FLAG_EXISTING_ONLY,
            NULL, NULL);

    if (factory != NULL) {
        handle_multiple_instance ();
        return EXIT_SUCCESS;
    }
   
    fsexam_app_server = fsexam_application_server_new (NULL);
   
    if (!setting->pref->dry_run && (dryrun_file != NULL)){
        /* Scenario based conversion */
        setting->dryrun_info = fsexam_dryrun_file_new (dryrun_file, TRUE);
        ret = fsexam_convert_scenario (setting);
    }else if (restore){
        /* Restore file name or file content */
        if (setting->pref->conv_content) {
            ret = fsexam_restore (setting, files, RestoreConvContent);
        }else{
            ret = fsexam_restore (setting, files, RestoreConvName);     
        }
    }else{
        /* Convert file name or file content */
        if ((NULL == setting->pref->encode_list) && 
                !(setting->pref->auto_detect)) {
            g_print (_("Error: No given encoding and disabled"
                        " auto detection.\n"));
            ret = FEEXIT_FAILURE;
            goto free;
        }

        if (NULL != dryrun_file) {
            setting->dryrun_info = fsexam_dryrun_file_new (dryrun_file, FALSE);
            
            if (setting->dryrun_info == NULL) {
                fprintf (stderr, _("Can't open dryrun file %s\n"), 
                         dryrun_file);
                ret = FEEXIT_FAILURE;
                goto free;
            }

            fsexam_dryrun_write_convtype (setting->dryrun_info,
                        setting->pref->conv_content ? ConvContent : ConvName);
        }

        if (setting->pref->conv_content) {
            ret = fsexam_convert_content_batch (setting, files);
        }else{
            ret = fsexam_convert_filename_batch (setting, files);
        }
    }   
        
free:
    fsexam_cleanup_all ();

    return ret;
}

static gint
GUI_processing (gint argc, gchar **argv)
{
    GnomeClient     *client = NULL;
    CORBA_Object    factory;

    program = gnome_program_init ("fsexam", VERSION,
                        LIBGNOMEUI_MODULE, argc, argv,
                        GNOME_PARAM_HUMAN_READABLE_NAME,
                        _("File Encoding Examiner"),
                        GNOME_PROGRAM_STANDARD_PROPERTIES,
                        GNOME_PARAM_APP_DATADIR, DATADIR,
                        NULL);
    
    client = gnome_master_client ();
    g_signal_connect (G_OBJECT (client), "die",
                      G_CALLBACK (exit),
                      NULL);
   
    if (! bonobo_activation_is_initialized ()) {
        bonobo_activation_init (argc, argv);
    }

    factory = bonobo_activation_activate_from_id (
            "OAFIID:GNOME_Fsexam_Factory",
            Bonobo_ACTIVATION_FLAG_EXISTING_ONLY,
            NULL, NULL);

    if (factory != NULL) {
        handle_multiple_instance ();
        return EXIT_SUCCESS;
    }

    fsexam_app_server = fsexam_application_server_new (
            gdk_screen_get_default ());
 
    view =  fsexam_view_new ();
    view->setting = setting;

    /* set the initial state of gui */
    fsexam_gui_set_initial_state ();

    /* set dryrun buffer */
    fsexam_dryrun_buffer_set_buffer (
            FSEXAM_DRYRUN_BUFFER (setting->dryrun_info),
            gtk_text_view_get_buffer (GTK_TEXT_VIEW (
                    g_object_get_data (G_OBJECT (view->mainwin), 
                        "textview_dryrun"))));

    
    if (files == NULL) {
        fsexam_construct_ui (".");
    }else if (g_list_length (files) == 1) {
        fsexam_construct_ui (files->data);
    }else{
        fsexam_construct_ui (".");
        fsexam_search_treeview_append_list (files);
    }

    fsexam_list_free (files);

    gtk_main ();

    return FEEXIT_SUCCESS;
}

/*
 * Create "$HOME/.fsexam"
 */
static gboolean
create_config_dir (const gchar *dir)
{
    struct stat statbuf;

    if (NULL == dir) {
        g_print (_("The configuration directory is NULL\n"));
        return FALSE;
    }

    if (stat (dir, &statbuf) == -1) {
        gint status = mkdir (dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        if (status == -1) {
            g_print (_("Can't create configuration directory %s for fsexam\n"), dir);
            return FALSE;
        }else{
            return TRUE;
        }
    }
        
    if (S_ISDIR (statbuf.st_mode)) {
        return TRUE;
    }
        
    g_print (_("file %s exist and is not one directory, please rename it.\n"), dir);

    return FALSE;
}

static FSEXAM_setting *
fsexam_init (gint argc, gchar **argv, gboolean cli_mode)
{
    FSEXAM_setting *setting = NULL;
    gchar          *fsexam_repo = NULL;
    gchar          *hist_file = NULL;
    
    fsexam_repo = g_build_filename (g_get_home_dir (), FSEXAM_HIDDEN, NULL);

    if (! create_config_dir (fsexam_repo)) {
        g_free (fsexam_repo);
        return NULL;
    }

    hist_file = g_build_filename (fsexam_repo, HISTORY_FILE, NULL);

    /* Init FSEXAM_setting */
    setting = fsexam_setting_init (cli_mode);

    decode_options (argc, argv, setting->pref); 

    fsexam_pref_set_encoding_list(setting->pref,
                                  encoding_list,
                                  append_encoding,
                                  prepend_encoding,
                                  save_encoding);
        
    /* Special file handling */
    if ((special != NULL) 
            && ((strcmp (special, "COMPRESS") == 0) 
                || strcmp (special, "ALL") == 0)) {
        setting->pref->special = SPECIAL_COMPRESS;
    }

    /* log */
    if (setting->pref->use_log) {
        if (setting->log_info != NULL) {
            fsexam_log_close (setting->log_info);
        }

        setting->log_info = fsexam_log_open (setting->pref->log_file);
        /* 
         * Need use abs path for log_file, otherwise will lost path informaiton 
         * after saved into gconf database
         */
        if (*setting->pref->log_file != '/') {
            char *tmp = get_abs_path (setting->pref->log_file);
            g_free (setting->pref->log_file);
            setting->pref->log_file = tmp;
        }
    }

    /* initiate remote path list */
    setting->remote_path = get_remote_paths ();
    setting->hist_info = fsexam_history_open (hist_file, 
            setting->pref->hist_len);
    
    /* Init function pointer */
    if (cli_mode) {
        setting->get_index = get_index_default;
        setting->update_gui = NULL;
        setting->display_msg = display_msg_default;
        setting->display_stats = fsexam_setting_display_stats;
    }else{
        setting->get_index = fsexam_gui_get_index;
        setting->update_gui = fsexam_gui_update;
        setting->display_msg = fsexam_gui_display_msg;
        setting->display_stats = fsexam_gui_display_stats;
    }

    if (fsexam_debug () & FSEXAM_DBG_OPTION){
        fsexam_setting_print (setting);
        g_list_foreach (files, list_print, "\t");
    }
    if (fsexam_debug () & FSEXAM_DBG_ENCODING) {
        g_list_foreach (setting->pref->encode_list, print_encoding, NULL);
    }

    g_free (hist_file);
    g_free (fsexam_repo);

    return setting;
}

static gint
all_in_one (gint argc, gchar *argv[])
{
    gint           ret;
    
    if ((signal (SIGINT, signal_handler) == SIG_ERR)
            || (signal (SIGQUIT, signal_handler) == SIG_ERR)
            || (signal (SIGALRM, signal_handler) == SIG_ERR)) {
        perror ("Can't set signal handler.");
        return EXIT_FAILURE;
    }

    g_type_init ();
    setting = fsexam_init (argc, argv, cli_mode);
    if (NULL == setting) {
        return EXIT_FAILURE;
    }

    if (cli_mode)
        ret = CLI_processing (argc, argv);
    else
        ret = GUI_processing (argc, argv);

    return ret;
}

static void
show_usage ()
{
    printf (_("Usage:\n"));
    printf (_("    fsexam [OPTION] ... [file]\n"));

    printf (_("\nSupported options:\n"));

    printf (_("    -a, --auto-detect      Enable encoding auto detection\n"));
    if (cli_mode)
        printf (_("    -d dry-run-result      Specify the dryrun result file\n"));
    printf (_("    -E module-name         Enable special file handling\n"));
    printf (_("    -e encoding-list       Specify additional encoding list\n"));
    printf (_("    -F, --force-convert    Forceful conversion mode\n"));
    printf (_("    -f 'expression'        Specify file filter criteria\n"));
    printf (_("    -H, --hidden           Turn on hidden file handling\n"));
    printf (_("    -b, --auto-conversion  Interactive mode\n"));
    printf (_("    -l, list-encoding      List all supported encoding\n"));
    printf (_("    -k, --no-check-symlink-content\n"));
    printf (_("                           Don't check the consistency between\n"
              "                           symbolic link and its target name\n"));
    printf (_("    -L logfile             Specify log file\n"));
    if (cli_mode)
        printf (_("    -n, --dry-run          Dryrun mode\n"));
    printf (_("    -P, --append-encoding-list\n"));
    printf (_("                           Append encoding list specified by\n"
              "                           '-e' to predefined encoding list\n"));
    printf (_("    -p, --prepend-encoding-list\n"));
    printf (_("                           Prepend encoding list specified by\n"
              "                           '-e' to predefined encoding list\n"));
    printf (_("    -R, --recursive        Recursive mode\n"));
    printf (_("    -r, --remote           Turn on nfs files handling\n"));
    printf (_("    -S, --save-encoding-list\n"));
    printf (_("                           Save encoding list specified by\n"
              "                           '-e' permanently\n"));
    printf (_("    -s, --restore          Restore the original name or\n"
              "                           content for given files\n"));
    printf (_("    -t, --conv-content     Convert file content rather than\n"
              "                           file name\n"));
    printf (_("    -w, --follow           Follow symbolic link\n"));
    printf (_("    -V, --version          Print the version information\n"));
    printf (_("    -?, --help             Print this usage information for fsexam\n"));

    return;
}

static void
show_version ()
{
    printf ("%s %s\n", 
            cli_mode ? FSEXAM_CLI_NAME : FSEXAM_GUI_NAME, 
            VERSION);

    return;
}
