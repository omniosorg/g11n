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
 *      (c) Copyright 1995 Sun Microsystems, Inc. 
 *	All rights reserved.  
 *	See LEGAL NOTICE file for terms of the license.
 */

#include <malloc.h>
#include <wchar.h> 
#include "layout.h"
#include "layout_int.h"
#define	 NUM_MODIFIERS 8


/* 
 *   This module contains the functions to parse the layout services modifier
 *   and return it as LayoutValues.
 */
 
typedef struct {
    char		*str;
    int			value;
} MappingRec;

MappingRec	name_map_tbl[] = {
    "orientation=",		Orientation,
    "context=",			Context,
    "typeoftext=",		TypeOfText,
    "implicitalg=",		ImplicitAlg,
    "swapping=",		Swapping,
    "numerals=",		Numerals,
    "shaping=",			TextShaping,
    "checkmode=",		CheckMode,
    "shapcharset=",		ShapeCharset,
};

MappingRec	orient_tbl[] = {
    "ltr",		ORIENTATION_LTR,
    "rtl",		ORIENTATION_RTL,
    "ttblr",		ORIENTATION_TTBRL,
    "ttbrl",		ORIENTATION_TTBLR,
    "contextual",	ORIENTATION_CONTEXTUAL,
};

MappingRec	context_tbl[] = {
    "ltr",		CONTEXT_LTR,
    "rtl",		CONTEXT_RTL,
};

MappingRec	text_type_tbl[] = {
    "visual",		TEXT_VISUAL,
    "implicit",		TEXT_IMPLICIT,
    "explicit",		TEXT_EXPLICIT,
};

MappingRec	algor_tbl[] = {
    "basic",		ALGOR_BASIC,
    "implicit",		ALGOR_IMPLICIT,
};

MappingRec	swapping_tbl[] = {
    "yes",		SWAPPING_YES,
    "no",		SWAPPING_NO,
};

MappingRec	num_tbl[] = {
    "nominal",		NUMERALS_NOMINAL,
    "national",		NUMERALS_NATIONAL,
    "contextual",	NUMERALS_CONTEXTUAL,

};

MappingRec	shape_tbl[] = {
    "shaped",		TEXT_SHAPED,
    "nominal",		TEXT_NOMINAL,
    "shform1",		TEXT_SHFORM1,
    "shform2",		TEXT_SHFORM2,
    "shform3",		TEXT_SHFORM3,
    "shform4",		TEXT_SHFORM4,
};


MappingRec	check_tbl[] = {
    "stream",		MODE_STREAM,
    "edit",		MODE_EDIT,
};



int tbl_cnt[] = {5,2,3,2,2,3,6,2 }; /* table of possible values of modifier attr*/

MappingRec      *tbl_index[8] = {
		orient_tbl,
		context_tbl,
		text_type_tbl,
		algor_tbl,
		swapping_tbl,
		num_tbl,
		shape_tbl,
		check_tbl
};


static char *
SkipSpace(str)
     char	*str;
{
     while (str && (isspace(*str)))
         str++;
         
     return(str);
}
     
static int
GetNumOfValue(modifier)
    char 		*modifier;
{
    int			result = 0;
    
    while (modifier) {
        if (strncmp(*modifier, '=', 1) == 0)
             result++;
        modifier++;
    }
    
    return result;
    
}

static void
GetValueByName(modifier, name, value_tbl, tbl_size, layout_values)
    char 			*modifier;
    MappingRec		 	name;
    MappingRec			value_tbl[];
    int				tbl_size;
    LayoutValues		layout_values;
{
#define	MAX_BUF_LEN		200
    char		dummy_buf1[MAX_BUF_LEN];
    int			i;
    char		*str1;
    
    modifier = SkipSpace(modifier);
    
    str1 = (char *)strstr(modifier, name.str);
    if (str1) {
        layout_values->name = name.value;
        dummy_buf1[0] = NULL;
        
        str1 = (char *)strchr(str1, '=');
        str1++;
        str1 = SkipSpace(str1);
        for (i = 0; (str1 && (i < MAX_BUF_LEN)); i++) {
            if (isspace(*str1)) {
                dummy_buf1[i] = NULL;
            } else if (*str1 == ',') {
                dummy_buf1[i] = NULL;
                break;
            } else
                dummy_buf1[i] = *str1;
            str1++;
        }
        
        
        if (dummy_buf1[0] != NULL) {
            if (value_tbl) {
                int 	num_of_item = tbl_size / sizeof(MappingRec);
                
                for (i = 0; i < num_of_item; i++) {
                    if (strcmp(dummy_buf1, value_tbl[i].str) == 0) {
                        layout_values->value = (LayoutValue)value_tbl[i].value;
                        break;   
                    }
                }	
            } else
                layout_values->value = (LayoutValue)strdup(dummy_buf1);
        }
    }
}


static void
GetDescByName(modifier, name, value_tbl, tbl_size, text_desc, mask)
    char 			*modifier;
    MappingRec		 	name;
    MappingRec			value_tbl[];
    int				tbl_size;
    LayoutTextDescriptor	text_desc;
    int				*mask;
{
#define	MAX_BUF_LEN		200
    char		dummy_buf1[MAX_BUF_LEN];
    char		dummy_buf2[MAX_BUF_LEN];
    int			i;
    int			has_out_value = FALSE;
    char		*str1;
    
    modifier = SkipSpace(modifier);
    
    /* match the modifier value specified by the user and the name string
     * of the table which was picked from name_map_tbl
     */
    str1 = (char *)strstr(modifier, name.str);
    if (str1) {
        *mask |= name.value; /* store value from name_map_tbl */
        dummy_buf1[0] = dummy_buf2[0] = NULL;
        
        str1 = (char *)strchr(str1, '=');
        str1++;
        str1 = SkipSpace(str1);
	/* parse for invalue:outvalue string */
        for (i = 0; (str1 && (i < MAX_BUF_LEN)); i++) {
            if (isspace(*str1)) {
                dummy_buf1[i] = NULL;
            } else if (*str1 == ',') {
                dummy_buf1[i] = NULL;
                break;
            } else if (*str1 == ':') {
                dummy_buf1[i] = NULL;
                str1++;
                has_out_value = TRUE;
                break;
            } else
                dummy_buf1[i] = *str1;
            str1++;
        }
        
/* If outvalue is specified using a colon, parse it and obtain the value */
        if (has_out_value) {
            str1 = SkipSpace(str1);
                        
	    for (i = 0; (str1 && (i < MAX_BUF_LEN)); i++) {
	        if (isspace(*str1)) {
                     dummy_buf2[i] = NULL;
                     break;
                } else if (*str1 == ',') {
		    dummy_buf2[i] = NULL;
		    break;
		} else
		    dummy_buf2[i] = *str1;
		str1++;
	    }
        }
        
        if (dummy_buf1[0] != NULL) {
           int		num_of_item = tbl_size / sizeof(MappingRec);
            
/* parse thru the table, match the string and obtain the corresponding value
 * from the global table referred to by value_tbl
 */
            for (i = 0; i < num_of_item; i++) {
                 if (strcmp(dummy_buf1, value_tbl[i].str) == 0) {
                      text_desc->inp |= value_tbl[i].value;
                      if ((!has_out_value) || dummy_buf2[0] == NULL)
                           text_desc->out |= value_tbl[i].value;
                      break;   
                 }
            }
            
            if (has_out_value ) {
                  for (i = 0; i < num_of_item; i++) {
                       if (strcmp(dummy_buf2, value_tbl[i].str) == 0) {
                           text_desc->out |= value_tbl[i].value;
                           break;   
                       }
                 }
            }
        }
    }
}


LayoutValues
_ModifierToLayoutValues(layout_obj, modifier)
    LayoutObj		layout_obj;
    char 		*modifier;

{


    LayoutValues		layout_values, temp_ptr;
    LayoutTextDescriptorRec	text_desc;
    int				mask = 0;
    int				i;
    
    modifier = SkipSpace(modifier);
    
    if (strncmp(modifier, "@ls", 3) != 0)
        return((LayoutValues)NULL);
        
    temp_ptr = layout_values = (LayoutValues)malloc(sizeof(LayoutValueRec) * 6);
    for (i= 0; i < 6 ; i++) {
	temp_ptr[i].name = 0;
	temp_ptr[i].value = 0;
    }
        
    layout_values->name = 0;
    layout_values->value = 0;
    
    text_desc.inp = text_desc.out = 0;

/*
 *  Current implementation ignore all illegal values.
 *  This will need to be change if the require errors to be raised.
 */    
    (void) GetDescByName(modifier, name_map_tbl[0], orient_tbl, sizeof(orient_tbl), &text_desc, &mask);
    (void) GetDescByName(modifier, name_map_tbl[1], context_tbl, sizeof(context_tbl), &text_desc, &mask);
    (void) GetDescByName(modifier, name_map_tbl[2], text_type_tbl, sizeof(text_type_tbl),  &text_desc, &mask);
    (void) GetDescByName(modifier, name_map_tbl[3], algor_tbl, sizeof(algor_tbl),  &text_desc, &mask);
    (void) GetDescByName(modifier, name_map_tbl[4], swapping_tbl,  sizeof(swapping_tbl), &text_desc, &mask);
    (void) GetDescByName(modifier, name_map_tbl[5], num_tbl,  sizeof(num_tbl), &text_desc, &mask);
    (void) GetDescByName(modifier, name_map_tbl[6], shape_tbl,  sizeof(shape_tbl), &text_desc, &mask);
    
    
    if (mask) {
        temp_ptr->name = InOutTextDescrMask;
        temp_ptr->value = (LayoutValue)mask;
        temp_ptr++;
        
        temp_ptr->name = InOnlyTextDescr;
        temp_ptr->value = (LayoutValue)text_desc.inp;
        temp_ptr++;

        temp_ptr->name = OutOnlyTextDescr;
        temp_ptr->value = (LayoutValue)text_desc.out;
        temp_ptr++;

    }
    
    (void) GetValueByName(modifier, name_map_tbl[7],  check_tbl,  sizeof(check_tbl), temp_ptr);
    if (temp_ptr->name)
        temp_ptr++;
    
    (void) GetValueByName(modifier, name_map_tbl[8],  NULL, 0, temp_ptr);
    
    if (temp_ptr->name) {
        temp_ptr++;
        temp_ptr->name = 0;
        temp_ptr->value = 0;
    }
    

    return(layout_values);
    
}

LayoutValues
_LayoutValuesToModifier(layout_obj, modifier)
    LayoutObj		layout_obj;
    char 		*modifier;

{


    LayoutValues		layout, temp_ptr;
    LayoutTextDescriptor	attr_value;
    LayoutDesc 			descr;
    int				mask = 0;
    char			*mptr = modifier; /* modifier ptr index */
    int				index, i, j;

    strcpy (modifier, "@ls= "); 
    layout=(LayoutValues)malloc((NUM_MODIFIERS + 1)*sizeof(LayoutValueRec));
    for (i = 0; i < NUM_MODIFIERS; i++) {
       switch(i){
	  case 0 :
		 layout[i].name=Orientation;
                 break;
	  case 1 :
		 layout[i].name=Context;
                 break;
	  case 2 :
		 layout[i].name=TypeOfText;
                 break;
	  case 3 :
		 layout[i].name=ImplicitAlg;
                 break;
	  case 4 :
		 layout[i].name=Swapping;
                 break;
	  case 5 :
		 layout[i].name=Numerals;
                 break;
	  case 6 :
		 layout[i].name=TextShaping;
                 break;
	  case 7 :
		 layout[i].name=CheckMode;
    		 layout[i].value=(LayoutDesc *)&(descr);
                 break;
	  default: break;
                }
	if (i != (NUM_MODIFIERS - 1)) /* Dont malloc for the check mode item */
          layout[i].value=(caddr_t )malloc(sizeof(LayoutTextDescriptorRec));
    }
    layout[NUM_MODIFIERS].name = 0;

    m_getvalues_layout(layout_obj, layout, &index);
    for (i = 0; i < NUM_MODIFIERS; i++) { /* Parse all the modifier attr */
	char invalue[30], outvalue[30];
	MappingRec *value_tbl = tbl_index[i]; /* choose attribute table */
	attr_value = (LayoutTextDescriptor) layout[i].value;
	strcat (modifier, name_map_tbl[i].str); /* copy attr= */
	/* look for values of this attribute */
	for (j = 0; j < tbl_cnt[i]; j++) {        
	    if (value_tbl[j].value == attr_value->inp) 
	        strcpy (invalue, value_tbl[j].str);
	    if (value_tbl[j].value == attr_value->out) 
	        strcpy (outvalue, value_tbl[j].str);
	}
	strcat(modifier, invalue);
	strcat(modifier, ":");
	strcat(modifier, outvalue);
	if (i != (NUM_MODIFIERS -1)) strcat(modifier, ","); /* Dont add comma at the end */
    }
}
