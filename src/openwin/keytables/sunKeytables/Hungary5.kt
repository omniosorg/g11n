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

#ident @(#)Hungary5.kt 1.3 93/02/05 KEYTABLES SMI

#Keyword -- do not remove
MODMAP

#Keyword -- do not remove
MAXKEYSPERMODIFIER 2

# Format of an entry:
# modifier_group_identifier scancode scancode ...
#
shift	 99	 110		# Left Shift, Right Shift
lock	119			# CapsLock
control	 76			# Control
mod1	120	 122		# Left Meta, Right Meta
mod2	 13			# Altgraph
mod3	 98			# NumLock
mod4	 19			# Alt
mod5	  0			# None

#Keyword -- do not remove
END

######################################################################

#Keyword -- do not remove
KEYSYMMAP

#Keyword -- do not remove
MINSCANCODE 1

#Keyword -- do not remove
MAXSCANCODE 125



#############################   First Row   ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#

118	NN	XK_Help

29	NN	XK_Escape

5	RN	XK_F1
6	RN	XK_F2
8	RN	XK_F3
10	RN	XK_F4

12	RN	XK_F5
14	RN	XK_F6
16	RN	XK_F7
17	RN	XK_F8

18	RN	XK_F9
7	RN	XK_F10
9	RN	SunXK_F36
11	RN	SunXK_F37

22	NN	XK_R2		XK_R2		XK_Print	SunXK_Sys_Req
23	NN	XK_R3		XK_R3		XK_Scroll_Lock
21	NN	XK_R1		XK_R1		XK_Pause	XK_Break

#############################   Second Row   ###############################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#

1	NN	XK_L1		XK_L1		SunXK_Stop
3	NN	XK_L2		XK_L2		SunXK_Again

42	RN	XK_0		XK_section
30      RN      XK_1		XK_quoteright
31      RN      XK_2		XK_quotedbl
32      RN	XK_3       	XK_plus 	XK_numbersign
33	RN      XK_4            XK_exclam	XK_dollar
34      RN      XK_5       	XK_percent
35      RN      XK_6       	XK_slash	XK_asciicircum
36      RN      XK_7       	XK_equal	XK_braceleft
37      RN      XK_8       	XK_parenleft	XK_bracketleft
38      RN      XK_9       	XK_parenright	XK_bracketright
39      RN      XK_odiaeresis	XK_Odiaeresis	XK_braceright
40      RN      XK_udiaeresis	XK_Udiaeresis 	XK_backslash
41      RN      XK_oacute       XK_Oacute	XK_asciitilde
43	RN	XK_BackSpace

44	NN	XK_Insert
52	NN	XK_Home
96	NN	XK_Prior

98	NP	XK_Num_Lock
46	RN	XK_R5		XK_R5		XK_KP_Divide
47	RN	XK_R6		XK_R6		XK_KP_Multiply
71	RN	XK_R4		XK_R4		XK_KP_Subtract

#############################   Third Row   ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#

25	NN	XK_L3		XK_L3		SunXK_Props
26	NN	XK_L4		XK_L4		SunXK_Undo

53	RN	XK_Tab
54	RN	XK_q		XK_Q		XK_at
55	RN	XK_W
56	RN	XK_E
57	RN	XK_R
58	RN	XK_T
59	RN	XK_Z
60	RN	XK_U
61	RN	XK_I
62	RN	XK_O
63	RN	XK_P
64	RN	XK_odoubleacute	XK_Odoubleacute
65	RN	XK_uacute	XK_Uacute

66	RN	XK_Delete
74	NN	XK_End
123	NN	XK_Next

68	RN	XK_R7		XK_R7		XK_KP_7		XK_Home
69	RN	XK_Up		XK_R8		XK_KP_8
70	RN	XK_R9		XK_R9		XK_KP_9		XK_Prior
125	RN	XK_KP_Add

#############################   Fourth Row  ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#

49	NN	XK_L5		XK_L5		SunXK_Front
51	NN	XK_L6		XK_L6		SunXK_Copy	

119	NP	XK_Caps_Lock
77	RN	XK_A
78	RN	XK_s		XK_S		XK_ssharp
79	RN	XK_D
80	RN	XK_F
81	RN	XK_G
82	RN	XK_H
83	RN	XK_J
84	RN	XK_k		XK_K		XK_ampersand
85	RN	XK_L
86	RN	XK_eacute	XK_Eacute	XK_semicolon
87	RN	XK_aacute	XK_Aacute	XK_adiaeresis	XK_Adiaeresis
88	RN	XK_udoubleacute	XK_Udoubleacute
89	RN	XK_Return

91	RN	XK_Left		XK_R10		XK_KP_4
92	RN	XK_R11		XK_R11		XK_KP_5
93	RN	XK_Right	XK_R12		XK_KP_6

#############################   Fifth Row   ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#

72	NN	XK_L7		XK_L7		SunXK_Open
73	NN	XK_L8		XK_L8		SunXK_Paste

99	NN	XK_Shift_L
124	RN	XK_iacute	XK_Iacute	XK_bar
100	RN	XK_y		XK_Y		XK_less
101	RN	XK_x		XK_X		XK_greater
102	RN	XK_c		XK_C		XK_quoteleft
103	RN	XK_V
104	RN	XK_B
105	RN	XK_N
106	RN	XK_M
107	RN	XK_comma	XK_question	XK_asterisk
108	RN	XK_period	XK_colon
109	RN	XK_minus	XK_underscore
110	NN	XK_Shift_R

20	RN	XK_Up

112	RN	XK_R13		XK_R13		XK_KP_1		XK_End
113	RN	XK_Down		XK_R14		XK_KP_2
114	RN	XK_R15		XK_R15		XK_KP_3		XK_Next
90	RN	XK_KP_Enter

#############################   Sixth Row   ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#

95	NN	XK_L9		XK_L9		SunXK_Find
97	NN	XK_L10		XK_L10		SunXK_Cut

76	NN	XK_Control_L
19	NN	XK_Alt_L
120	NN	XK_Meta_L
121	RN	XK_space
122	NN	XK_Meta_R
67	NN	SunXK_Compose
13	NN	SunXK_AltGraph

24	RN	XK_Left
27	RN	XK_Down
28	RN	XK_Right

94	NN	XK_KP_Insert	XK_KP_Insert	XK_KP_0
50	RN	XK_Delete	XK_Delete 	XK_KP_Decimal

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

# Type 5 keyboard has 4 LEDs.

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

1		# XK_L1
19		# XK_Alt_L
66		# XK_Delete

#Keyword -- do not remove
END

######################################################################
