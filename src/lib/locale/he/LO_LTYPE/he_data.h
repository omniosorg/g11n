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
 * Copyright (c) 1998, by Sun Microsystems, Inc.
 * All rights reserved.
 */

const int LOtypes::charDataIndex[] = {
0xffff
};
const LOtypes::lotype_t LOtypes::charData[] = {
/*0x0020*/ _CELL ,
/*0x0021*/ _CELL ,
/*0x0022*/ _CELL ,
/*0x0023*/ _CELL ,
/*0x0024*/ _CELL ,
/*0x0025*/ _SEP|_CELL ,
/*0x0026*/ _CELL ,
/*0x0027*/ _CELL ,
/*0x0028*/ _SWAP|_CELL ,
/*0x0029*/ _SWAP|_CELL ,
/*0x002a*/ _SEP|_CELL ,
/*0x002b*/ _SEP|_CELL ,
/*0x002c*/ _SEP|_CELL ,
/*0x002d*/ _SEP|_CELL ,
/*0x002e*/ _SEP|_CELL ,
/*0x002f*/ _SEP|_CELL ,
/*0x0030*/ _NUM|_CELL ,
/*0x0031*/ _NUM|_CELL ,
/*0x0032*/ _NUM|_CELL ,
/*0x0033*/ _NUM|_CELL ,
/*0x0034*/ _NUM|_CELL ,
/*0x0035*/ _NUM|_CELL ,
/*0x0036*/ _NUM|_CELL ,
/*0x0037*/ _NUM|_CELL ,
/*0x0038*/ _NUM|_CELL ,
/*0x0039*/ _NUM|_CELL ,
/*0x003a*/ _SEP|_CELL ,
/*0x003b*/ _CELL ,
/*0x003c*/ _SEP|_SWAP|_CELL ,
/*0x003d*/ _SEP|_CELL ,
/*0x003e*/ _SEP|_SWAP|_CELL ,
/*0x003f*/ _CELL ,
/*0x0040*/ _CELL ,
/*0x0041*/ _LTR|_CELL ,
/*0x0042*/ _LTR|_CELL ,
/*0x0043*/ _LTR|_CELL ,
/*0x0044*/ _LTR|_CELL ,
/*0x0045*/ _LTR|_CELL ,
/*0x0046*/ _LTR|_CELL ,
/*0x0047*/ _LTR|_CELL ,
/*0x0048*/ _LTR|_CELL ,
/*0x0049*/ _LTR|_CELL ,
/*0x004a*/ _LTR|_CELL ,
/*0x004b*/ _LTR|_CELL ,
/*0x004c*/ _LTR|_CELL ,
/*0x004d*/ _LTR|_CELL ,
/*0x004e*/ _LTR|_CELL ,
/*0x004f*/ _LTR|_CELL ,
/*0x0050*/ _LTR|_CELL ,
/*0x0051*/ _LTR|_CELL ,
/*0x0052*/ _LTR|_CELL ,
/*0x0053*/ _LTR|_CELL ,
/*0x0054*/ _LTR|_CELL ,
/*0x0055*/ _LTR|_CELL ,
/*0x0056*/ _LTR|_CELL ,
/*0x0057*/ _LTR|_CELL ,
/*0x0058*/ _LTR|_CELL ,
/*0x0059*/ _LTR|_CELL ,
/*0x005a*/ _LTR|_CELL ,
/*0x005b*/ _SWAP|_CELL ,
/*0x005c*/ _CELL ,
/*0x005d*/ _SWAP|_CELL ,
/*0x005e*/ _CELL ,
/*0x005f*/ _CELL ,
/*0x0060*/ _LTR|_CELL ,
/*0x0061*/ _LTR|_CELL ,
/*0x0062*/ _LTR|_CELL ,
/*0x0063*/ _LTR|_CELL ,
/*0x0064*/ _LTR|_CELL ,
/*0x0065*/ _LTR|_CELL ,
/*0x0066*/ _LTR|_CELL ,
/*0x0067*/ _LTR|_CELL ,
/*0x0068*/ _LTR|_CELL ,
/*0x0069*/ _LTR|_CELL ,
/*0x006a*/ _LTR|_CELL ,
/*0x006b*/ _LTR|_CELL ,
/*0x006c*/ _LTR|_CELL ,
/*0x006d*/ _LTR|_CELL ,
/*0x006e*/ _LTR|_CELL ,
/*0x006f*/ _LTR|_CELL ,
/*0x0070*/ _LTR|_CELL ,
/*0x0071*/ _LTR|_CELL ,
/*0x0072*/ _LTR|_CELL ,
/*0x0073*/ _LTR|_CELL ,
/*0x0074*/ _LTR|_CELL ,
/*0x0075*/ _LTR|_CELL ,
/*0x0076*/ _LTR|_CELL ,
/*0x0077*/ _LTR|_CELL ,
/*0x0078*/ _LTR|_CELL ,
/*0x0079*/ _LTR|_CELL ,
/*0x007a*/ _LTR|_CELL ,
/*0x007b*/ _SWAP|_CELL ,
/*0x007c*/ _SEP|_CELL ,
/*0x007d*/ _SWAP|_CELL ,
/*0x007e*/ _CELL,
/*0x007f dummy */ _END|_CELL,
/*0x00a0*/ _RTL|_CELL ,
/*0x00a1*/ _CELL,
/*0x00a2*/ _CELL,
/*0x00a3*/ _CELL,
/*0x00a4*/ _CELL,
/*0x00a5*/ _CELL,
/*0x00a6*/ _CELL,
/*0x00a7*/ _CELL,
/*0x00a8*/ _CELL,
/*0x00a9*/ _CELL,
/*0x00aa*/ _RTL|_CELL,
/*0x00ab*/ _CELL,
/*0x00ac*/ _RTL|_CELL ,
/*0x00ad*/ _RTL|_CELL ,
/*0x00ae*/ _END|_CELL,
/*0x00af*/ _END|_CELL,
/*0x00b0*/ _END|_CELL,
/*0x00b1*/ _END|_CELL,
/*0x00b2*/ _END|_CELL,
/*0x00b3*/ _END|_CELL,
/*0x00b4*/ _END|_CELL,
/*0x00b5*/ _END|_CELL,
/*0x00b6*/ _END|_CELL,
/*0x00b7*/ _END|_CELL,
/*0x00b8*/ _END|_CELL,
/*0x00b9*/ _END|_CELL,
/*0x00ba*/ _RTL|_CELL,
/*0x00bb*/ _END|_CELL ,
/*0x00bc*/ _END|_CELL,
/*0x00bd*/ _END|_CELL,
/*0x00be*/ _END|_CELL,
/*0x00bf*/ _END|_CELL ,
/*0x00c0 dummy*/ _END|_CELL ,
/*0x00c1 dummy*/ _END|_CELL ,
/*0x00c2 dummy*/ _END|_CELL ,
/*0x00c3 dummy*/ _END|_CELL ,
/*0x00c4 dummy*/ _END|_CELL ,
/*0x00c5 dummy*/ _END|_CELL ,
/*0x00c6 dummy*/ _END|_CELL ,
/*0x00c7 dummy*/ _END|_CELL ,
/*0x00c8 dummy*/ _END|_CELL ,
/*0x00c9 dummy*/ _END|_CELL ,
/*0x00ca dummy*/ _END|_CELL ,
/*0x00cb dummy*/ _END|_CELL ,
/*0x00cc dummy*/ _END|_CELL ,
/*0x00cd dummy*/ _END|_CELL ,
/*0x00ce dummy*/ _END|_CELL ,
/*0x00cf dummy*/ _END|_CELL ,
/*0x00d0 dummy*/ _END|_CELL ,
/*0x00d1 dummy*/ _END|_CELL ,
/*0x00d2 dummy*/ _END|_CELL ,
/*0x00d3 dummy*/ _END|_CELL ,
/*0x00d4 dummy*/ _END|_CELL ,
/*0x00d5 dummy*/ _END|_CELL ,
/*0x00d6 dummy*/ _END|_CELL ,
/*0x00d7 dummy*/ _END|_CELL ,
/*0x00d8 dummy*/ _END|_CELL ,
/*0x00d9 dummy*/ _END|_CELL ,
/*0x00da dummy*/ _END|_CELL ,
/*0x00db dummy*/ _END|_CELL ,
/*0x00dc dummy*/ _END|_CELL ,
/*0x00dd dummy*/ _END|_CELL ,
/*0x00de dummy*/ _END|_CELL ,
/*0x00df*/ _RTL|_CELL,
/*0x00e0*/ _RTL|_CELL ,
/*0x00e1*/ _RTL|_CELL ,
/*0x00e2*/ _RTL|_CELL ,
/*0x00e3*/ _RTL|_CELL ,
/*0x00e4*/ _RTL|_CELL ,
/*0x00e5*/ _RTL|_CELL ,
/*0x00e6*/ _RTL|_CELL ,
/*0x00e7*/ _RTL|_CELL ,
/*0x00e8*/ _RTL|_CELL ,
/*0x00e9*/ _RTL|_CELL ,
/*0x00ea*/ _RTL|_CELL ,
/*0x00eb*/ _RTL|_CELL ,
/*0x00ec*/ _RTL|_CELL ,
/*0x00ed*/ _RTL|_CELL ,
/*0x00ee*/ _RTL|_CELL ,
/*0x00ef*/ _RTL|_CELL ,
/*0x00f0*/ _RTL|_CELL ,
/*0x00f1*/ _RTL|_CELL ,
/*0x00f2*/ _RTL|_CELL ,
/*0x00f3*/ _RTL|_CELL ,
/*0x00f4*/ _RTL|_CELL ,
/*0x00f5*/ _RTL|_CELL ,
/*0x00f6*/ _RTL|_CELL ,
/*0x00f7*/ _RTL|_CELL ,
/*0x00f8*/ _RTL|_CELL ,
/*0x00f9*/ _RTL|_CELL , 
/*0x00fa*/ _RTL|_CELL ,
/*0x00fb dummy*/ _END|_CELL ,
/*0x00fc dummy*/ _END|_CELL ,
/*0x00fd dummy*/ _END|_CELL ,
/*0x00fe dummy*/ _END|_CELL ,
/*0x00ff dummy*/ _END|_CELL
};
const int LOtypes::shapeDataIndex[] = {
0xffff
};    
const LOtypes::shapeData_t LOtypes::shapeData[] = {
 {0xffff,0xffff,0xffff,0xffff}
};
const int LOtypes::comboDataIndex[] = {
0xffff
};
const LOtypes::val_t LOtypes::comboData[] = {
0xffff    
};
const LOtypes::val_t LOtypes::numData[] = {
0
};

const int LOtypes::swapDataIndex[] = {
	(val_t)'(',
	(val_t)')',
	(val_t)'/',
	(val_t)'<',
	(val_t)'>',
        (val_t)'[',
	(val_t)'\\',
	(val_t)']',
	(val_t)'{',
	(val_t)'}'
    };

const LOtypes::val_t LOtypes::swapData[] = {
        (val_t)')',
	(val_t)'(',
	(val_t)'\\',
	(val_t)'>',
	(val_t)'<',
	(val_t)']',
	(val_t)'/',
	(val_t)'[',
	(val_t)'}',
	(val_t)'{'
    }; 


