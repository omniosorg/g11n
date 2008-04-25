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

#ifndef _FSEXAM_APPLICATION_SERVER_H
#define _FSEXAM_APPLICATION_SERVER_H

#include "GNOME_Fsexam.h"
#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-object.h>

G_BEGIN_DECLS

#define FSEXAM_APPLICATION_SERVER_TYPE      (fsexam_application_server_get_type ())
#define FSEXAM_APPLICATION_SERVER(o)        (G_TYPE_CHECK_INSTANCE_CAST((o), FSEXAM_APPLICATION_SERVER_TYPE, FsexamApplicationServer))
#define FSEXAM_APPLICATION_SERVER_CLASS(k)  (G_TYPE_CHECK_CLASS_CAST((k), FSEXAM_APPLICATION_SERVER_TYPE, FsexamApplicationServerClass))
#define FSEXAM_APPLICATION_SERVER_IS_OBJECT(o)  (G_TYPE_CHECK_INSTANCE_TYPE((o), FSEXAM_APPLICATION_SERVER_TYPE))
#define FSEXAM_APPLICATION_SERVER_IS_CLASS(k)   (G_TYPE_CHECK_CLASS_TYPE((k), FSEXAM_APPLICATION_SERVER_TYPE))
#define FSEXAM_APPLICATION_SERVER_GET_CLASS(o)  (G_TYPE_INSTANCE_GET_CLASS((o), FSEXAM_APPLICATION_SERVER_TYPE, FsexamApplicationServerClass))

typedef struct {
    BonoboObject parent;
} FsexamApplicationServer;

typedef struct {
    BonoboObjectClass parent_class;

    POA_GNOME_Fsexam_Application__epv epv;
} FsexamApplicationServerClass;

GType       fsexam_application_server_get_type (void);
BonoboObject *fsexam_application_server_new (GdkScreen *screen);

G_END_DECLS

#endif  /* _FSEXAM_APPLICATION_SERVER_H */
