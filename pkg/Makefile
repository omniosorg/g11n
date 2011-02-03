#
# CDDL HEADER START
#
# The contents of this file are subject to the terms of the
# Common Development and Distribution License (the "License").
# You may not use this file except in compliance with the License.
#
# You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
# or http://www.opensolaris.org/os/licensing.
# See the License for the specific language governing permissions
# and limitations under the License.
#
# When distributing Covered Code, include this CDDL HEADER in each
# file and include the License file at usr/src/OPENSOLARIS.LICENSE.
# If applicable, add the following below this CDDL HEADER, with the
# fields enclosed by brackets "[]" replaced with your own identifying
# information: Portions Copyright [yyyy] [name of copyright owner]
#
# CDDL HEADER END
#

#
# Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
#

include $(SRC)/Makefile.master
include $(SRC)/Makefile.buildnum

PRE_POUND=				pre\#
POUND_SIGN=				$(PRE_POUND:pre\%=%)

#
# Make sure we're getting a consistent execution environment for the
# embedded scripts.
#
SHELL= /usr/bin/ksh93

#
# To suppress package dependency generation on any system, regardless
# of how it was installed, set SUPPRESSPKGDEP=true in the build
# environment.
#
SUPPRESSPKGDEP= false

#
# Comment this line out or set "PKGDEBUG=" in your build environment
# to get more verbose output from the make processes in usr/src/pkg
#
PKGDEBUG= @

#
# Cross platform packaging notes
#
# By default, we package the proto area from the same architecture as
# the packaging build.  In other words, if you're running nightly or
# bldenv on an x86 platform, it will take objects from the x86 proto
# area and use them to create x86 repositories.
#
# If you want to create repositories for an architecture that's
# different from $(uname -p), you do so by setting PKGMACH in your
# build environment.
#
# For this to work correctly, the following must all happen:
#
#   1. You need the desired proto area, which you can get either by
#      doing a gatekeeper-style build with the -U option to
#      nightly(1), or by using rsync.  If you don't do this, you will
#      get packaging failures building all packages, because pkgsend
#      is unable to find the required binaries.
#   2. You need the desired tools proto area, which you can get in the
#      same ways as the normal proto area.  If you don't do this, you
#      will get packaging failures building SUNWonbld, because pkgsend is
#      unable to find the tools binaries.
#   3. You need to have built the appropriate third party license
#      files in $SRC, which generally means you should override SRC in
#      your build environment.  If you don't do this, you will get
#      packaging failures because pkgsend is unable to find various
#      THIRDPARTYLICENSE files.
#   4. The remainder of this Makefile should never refer directly to
#      $(MACH).  Instead, $(PKGMACH) should be used whenever an
#      architecture-specific path or token is needed.  If this is done
#      incorrectly, then packaging will fail, and you will see the
#      value of $(uname -p) instead of the value of $(PKGMACH) in the
#      commands that fail.
#   5. Each time a rule in this Makefile invokes $(MAKE), it should
#      pass PKGMACH=$(PKGMACH) explicitly on the command line.  If
#      this is done incorrectly, then packaging will fail, and you
#      will see the value of $(uname -p) instead of the value of
#      $(PKGMACH) in the commands that fail.
#
# Refer also to the convenience targets defined later in this
# Makefile.
#
PKGMACH=	$(MACH)

#
# ROOT, TOOLS_PROTO, and PKGARCHIVE should be set by nightly or
# bldenv.  These macros translate them into terms of $PKGMACH, instead
# of $ARCH.
#
PKGROOT.cmd=	echo $(ROOT) | sed -e s:/$(MACH):/$(PKGMACH):
PKGROOT=	$(PKGROOT.cmd:sh)/fileroot
PKGDEST.cmd=	echo $(PKGARCHIVE) | sed -e s:/$(MACH)/:/$(PKGMACH)/:
PKGDEST=	$(PKGDEST.cmd:sh)

#
# The publish transforms, EXCEPTIONS list, and some manifests need to
# know when we're building open-only and when we're using internal
# crypto bits.
#
$(CLOSED_BUILD)OPEN_ONLY_BUILD= $(POUND_SIGN)

EXCEPTIONS= packaging
$(CLOSED_BUILD)EXCEPTIONS += packaging.closed
$(OPEN_ONLY_BUILD)EXCEPTIONS += packaging.open

PKGMOGRIFY= pkgmogrify

#
# Always build the redistributable repository, but only build the
# nonredistributable bits if we have access to closed source.
#
# Some objects that result from the closed build are still
# redistributable, and should be packaged as part of an open-only
# build.  Access to those objects is provided via the closed-bins
# tarball.  See usr/src/tools/scripts/bindrop.sh for details.
#
REPOS= redist
$(CLOSED_BUILD)REPOS += extra

#
# The packages directory will contain the processed manifests as
# direct build targets and subdirectories for package metadata extracted
# incidentally during manifest processing.
#
# Nothing underneath $(PDIR) should ever be managed by SCM.
#
PDIR= packages.$(PKGMACH)

PKGPUBLISHER= $(PKGPUBLISHER_REDIST)
$(PKGDEST)/repo.extra:= PKGPUBLISHER= $(PKGPUBLISHER_NONREDIST)

#
# To get these defaults, manifests should simply refer to $(PKGVERS).
#
PKGVERS_COMPONENT= 0.$(RELEASE)
PKGVERS_BUILTON= $(RELEASE)
PKGVERS_BRANCH= 0.$(L10N_BUILDNUM)
PKGVERS= $(PKGVERS_COMPONENT),$(PKGVERS_BUILTON)-$(PKGVERS_BRANCH)

#
# Date stamp used for legacy actions
# Any real SVR4 packages delivered should have REV earlier than this.
#
LEGACY_REV_DATE=2010.12.06

#
# The ARCH32 and ARCH64 macros are used in the manifests to express
# architecture-specific subdirectories in the installation paths
# for isaexec'd commands.
#
# We can't simply use $(MACH32) and $(MACH64) here, because they're
# only defined for the build architecture.  To do cross-platform
# packaging, we need both values.
#
i386_ARCH32= i86
sparc_ARCH32= sparcv7
i386_ARCH64= amd64
sparc_ARCH64= sparcv9

#
# macros and transforms needed by pkgmogrify
#
# If you append to this list using target-specific assignments (:=),
# be very careful that the targets are of the form $(PDIR)/pkgname.  If
# you use a higher level target, or a package list, you'll trigger a
# complete reprocessing of all manifests because they'll fail command
# dependency checking.
#
PM_TRANSFORMS_BRANDING= branding_Oracle
PM_TRANSFORMS_OTHERS= common_actions publish restart_fmri defaults extract_metadata
PM_TRANSFORMS= $(PM_TRANSFORMS_BRANDING) $(PM_TRANSFORMS_OTHERS)
PM_INC= transforms

PKGMOG_DEFINES= \
	i386_ONLY=$(POUND_SIGN) \
	sparc_ONLY=$(POUND_SIGN) \
	$(PKGMACH)_ONLY= \
	ARCH=$(PKGMACH) \
	ARCH32=$($(PKGMACH)_ARCH32) \
	ARCH64=$($(PKGMACH)_ARCH64) \
	PKGVERS_COMPONENT=$(PKGVERS_COMPONENT) \
	PKGVERS_BUILTON=$(PKGVERS_BUILTON) \
	PKGVERS_BRANCH=$(PKGVERS_BRANCH) \
	PKGVERS=$(PKGVERS) \
	SRC=$(SRC) \
	CLOSED_BUILD=$(CLOSED_BUILD) \
	OPEN_BUILD=$(OPEN_ONLY_BUILD) \
        LEGACY_REV_DATE=$(LEGACY_REV_DATE)


PKGDEP_TOKENS_i386= \
	'PLATFORM=i86hvm' \
	'PLATFORM=i86pc' \
	'PLATFORM=i86xpv' \
	'ISALIST=amd64' \
	'ISALIST=i386'
PKGDEP_TOKENS_sparc= \
	'PLATFORM=sun4u' \
	'PLATFORM=sun4v' \
	'ISALIST=sparcv9' \
	'ISALIST=sparc'
PKGDEP_TOKENS= $(PKGDEP_TOKENS_$(PKGMACH))

#
# Specify IPS image "pkgdepend resolve" checks dependencies to
# other conolidations 'packages.
# Use "/" as IPS image by default.
#
SPECIFY_IPS_REFIMG= $(POUND_SIGN)
$(SPECIFY_IPS_REFIMG)USE_ROOT_AS_IPS_REFIMG= $(POUND_SIGN)
$(SPECIFY_IPS_REFIMG)PKGDEP_REFIMG_DIR= /opt/img4dep-$(PKGMACH)
$(USE_ROOT_AS_IPS_REFIMG)PKGDEP_REFIMG_DIR= /
PKGDEP_REFIMG= -R $(PKGDEP_REFIMG_DIR)

#
# The package lists are generated with $(PKGDEP_TYPE) as their
# dependency types, so that they can be included by either an
# incorporation or a group package.
#
$(PDIR)/l10n-redist.mog $(PDIR)/l10n-extra.mog:= PKGDEP_TYPE= require
$(PDIR)/l10n-incorporation.mog:= PKGDEP_TYPE= incorporate

PKGDEP_INCORP= \
	depend fmri=consolidation/l10n/l10n-incorporation type=require

#
# All packaging build products should go into $(PDIR), so they don't
# need to be included separately in CLOBBERFILES.
#
CLOBBERFILES= $(PDIR) proto_list_$(PKGMACH)

#
# By default, PKGS will list all manifests.  To build and/or publish a
# subset of packages, override this on the command line or in the
# build environment and then reference (implicitly or explicitly) the all
# or install targets.
#
# manifests for locales from g11n-locale source tree will be obtained
# from g11n-locale results so handling separately from statically
# maintained manifests.
#
MFDIR_STATIC= manifests
MFDIR_CLUSTER= manifests_cluster
MFDIR_RENAMED= manifests_renamed
MFDIR_OBSOLETE= manifests_obsolete
MFDIR_LOCALE= manifests_locale
MANIFESTS_STATIC.cmd= (cd $(MFDIR_STATIC); echo *.mf)
MANIFESTS_STATIC= $(MANIFESTS_STATIC.cmd:sh)
MANIFESTS_CLUSTER.cmd= (cd $(MFDIR_CLUSTER); echo *.mf)
MANIFESTS_CLUSTER= $(MANIFESTS_CLUSTER.cmd:sh)
MANIFESTS_RENAMED.cmd= (cd $(MFDIR_RENAMED); echo *.mf)
MANIFESTS_RENAMED= $(MANIFESTS_RENAMED.cmd:sh)
MANIFESTS_OBSOLETE.cmd= (cd $(MFDIR_OBSOLETE); echo *.mf)
MANIFESTS_OBSOLETE= $(MANIFESTS_OBSOLETE.cmd:sh)
MANIFESTS_LOCALE.cmd= (if [ -d $(MFDIR_LOCALE) ]; then cd $(MFDIR_LOCALE); echo *.mf; fi)
MANIFESTS_LOCALE= $(MANIFESTS_LOCALE.cmd:sh)
MANIFESTS= $(MANIFESTS_STATIC) $(MANIFESTS_CLUSTER) $(MANIFESTS_RENAMED) $(MANIFESTS_OBSOLETE) $(MANIFESTS_LOCALE)
PKGS= $(MANIFESTS:%.mf=%)
DEP_PKGS= $(PKGS:%=$(PDIR)/%.dep)
PROC_PKGS= $(PKGS:%=$(PDIR)/%.mog)

#
# Track the synthetic manifests separately so we can properly express
# build rules and dependencies.  The synthetic and real packages use
# different sets of transforms and macros for pkgmogrify.
#
SYNTH_PKGS= l10n-incorporation l10n-redist
$(CLOSED_BUILD)SYNTH_PKGS += l10n-extra
DEP_SYNTH_PKGS= $(SYNTH_PKGS:%=$(PDIR)/%.dep)
PROC_SYNTH_PKGS= $(SYNTH_PKGS:%=$(PDIR)/%.mog)

#
# For each package, we determine the target repository based on
# manifest-embedded metadata.  Because we make that determination on
# the fly, the publication target cannot be expressed as a
# subdirectory inside the unknown-by-the-makefile target repository.
#
# In order to limit the target set to real files in known locations,
# we use a ".pub" file in $(PDIR) for each processed manifest, regardless
# of content or target repository.
#
PUB_PKGS= $(SYNTH_PKGS:%=$(PDIR)/%.pub) $(PKGS:%=$(PDIR)/%.pub)

#
# Any given repository- and status-specific package list may be empty,
# but we can only determine that dynamically, so we always generate all
# lists for each repository we're building.
#
# The meanings of each package status are as follows:
#
# 	PKGSTAT		meaning
# 	----------	----------------------------------------------------
# 	noincorp	Do not include in incorporation or group package
#	obsolete	Include in incorporation, but not group package
#	renamed		Include in incorporation, but not group package
#	current		Include in incorporation and group package
#
# Since the semantics of the "noincorp" package status dictate that
# such packages are not included in the incorporation or group packages,
# there is no need to build noincorp package lists.
#
PKGLISTS= \
	$(REPOS:%=$(PDIR)/packages.%.current) \
	$(REPOS:%=$(PDIR)/packages.%.renamed) \
	$(REPOS:%=$(PDIR)/packages.%.obsolete)

.KEEP_STATE:

.PARALLEL: $(PKGS) $(PROC_PKGS) $(DEP_PKGS) \
	$(PROC_SYNTH_PKGS) $(DEP_SYNTH_PKGS) $(PUB_PKGS)

#
# For a single manifest, the dependency chain looks like this:
#
#	raw manifest (mypkg.mf)
#		|
#		| use pkgmogrify to process raw manifest
#		|
#	processed manifest (mypkg.mog)
#		|
#	   *    | use pkgdepend generate to generate dependencies
#		|
#	manifest with TBD dependencies (mypkg.dep)
#		|
#	   %    | use pkgdepend resolve to resolve dependencies
#		|
#	manifest with dependencies resolved (mypkg.res)
#		|
#		| use pkgsend to publish the package
#		|
#	placeholder to indicate successful publication (mypkg.pub)
#	
# * This may be suppressed via SUPPRESSPKGDEP.  The resulting
#   packages will install correctly, but care must be taken to
#   install all dependencies, because pkg will not have the input
#   it needs to determine this automatically.
#
# % This is included in this diagram to make the picture complete, but
#   this is a point of synchronization in the build process.
#   Dependency resolution is actually done once on the entire set of
#   manifests, not on a per-package basis.
#
# The full dependency chain for generating everything that needs to be
# published, without actually publishing it, looks like this:
#
#	processed synthetic packages
#		|		|
#       package lists	    synthetic package manifests
#		|
#	processed real packages
#	    |		|
#	package dir	real package manifests
#
# Here, each item is a set of real or synthetic packages.  For this
# portion of the build, no reference is made to the proto area.  It is
# therefore suitable for the "all" target, as opposed to "install."
#
# Since each of these steps is expressed explicitly, "all" need only
# depend on the head of the chain.
#
# From the end of manifest processing, the publication dependency
# chain looks like this:
#
#		repository metadata (catalogs and search indices)
#			|
#			| pkg.depotd
#			|
#		published packages
#		 |		|
#		 |		| pkgsend publish
#		 |		|
#	  repositories 	    resolved dependencies
#		 |			|
# pkgsend	 |			| pkgdepend resolve
# create-repository	 	 	|
#		 |		generated dependencies
#	 repo directories		|
#					| pkgdepend
#					|
#				processed manifests
#
# Due to limitations in pkgdepend, we cannot simply treat synthetic
# and real manifests identically.  But we don't really want to
# maintain a separate chain for synthetic manifests, so for the left
# side of this diagram, we actually do faux dependency generation and
# resolution, so we end up with the expected set of files in $(PDIR),
# per the individual file chain described above: mf, mog, dep, res,
# and pub files for each manifest.
# 
ALL_TARGETS= $(PROC_SYNTH_PKGS) proto_list_$(PKGMACH)

all: $(ALL_TARGETS)

#
# This will build the directory to contain the processed manifests
# and the metadata symlinks.
#
$(PDIR):
	@echo "Creating $(@)"
	$(PKGDEBUG)$(INSDIR) $(@)

#
# This rule resolves dependencies across all published manifests.
# We should be able to do this with
#
#    pkgdepend resolve -m $(PUB_PKGS:%.pub=%.dep)
#
# but until 14113 is fixed, the incorporations confuse pkgdepend, so we
# just create the .res file for DEP_SYNTH_PKGS directly.
#
# We also shouldn't have to ignore the error from pkgdepend, but
# until at least 14110 is resolved, pkgdepend will always exit with
# an error.
#
$(PDIR)/gendeps: $(DEP_SYNTH_PKGS) $(DEP_PKGS)
	-$(PKGDEBUG)if [ "$(SUPPRESSPKGDEP)" = "true" ]; then \
		echo "Suppressing dependency resolution"; \
		for p in $(DEP_PKGS:%.dep=%); do \
			$(CP) $$p.dep $$p.res; \
		done; \
	else \
		echo "Resolving dependencies"; \
		pkgdepend $(PKGDEP_REFIMG) resolve -m $(DEP_PKGS); \
		for p in $(DEP_PKGS:%.dep=%); do \
			if [ "$$(echo $$p.metadata.*)" = \
			     "$$(echo $$p.metadata.noincorp.*)" ]; \
			then \
				echo "Removing dependency versions from $$p"; \
				$(PKGMOGRIFY) $(PKGMOG_VERBOSE) \
				    -O $$p.res -I transforms \
				    strip_versions $$p.dep.res; \
				$(RM) $$p.dep.res; \
			else \
				$(MV) $$p.dep.res $$p.res; \
			fi; \
		done; \
	fi
	$(PKGDEBUG)for p in $(DEP_SYNTH_PKGS:%.dep=%); \
	do \
		$(CP) $$p.dep $$p.res; \
	done
	$(PKGDEBUG)$(TOUCH) $(@)

install: $(ALL_TARGETS) repository-metadata

repository-metadata: publish_pkgs
	@echo "Creating repository metadata"
	$(PKGDEBUG)for r in $(REPOS); do \
		/usr/lib/pkg.depotd -d $(PKGDEST)/repo.$$r \
			--add-content --exit-ready; \
		chmod a+r $(PKGDEST)/repo.$$r/cfg_cache; \
	done

#
# Since we create zero-length processed manifests for a graceful abort
# from pkgmogrify, we need to detect that here and make no effort to
# publish the package.
#
# For all other packages, we publish them regardless of status.  We
# derive the target repository as a component of the metadata-derived
# symlink for each package.
#
publish_pkgs: $(REPOS:%=$(PKGDEST)/repo.%) $(PDIR)/gendeps .WAIT $(PUB_PKGS)

$(PUB_PKGS): FRC

#
# Initialize the empty on-disk repositories
#
$(REPOS:%=$(PKGDEST)/repo.%):
	@echo "Initializing $(@F)"
	$(RM) -r $(@)
	$(PKGDEBUG)$(INSDIR) $(@)
	$(PKGDEBUG)pkgrepo create --version 3 $(@)
	$(PKGDEBUG)pkgrepo set -s file://$(@)/ publisher/prefix=$(PKGPUBLISHER)

#
# rule to process real manifests
#
# To allow redistributability and package status to change, we must
# remove not only the actual build target (the processed manifest), but
# also the incidental ones (the metadata-derived symlinks).
#
# If pkgmogrify exits cleanly but fails to create the specified output
# file, it means that it encountered an abort directive.  That means
# that this package should not be published for this particular build
# environment.  Since we can't prune such packages from $(PKGS)
# retroactively, we need to create an empty target file to keep make
# from trying to rebuild it every time.  For these empty targets, we
# do not create metadata symlinks.
#
# Automatic dependency resolution to files is also done at this phase of
# processing.  The skipped packages are skipped due to existing bugs
# in pkgdepend.
#
# The incorporation dependency is tricky: it needs to go into all
# current and renamed manifests (ie all incorporated packages), but we
# don't know which those are until after we run pkgmogrify.  So
# instead of expressing it as a transform, we tack it on ex post facto.
#
# Implementation notes:
#
# - The first $(RM) must not match other manifests, or we'll run into
#   race conditions with parallel manifest processing.
#
# - The make macros [ie $(MACRO)] are evaluated when the makefile is
#   read in, and will result in a fixed, macro-expanded rule for each
#   target enumerated in $(PROC_PKGS).
#
# - The shell variables (ie $$VAR) are assigned on the fly, as the rule
#   is executed.  The results may only be referenced in the shell in
#   which they are assigned, so from the perspective of make, all code
#   that needs these variables needs to be part of the same line of
#   code.  Hence the use of command separators and line continuation
#   characters.
#
# - The extract_metadata transforms are designed to spit out shell
#   variable assignments to stdout.  Those are published to the
#   .vars temporary files, and then used as input to the eval
#   statement.  This is done in stages specifically so that pkgmogrify
#   can signal failure if the manifest has a syntactic or other error.
#   The eval statement should begin with the default values, and the
#   output from pkgmogrify (if any) should be in the form of a
#   variable assignment to override those defaults.
#
# - When this rule completes execution, it must leave an updated
#   target file ($@) in place, or make will reprocess the package
#   every time it encounters it as a dependency.  Hence the "touch"
#   statement to ensure that the target is created, even when
#   pkgmogrify encounters an abort in the publish transforms.  This
#   will not cause publication failures when switching build
#   environments, because $(CLOSED_BUILD) and $(OPEN_ONLY) are
#   referenced in $(PKGMOG_DEFINES), and changes will therefore
#   trigger a rebuild for command dependency failure.  (Command
#   dependency checking is turned on by .KEEP_STATE: above.)
#

.SUFFIXES: .mf .mog .dep .res .pub

$(MANIFESTS_STATIC:%=$(PDIR)/%): $$(@:$(PDIR)/%=$(MFDIR_STATIC)/%)
	$(CP) $(@:$(PDIR)/%=$(MFDIR_STATIC)/%) $(@)

$(MANIFESTS_CLUSTER:%=$(PDIR)/%): $$(@:$(PDIR)/%=$(MFDIR_CLUSTER)/%)
	$(CP) $(@:$(PDIR)/%=$(MFDIR_CLUSTER)/%) $(@)

$(MANIFESTS_RENAMED:%=$(PDIR)/%): $$(@:$(PDIR)/%=$(MFDIR_RENAMED)/%)
	$(CP) $(@:$(PDIR)/%=$(MFDIR_RENAMED)/%) $(@)

$(MANIFESTS_OBSOLETE:%=$(PDIR)/%): $$(@:$(PDIR)/%=$(MFDIR_OBSOLETE)/%)
	$(CP) $(@:$(PDIR)/%=$(MFDIR_OBSOLETE)/%) $(@)

$(MANIFESTS_LOCALE:%=$(PDIR)/%): $$(@:$(PDIR)/%=$(MFDIR_LOCALE)/%)
	$(CP) $(@:$(PDIR)/%=$(MFDIR_LOCALE)/%) $(@)

$(PDIR)/%.mog: $(PDIR)/%.mf
	@echo "Processing manifest $(<F)"
	$(PKGDEBUG)$(RM) $(@) $(@:%.mog=%) $(@:%.mog=%.nodepend) \
		$(PDIR)/$(@F:%.mog=%).metadata.* $(@).vars
	$(PKGDEBUG)$(PKGMOGRIFY) $(PKGMOG_VERBOSE) $(PM_INC:%= -I %) \
		$(PKGMOG_DEFINES:%=-D %) -P $(@).vars -O $(@) \
		$(<) $(PM_TRANSFORMS)
	$(PKGDEBUG)eval REPO=redist PKGSTAT=current NODEPEND=$(SUPPRESSPKGDEP) \
		`$(CAT) -s $(@).vars`; \
	if [ -f $(@) ]; then \
		if [ "$$NODEPEND" != "false" ]; then \
			 $(TOUCH) $(@:%.mog=%.nodepend); \
		fi; \
		$(LN) -s $(@F) \
			$(PDIR)/$(@F:%.mog=%).metadata.$$PKGSTAT.$$REPO; \
		if [ \( "$$PKGSTAT" = "current" \) -o \
		     \( "$$PKGSTAT" = "renamed" \) ]; \
			then echo $(PKGDEP_INCORP) >> $(@); \
		fi; \
	else \
		$(TOUCH) $(@); \
	fi
	$(PKGDEBUG)$(RM) $(@).vars

$(PDIR)/%.dep: $(PDIR)/%.mog
	@echo "Generating dependencies for $(<F)"
	$(PKGDEBUG)$(RM) $(@)
	$(PKGDEBUG)if [ ! -f $(@:%.dep=%.nodepend) ]; then \
		pkgdepend generate -m $(PKGDEP_TOKENS:%=-D %) $(<) \
			$(PKGROOT) > $(@); \
	else \
		$(CP) $(<) $(@); \
	fi

#
# The full chain implies that there should be a .dep.res suffix rule,
# but dependency generation is done on a set of manifests, rather than
# on a per-manifest basis.  Instead, see the gendeps rule above.
#

$(PDIR)/%.pub: $(PDIR)/%.res
	$(PKGDEBUG)m=$$(basename $(@:%.pub=%).metadata.*); \
	r=$${m#$(@F:%.pub=%.metadata.)+(?).}; \
	if [ -s $(<) ]; then \
		echo "Publishing $(@F:%.pub=%) to $$r repository"; \
		pkgsend -s file://$(PKGDEST)/repo.$$r publish \
		    -d $(PKGROOT) -d $(SRC)/pkg/license_files \
		    -d $(SRC) --fmri-in-manifest --no-index --no-catalog $(<) \
		    > /dev/null; \
	fi; \
	$(TOUCH) $(@);

#
# rule to build the synthetic manifests
#
# This rule necessarily has PKGDEP_TYPE that changes according to
# the specific synthetic manifest.  Rather than escape command
# dependency checking for the real manifest processing, or failing to
# express the (indirect) dependency of synthetic manifests on real
# manifests, we simply split this rule out from the one above.
#
# The implementation notes from the previous rule are applicable
# here, too.
#
$(PROC_SYNTH_PKGS): $(PKGLISTS) $$(@F:%.mog=%.mf)
	@echo "Processing synthetic manifest $(@F:%.mog=%.mf)"
	$(PKGDEBUG)$(RM) $(@) $(PDIR)/$(@F:%.mog=%).metadata.* $(@).vars
	$(PKGDEBUG)$(PKGMOGRIFY) $(PKGMOG_VERBOSE) -I transforms -I $(PDIR) \
		$(PKGMOG_DEFINES:%=-D %) -D PKGDEP_TYPE=$(PKGDEP_TYPE) \
		-P $(@).vars -O $(@) $(@F:%.mog=%.mf) \
		$(PM_TRANSFORMS) synthetic
	$(PKGDEBUG)eval REPO=redist PKGSTAT=current `$(CAT) -s $(@).vars`; \
	if [ -f $(@) ]; then \
		$(LN) -s $(@F) \
			$(PDIR)/$(@F:%.mog=%).metadata.$$PKGSTAT.$$REPO; \
	else \
		$(TOUCH) $(@); \
	fi
	$(PKGDEBUG)$(RM) $(@).vars

$(DEP_SYNTH_PKGS): $$(@:%.dep=%.mog)
	@echo "Skipping dependency generation for $(@F:%.dep=%)"
	$(PKGDEBUG)$(CP) $(@:%.dep=%.mog) $(@)

clean:

clobber: clean
	$(RM) -r $(CLOBBERFILES)

#
# This rule assumes that all links in the $PKGSTAT directories
# point to valid manifests, and will fail the make run if one
# does not contain an fmri.
#
# We do this in the BEGIN action instead of using pattern matching
# because we expect the fmri to be at or near the first line of each input
# file, and this way lets us avoid reading the rest of the file after we
# find what we need.
#
# We keep track of a failure to locate an fmri, so we can fail the
# make run, but we still attempt to process each package in the
# repo/pkgstat-specific subdir, in hopes of maybe giving some
# additional useful info.
#
# The protolist is used for bfu archive creation, which may be invoked
# interactively by the user.  Both protolist and PKGLISTS targets
# depend on $(PROC_PKGS), but protolist builds them recursively.
# To avoid collisions, we insert protolist into the dependency chain
# here.  This has two somewhat subtle benefits: it allows bfu archive
# creation to work correctly, even when -a was not part of NIGHTLY_OPTIONS,
# and it ensures that a protolist file here will always correspond to the
# contents of the processed manifests, which can vary depending on build
# environment.
#
$(PKGLISTS): $(PROC_PKGS)
	$(PKGDEBUG)sdotr=$(@F:packages.%=%); \
	r=$${sdotr%.+(?)}; s=$${sdotr#+(?).}; \
	echo "Generating $$r $$s package list"; \
	$(RM) $(@); $(TOUCH) $(@); \
	$(NAWK) 'BEGIN { \
		if (ARGC < 2) { \
			exit; \
		} \
		retcode = 0; \
		for (i = 1; i < ARGC; i++) { \
			do { \
				e = getline f < ARGV[i]; \
			} while ((e == 1) && (f !~ /name=pkg.fmri/)); \
			close(ARGV[i]); \
			if (e == 1) { \
				l = split(f, a, "="); \
				print "depend fmri=" a[l], \
					"type=$$(PKGDEP_TYPE)"; \
			} else { \
				print "no fmri in " ARGV[i] >> "/dev/stderr"; \
				retcode = 2; \
			} \
		} \
		exit retcode; \
	}' `find $(PDIR) -type l -a \( $(PKGS:%=-name %.metadata.$$s.$$r -o) \
		-name NOSUCHFILE \)` >> $(@); \
	cat ../../g11n-spec/build.`uname -p`/repo.spec/pkg/*/* | \
	grep name=pkg.name | sed s%.*value=%depend\ fmri=pkg:/% | \
	sed s%$$%@$(PKGVERS)\ type=$$\(PKGDEP_TYPE\)% >> $(@); \
	cat ../../inputmethod/specs/build.`uname -p`/repo.imspec/pkg/*/* | \
	grep name=pkg.name | sed s%.*value=%depend\ fmri=pkg:/% | \
	sed s%$$%@$(PKGVERS)\ type=$$\(PKGDEP_TYPE\)% >> $(@)


#
# rules to validate proto area against manifests, check for safe
# file permission modes, and generate a faux proto list
#
# For the check targets, the dependencies on $(PROC_PKGS) is specified
# as a subordinate make process in order to suppress output.
#
makesilent:
	@$(MAKE) -e $(PROC_PKGS) PKGMACH=$(PKGMACH) \
		SUPPRESSPKGDEP=$(SUPPRESSPKGDEP) > /dev/null

protocmp: makesilent
	@/opt/onbld/bin/validate_pkg -a $(PKGMACH) -v \
		$(EXCEPTIONS:%=-e $(CODEMGR_WS)/exception_lists/%) \
		-m $(PDIR) -p $(PKGROOT)

pmodes: makesilent
	@/opt/onbld/bin/validate_pkg -a $(PKGMACH) -M -m $(PDIR) \
		-e $(CODEMGR_WS)/exception_lists/pmodes 

check: protocmp pmodes

protolist: proto_list_$(PKGMACH)

proto_list_$(PKGMACH): $(PROC_PKGS)
	@/opt/onbld/bin/validate_pkg -a $(PKGMACH) -L -m $(PDIR) > $(@)

$(PROC_PKGS): $(PDIR)

#
# This is a convenience target to allow package names to function as
# build targets.  Generally, using it is only useful when iterating on
# development of a manifest.
#
# When processing a manifest, use the basename (without extension) of
# the package.  When publishing, use the basename with a ".pub"
# extension.
#
# Other than during manifest development, the preferred usage is to
# avoid these targets and override PKGS on the make command line and
# use the provided all and install targets.
#
$(PKGS) $(SYNTH_PKGS): $(PDIR)/$$(@:%=%.mog)

$(PKGS:%=%.pub) $(SYNTH_PKGS:%=%.pub): $(PDIR)/$$(@)

#
# This is a convenience target to resolve dependencies without publishing
# packages.
#
gendeps: $(PDIR)/gendeps

#
# These are convenience targets for cross-platform packaging.  If you
# want to build any of "the normal" targets for a different
# architecture, simply use "arch/target" as your build target.
#
# Since the most common use case for this is "install," the architecture
# specific install targets have been further abbreviated to elide "/install."
#
i386/% sparc/%:
	$(MAKE) -e $(@F) PKGMACH=$(@D) SUPPRESSPKGDEP=$(SUPPRESSPKGDEP)

i386 sparc: $$(@)/install

FRC:
