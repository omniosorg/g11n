$ CDDL HEADER START
$
$ The contents of this file are subject to the terms of the
$ Common Development and Distribution License (the "License").
$ You may not use this file except in compliance with the License.
$
$ You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
$ or http://www.opensolaris.org/os/licensing.
$ See the License for the specific language governing permissions
$ and limitations under the License.
$
$ When distributing Covered Code, include this CDDL HEADER in each
$ file and include the License file at src/OPENSOLARIS.LICENSE.
$ If applicable, add the following below this CDDL HEADER, with the
$ fields enclosed by brackets "[]" replaced with your own identifying
$ information: Portions Copyright [yyyy] [name of copyright owner]
$
$ CDDL HEADER END

$ -------------------------------------------------------------------------
$ This message catalog file is divided into 3 parts WARN_SET 
$ and ERR_SET set, and USAGE_SET. 1 is WARN_SET , set 2 is ERR_SET
$ and 3 is USAGE_SET.
$ ------------------------------------------------------------------------

$quote "

$ set 1 corresponds to #define WARN_SET in the
$ application source code

$ -------------------------------------------------------------------------
$ WARN_SET
$ -------------------------------------------------------------------------

$set 1

$ Warning message when a particular code point does not have valid font.

1 "%s: cannot load Type1/pcf/TT fonts for val 0x%08x\n"

$ Warning message when a font file cannot be loaded.

2 "%s: cannot load font file (%s)\n"

$ Warning message when a llegal layout value is set in config. file.

3 "%s: warning: illegal layout value: %s\n"

$ Not able to set the layout values as given

4 "%s: unable to set layout values as specified\n"

$ Not able to get the extends of the fontset.

5 "%s: Failed to get extents of font set\n"

$ Warning when CTL buffer overflow

6 "%s: Input line too long\n"

$ set 2 corresponds to #define ERR_SET in the
$ applications source code

$ -------------------------------------------------------------------------
$ ERR_SET
$ -------------------------------------------------------------------------

$set 2

$ Error message when not able to open the file given for printing.

1 "%s: Cannot open %s\n"

$ Error message when input file given is empty.

2 "%s: empty input file %s, nothing printed\n"

$ Error message when a common prolgue file is not found.

3 "%s: Unable to open common prologue file %s\n"

$ Error message unable to open the common prologue file.

4 "%s: Common prologue file %s not found.\n"

$ A wrong condition inside ufmap_compare function.

5  "%s: font indexing error for presentation form ranges: %x %x\n and \t\t\t%x %x\n"

$ A wrong condition inside mmap_compare function.

6 "%s: incorrectly specfified %s values found. %s %x %x\n\t%s %x %x\n"

$ Error in reading the font configuration file.

7 "%s: error reading configuration file\n"

$ Error due to a invalid value in configuration file line.

8 "%s: config file line: %d invalid value\n"

$ Configuration file line too long error.

9 "%s: config file line too long; line #%d\n"

$ Configuration file invalid type1 alias name error

10 "%s: config file line # %d invalid Type1 alias name\n"

$  Configuration file unknown font name

11 "%s: config file line: %d; unknown font name (%s).\n"

$ Error message  for internal structure error

12 "%s:  wrong groupmap : internal structure error\n" 

$ Error message  for invalid font group name

13 "%s: config file line: %d; unknown group name (%s).\n"

$ Error message  for very big integers specified in configuration file.

14 "%s: string to integer conversion overflow: %s.\n"

$ Another error message for a wrong entry in configuration file.

15 "%s: config file line: %d; non-numeric in numeric field.\n"

$ Another error message when a starting range greater than ending

16 "%s: config file line:%d starting range greater than ending range\n"

$ Error message when a unknown pcf font name is encountered.

17 "%s: config file line: %d; unknown pcf font name (%s).\n"

$ Error message when a unknown ttf font name is encountered.

18 "%s: config file line: %d; unknown ttf font name (%s).\n"

$ Error message when a unknown type1 font name is encountered.

19 "%s: config file line: %d; unknown type1 font name (%s).\n"

$ Error message when an incomplete configuration file line is discovered

20 "%s:  config file line: %d; missing field(s).\n"

$ Error message when cannot stat font file.

21 "%s: config file line :%d, cannot stat font file (%s)\n"

$ Error message when a unknown keyword appears inside the configuration file.

22 "%s: unknown keyword: %s in line %d\n"

$ Error message when configuration file don't have read permissions.

23 "%s: config file %s don't have read permissions. Exiting..\n"

$ Another configuration file error message.

24 "%s: number %s entries processed != %s count.\n"

$ general error message for malloc failure.

25 "%s: Malloc failure; line %d, file %s\n"

$ Error when trying to open a shared object from the configuration file.

26 "%s: config file %s dlopen error: %s\n"

$ Error when trying to get the address of a symbol defined within the shared 
$ object.

27 "%s: config file %s dlsym error: %s\n"

$ Truetype file open error

28 "%s: config file can't open ttfont %s\n"

$ Truetype invalid format

29 "%s: config file: %s is not a true type font\n"

$ truetype file invalid character map.

30 "%s: config file :%s does not have valid cmap.\n"

$ Invalid range entry

31 "%s: config file: should be a number: %s.\n"

$ Too long any config file line

32 "%s: file: %s, line:%d too long\n"

$ Incorrect layout size.

33 "%s: incorrect layout size\n"

$ Error in conversion of a presentation form value to bitmap for pcf font.

34 "%s: Could not convert presentation form value to a bitmap; value:0x%x\n"

$ pcf font error , coulld not get metrics for reference character.

35 "%s:  pcf font error : could not get metrics for REFERENCE character.\n"

$ pcf font error , coulld not get metrics for spacing character.

36 "%s:  pcf font error : could not get metrics for SPACINGchar character.\n"

$ Internal program error 

37 "%s: internal error: unable to get pcf glyph for 0x%08x\n file name:%s , line number:%d\n"

$ Unable to set locale

38 "%s: Not able to set locale\n"

$ Ctl transform error

39 "%s: ctl transform error\n"

$ Error when unable to get a font name out of a font file.

40 "%s: unable to get font name from type1 file :%s\n"

$ Error when failed to convert to wide character

41 "%s: failed converting to wide character\n"

$ Error when substituting space for a non printable character.

42 "%s: Substituting space for a non printable character failed\n"

$ Error when no XPDISPLAY env. var found 

43 "%s: You need to set XPDISPLAY to X Print Server\n"

$ Error when cannot establish a display connection to XPDISPLAY

44 "%s: Cannot open X Print Server Display\n"

$ Error when given printer name does not exist as client for print server

45 "%s: Invalid printer name for Print Server\n"

$ Error for an invalid printer resolution

46 "%s:  Invalid Printer Resolution\n"

$ Error when print failed due to unknown reason

47 "%s: Printing not finished properly\n"

$ Error when a print extension is not supported.

48 "%s: X print extention not supported\n"

$ Error when reopening stdout fails

49 "%s: Failed reopening output file\n"

$ Error when unable to register file output data save & finish routines

50 "%s: Unable to register output data save & finish routines\n"

$ Error unable to create fotset from fontspec

51 "%s: Fail creating fontset from fontspec %s\n"

$ Error when no fonts in fontset

52 "%s: No fonts in fontset\n"

$ Error when inpit line too long.

53 "%s: Input line too long\n"

$ Error xpr config. file has non-integer in integer filed.

54 "%s: confile file:%s line:%d; non-unsigned integer in unsigned integer field.\n"

$ Error xpr config. file lines has lesser filed than expected.

55 "%s: confile file:%s line:%d; missing field(s).\n"

$ Error message unable to open the prologue file.

56 "%s: Unable to open Prologue file %s.\n"

$ Error message for unrecognized keyword in .xpr prolog file

57 "%s: Unrecognized keyword %s in config. file %s\n"

$ Error when a other than signed integer appears in a expected filed

58 "%s: config file:%s line: %d; expected value is signed/unsigned integer.\n"

$ Error when print cancelled

59 "%s: Print cancelled\n"

$ -------------------------------------------------------------------------
$ USAGE_SET
$ -------------------------------------------------------------------------

$set 3

$ Message for printing out the version.

1 "%s version 2.5.%d\n\n"

$ Usage messages

2 "Usage: %s "

$ Another usage message

3 "filename ...\n"
