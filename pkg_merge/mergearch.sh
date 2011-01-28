#!/bin/ksh

# merge repos between architectures (sparc and i386)

export LC_ALL=C

date

MERGEPY=./merge.py


if [ -z "$PKGDEST" ]
then
	REPOTOP="`cd ../pkgdest; pwd`"
else
	REPOTOP="$PKGDEST/.."
fi

[ ! -z "$REPOTOP_I386" ]	|| REPOTOP_I386=$REPOTOP/i386
[ ! -z "$REPOTOP_SPARC" ]	|| REPOTOP_SPARC=$REPOTOP/sparc
[ ! -z "$REPOTOP_MERGED" ]	|| REPOTOP_MERGED=$REPOTOP/merged
[ ! -z "$REPOTOP_TMP" ]		|| REPOTOP_TMP=$REPOTOP

merge_one_pair()
{
	reponame=$1
	publisher=$2
	pkgs="$3"

	srs="-v sparc,file://$REPOTOP_SPARC/$reponame"
	srx="-v i386,file://$REPOTOP_I386/$reponame"
	drd="$REPOTOP_MERGED/$reponame"
	dr="file://$drd"
	tmpd="$REPOTOP_TMP/mergearch.$$.$reponame"

	rm -rf $tmpd
	$MERGEPY -r -d $tmpd $srs $srx arch $pkgs

	rm -rf $drd
	pkgrepo create --version 3 $drd
	pkgrepo set -s $drd publisher/prefix=$publisher

	/bin/ls $tmpd/*/manifest | while read m
	do
		d=`dirname $m`
		echo "publishing `basename $d`..."
		pkgsend -s $dr publish --fmri-in-manifest --no-catalog \
			--no-index -d $d $m
	done

	rm -rf $tmpd

	echo "updating catalog and index..."
	/usr/lib/pkg.depotd -d $drd --add-content --exit-ready
}

REPONAME="repo.l10n"
PUBLISHER=${L10N_PUBLISHER:-"l10n-nightly"}
PKGS="consolidation/l10n/l10n-redistributable consolidation/l10n/l10n-incorporation"

merge_one_pair $REPONAME $PUBLISHER "$PKGS"

if [ "$MERGE_EXTRA" = "true" ]
then
	REPONAME_EXTRA="repo.extra"
	PUBLISHER_EXTRA=${PKGPUBLISHER_NONREDIST:-"l10n-extra"}
	PKGS_EXTRA="consolidation/l10n/l10n-extra"
	PKGS_EXTRA="$PKGS_EXTRA system/font/truetype/fonts-core"
	PKGS_EXTRA="$PKGS_EXTRA system/font/truetype/ttf-fonts-core"
	PKGS_EXTRA="$PKGS_EXTRA system/iiim/ja/atok"
	PKGS_EXTRA="$PKGS_EXTRA system/iiim/ja/wnn8"
	PKGS_EXTRA="$PKGS_EXTRA system/input-method/iiim/atok"
	PKGS_EXTRA="$PKGS_EXTRA system/input-method/iiim/wnn"

	merge_one_pair $REPONAME_EXTRA $PUBLISHER_EXTRA "$PKGS_EXTRA"
fi

date
