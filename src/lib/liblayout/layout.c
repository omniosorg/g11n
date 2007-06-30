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
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.  
 * Use is subject to license terms.
 *
 * See LEGAL NOTICE file for terms of the license.
 */


/* 
 * This module contains the functions for loading the locale-dependent
 * layout services module, create and initialize LayoutObject.
 */

#include <sys/isa_defs.h>
#include <sys/types.h>
#include <errno.h>
#include <malloc.h>
#include <wchar.h>
#include <dlfcn.h>
#include "layout.h"
#include "layout_int.h"


#ifdef  _LP64

#if	defined(__sparcv9)
#define	MACH64_NAME		"sparcv9"
#elif	defined(__x86_64) || defined(__amd64)
#define	MACH64_NAME		"amd64"
#elif	defined(__ia64)
#define	MACH64_NAME		"ia64"
#else
#error	"unknown architecture"
#endif

#endif


#define	CTL_OBJECT_RELEASE	1


typedef struct _InitFuncListRec		*InitFuncList;

typedef struct  _InitFuncListRec {
    LayoutObj		(*func)(char *);/* Pointer to _LayoutObjectInit() */
    char		*locale_name;	/* Locale of _LayoutObjectInit() */
    InitFuncList	next;
} InitFuncListRec;


/*
 * Search if a layout object of the requested locale is created.
 * If not, create one and add it to the list.
 */
LayoutObj
_LayoutObjectGetHandle(char *locale_name)
{
    static InitFuncListRec init_func_list = {NULL, NULL, NULL};
    InitFuncList	   temp_list_ptr = &init_func_list;
    LayoutObj        	   (*layout_obj_init_func)(char *) =
						(LayoutObj(*)(char *))NULL;
    static int		   C_locale_created = FALSE;
    char        	   so_path[FILENAME_MAX];
    void 		   *so;

    extern LayoutObject    _LayoutObjectDefaultInit();


#ifndef notdef    
    if (!C_locale_created) {
        temp_list_ptr->func = _LayoutObjectDefaultInit;
        temp_list_ptr->locale_name = (char *)malloc(2);
        temp_list_ptr->locale_name[0] = 'C';
        temp_list_ptr->locale_name[1] = NULL;
        temp_list_ptr->next = NULL;  
        C_locale_created = TRUE;
    }
#endif    
  
    if (locale_name == NULL)
	locale_name = "C";

    /* 
     * Look up a list of existing loaded object.
     * If the object is already loaded for the specified locale,
     * retrieve the pointer to _LayoutObjectInit().
     */
    while (temp_list_ptr->locale_name) {
	if (!strcmp (temp_list_ptr->locale_name, locale_name)) {
	    goto _LayoutObjectGetHandle_FOUND;
        }
        
        if (temp_list_ptr->next == NULL)
            break;
	temp_list_ptr = temp_list_ptr->next;
    }
    
    /*
     * It seems this is the first time for the locale; try to load
     * the requested locale's layout engine, <locale>.layout.so.1.
     */
#ifdef  _LP64
    sprintf(so_path, "/usr/lib/locale/%s/LO_LTYPE/%s/%s.layout.so.%d",
		locale_name, MACH64_NAME, locale_name, CTL_OBJECT_RELEASE);
#else
    sprintf(so_path, "/usr/lib/locale/%s/LO_LTYPE/%s.layout.so.%d",
                locale_name, locale_name, CTL_OBJECT_RELEASE);
#endif

    so = dlopen(so_path, RTLD_LAZY);
    if (so != (void *)NULL)
	layout_obj_init_func = (LayoutObj(*)())dlsym(so, "_LayoutObjectInit");

    /*
     * If temp_list_ptr is not pointing to the first item of the list or
     * the first is already being used, then malloc space.
     * Otherwise, just store into the static struct which is the first
     * item of the list.
     */
    if ((temp_list_ptr != &init_func_list) || (temp_list_ptr->func != NULL)) {
	temp_list_ptr->next = (InitFuncList)malloc(sizeof(InitFuncListRec));
	temp_list_ptr = temp_list_ptr->next;
    }

    if (layout_obj_init_func) {
	temp_list_ptr->func = layout_obj_init_func;
    } else {
	/*
	 * If control reached here, what that means is we have
	 * a locale that doesn't have <locale>.layout.so.1 or similar and
	 * there is no entry for the locale at the init_func_list.
	 * We rig one up for the locale by copying C locale one for
	 * better performance.
	 */
	temp_list_ptr->func = _LayoutObjectDefaultInit;
    }
    temp_list_ptr->locale_name = (char *)malloc(strlen(locale_name) + 1);
    (void)strcpy (temp_list_ptr->locale_name, locale_name);
    temp_list_ptr->next = NULL;

    /*
     * Call _LayoutObjectInit() to initialize the layout object and
     * then return with a handle for the layout object.
     */
_LayoutObjectGetHandle_FOUND:
    return ((*(temp_list_ptr->func))(locale_name));
}
