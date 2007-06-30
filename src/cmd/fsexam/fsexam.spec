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
# Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
# Use is subject to license terms.
#
Summary: File System Examiner
Name: fsexam
Version: 0.3
Release: 32
License: CDDL
Group: System/GUI/GNOME
Distribution: Cinnabar
Vendor: Sun Microsystems, Inc
Source: %{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Autoreqprov: on
Prereq:      GConf
Prereq:      scrollkeeper

%define scrollkeeper_version 0.3.11

BuildRequires: scrollkeeper >= %{scrollkeeper_version}
Requires: scrollkeeper >= %{scrollkeeper_version}

%description
File System Examiner is to help user migrate file name and file content from
legacy encoding to UTF8 encoding.

%prep
%setup

%build
aclocal
autoconf

CFLAGS="$RPM_OPT_FLAGS"                      \
./configure --prefix=%{_prefix}              \
            --sysconfdir=%{_sysconfdir}
make

%install
rm -rf ${RPM_BUILD_ROOT}
export GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL=1
%makeinstall
%find_lang %{name}

javahelp-convert-install ${RPM_BUILD_ROOT} %{name} %{name}.xml

%clean
rm -rf ${RPM_BUILD_ROOT}

%post
export GCONF_CONFIG_SOURCE=`gconftool-2 --get-default-source`
gconftool-2 --makefile-install-rule %{_sysconfdir}/gconf/schemas/%{name}.schemas

for i in zh_CN; do
    langtag=$i
    [ ${i:0:2} == "zh" ] || langtag=${i:0:2}
    [ -e %{_datadir}/omf/fsexam/fsexam-$langtag.omf ] && \
    env LANG=$i LC_ALL=$i scrollkeeper-install -q %{_datadir}/omf/fsexam/fsexam-$langtag.omf
done
scrollkeeper-update -q

%postun
for i in zh_CN; do
    langtag=$i
    [ ${i:0:2} == "zh" ] || langtag=${i:0:2}
    [ -e %{_datadir}/omf/fsexam/fsexam-$langtag.omf ] && \
    env LANG=$i LC_ALL=$i scrollkeeper-uninstall -q %{_datadir}/omf/fsexam/fsexam-$langtag.omf
done
scrollkeeper-update -q

%files -f %{name}.lang
%defattr(-, root, root)
%{_datadir}/applications/*.desktop
%{_datadir}/gnome/help/%{name}
%{_datadir}/gnome/javahelp/%{name}
%{_datadir}/omf/%{name}/*
%{_datadir}/pixmaps/*.png
%{_bindir}/*
%{_sysconfdir}/gconf/schemas/*
%doc AUTHORS ChangeLog COPYING COPYING.LIB NEWS README

%changelog
* Tue Aug 17 2004 Federic Zhang <federic.zhang@sun.com>
- Add scrollkeep-update to register the omf file and support javahelp

* Wed Aug 11 2004 Federic Zhang <federic.zhang@sun.com>
- bump to 0.3

* Mon Jul 12 2004 Federic Zhang <federic.zhang@sun.com>
- For Cinnabar build 14, Add the encoding list setting for european
  language according to http://www.cs.tut.fi/~jkorpela/8859.html.

* Tue Jun 29 2004 Federic Zhang <federic.zhang@sun.com>
- For Cinnabar build 13, added file content conversion and preview
  content support. UI and OLH freeze for Cinnabar beta

* Tue Jun 15 2004 Federic Zhang <federic.zhang@sun.com>
- For Cinnabar build 12, fix the undo crash problem

* Mon May 31 2004 Federic Zhang <federic.zhang@sun.com>
- For Cinnabar build 11, added help information

* Thu May 13 2004 Federic Zhang <federic.zhang@sun.com>
- For Cinnabar build 10, added fsexam.desktop and fsexam-icon.png

* Tue Apr 20 2004 Federic Zhang <federic.zhang@sun.com>
- For Cinnabar build8, added CFLAGS env and prefix option

* Mon Apr 11 2004 Federic Zhang <federic.zhang@sun.com>
- version 0.1
