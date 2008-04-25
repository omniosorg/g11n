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

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-generic-factory.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-context.h>
#include <bonobo-activation/bonobo-activation-register.h>

#include "fsexam-application-server.h"
#include "GNOME_Fsexam.h"

#include "fsexam-header.h"
#include "fsexam-ui.h"
#include "fsexam.h"

static void fsexam_application_server_class_init (FsexamApplicationServerClass *klass);
static void fsexam_application_server_init (FsexamApplicationServer *a);
static void fsexam_application_server_object_finalize (GObject *object);
static GObjectClass *fsexam_application_server_parent_class;

static BonoboObject *
fsexam_application_server_factory (BonoboGenericFactory *this_factory,
        const gchar *iid,
        gpointer user_data)
{
    FsexamApplicationServer *a;

    a = (FsexamApplicationServer *)g_object_new (FSEXAM_APPLICATION_SERVER_TYPE, NULL);

    return BONOBO_OBJECT (a);
}

BonoboObject *
fsexam_application_server_new (GdkScreen *screen)
{
    BonoboGenericFactory    *factory = NULL;
    //char                    *display_name = NULL;
    char                    *registration_id = NULL;

    if (screen != NULL) {
        //display_name = gdk_screen_make_display_name (screen);
    }

    registration_id = bonobo_activation_make_registration_id (
            "OAFIID:GNOME_Fsexam_Factory",
            //display_name);
            NULL);

    factory = bonobo_generic_factory_new (registration_id,
            fsexam_application_server_factory,
            NULL);

    //g_free (display_name);
    g_free (registration_id);

    return BONOBO_OBJECT (factory);
}

static void
impl_fsexam_application_server_grabFocus (PortableServer_Servant _servant,
        CORBA_Environment *ev)
{
    if (cli_mode) {
        g_print (_("One instance of fsexam is running already.\n"));
    }else{
        gtk_window_present (GTK_WINDOW (view->mainwin));
    }

    return;
}

static void
fsexam_application_server_class_init (FsexamApplicationServerClass *klass)
{
    GObjectClass *object_class = (GObjectClass *)klass;
    POA_GNOME_Fsexam_Application__epv *epv = &klass->epv;

    fsexam_application_server_parent_class = (GObjectClass *)g_type_class_peek_parent (klass);
    object_class->finalize = fsexam_application_server_object_finalize;

    /* connect implementation callbacks */
    epv->grabFocus = impl_fsexam_application_server_grabFocus;

    return;
}

static void
fsexam_application_server_init (FsexamApplicationServer *c)
{
}

static void
fsexam_application_server_object_finalize (GObject *object)
{
    FsexamApplicationServer *a = FSEXAM_APPLICATION_SERVER(object);

    fsexam_application_server_parent_class->finalize (G_OBJECT (a));
}


BONOBO_TYPE_FUNC_FULL (
        FsexamApplicationServer,                    
        GNOME_Fsexam_Application, 
        BONOBO_TYPE_OBJECT,           
        fsexam_application_server)
