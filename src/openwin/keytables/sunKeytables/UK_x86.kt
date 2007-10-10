#ident "@(#)UK_x86.kt	35.2	00/01/27 SMI"
#
# Copyright (c) 1993, Sun Microsystems, Inc.  RESTRICTED RIGHTS LEGEND:
# Use, duplication, or disclosure by the Government is subject to
# restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in
# Technical Data and Computer Software clause at DFARS 52.227-7013 and
# in similar clauses in the FAR and NASA FAR Supplement.
#
#       Copyright (c) 1993 Sun Microsystems, Inc.
#
# Keytable for a PC/AT United Kingdom 101 keyboard (UK_x86.kt)
#
# -------------------  PC LAYOUT  -----------------------
#
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
shift	 44	 57		# Left Shift, Right Shift
lock	 30			# CapsLock
control	 58	 64		# Control
mod1	  0 			# Meta
mod2	 62			# AltGraph
mod3	 90			# NumLock
mod4	 60			# Left Alt
mod5	  0			# None

#Keyword -- do not remove
END

######################################################################

#Keyword -- do not remove
KEYSYMMAP

#Keyword -- do not remove
MINSCANCODE 1

#Keyword -- do not remove
MAXSCANCODE 129

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
110	NN	XK_Escape

112 	RN	XK_F1
113	RN	XK_F2
114 	RN	XK_F3
115	RN	XK_F4

116 	RN	XK_F5
117	RN	XK_F6
118	RN	XK_F7
119	RN	XK_F8

120	RN	XK_F9
121 	RN	XK_F10
122 	RN	SunXK_F36	# F11 - Sparc calls this SunXK_F36
123 	RN	SunXK_F37	# F12

124     NN      XK_Print        SunXK_Sys_Req
125     NN      XK_Scroll_Lock
126     NN      XK_Pause        XK_Break

#############################   Second Row   ###############################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#
1 	RN	XK_quoteleft	XK_notsign	XK_brokenbar
2	RN	XK_1		XK_exclam
3	RN	XK_2		XK_quotedbl
4	RN	XK_3		XK_sterling
5	RN	XK_4		XK_dollar
6	RN	XK_5		XK_percent
7	RN	XK_6		XK_asciicircum
8	RN	XK_7		XK_ampersand
9	RN	XK_8		XK_asterisk
10	RN	XK_9		XK_parenleft
11	RN	XK_0		XK_parenright
12	RN	XK_minus	XK_underscore
13	RN	XK_equal	XK_plus
15 	RN	XK_BackSpace

75	NN	XK_Insert
80	NN	XK_Home
85	NN	XK_Prior

90	NP	XK_Num_Lock
95	RN	XK_KP_Divide	
100	RN	XK_KP_Multiply
105	RN	XK_KP_Subtract

#############################   Third Row   ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#
16	RN	XK_Tab
17	RN	XK_Q
18	RN	XK_W
19	RN	XK_E
20	RN	XK_R
21	RN	XK_T
22	RN	XK_Y
23	RN	XK_U
24	RN	XK_I
25	RN	XK_O
26	RN	XK_P
27	RN	XK_bracketleft	XK_braceleft
28	RN	XK_bracketright	XK_braceright

29	RN	XK_numbersign	XK_asciitilde

76	RN	XK_Delete
81	NN	XK_End
86	NN	XK_Next

91	RN	XK_Home		XK_KP_7		XK_KP_7
96	RN	XK_Up		XK_KP_8		XK_KP_8
101	RN	XK_Prior	XK_KP_9		XK_KP_9
106	RN	XK_KP_Add


#############################   Fourth Row   ###############################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#
30	NP	XK_Caps_Lock
31	RN	XK_A
32	RN	XK_S
33	RN	XK_D
34	RN	XK_F
35	RN	XK_G
36	RN	XK_H
37	RN	XK_J
38	RN	XK_K
39	RN	XK_L
40	RN	XK_semicolon	XK_colon
41	RN	XK_quoteright	XK_at
43	RN	XK_Return

92	RN	XK_Left		XK_KP_4		XK_KP_4
97	RN	XK_KP_5		XK_KP_5		XK_KP_5
102	RN	XK_Right	XK_KP_6		XK_KP_6

#############################   Fifth Row   ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#
44	NN	XK_Shift_L
45	RN	XK_backslash	XK_bar
46	RN	XK_Z
47	RN	XK_X
48	RN	XK_C
49	RN	XK_V
50	RN	XK_B
51	RN	XK_N
52	RN	XK_M
53	RN	XK_comma	XK_less
54	RN	XK_period	XK_greater
55	RN	XK_slash	XK_question
57	NN	XK_Shift_R

83	RN	XK_Up

93	RN	XK_End		XK_KP_1		XK_KP_1
98	RN	XK_Down		XK_KP_2		XK_KP_2
103	RN	XK_Next		XK_KP_3		XK_KP_3
108	RN	XK_KP_Enter

#############################   Sixth Row   ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#
58	NN	XK_Control_L

60	NN	XK_Alt_L
61	RN	XK_space
62	NN	SunXK_AltGraph

64	NN	XK_Control_R

79	RN	XK_Left
84	RN	XK_Down
89	RN	XK_Right

99	NN	XK_KP_Insert	XK_KP_0		XK_KP_0
104	RN	XK_Delete	XK_KP_Decimal	XK_KP_Decimal

#############################   Synthetic   ################################
#
#Scan  Atts     Col1            Col2            Col3		Col4
#

128     NN      XK_Multi_key
129     NN      XK_Mode_switch

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

# Type 101 keyboard has 3 LEDs.

#Keyword -- do not remove
MAXLED 3

XK_Num_Lock	1	# NumLock
XK_Scroll_Lock	2	# ScrollLock
XK_Caps_Lock	3	# CapsLock

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

60		# XK_Alt_L
126             # XK_Pause
76		# XK_Delete

#Keyword -- do not remove
END

######################################################################
