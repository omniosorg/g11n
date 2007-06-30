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
 * Copyright (c) 1999 by Sun Microsystems, Inc.
 * All rights reserved.
 */
#pragma ident "@(#)ctl_processing.c	1.12	01/12/07	SMI"

#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <sys/layout.h>
#include <sys/types.h>
#include "mp.h"
#include "extern.h"
#include "general_header.h"
#include "ctl_processing.h"

extern int parseShape(char *);
int shapecharsetsize=0;

bool ctl_check( void );
transform_arr *ctl_processing( unsigned char *);
static bool create_layout_obj( LayoutObject *);
static BooleanValue is_ctl( LayoutObject );
static int read_ctl_config_options(void) ;
static void set_layout_object(int) ;
static transform_arr *transform_inputline(unsigned char *);
static int get_n_parseShape();

static LayoutObject    layout_obj;

static unsigned int orientation=NULL;
static unsigned int numerals=NULL;
static unsigned int textshaping=NULL;
static unsigned int swapping=NULL;
static unsigned int context=NULL;

static int get_n_parseShape() {
	LayoutValueRec lv1[2];
	int sizeShapeCharsetSize;
	size_t ret = 0, size = 1024;

	lv1[0].name = QueryValueSize | ShapeCharset;
	lv1[0].value = &size;
	lv1[1].name = 0;
	if (m_getvalues_layout(layout_obj, lv1, &ret))
		perror("Warning! m_getvalues_layout failure in ShapeCharset");

	if (size != (size_t)0) {
	  LayoutValueRec layout_value[2];	
	  char 		 *tmp_shape_charset = (char*) XtMalloc(size);
	  int		 index = 0;

	  layout_value[0].name = ShapeCharset;
	  layout_value[0].value = &tmp_shape_charset;
	  layout_value[1].name = 0;
	  if (m_getvalues_layout(layout_obj,layout_value, &index))
		perror("Warning! m_getvalues_layout failure in get_shape");
	  else {
		/* If ULE enabled ";" is the delimiter */
		if ((tmp_shape_charset != NULL) && 
			(strchr(tmp_shape_charset, ';') != (char *)NULL))
		   parseShape(tmp_shape_charset);
	  }
	  if (tmp_shape_charset) XtFree(tmp_shape_charset);
	}
}
static int get_shapecharsetsize() {
	LayoutValueRec lv1[2];
	int	sizeShapeCharsetSize=0;
	size_t ret=0;	

	lv1[0].name =  ShapeCharsetSize;
	lv1[0].value = (int*)&sizeShapeCharsetSize;
	lv1[1].name = 0;
	m_getvalues_layout(layout_obj, lv1, &ret);
	return sizeShapeCharsetSize;
}

static transform_arr *transform_inputline(unsigned char *s) {
	unsigned char *outbuff=NULL;
	size_t *inptoout=NULL;
	size_t *outtoinp=NULL;
	unsigned char *property=NULL;
	transform_arr *trans_arr=NULL;
	size_t outsize;
	int ret, inpsize, i;

	inpsize = strlen((char *)s);

	shapecharsetsize = get_shapecharsetsize();

	trans_arr = ( transform_arr *)calloc(sizeof(transform_arr),1);
	if (trans_arr == NULL) {
		perror("\ntrans_arr = NULL");
		exit(0);
	}

	trans_arr->outsize = 2*inpsize*shapecharsetsize;

	trans_arr->outbuff=(unsigned char *)calloc(sizeof(unsigned char), trans_arr->outsize);
	if(trans_arr->outbuff == NULL ) {
		perror("\ntrans_arr->outbuff = NULL");
		exit(0);
	}

	trans_arr->outtoinp = ( size_t *)calloc(sizeof(size_t), trans_arr->outsize);
	if(trans_arr->outtoinp == NULL ) {
		perror("\ntrans_arr->outtoinp = NULL");
		exit(0);
	}

	trans_arr->inptoout = ( size_t *)calloc(sizeof(size_t), trans_arr->outsize);
	if(trans_arr->inptoout == NULL ) {
		perror("\ntrans_arr->inptoout = NULL");
		exit(0);
	}

	trans_arr->property = ( unsigned char *)calloc(sizeof(unsigned char), trans_arr->outsize);
	if(trans_arr->property == NULL ) {
		perror("\ntrans_arr->property = NULL");
		exit(0);
	}
	trans_arr->inpbufindex=NULL;
	errno=0;
	ret = m_transform_layout(layout_obj, s, inpsize,
		trans_arr->outbuff, &trans_arr->outsize, \
		trans_arr->inptoout,\
		trans_arr->outtoinp, trans_arr->property, \
		&trans_arr->inpbufindex);

	if( ret != 0 ) {
		fprintf(stderr, catgets(cat_fd, ERR_SET, 39,
			"%s: ctl transform error\n"),progname);
		exit(0);
	}

	trans_arr->inpsize = inpsize;
	trans_arr->layout_size = shapecharsetsize;

	return(trans_arr);
}	

static void set_layout_object(int opt_cnt) {

	LayoutValueRec	*lv;
	LayoutTextDescriptorRec *lr;
	int i=0, ind_ret=0;
	int num_cnt, ori_cnt, tex_cnt, swa_cnt, con_cnt;

	opt_cnt++;	/* For accommodating final null element */

	lv = ( LayoutValueRec  *)malloc(sizeof(LayoutValueRec)*opt_cnt);
	if (lv == NULL) {
		perror("\nlv = NULL");
		exit(0);
	}
	memset(lv, 0, sizeof(LayoutValueRec)*opt_cnt);

	lr = ( LayoutTextDescriptorRec *)malloc(sizeof(LayoutTextDescriptorRec)*opt_cnt);
	if (lr == NULL) {
		perror("\nlr = NULL");
		exit(0);
	}
	memset(lr, 0, sizeof(LayoutTextDescriptorRec)*opt_cnt);

	if(numerals) {
		lv[i].name =  Numerals | OutOnlyTextDescr;
		lv[i].value = &lr[i];
		((LayoutTextDescriptorRec *)(lv[i].value))->inp = 0;
		((LayoutTextDescriptorRec *)(lv[i].value))->out = numerals;
		num_cnt=i;
		i++;
	}

	if(orientation) {
		lv[i].name =  Orientation | OutOnlyTextDescr;
		lv[i].value = &lr[i];
		((LayoutTextDescriptorRec *)(lv[i].value))->inp = 0;
		((LayoutTextDescriptorRec *)(lv[i].value))->out = orientation;
		ori_cnt=i;
		i++;
	}
	if(textshaping) {
		lv[i].name =  TextShaping | OutOnlyTextDescr;
		lv[i].value = &lr[i];
		((LayoutTextDescriptorRec *)(lv[i].value))->inp = 0;
		((LayoutTextDescriptorRec *)(lv[i].value))->out = textshaping;
		tex_cnt=i;
		i++;
	}
	if(swapping) {
		lv[i].name =  Swapping | OutOnlyTextDescr;
		lv[i].value = &lr[i];
		((LayoutTextDescriptorRec *)(lv[i].value))->inp = 0;
		((LayoutTextDescriptorRec *)(lv[i].value))->out = swapping;
		swa_cnt=i;
		i++;
	}
	if(context) {
		lv[i].name =  Context | OutOnlyTextDescr;
		lv[i].value = &lr[i];
		((LayoutTextDescriptorRec *)(lv[i].value))->inp = 0;
		((LayoutTextDescriptorRec *)(lv[i].value))->out = context;
		con_cnt=i;
		i++;
	}

	lv[i].name = NULL;
	lv[i].value = NULL;

	errno=0;
	m_setvalues_layout(layout_obj, lv, &ind_ret);

	if (errno) {
		errno=0;
		fprintf(stderr, catgets(cat_fd, WARN_SET,4,
			"%s: unable to set layout values as specified\n"),
			progname);
		if (num_cnt==ind_ret) fprintf(stderr,"Unable to set "
				"Numerals value as given in mp.conf\n");
		if (ori_cnt==ind_ret) fprintf(stderr,"Unable to set "
				"Orientation value as in mp.conf\n");
		if (con_cnt==ind_ret) fprintf(stderr,"Unable to set "
				"Context value as in mp.conf\n");
		if (tex_cnt==ind_ret) fprintf(stderr,"Unable to set "
				"TextShaping value as in mp.conf\n");
		if (swa_cnt==ind_ret) fprintf(stderr,"Unable to set "
				"Swapping value as in mp.conf\n");
		exit(0);
	}
	free(lv);
	free(lr);
}

static int read_ctl_config_options(void) {
	int opt_cnt=0;
	if(target_printer) return 0; /* config file not read */

	if((orientation  = (unsigned int)read_orientation())!=NULL) opt_cnt++;
	if((numerals  = (unsigned int)read_numerals())!=NULL) opt_cnt++;
	if((textshaping  = (unsigned int)read_textshaping())!=NULL) opt_cnt++;
	if((swapping  = (unsigned int)read_swapping())!=NULL) opt_cnt++;
	if((context  = (unsigned int)read_context())!=NULL) opt_cnt++;
	return (opt_cnt);
}
	
void free_transform_arr(transform_arr *t_arr) {
	free(t_arr->outbuff);
	free(t_arr->inptoout);
	free(t_arr->outtoinp);
	free(t_arr->property);
	free(t_arr);
}

transform_arr *ctl_processing( unsigned char *s) {
	int opt;
	static int first_time=1;

	if(first_time==1) {
		opt=read_ctl_config_options();
		set_layout_object(opt);
		get_n_parseShape();
		first_time++;
	}	
	return(transform_inputline(s));
}	

static BooleanValue is_ctl( LayoutObject plh ) {
	size_t index = 0;
	LayoutValueRec lo_values[3];
	int rc;
	BooleanValue directional, shaping ;

	lo_values[0].name = ActiveShapeEditing; 
	lo_values[0].value = (LayoutValue) &shaping;
	lo_values[1].name = ActiveDirectional;
	lo_values[1].value = (LayoutValue) &directional; 
	lo_values[2].name = 0;

	rc = m_getvalues_layout(plh, lo_values, &index);
	if (rc) {
		perror("m_getvalues_layout");
		exit(1);
	}
	return ( shaping || directional );


}
static bool create_layout_obj(LayoutObject *l_o) {
	errno = 0;
	*l_o = m_create_layout(NULL, NULL);
	if (*l_o == (LayoutObject)0) {
		if(errno) {
			errno=0;
			return FALSE;
		}
	}	
	return TRUE;
}

bool ctl_check(void) {
	bool obj_flag, ctl_flag ;
	if ( ( obj_flag=create_layout_obj(&layout_obj) ) == TRUE ) {
		if ( ( ctl_flag = is_ctl(layout_obj) ) == TRUE ) {
			return(TRUE);
		}
	} 
	return(FALSE);
	
}
