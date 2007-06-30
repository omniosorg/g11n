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
%include Solaris.inc

%define name    SUNWfsexam 
%define cmpt    fsexam

Summary: File System Examiner
Name: SUNWfsexam 
Version: 0.3
Release: 16
License: CDDL
Group: User Interface/Desktop
Source: %{name}-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%include default-depend.inc
#BuildRequires: SUNWgnome-javahelp-convert
Requires: SUNWgnome-libs
Requires: SUNWgnome-base-libs
Requires: SUNWgnome-config

%description
File System Examiner is to help user migrate file name and file content from
legacy encoding to UTF8 encoding.


%prep
%setup

%build
export LDFLAGS="-L%{_libdir} -R%{_libdir}"
export CFLAGS="-I%{_includedir} %optflags"
export MSGFMT="/usr/bin/msgfmt"
export ACLOCAL_FLAGS="-I %{_datadir}/aclocal"
export PERL5LIB=%{_prefix}/perl5/site_perl/5.6.1/sun4-solaris-64int
export PKG_CONFIG_PATH=%{_libdir}/pkgconfig:/usr/lib/pkgconfig
autoconf

CFLAGS="$RPM_OPT_FLAGS"                      \
./configure --prefix=%{_prefix}              \
            --sysconfdir=%{_sysconfdir}
make

%install
rm -rf ${RPM_BUILD_ROOT}
export GCONF_DISABLE_MAKEFILE_SCHEMA_INSTALL=1
%makeinstall
javahelp-convert-install $RPM_BUILD_ROOT %{cmpt} %{cmpt}.xml

%clean
rm -rf ${RPM_BUILD_ROOT}

%post
GCONF_CONFIG_SOURCE=`%{_bindir}/gconftool-2 --get-default-source` %{_bindir}/gconftool-2 --makefile-install-rule %{_sysconfdir}/gconf/schemas/fsexam.schemas

%files 
%defattr(-, root, other)
%{_bindir}/*
%{_sysconfdir}/gconf/schemas/*
%{_datadir}/applications/fsexam.desktop
%{_datadir}/pixmaps/*.png
%{_datadir}/locale/*/LC_MESSAGES/fsexam.mo
%{_datadir}/omf
%{_datadir}/gnome/help/fsexam/*
%{_datadir}/gnome/javahelp/fsexam
%doc AUTHORS ChangeLog COPYING COPYING.LIB NEWS README

%changelog
* Wed Aug 11 2004 Federic Zhang <federic.zhang@sun.com>
- Bump to 0.3

* Tue Aug 10 2004 Federic Zhang <federic.zhang@sun.com>
- For Cinnabar build 16, implement Undo and UI polish
 
* Fri Jul 23 2004 Federic Zhang <federic.zhang@sun.com>
- For Cinnbar Beta build 15

* Mon May 31 2004 Gain  Tu <gavin.tu@sun.com>
- For Cinnabar build 11, add l10en menu, context-sensitive menu, help, reverse

* Mon May 24 2004 Gavin Tu <gavin.tu@sun.com>
- menu can be l10ned, alloc memory for name dynamiclly 

* Thu May 13 2004 Federic Zhang <federic.zhang@sun.com>
- For Cinnabar build 10, added fsexam.desktop and fsexam-icon.png

* Tue Apr 20 2004 Federic Zhang <federic.zhang@sun.com>
- For Cinnabar build8, added CFLAGS env and prefix option

* Mon Apr 11 2004 Federic Zhang <federic.zhang@sun.com>
- version 0.1
