#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#

#ident "@(#)Latvia6.kt 1.2	06/03/15 KEYTABLES SMI"
#
# Copyright (c) 1991, Sun Microsystems, Inc.  RESTRICTED RIGHTS LEGEND:
# Use, duplication, or disclosure by the Government is subject to
# restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in
# Technical Data and Computer Software clause at DFARS 52.227-7013 and
# in similar clauses in the FAR and NASA FAR Supplement.
#
#       Copyright (c) 1991 Sun Microsystems, Inc.
#
# Keytable for a Sun USB Latvian keyboard (SUN-USB.kt) 
######################################################################
# Important Note:
#
# If you would just like to remap some keys on your keyboard
# you should use xmodmap rather than building a new keytable.
#
######################################################################
# The current (default) column mappings are for standard X (see
# Section 5 of the X Window System Protocol):
#     
# -----------------------------------------------------------------------------
# |     column 1     |     column 2     |     column 3     |    column 4      |
# =============================================================================
# |      no mods     |      Shift       |    ModeSwitch    | ModeSwitch Shift |
# -----------------------------------------------------------------------------
#
# To build a keytable for a new keyboard layout, you will need the
# scancode map, i.e., a scancode for each hardware key, and a keysym
# map, i.e., the symbols on the keycap associated with each scancode.
# You may also want to include an LED map, i.e., a number (1 to 32)
# assigned to each existing LED (e.g., the CapsLock LED).
#
# You should probably start with a copy of an existing keytable and
# replace the existing scancodes with your new scancodes.  Then,
# incrementally modify entries as needed.  To test a new keytable,
# copy it to $HOME/.keytable and start Xsun.
#
# There are two maps that need to be constructed:
# 1. Modifier map
# 2. Keysym map
#
# and one optional map:
# 3. LED map
#
# To reduce loading time at startup, remove all comments and convert
# spaces to tabs.
######################################################################

#Keyword -- do not remove
MODMAP

#Keyword -- do not remove
MAXKEYSPERMODIFIER 2

#
# Modifier map
#
# The modifier map gets loaded according to the entries below.
# Scancodes generally range from 1 to 255.  Since X keycodes range
# from 8 to 255, the MAXSCANCODE must be less than or equal to the
# MINSCANCODE + 247.  Consult your keyboard documentation to obtain
# the scancode for each key.
#
# Note that you enter **scancodes** in the entries below, not keycodes.
# For example, if you wanted the "lock" modifier group to contain the
# keycode for XK_Caps_Lock, you would enter the scancode for the key
# that has "CapsLock" printed on it.
#
# Notes:
# The xmodmap utility can be used to verify the map during development.
# In the output from xmodmap, KEYCODE = SCANCODE + (8 - MINSCANCODE).
#
# Format of an entry:
# modifier_group_identifier scancode scancode ...
#
shift	225	229		# Left Shift, Right Shift
lock	57			# CapsLock
control	224	228		# Control
mod1	227	231		# Left Meta, Right Meta
mod2	230			# Altgraph
mod3	83			# NumLock
mod4	226			# Alt
mod5	0			# None

#Keyword -- do not remove
END

######################################################################

#Keyword -- do not remove
KEYSYMMAP

#Keyword -- do not remove
MINSCANCODE 1

#Keyword -- do not remove
MAXSCANCODE 247

#
# Keysym table
#
# The goal here is to specify all of the different keysyms that
# appear on each phyical key.
#
# If "R" appears as the first attribute after the scancode, the
# appropriate action will be taken to make this key REPEAT.
# The default is NOREPEAT so be sure to specify "R" for all keys
# you wish to be repeating keys (repeat keys generate multiple
# keystrokes when held down -- see man page for xset).
#
# If "P" appears as the second attribute after the scancode, the
# appropriate action will be taken to make the key a PSEUDOLOCK key.
# The default is NOPSEUDOLOCK.  Pseudolock keys are keys that do
# not physically lock in the down state.  The server simulates lock
# key action by setting and clearing the appropriate modifier bit
# in the modifier state mask on alternate KeyPress and KeyRelease
# events of KeyPress-KeyRelease pairs.  In other words, the first
# time you press (and release) the key, lock is turned ON, the
# next time you press (and release) the key, lock is turned OFF, etc.
#
# Format of an entry:
# scancode {N|R}{N|P} keysym keysym ...
#
# Entry template:
# 000	RN	XK_Keysym	XK_Keysym	XK_Keysym	XK_Keysym
#
# Notes:
# Entries may be in any order.	If there are duplicates, the
# last of the duplicate entries takes precedence.
# No space may appear between the attribute code letters.
#

#############################   First Row   ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#

117	NN	XK_Help

41	NN	XK_Escape

58	RN	XK_F1
59	RN	XK_F2
60	RN	XK_F3
61	RN	XK_F4

62	RN	XK_F5
63	RN	XK_F6
64	RN	XK_F7
65	RN	XK_F8

66	RN	XK_F9
67	RN	XK_F10
68	RN	SunXK_F36
69	RN	SunXK_F37

127	NN	SunAudioMute		SunVideoDegauss
129	RN	SunAudioLowerVolume SunVideoLowerBrightness
128	RN	SunAudioRaiseVolume SunVideoRaiseBrightness
102	NN	SunPowerSwitch	SunPowerSwitchShift

70	NN	XK_R2		XK_R2		XK_Print	SunXK_Sys_Req
71	NN	XK_R3		XK_R3		XK_Scroll_Lock
72	NN	XK_R1		XK_R1		XK_Pause	XK_Break

#############################   Second Row   ###############################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#

120	NN	XK_L1		XK_L1		SunXK_Stop
121	NN	XK_L2		XK_L2		SunXK_Again

53	RN	XK_quoteleft   	XK_asciitilde   XK_hyphen	XK_question    
30	RN	XK_1		XK_exclam
31	RN	XK_2		XK_at		XK_2	XK_guillemotleft
32	RN	XK_3		XK_numbersign  	XK_3    XK_guillemotright	
33	RN	XK_4		XK_dollar	XK_4	XK_section
34	RN	XK_5		XK_percent	XK_EuroSign
35	RN	XK_6		XK_asciicircum  XK_6	XK_slash
36	RN	XK_7		XK_ampersand
37	RN	XK_8		XK_asterisk	XK_8	XK_multiply	
38	RN	XK_9		XK_parenleft
39	RN	XK_0		XK_parenright
45	RN	XK_minus	XK_underscore
46	RN	XK_equal	XK_plus		XK_f	XK_F
42	RN	XK_BackSpace


73	NN	XK_Insert
74	NN	XK_Home
75	NN	XK_Prior

83	NP	XK_Num_Lock
84	RN	XK_R5		XK_R5		XK_KP_Divide
85	RN	XK_R6		XK_R6		XK_KP_Multiply
86	RN	XK_R4		XK_R4		XK_KP_Subtract

#############################   Third Row   ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#

118	NN	XK_L3		XK_L3		SunXK_Props
122	NN	XK_L4		XK_L4		SunXK_Undo

43	RN	XK_Tab
20	RN	XK_q		XK_Q		XK_umacron	XK_Umacron
26	RN	XK_w		XK_W		XK_g		XK_G
8	RN	XK_e		XK_E		XK_j		XK_J
21	RN	XK_r		XK_R		
23	RN	XK_t		XK_T		XK_m		XK_M
28	RN	XK_y		XK_Y		XK_v		XK_V
24	RN	XK_u		XK_U		XK_n		XK_N
12	RN	XK_i		XK_I		XK_z		XK_Z
18	RN	XK_o		XK_O		XK_emacron	XK_Emacron
19	RN	XK_p		XK_P		XK_ccaron	XK_Ccaron
47	RN	XK_bracketleft	XK_braceleft	XK_zcaron	XK_Zcaron
48	RN	XK_bracketright	XK_braceright	XK_h		XK_H


76	RN	XK_Delete
77	NN	XK_End
78	NN	XK_Next

95	RN	XK_R7		XK_R7		XK_KP_7		XK_Home
96	RN	XK_Up		XK_R8		XK_KP_8
97	RN	XK_R9		XK_R9		XK_KP_9		XK_Prior
87	RN	XK_KP_Add

#############################   Fourth Row  ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#

119	NN	XK_L5		XK_L5		SunXK_Front
124	NN	XK_L6		XK_L6		SunXK_Copy	

57	NP	XK_Caps_Lock
4	RN	XK_a		XK_A		XK_scaron	XK_Scaron  
22	RN	XK_s		XK_S		XK_u		XK_U
7	RN	XK_d		XK_D		XK_s		XK_S
9	RN	XK_f		XK_F		XK_i		XK_I
10	RN	XK_g		XK_G		XK_l		XK_L
11	RN	XK_h		XK_H		XK_d		XK_D
13	RN	XK_j		XK_J		XK_a		XK_A
14	RN	XK_k		XK_K		XK_t		XK_T
15	RN	XK_l		XK_L		XK_e		XK_E
51	RN	XK_semicolon	XK_colon	XK_c		XK_C
52	RN	XK_apostrophe	XK_quotedbl	XK_apostrophe	XK_degree
49	RN	XK_degree	XK_brokenbar	XK_kcedilla	XK_Kcedilla
50      RN      XK_degree       XK_brokenbar    XK_kcedilla     XK_Kcedilla

40	RN	XK_Return


92	RN	XK_Left		XK_R10		XK_KP_4
93	RN	XK_R11		XK_R11		XK_KP_5
94	RN	XK_Right	XK_R12		XK_KP_6

#############################   Fifth Row   ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#

116	NN	XK_L7		XK_L7		SunXK_Open
125	NN	XK_L8		XK_L8		SunXK_Paste

225	NN	XK_Shift_L
100	RN	XK_backslash	XK_bar		XK_gcedilla	XK_Gcedilla
29	RN	XK_z		XK_Z		XK_ncedilla	XK_Ncedilla
27	RN	XK_x		XK_X		XK_b		XK_B
6	RN	XK_c		XK_C		XK_imacron	XK_Imacron
25	RN	XK_v		XK_V		XK_k		XK_K
5	RN	XK_b		XK_B		XK_p		XK_P
17	RN	XK_n		XK_N		XK_o		XK_O
16	RN	XK_m		XK_M		XK_amacron	XK_Amacron
54	RN	XK_comma	XK_less		XK_comma	XK_semicolon	
55	RN	XK_period	XK_greater	XK_period	XK_colon
56	RN	XK_slash	XK_question	XK_lcedilla	XK_Lcedilla
229	NN	XK_Shift_R


82	RN	XK_Up

89	RN	XK_R13		XK_R13		XK_KP_1		XK_End
90	RN	XK_Down		XK_R14		XK_KP_2
91	RN	XK_R15		XK_R15		XK_KP_3		XK_Next
88	RN	XK_KP_Enter

#############################   Sixth Row   ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#

126	NN	XK_L9		XK_L9		SunXK_Find
123	NN	XK_L10		XK_L10		SunXK_Cut

224	NN	XK_Control_L
228	NN	XK_Control_R
226	NN	XK_Alt_L
227	NN	XK_Meta_L
44	RN	XK_space
231	NN	XK_Meta_R
101	NN	SunXK_Compose
230	NN	SunXK_AltGraph

80	RN	XK_Left
81	RN	XK_Down
79	RN	XK_Right

98	NN	XK_KP_Insert	XK_KP_Insert	XK_KP_0
99	RN	XK_Delete	XK_Delete 	XK_KP_Decimal

#Keyword -- do not remove
END

######################################################################

#Keyword -- do not remove
LEDMAP

# LED map
#
# The LED map gets placed in a property on the root window.
#
# Format of an entry:
# keysym number
#

# SUN USB keyboard has 4 LEDs.

#Keyword -- do not remove
MAXLED 4

XK_Num_Lock	1	# NumLock
XK_Multi_key	2	# Compose
XK_Scroll_Lock	3	# ScrollLock
XK_Caps_Lock	4	# CapsLock

#Keyword -- do not remove
END

######################################################################

#Keyword -- do not remove
ESCSEQUENCE

# Escape Sequence
#
# The Escape Sequence is stored in the server.
#
# Format of an entry:
# scancode
#
# Notes: Entries appear in the order they must be held down to trigger an
# escape from the server. The maximum length of an escape sequence is 5 keys.
#

120		# XK_L1
226		# XK_Alt_L
76		# XK_Delete

#Keyword -- do not remove
END

######################################################################
