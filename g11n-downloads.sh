#!/bin/sh


x() {
	REPO=http://pkg.opensolaris.org/release

	if [ $ARCH = all ] ; then
		[ -d i386/fileroot/`dirname $2` ] || mkdir -p i386/fileroot/`dirname $2`
		[ -d sparc/fileroot/`dirname $2` ] || mkdir -p sparc/fileroot/`dirname $2`
		[ -f i386/fileroot/$2 ] || wget -q -O - $REPO/file/0/$1|gzip -dc > i386/fileroot/$2
		[ -f sparc/fileroot/$2 ] || cp i386/fileroot/$2 sparc/fileroot/$2
	else
		[ -d $ARCH/fileroot/`dirname $2` ] || mkdir -p $ARCH/fileroot/`dirname $2`
		[ -f $ARCH/fileroot/$2 ] || wget -q -O - $REPO/file/0/$1|gzip -dc > $ARCH/fileroot/$2
	fi
}

# file hashes from pkg://opensolaris.org/print/mp@0.5.11,5.11-0.134.0.2:20100529T002110Z

ARCH=i386
x 3290fd7f98b4d1c505794cd743bcfe56a38864ad usr/lib/lp/locale/en_US.UTF-8/mp/xugb18030.so
x 708c12f5fa1cb5149c854015fc8673a71f612c72 usr/lib/lp/locale/en_US.UTF-8/mp/xuhkscs.so
x 2cfc94f3d3656b6e89a5a33389cfb150efaace59 usr/lib/lp/locale/en_US.UTF-8/mp/xwc2unicode.so
x 61a9454fd93f2af679db87720686c8c3107c909d usr/lib/lp/locale/en_US.UTF-8/mp/xwccsi.so
x 57648d2292aea12f5111f0f0e2ae2e246b1ae018 usr/lib/lp/locale/en_US.UTF-8/mp/xwceuc.so

ARCH=sparc
x 79a218fa762c3492e29d95c61a1a98bd94bebe19 usr/lib/lp/locale/en_US.UTF-8/mp/xugb18030.so
x 2669744896fc65c31ae55d1562c5be1e0dfc121a usr/lib/lp/locale/en_US.UTF-8/mp/xuhkscs.so
x d88d011bc7275a9db998b93356b7f1b329841061 usr/lib/lp/locale/en_US.UTF-8/mp/xwc2unicode.so
x dc920a1f39f660d8e37c37df0702836971da9305 usr/lib/lp/locale/en_US.UTF-8/mp/xwccsi.so
x 8814c146a6348dc5a364f87456ad67955bd6999b usr/lib/lp/locale/en_US.UTF-8/mp/xwceuc.so

# hash from pkg://opensolaris.org/system/library/iconv/utf-8/manual@0.5.11,5.11-0.134.0.2:20100529T005755Z

ARCH=all
x 2b873c374c74a126c0c4c609b8ee2c7a3c70ddc3 usr/share/man/man5/iconv_en_US.UTF-8.5

# hashes from pkg://opensolaris.org/system/manual/locale/ja@0.5.11,5.11-0.134.0.2:20100529T011856Z

ARCH=all


x 7b4f43404a684ff7d635dc1f39b9d8a9184fbd9e usr/share/man/ja_JP.UTF-8/man1/acctcom.1
x b5495d95f1976c457885501e7efbc01789bcf188 usr/share/man/ja_JP.UTF-8/man1/adb.1
x 1a85fb12a99caeefc1950cb9d7d386adbc958b26 usr/share/man/ja_JP.UTF-8/man1/addbib.1
x 73e0104397163cb62c9e17b21ee7fdea5d4e9e78 usr/share/man/ja_JP.UTF-8/man1/apptrace.1
x 1161520f053660fef4d46ddcd16f86dcd85a16fc usr/share/man/ja_JP.UTF-8/man1/arch.1
x 0068ef4d9e0a9e4f48db82a5f82d204833fa43c5 usr/share/man/ja_JP.UTF-8/man1/as.1
x 69e6b7ac6955750fed83ec34aea86826281aef76 usr/share/man/ja_JP.UTF-8/man1/atq.1
x 2af4cc4fcc189d7b1150a86ed09e058f17405c65 usr/share/man/ja_JP.UTF-8/man1/atrm.1
x 669051ea1e3ed5f6acc9ad8c05ce367e2e640ba2 usr/share/man/ja_JP.UTF-8/man1/audioconvert.1
x 4651ba464fbf6887941e57a862e5881b3be33959 usr/share/man/ja_JP.UTF-8/man1/audioplay.1
x cb26df918b629082ea401ee735d2ab337a65adac usr/share/man/ja_JP.UTF-8/man1/audiorecord.1
x ee1578bf8e5bfa590ccec5dce1c183e19875334b usr/share/man/ja_JP.UTF-8/man1/auths.1
x 02ceacec4831b66800ff53286c47e88942bea721 usr/share/man/ja_JP.UTF-8/man1/banner.1
x e93f0acf20cef4bacaf91cd06d96f98e098c6a71 usr/share/man/ja_JP.UTF-8/man1/bdiff.1
x def5370960f4c8970550a17c7aac0aece2a3fba1 usr/share/man/ja_JP.UTF-8/man1/bfs.1
x 324332e78bbe763594df6392621f87dfb631d78c usr/share/man/ja_JP.UTF-8/man1/break.1
x 568336b6c111fd80fd5f399f01354b8bd3335f42 usr/share/man/ja_JP.UTF-8/man1/case.1
x 64e79be9115c5f5bc969e960b838b3b8ab861171 usr/share/man/ja_JP.UTF-8/man1/checkeq.1
x 046b6b5152954a71ec8b32ae1d5dc7a549635330 usr/share/man/ja_JP.UTF-8/man1/checknr.1
x ea224c5c53b3ff60787db0847961bb77a5ff53d4 usr/share/man/ja_JP.UTF-8/man1/chkey.1
x 79c9dc30155a5fbc9924283e69d09d07afaea89d usr/share/man/ja_JP.UTF-8/man1/ckdate.1
x 64e0d5a42f44a85d2ce7f11d0967ff259c91a7bc usr/share/man/ja_JP.UTF-8/man1/ckgid.1
x fff1e6dba68305ac552174b5f8dcdf16150c5bde usr/share/man/ja_JP.UTF-8/man1/ckint.1
x d95eed66142c37df94123405b522071f36d879ec usr/share/man/ja_JP.UTF-8/man1/ckitem.1
x 6ce31199a8704b9f959fc0d8aabf475cc934aec1 usr/share/man/ja_JP.UTF-8/man1/ckkeywd.1
x 9e878dfd36e89c1787789e9c1e8654ff3dad134f usr/share/man/ja_JP.UTF-8/man1/ckpath.1
x 8272fbb1f4f43dd6366563894505ce0517cebebc usr/share/man/ja_JP.UTF-8/man1/ckrange.1
x 3aaa38b55fe6addd424be6fb109aee314a21af65 usr/share/man/ja_JP.UTF-8/man1/ckstr.1
x d70f0754c931a48f26840825e2da53e9352e14e1 usr/share/man/ja_JP.UTF-8/man1/cktime.1
x b59f6601e90d8a8905799fcbc87896de11c35fae usr/share/man/ja_JP.UTF-8/man1/ckuid.1
x 6e64be2191c1dae80763ff964391890ae8a0e034 usr/share/man/ja_JP.UTF-8/man1/ckyorn.1
x b9e2606da4d0072e6606ce6e0a94d7796c7d8fd0 usr/share/man/ja_JP.UTF-8/man1/clear.1
x 4b928753f6aea4345fa13f9b942371cac648d656 usr/share/man/ja_JP.UTF-8/man1/continue.1
x 80d51617ad832b1f8cd21d3fbd9bf2183220d55a usr/share/man/ja_JP.UTF-8/man1/crypt.1
x 88136ae1464d5aadb3053b3476e30b77feca8141 usr/share/man/ja_JP.UTF-8/man1/csh.1
x 92080708a05170d16c10380f9a1a51bc879e35c1 usr/share/man/ja_JP.UTF-8/man1/dc.1
x 27959f8a8151c9bee52e1cf009ee590bf6436eca usr/share/man/ja_JP.UTF-8/man1/deroff.1
x 17178c50fceec8641c3a5ac23729f8cda4660ba4 usr/share/man/ja_JP.UTF-8/man1/dhcpinfo.1
x 12952671954f80f62c2e465df23d0867f0400e65 usr/share/man/ja_JP.UTF-8/man1/diff3.1
x f5fbb9fedd7706151435d9bfbeb68a9d3c9a6245 usr/share/man/ja_JP.UTF-8/man1/diffmk.1
x d93eb85b94c46f22bef20d0795453054133967bb usr/share/man/ja_JP.UTF-8/man1/disable.1
x f9ee3d5ac05df164a577004cc368de479799e47b usr/share/man/ja_JP.UTF-8/man1/dispgid.1
x 09395073c960ca30ed0f63870d7926de7f985b4d usr/share/man/ja_JP.UTF-8/man1/dispuid.1
x b56d4ecc79fa6109899c8e6f6d8d8f22bf4bf300 usr/share/man/ja_JP.UTF-8/man1/dos2unix.1
x 3d92143f5652c9dc31313e024c078b5d836ed79f usr/share/man/ja_JP.UTF-8/man1/download.1
x ce9cab3867c3b464de23788d463b6d0e5b542a6a usr/share/man/ja_JP.UTF-8/man1/dpost.1
x 646f607ad102b2b35af0c9f0570d590d09ff8491 usr/share/man/ja_JP.UTF-8/man1/dtappsession.1
x 51db101876354c67c766dde974719dc1c11c3177 usr/share/man/ja_JP.UTF-8/man1/dump.1
x 4efd3ad7cdf23b72b49616ce3f9e2c61342075b8 usr/share/man/ja_JP.UTF-8/man1/dumpcs.1
x 63ebb1b2a6c61c8da8bc2dc77a97b3bcd2d53978 usr/share/man/ja_JP.UTF-8/man1/enable.1
x 5edb0f5f36165f5c7c1f85a305e416e055db97cc usr/share/man/ja_JP.UTF-8/man1/eqn.1
x d9dc9111b774bee703fb3a7363dd245c67348f8f usr/share/man/ja_JP.UTF-8/man1/errange.1
x c9a65b4b1ac94a7da3ab3525ff40665ffbc50776 usr/share/man/ja_JP.UTF-8/man1/errdate.1
x e17afea0536d1bd09c295e40069c78271f25e87b usr/share/man/ja_JP.UTF-8/man1/errgid.1
x 764ad44dd8f24866aa8d2903d5ad32750dd17f0b usr/share/man/ja_JP.UTF-8/man1/errint.1
x 083c228bd282dd8f0b1ad9e0d650b33e05cb494c usr/share/man/ja_JP.UTF-8/man1/erritem.1
x a9ba3244a1a98ed86adbd25771a6db22c94dbc61 usr/share/man/ja_JP.UTF-8/man1/errpath.1
x 0d8335b8e3f74d5218bda670499e3d38d5d6deab usr/share/man/ja_JP.UTF-8/man1/errstr.1
x e557b4522e5a1507a22aae7be0a720dd6a68222b usr/share/man/ja_JP.UTF-8/man1/errtime.1
x 4116e5b655f887aaeb13a40d2749cca0d93bfb07 usr/share/man/ja_JP.UTF-8/man1/erruid.1
x 3d5d31c51f643ee3329a9f663bc382da4bbfd306 usr/share/man/ja_JP.UTF-8/man1/erryorn.1
x 97597de0d6c5cf30b8d48b591b7583d234eabe97 usr/share/man/ja_JP.UTF-8/man1/eval.1
x 537f469dd2d9ccb03806943d7ab5cc4ee58089ab usr/share/man/ja_JP.UTF-8/man1/exec.1
x 5d6355a04bd50ee45410a4b678048c8c3f51a287 usr/share/man/ja_JP.UTF-8/man1/exit.1
x eb1f899bdffaecd2078ae4fa122f97efa3549ea8 usr/share/man/ja_JP.UTF-8/man1/export.1
x 1e22f0d4ca68337a08163cb6faff1315a6a30558 usr/share/man/ja_JP.UTF-8/man1/exstr.1
x ab437a45cd561d9a31d11f29165b1547fbaa566a usr/share/man/ja_JP.UTF-8/man1/factor.1
x 357d80b9663caad01bdd431808f037984fe743b2 usr/share/man/ja_JP.UTF-8/man1/fdformat.1
x 8572257617bad00f4d85780cc634e7e93abbd5cb usr/share/man/ja_JP.UTF-8/man1/filesync.1
x 34fe20d2fdde3a1411ae609af4cb8b1d1fd920c1 usr/share/man/ja_JP.UTF-8/man1/finger.1
x c9c9dc21607f66d1f56e1f287c1f67b0f8c2adcf usr/share/man/ja_JP.UTF-8/man1/fmli.1
x 6438b6b9d50871e4700e6d36b8dc81f03267341f usr/share/man/ja_JP.UTF-8/man1/fmt.1
x 7a081b31a93326260acd3edeb00e015c60569177 usr/share/man/ja_JP.UTF-8/man1/fmtmsg.1
x 568336b6c111fd80fd5f399f01354b8bd3335f42 usr/share/man/ja_JP.UTF-8/man1/for.1
x 568336b6c111fd80fd5f399f01354b8bd3335f42 usr/share/man/ja_JP.UTF-8/man1/foreach.1
x f1bca2190e6c1a15277b01c7f48d6ffbe1990329 usr/share/man/ja_JP.UTF-8/man1/ftp.1
x 568336b6c111fd80fd5f399f01354b8bd3335f42 usr/share/man/ja_JP.UTF-8/man1/function.1
x cd5f6088b161d34a6b6bd59f67d9ae5966398c43 usr/share/man/ja_JP.UTF-8/man1/gcore.1
x d0ea2c273d7e61d929f15ba19b639c6244640d03 usr/share/man/ja_JP.UTF-8/man1/getlabel.1
x 3748d5b4c0bc74d611fad1d672608cb6f6b10683 usr/share/man/ja_JP.UTF-8/man1/getopt.1
x 6ee23c8b9d059e38e6d9ae7e62ed609ecd9741a6 usr/share/man/ja_JP.UTF-8/man1/getoptcvt.1
x 18e69e7820709aa872479a626ae6247f8021c8ca usr/share/man/ja_JP.UTF-8/man1/gettxt.1
x 5a7068b663a72c67b70d2894b4c1c88408d50219 usr/share/man/ja_JP.UTF-8/man1/getzonepath.1
x 3935d6965f23c0114d42ed2de591e890ba1dfecb usr/share/man/ja_JP.UTF-8/man1/glob.1
x 4e350184fca3aa774f257a40030a69a213dfa146 usr/share/man/ja_JP.UTF-8/man1/goto.1
x 8942764b63046f796c3e6beb23ead4ea96129637 usr/share/man/ja_JP.UTF-8/man1/groups.1
x c9a65b4b1ac94a7da3ab3525ff40665ffbc50776 usr/share/man/ja_JP.UTF-8/man1/helpdate.1
x e17afea0536d1bd09c295e40069c78271f25e87b usr/share/man/ja_JP.UTF-8/man1/helpgid.1
x 764ad44dd8f24866aa8d2903d5ad32750dd17f0b usr/share/man/ja_JP.UTF-8/man1/helpint.1
x 083c228bd282dd8f0b1ad9e0d650b33e05cb494c usr/share/man/ja_JP.UTF-8/man1/helpitem.1
x a9ba3244a1a98ed86adbd25771a6db22c94dbc61 usr/share/man/ja_JP.UTF-8/man1/helppath.1
x d9dc9111b774bee703fb3a7363dd245c67348f8f usr/share/man/ja_JP.UTF-8/man1/helprange.1
x 0d8335b8e3f74d5218bda670499e3d38d5d6deab usr/share/man/ja_JP.UTF-8/man1/helpstr.1
x e557b4522e5a1507a22aae7be0a720dd6a68222b usr/share/man/ja_JP.UTF-8/man1/helptime.1
x 4116e5b655f887aaeb13a40d2749cca0d93bfb07 usr/share/man/ja_JP.UTF-8/man1/helpuid.1
x 3d5d31c51f643ee3329a9f663bc382da4bbfd306 usr/share/man/ja_JP.UTF-8/man1/helpyorn.1
x 568336b6c111fd80fd5f399f01354b8bd3335f42 usr/share/man/ja_JP.UTF-8/man1/if.1
x 545eab577026dde9b1ac87e9fa33ab357cb10c82 usr/share/man/ja_JP.UTF-8/man1/intro.1
x 43c53c85d81037daea58cf1d6a2dc2e179dc626d usr/share/man/ja_JP.UTF-8/man1/Intro.1
x 11173b0b9f0ef8edf7b015a661e1d1b62d34329f usr/share/man/ja_JP.UTF-8/man1/isainfo.1
x 3dd7036f2c83897f6a485f663a32a1fe5529af5a usr/share/man/ja_JP.UTF-8/man1/isalist.1
x a3e0d44ab4df4e87fac9133fdb175e08e9b5bf1f usr/share/man/ja_JP.UTF-8/man1/ld.1
x c6d9263dd08ef03d65201ba56f6bca3f3bd0c2b1 usr/share/man/ja_JP.UTF-8/man1/ldd.1
x 130dbd55ac6e63b64293677be1c2a4320e25c110 usr/share/man/ja_JP.UTF-8/man1/let.1
x ee71cf1ae4e1003d0a0d13d6d7796ee4abe2d074 usr/share/man/ja_JP.UTF-8/man1/login.1
x eb8330155db9ff55a5b0d972be49b0be3313363d usr/share/man/ja_JP.UTF-8/man1/logout.1
x a6236aed938b3844937d646a761657811ee5fdc8 usr/share/man/ja_JP.UTF-8/man1/man.1
x 49147fcfa2716979860482e409de00a19b4c5cfe usr/share/man/ja_JP.UTF-8/man1/mkmsgs.1
x 6292255fc4169e212f87cc55618340d8dab73461 usr/share/man/ja_JP.UTF-8/man1/mt.1
x 64e79be9115c5f5bc969e960b838b3b8ab861171 usr/share/man/ja_JP.UTF-8/man1/neqn.1
x 696bcc7ded45366e47ac471004a96b94b734a2bd usr/share/man/ja_JP.UTF-8/man1/newform.1
x fd3c4b54ceb9b49f4dbd19d59c40e58fae535196 usr/share/man/ja_JP.UTF-8/man1/news.1
x 72b25c43e923d9f5190623b70c3e7084b3eb5445 usr/share/man/ja_JP.UTF-8/man1/nroff.1
x b33cc818f0ea9cfed3fa0ee10c739ed464f388a5 usr/share/man/ja_JP.UTF-8/man1/onintr.1
x 18488b7e88a5f820bbca15ed259afff1ec0ad60a usr/share/man/ja_JP.UTF-8/man1/optisa.1
x ef85aea879b8eb1aeabcd517c1db63f1dcf3efe0 usr/share/man/ja_JP.UTF-8/man1/pargs.1
x b7b7c2c2318bda64f3c1c463d520953c9d730c9c usr/share/man/ja_JP.UTF-8/man1/pfexec.1
x fb722505b00e0768d5424c8516733b3f101e0c66 usr/share/man/ja_JP.UTF-8/man1/pgrep.1
x b574b6e00bb28740b60bac58ec7283de7b71bc25 usr/share/man/ja_JP.UTF-8/man1/pkginfo.1
x eb1100e0dcc6b2550f4547c0d4de595d49b24cd7 usr/share/man/ja_JP.UTF-8/man1/pkill.1
x e7309904f99f68c07dc361d7a4c4791f6dc81c65 usr/share/man/ja_JP.UTF-8/man1/plabel.1
x a2d3433efd170b179a479973ff9cdb4eab0647d2 usr/share/man/ja_JP.UTF-8/man1/pmap.1
x be570ee359991dc37e3d2b5050148e65ef2be8c3 usr/share/man/ja_JP.UTF-8/man1/prctl.1
x 48cb8688a0021d94f5169578b4baf3901d3a45ef usr/share/man/ja_JP.UTF-8/man1/print.1
x 22b9b8716aea7335254b2076634ceb3a89ad65d2 usr/share/man/ja_JP.UTF-8/man1/priocntl.1
x 3679d153c3f0882761affe07471d4a49ab7d32ba usr/share/man/ja_JP.UTF-8/man1/proc.1
x 300de6ee8469ec629e727da4f88c3375f2d0a833 usr/share/man/ja_JP.UTF-8/man1/profiles.1
x d4da4f785370984216f918c8ef174ae3aad198bb usr/share/man/ja_JP.UTF-8/man1/ptree.1
x a132cd46ff8e4ecff4339c448510969c71642fa7 usr/share/man/ja_JP.UTF-8/man1/rcp.1
x 054520182fbde4db416584bdc2102de283ec1879 usr/share/man/ja_JP.UTF-8/man1/rdist.1
x 437954231a2307f4e3baa08f403c9e1ff6b8a1bc usr/share/man/ja_JP.UTF-8/man1/readonly.1
x 92b8e23a3c5455f0d2344381b28423b8deb52c35 usr/share/man/ja_JP.UTF-8/man1/regcmp.1
x 006b83a1bd7340b89761deb8e3079de40face24f usr/share/man/ja_JP.UTF-8/man1/remote_shell.1
x 006b83a1bd7340b89761deb8e3079de40face24f usr/share/man/ja_JP.UTF-8/man1/remsh.1
x 568336b6c111fd80fd5f399f01354b8bd3335f42 usr/share/man/ja_JP.UTF-8/man1/repeat.1
x 4e350184fca3aa774f257a40030a69a213dfa146 usr/share/man/ja_JP.UTF-8/man1/return.1
x 1e0e1b2a6fb84987c890f25671b5b663c1bfc71a usr/share/man/ja_JP.UTF-8/man1/rlogin.1
x a74e9f9b220e3c40e71af3dcfc140fdd78fb7c19 usr/share/man/ja_JP.UTF-8/man1/rmformat.1
x 0be70a899d97a205f195ee5a1fb98275c104e871 usr/share/man/ja_JP.UTF-8/man1/roles.1
x 6dce42d7b1a5ec6dbf5d523fa921dbb3557705f8 usr/share/man/ja_JP.UTF-8/man1/rsh.1
x 580e953c3b24047643a4b85ffc5cd87db95bff53 usr/share/man/ja_JP.UTF-8/man1/rusers.1
x 05c735ce6c6a80b95bcdd2c994d83bc341fc658b usr/share/man/ja_JP.UTF-8/man1/script.1
x 2e4be5783b4d58e5353f5029275c47b119766f7d usr/share/man/ja_JP.UTF-8/man1/sdiff.1
x 568336b6c111fd80fd5f399f01354b8bd3335f42 usr/share/man/ja_JP.UTF-8/man1/select.1
x dc6739dbde4a3aed600217b825dea90887b690f4 usr/share/man/ja_JP.UTF-8/man1/set.1
x eb1f899bdffaecd2078ae4fa122f97efa3549ea8 usr/share/man/ja_JP.UTF-8/man1/setenv.1
x 69d018dd175c97f65253391fe30db0365d0003b7 usr/share/man/ja_JP.UTF-8/man1/setlabel.1
x 39f7563a4dd1cc684c4178d956878356259dd9b4 usr/share/man/ja_JP.UTF-8/man1/shell_builtins.1
x 91201950587b18a6759797e3c10dc6e66ae3d5c1 usr/share/man/ja_JP.UTF-8/man1/shift.1
x 97597de0d6c5cf30b8d48b591b7583d234eabe97 usr/share/man/ja_JP.UTF-8/man1/source.1
x a67d1d869cb0a7bbb428e656da9aeca9657cec9a usr/share/man/ja_JP.UTF-8/man1/srchtxt.1
x 95df285ec994856e11e6620b280d8dd4ae843dde usr/share/man/ja_JP.UTF-8/man1/strchg.1
x ca51c1b34a36428b586c5570d0147cdd07c5c0b4 usr/share/man/ja_JP.UTF-8/man1/strconf.1
x 31aa45f79214a74876411e8c5493ff3cf1c91f66 usr/share/man/ja_JP.UTF-8/man1/suspend.1
x 33aee2573819c1598542a1a652cd5b172a455dc5 usr/share/man/ja_JP.UTF-8/man1/svcprop.1
x 02154b76d30d33a64e489a5cfa7c8db18daff16b usr/share/man/ja_JP.UTF-8/man1/svcs.1
x 568336b6c111fd80fd5f399f01354b8bd3335f42 usr/share/man/ja_JP.UTF-8/man1/switch.1
x 01aba7aa7c1286d97e82688b36330f256b39562f usr/share/man/ja_JP.UTF-8/man1/tbl.1
x 170b53e52653aca6f0a6100e63f7a3410b6bfb9e usr/share/man/ja_JP.UTF-8/man1/times.1
x c224d1d81de63414330215dfd5952b799b850dc5 usr/share/man/ja_JP.UTF-8/man1/trap.1
x d655805d5b8d674cc90399e252dcd2d523f6e986 usr/share/man/ja_JP.UTF-8/man1/troff.1
x e495d8b36a78966b1c633da83e59026d0420d6d5 usr/share/man/ja_JP.UTF-8/man1/truss.1
x eb1f899bdffaecd2078ae4fa122f97efa3549ea8 usr/share/man/ja_JP.UTF-8/man1/unset.1
x eb1f899bdffaecd2078ae4fa122f97efa3549ea8 usr/share/man/ja_JP.UTF-8/man1/unsetenv.1
x 568336b6c111fd80fd5f399f01354b8bd3335f42 usr/share/man/ja_JP.UTF-8/man1/until.1
x 5c0cab7afe95c20049b25c673e5b6f79591e2442 usr/share/man/ja_JP.UTF-8/man1/vacation.1
x c9a65b4b1ac94a7da3ab3525ff40665ffbc50776 usr/share/man/ja_JP.UTF-8/man1/valdate.1
x e17afea0536d1bd09c295e40069c78271f25e87b usr/share/man/ja_JP.UTF-8/man1/valgid.1
x 764ad44dd8f24866aa8d2903d5ad32750dd17f0b usr/share/man/ja_JP.UTF-8/man1/valint.1
x a9ba3244a1a98ed86adbd25771a6db22c94dbc61 usr/share/man/ja_JP.UTF-8/man1/valpath.1
x d9dc9111b774bee703fb3a7363dd245c67348f8f usr/share/man/ja_JP.UTF-8/man1/valrange.1
x 0d8335b8e3f74d5218bda670499e3d38d5d6deab usr/share/man/ja_JP.UTF-8/man1/valstr.1
x e557b4522e5a1507a22aae7be0a720dd6a68222b usr/share/man/ja_JP.UTF-8/man1/valtime.1
x 4116e5b655f887aaeb13a40d2749cca0d93bfb07 usr/share/man/ja_JP.UTF-8/man1/valuid.1
x 3d5d31c51f643ee3329a9f663bc382da4bbfd306 usr/share/man/ja_JP.UTF-8/man1/valyorn.1
x a5a2d861684ca50d3b4f10d8a7d2c3ace5930f8f usr/share/man/ja_JP.UTF-8/man1/volcheck.1
x 50862c685ed9386460bdf0b4f0dfff8d54ae8dad usr/share/man/ja_JP.UTF-8/man1/whatis.1
x 568336b6c111fd80fd5f399f01354b8bd3335f42 usr/share/man/ja_JP.UTF-8/man1/while.1
x b3423b02a15bdfbf53923e8608db5767056cd98c usr/share/man/ja_JP.UTF-8/man1/whois.1
x 34865540d25bdbaaa55d7008fe9ef9762caa9788 usr/share/man/ja_JP.UTF-8/man1/ypcat.1
x d90f4281b4db1214be409296b727828252b68f4c usr/share/man/ja_JP.UTF-8/man1/ypmatch.1
x 4dc12618af2d1ba87ba928546740db385d73d228 usr/share/man/ja_JP.UTF-8/man1/yppasswd.1
x b01f6fccacdf3d224b0ac37298da785622667109 usr/share/man/ja_JP.UTF-8/man1/ypwhich.1
x 6b4cf74f48feacb9456ee10650ae4da40a79ae66 usr/share/man/ja_JP.UTF-8/man1/zlogin.1
x a02e6c5d4bc9528cc1584940df19588a0746cc35 usr/share/man/ja_JP.UTF-8/man1/zonename.1
x 757f4f22a3ba8f3d6ef1f2ba0c8d417124889234 usr/share/man/ja_JP.UTF-8/man1b/du.1b
x 65f002b8461a8d3eda87ec8e62f88c612ea7578c usr/share/man/ja_JP.UTF-8/man1b/file.1b
x bb3b7243e3d05333c23cda853113a8df876e6f19 usr/share/man/ja_JP.UTF-8/man1b/lpr.1b
x e1fd48a866d51e9196ff879f9c966e7eab70a444 usr/share/man/ja_JP.UTF-8/man1m/accept.1m
x f8276dab5d104a047f50bb6cbe52903764debccf usr/share/man/ja_JP.UTF-8/man1m/acct.1m
x 5d36f284e600877b02bb52ab570483dc26b97894 usr/share/man/ja_JP.UTF-8/man1m/acctdisk.1m
x 5d36f284e600877b02bb52ab570483dc26b97894 usr/share/man/ja_JP.UTF-8/man1m/acctdusg.1m
x 5d36f284e600877b02bb52ab570483dc26b97894 usr/share/man/ja_JP.UTF-8/man1m/accton.1m
x 5d36f284e600877b02bb52ab570483dc26b97894 usr/share/man/ja_JP.UTF-8/man1m/acctwtmp.1m
x 85f878316062acc03d06d1589cde1679c3abc82d usr/share/man/ja_JP.UTF-8/man1m/add_allocatable.1m
x e03b1243c25c7aed35596870832196c5dc900f65 usr/share/man/ja_JP.UTF-8/man1m/atohexlabel.1m
x 84512eb70a0f2cb201cd803b3eb1958f3b58ce40 usr/share/man/ja_JP.UTF-8/man1m/automount.1m
x 387d015247c0028c1909bba49925ca6e1b302132 usr/share/man/ja_JP.UTF-8/man1m/boot.1m
x 33b3a03ff1ce0326c05cad3235257d44147a0878 usr/share/man/ja_JP.UTF-8/man1m/bootadm.1m
x 8658c6c2bd9060b8b576c15ace54a6ed2686ab25 usr/share/man/ja_JP.UTF-8/man1m/catman.1m
x 7d6bb89f30b142b3488188cd7f510d35821a1dfe usr/share/man/ja_JP.UTF-8/man1m/cfgadm_ac.1m
x 1769632f0c891e5dd444476ae017dca380d53494 usr/share/man/ja_JP.UTF-8/man1m/cfgadm_sysctrl.1m
x f6feae078521afd32ddd1039a3968d5750f6c449 usr/share/man/ja_JP.UTF-8/man1m/cfgadm.1m
x 5d36f284e600877b02bb52ab570483dc26b97894 usr/share/man/ja_JP.UTF-8/man1m/closewtmp.1m
x 061cc4487a22529c5b5af171e7e22c6235741341 usr/share/man/ja_JP.UTF-8/man1m/coreadm.1m
x d0830de3fb341597e3b990fedd233add0f025c31 usr/share/man/ja_JP.UTF-8/man1m/cvcd.1m
x 3b4d030628f1e5832e9b1e85308c714ac7abf9a8 usr/share/man/ja_JP.UTF-8/man1m/dtrace.1m
x 38738bcdbdbfa81a75b390daf4dfff582ef781f8 usr/share/man/ja_JP.UTF-8/man1m/fdisk.1m
x 4e2d96ba76059aee9b831e65436094600ec9803d usr/share/man/ja_JP.UTF-8/man1m/format.1m
x 1711fc259ca49612e70d0ccb79f03053033205d4 usr/share/man/ja_JP.UTF-8/man1m/fsck.1m
x 26359281ac81a9b14cae2e90b8a461b1baaab86c usr/share/man/ja_JP.UTF-8/man1m/fuser.1m
x 1f78b86523643667f4984f1885a1d8372208107c usr/share/man/ja_JP.UTF-8/man1m/growfs.1m
x 3ea223b386e3394494b64101b8d94f2fd6522e85 usr/share/man/ja_JP.UTF-8/man1m/halt.1m
x f8057d0a68caff19375069600c116697ef8a7d00 usr/share/man/ja_JP.UTF-8/man1m/hextoalabel.1m
x 940ec27886ba23ffe69c5b7ac230218e99ab0a1b usr/share/man/ja_JP.UTF-8/man1m/ifconfig.1m
x fba073fec1df31df9582a7901c345afa4742f4b7 usr/share/man/ja_JP.UTF-8/man1m/inetadm.1m
x 024da1683c795ba4498322d56a17455dc23e8586 usr/share/man/ja_JP.UTF-8/man1m/inetconv.1m
x 09c3d3f4a93f5eba6bba6e76feca3f73f9e8cb8c usr/share/man/ja_JP.UTF-8/man1m/init.1m
x c6f72e0e45e665c439c7dcbc25f2969c8da4f3fe usr/share/man/ja_JP.UTF-8/man1m/installgrub.1m
x 5d14cadff439b2e36ea1927af531d9440bdab84c usr/share/man/ja_JP.UTF-8/man1m/kdmconfig.1m
x d9732ccfc13206090105ed7ff451556f99c5d03b usr/share/man/ja_JP.UTF-8/man1m/lpsched.1m
x bc79369332dcaed972b639b321735acd231707cd usr/share/man/ja_JP.UTF-8/man1m/lpshut.1m
x 13f7405b5343405f2f89100c789b6d819a19339d usr/share/man/ja_JP.UTF-8/man1m/luxadm.1m
x 5383bb4ffb79183c1b65827b7e54d40736092076 usr/share/man/ja_JP.UTF-8/man1m/mdlogd.1m
x 78aa7d10beee13cdbecd5319e5e5e7f2a5d6ba5a usr/share/man/ja_JP.UTF-8/man1m/metaclear.1m
x ac956ab84f276b3e89455ad2aa7b47357d71954b usr/share/man/ja_JP.UTF-8/man1m/metadb.1m
x 33164d9f49e9b867c4e57598a925d0d053d50ab4 usr/share/man/ja_JP.UTF-8/man1m/metadetach.1m
x 8368a4ac9ed6ff6797b2ae946e531d6632d0bff4 usr/share/man/ja_JP.UTF-8/man1m/metahs.1m
x 8d2529512f17ff579f685feb6129a2a80d417380 usr/share/man/ja_JP.UTF-8/man1m/metainit.1m
x bbfdd6086d3cb2da498c8c75961d07f64423cadc usr/share/man/ja_JP.UTF-8/man1m/metaoffline.1m
x 35f7d5e2e9952c92d872c109440d221e460185bc usr/share/man/ja_JP.UTF-8/man1m/metaonline.1m
x 459f6d31a8c9e1f3fe07c57d846a502277b92137 usr/share/man/ja_JP.UTF-8/man1m/metaparam.1m
x 53228d72164be269ab564534ceabb080d4f5bb44 usr/share/man/ja_JP.UTF-8/man1m/metarename.1m
x e09dfa728829966f1f688df28f9abeca8e522df1 usr/share/man/ja_JP.UTF-8/man1m/metareplace.1m
x 9229d3857bc3bb1c5916960f9568f2bd72db1100 usr/share/man/ja_JP.UTF-8/man1m/metaroot.1m
x 56f70c79f2a98bf7937ce51dd21e92a1cece130e usr/share/man/ja_JP.UTF-8/man1m/metaset.1m
x ef1047bf6978bff2c1f79ba0225e7714ddf755bf usr/share/man/ja_JP.UTF-8/man1m/metastat.1m
x a0e8219dabd5e08ebcf830c213b07b9870d642cf usr/share/man/ja_JP.UTF-8/man1m/metasync.1m
x 901d6e7cf2c2a001d5e512f7bf692f1560046511 usr/share/man/ja_JP.UTF-8/man1m/metattach.1m
x 9a19e5d276158498603f0ba911a2736636dc2042 usr/share/man/ja_JP.UTF-8/man1m/mkfile.1m
x b568da88c4770bca84f820564b614515265a847d usr/share/man/ja_JP.UTF-8/man1m/mkfs.1m
x 26118b5b789e7d6ceadc601c1d1960e60f8a4265 usr/share/man/ja_JP.UTF-8/man1m/modinfo.1m
x 0398887a184d42dfac1423c1d68e9d725bebc459 usr/share/man/ja_JP.UTF-8/man1m/modload.1m
x 3f41eea41756c96fc971eebb1bc6af82e6c69ddc usr/share/man/ja_JP.UTF-8/man1m/modunload.1m
x bab76169ff82c4e96e604088ef790cdec3048ef9 usr/share/man/ja_JP.UTF-8/man1m/mount.1m
x 9a76bf7011830b06b40b002e251ae7801f8912b2 usr/share/man/ja_JP.UTF-8/man1m/mountall.1m
x adcc15570e33228ca9a2248f864b92667146b251 usr/share/man/ja_JP.UTF-8/man1m/newfs.1m
x ef10dfca5c41409ccf43898ffa72d7b9a0ecc4a0 usr/share/man/ja_JP.UTF-8/man1m/patchadd.1m
x 1c8af7695d768394560318ab3017028338408875 usr/share/man/ja_JP.UTF-8/man1m/patchrm.1m
x e240b02237a09a5fd67c947d4826f6c5269157ed usr/share/man/ja_JP.UTF-8/man1m/pkgadd.1m
x 9db886358ce954425156ea19ce7c653524ea68af usr/share/man/ja_JP.UTF-8/man1m/pkgrm.1m
x 1ffaa92e61c6d43c5b717c6e573048b45f591472 usr/share/man/ja_JP.UTF-8/man1m/pmconfig.1m
x 8015b359e3a56d655a1d12547e7ab7ebd46c5e9d usr/share/man/ja_JP.UTF-8/man1m/pooladm.1m
x 57345ee70ab2c340404d71b264f3a833585b5a9c usr/share/man/ja_JP.UTF-8/man1m/poolcfg.1m
x 47d49efc60719733249d09e32c70863499e71153 usr/share/man/ja_JP.UTF-8/man1m/powerd.1m
x 2f44113a5aacd01f94225c17a4564362eaacbcfa usr/share/man/ja_JP.UTF-8/man1m/poweroff.1m
x c0a156864de0147fb90b0d279307785a9e86ae3b usr/share/man/ja_JP.UTF-8/man1m/prodreg.1m
x 981c77db51d60f7c4c7fdea4502ca0c4721df300 usr/share/man/ja_JP.UTF-8/man1m/prstat.1m
x 4e6cfa5d795cef24419854b7e8158b27dd32dcc4 usr/share/man/ja_JP.UTF-8/man1m/prtconf.1m
x 5dc8ca898e2ffb9e3c4c9f858bb9473b5adee853 usr/share/man/ja_JP.UTF-8/man1m/prtdiag.1m
x 9455a331bc9c5b40d615d5ca1e18e989d2b5a873 usr/share/man/ja_JP.UTF-8/man1m/raidctl.1m
x 6e031c0ea41d78fe7b4e531460ed0516961823ba usr/share/man/ja_JP.UTF-8/man1m/rctladm.1m
x 7cb3c32316d4c505f53fda8abffc2a18c89d5539 usr/share/man/ja_JP.UTF-8/man1m/reboot.1m
x 213b74dc0a4a1c649b2c0b477b607399c0fceb1f usr/share/man/ja_JP.UTF-8/man1m/reject.1m
x 03fd75d08c76911dfd685f79c85842295d286562 usr/share/man/ja_JP.UTF-8/man1m/remove_allocatable.1m
x 77931a43f33bbfddcf6e2d04ced05679fb259e5f usr/share/man/ja_JP.UTF-8/man1m/restricted_shell.1m
x f7e589aeb0cdf9fc760ecab6959f9db514df0625 usr/share/man/ja_JP.UTF-8/man1m/rpc.metad.1m
x 6a855fc3e426fd0d2352abe654cc4afb11870375 usr/share/man/ja_JP.UTF-8/man1m/rpc.metamhd.1m
x 2f1ca67bad573eead3ad1e69a9a327bf8e70ce70 usr/share/man/ja_JP.UTF-8/man1m/rsh.1m
x cda7da277fab9210564f2872ae3e857f586894e9 usr/share/man/ja_JP.UTF-8/man1m/scadm.1m
x a02201b8e2ecc408cd3c76acae31142f13cb854c usr/share/man/ja_JP.UTF-8/man1m/share.1m
x b522ea81c9dae420123649a77348ae1451697c06 usr/share/man/ja_JP.UTF-8/man1m/shareall.1m
x 0a98a4738ca013ad06cf76ec3e376d110eadad79 usr/share/man/ja_JP.UTF-8/man1m/showmount.1m
x e5643fc8580afa100f51d25dc9b207b0f54dfc9f usr/share/man/ja_JP.UTF-8/man1m/shutdown.1m
x 0ee25af10f36634084cf8c4b4b68455995807f41 usr/share/man/ja_JP.UTF-8/man1m/smtnrhdb.1m
x 174afc8d6620c71fbf747c0dfebcc7878e005d6b usr/share/man/ja_JP.UTF-8/man1m/smtnrhtp.1m
x aa6e502862f2a2afb30b945ceb80e3a17b0a1191 usr/share/man/ja_JP.UTF-8/man1m/smtnzonecfg.1m
x 5b7f16b50088c64b2d0314784bbd089fbdc3e26e usr/share/man/ja_JP.UTF-8/man1m/snoop.1m
x 05f2b865b75159187df9d811301c00ca14bdaa9d usr/share/man/ja_JP.UTF-8/man1m/su.1m
x 5f909a6d643c5fc88778b505040ad6b3bfca0526 usr/share/man/ja_JP.UTF-8/man1m/svc.startd.1m
x c95664a8f4365c6b48a92a4b227f8d9fed7f202b usr/share/man/ja_JP.UTF-8/man1m/svcadm.1m
x 2412489c1fd1315f6ee67a1440079f21227779c0 usr/share/man/ja_JP.UTF-8/man1m/svccfg.1m
x 3d13a5e5cbe0c3dd31d55e635903270dae6d61c5 usr/share/man/ja_JP.UTF-8/man1m/swap.1m
x 51d8bc7583462434117f489677e5098439056e0b usr/share/man/ja_JP.UTF-8/man1m/sys-unconfig.1m
x 8278252691f9144a5e55f61836bf5fb92257adb2 usr/share/man/ja_JP.UTF-8/man1m/telinit.1m
x 9df3568a0afefa20233ab42345d801667a7598cc usr/share/man/ja_JP.UTF-8/man1m/tnchkdb.1m
x c6d7b2ecb4239b87fe397e0855863044273d1eb8 usr/share/man/ja_JP.UTF-8/man1m/tnctl.1m
x 0deca72ea9c8288b4915c7c813093abe6b1b69b6 usr/share/man/ja_JP.UTF-8/man1m/tnd.1m
x e4581379dc56fbd6fa2b63ae3b5f67208319e329 usr/share/man/ja_JP.UTF-8/man1m/tninfo.1m
x ae639f7501809259ce9d44c6e68e48dcce864ca4 usr/share/man/ja_JP.UTF-8/man1m/ttymon.1m
x d1b78a9727d2fa411c43134492e8d494fa03b2ce usr/share/man/ja_JP.UTF-8/man1m/umount.1m
x d72e8dd214cb021b39cc0b6316eb000d917796aa usr/share/man/ja_JP.UTF-8/man1m/umountall.1m
x f944c8a2f9e33810e9c68491cc58a77745a31c83 usr/share/man/ja_JP.UTF-8/man1m/unshare.1m
x 2a328b297b76215c0c106ba2662d07b629758dc9 usr/share/man/ja_JP.UTF-8/man1m/unshareall.1m
x a6a6db2c4886f56f7a5c812edd902fa975d828e4 usr/share/man/ja_JP.UTF-8/man1m/updatehome.1m
x 5d36f284e600877b02bb52ab570483dc26b97894 usr/share/man/ja_JP.UTF-8/man1m/utmp2wtmp.1m
x 1e9dffc19c0b4671f25adfbbcaf736515e01da3a usr/share/man/ja_JP.UTF-8/man1m/wall.1m
x 2699912b7aa532043c5bec6d6aeee8f7786fe70c usr/share/man/ja_JP.UTF-8/man1m/zfs.1m
x d6a92ac6199a705718a0d5d74513c09ef702125d usr/share/man/ja_JP.UTF-8/man1m/zoneadm.1m
x f2c78632efa948e8338459a22f6c5af5b8abc948 usr/share/man/ja_JP.UTF-8/man1m/zonecfg.1m
x d01379b9dc6409c9c63fc9ccf0898a7c215f222b usr/share/man/ja_JP.UTF-8/man1m/zpool.1m
x fdea07c4354f6c17c8ad3116a7b606e3a374aae9 usr/share/man/ja_JP.UTF-8/man2/fgetlabel.2
x 33b8a66529c99651296ef02cad1d41f9d9023ac5 usr/share/man/ja_JP.UTF-8/man2/getlabel.2
x a1050cadf00552b890e80374f342c7b59ba6663b usr/share/man/ja_JP.UTF-8/man2/intro.2
x 1dfbb5a1eb975944c517d078b30a3f1a0cce89e1 usr/share/man/ja_JP.UTF-8/man2/Intro.2
x 250ba5c45011ab30a008038fdc4dd2c64cc17b57 usr/share/man/ja_JP.UTF-8/man3c/bind_textdomain_codeset.3c
x 250ba5c45011ab30a008038fdc4dd2c64cc17b57 usr/share/man/ja_JP.UTF-8/man3c/bindtextdomain.3c
x c486fa0c54611f9d08658406068c065083bc2b5b usr/share/man/ja_JP.UTF-8/man3c/cset.3c
x 6221bd5c190f3786bb394a7d8bfb573d25f72037 usr/share/man/ja_JP.UTF-8/man3c/csetcol.3c
x 6221bd5c190f3786bb394a7d8bfb573d25f72037 usr/share/man/ja_JP.UTF-8/man3c/csetlen.3c
x 6221bd5c190f3786bb394a7d8bfb573d25f72037 usr/share/man/ja_JP.UTF-8/man3c/csetno.3c
x 250ba5c45011ab30a008038fdc4dd2c64cc17b57 usr/share/man/ja_JP.UTF-8/man3c/dcgettext.3c
x 250ba5c45011ab30a008038fdc4dd2c64cc17b57 usr/share/man/ja_JP.UTF-8/man3c/dcngettext.3c
x 250ba5c45011ab30a008038fdc4dd2c64cc17b57 usr/share/man/ja_JP.UTF-8/man3c/dgettext.3c
x 250ba5c45011ab30a008038fdc4dd2c64cc17b57 usr/share/man/ja_JP.UTF-8/man3c/dngettext.3c
x 973fb853e679e832fc482a4e90671fb32e9b2e2d usr/share/man/ja_JP.UTF-8/man3c/euccol.3c
x c35867c382c45603a86ae2cb0d81a724f8a98c23 usr/share/man/ja_JP.UTF-8/man3c/euclen.3c
x 973fb853e679e832fc482a4e90671fb32e9b2e2d usr/share/man/ja_JP.UTF-8/man3c/eucscol.3c
x 64b86fe45910c3e19699b50af5ac91ccfb31fbad usr/share/man/ja_JP.UTF-8/man3c/fgetws.3c
x 8586edd239b68de7923418ca4dcff26ba806fdbe usr/share/man/ja_JP.UTF-8/man3c/gettext.3c
x 68da2a77c997a0606331b93de7f45263beea8926 usr/share/man/ja_JP.UTF-8/man3c/getwidth.3c
x 5ae74168f6738bca6681ffc032e77225379e9d7b usr/share/man/ja_JP.UTF-8/man3c/getws.3c
x 250ba5c45011ab30a008038fdc4dd2c64cc17b57 usr/share/man/ja_JP.UTF-8/man3c/ngettext.3c
x 40b17278a83214f866370aa26e49e7470f1aa5b5 usr/share/man/ja_JP.UTF-8/man3c/setlocale.3c
x 250ba5c45011ab30a008038fdc4dd2c64cc17b57 usr/share/man/ja_JP.UTF-8/man3c/textdomain.3c
x 408790ee38e8d46fcb63893a889824a3614b95df usr/share/man/ja_JP.UTF-8/man3c/toascii.3c
x 846902b1e82de78e00211f78cf2bf2e86538e7fb usr/share/man/ja_JP.UTF-8/man3c/tolower.3c
x f7a778b9d6d65865835f2b98de4bb962ed504601 usr/share/man/ja_JP.UTF-8/man3c/toupper.3c
x 6221bd5c190f3786bb394a7d8bfb573d25f72037 usr/share/man/ja_JP.UTF-8/man3c/wcsetno.3c
x 2c6678761cf83b8ed22dee3b7445800114283718 usr/share/man/ja_JP.UTF-8/man3c/wscasecmp.3c
x 2c6678761cf83b8ed22dee3b7445800114283718 usr/share/man/ja_JP.UTF-8/man3c/wscol.3c
x 2c6678761cf83b8ed22dee3b7445800114283718 usr/share/man/ja_JP.UTF-8/man3c/wsdup.3c
x 2c6678761cf83b8ed22dee3b7445800114283718 usr/share/man/ja_JP.UTF-8/man3c/wsncasecmp.3c
x 90b56ef93910bc216aae94efb5f4a26fc566cbc2 usr/share/man/ja_JP.UTF-8/man3c/wsprintf.3c
x 16ab87e5e339feea2ce06bedf929a9f221cac977 usr/share/man/ja_JP.UTF-8/man3c/wsscanf.3c
x a4b62e249ca317b99db28aa19d51d5c13197e1a1 usr/share/man/ja_JP.UTF-8/man3c/wstring.3c
x 06ee3b16d231a4a017c255af7fe49d101c654c0e usr/share/man/ja_JP.UTF-8/man3cfgadm/config_admin.3cfgadm
x 2d2ecb95cc2c2f8a7cf9219961ce6eef0f840758 usr/share/man/ja_JP.UTF-8/man3cfgadm/config_ap_id_cmp.3cfgadm
x 2d2ecb95cc2c2f8a7cf9219961ce6eef0f840758 usr/share/man/ja_JP.UTF-8/man3cfgadm/config_change_state.3cfgadm
x 2d2ecb95cc2c2f8a7cf9219961ce6eef0f840758 usr/share/man/ja_JP.UTF-8/man3cfgadm/config_list_ext.3cfgadm
x 2d2ecb95cc2c2f8a7cf9219961ce6eef0f840758 usr/share/man/ja_JP.UTF-8/man3cfgadm/config_list.3cfgadm
x 2d2ecb95cc2c2f8a7cf9219961ce6eef0f840758 usr/share/man/ja_JP.UTF-8/man3cfgadm/config_private_func.3cfgadm
x 2d2ecb95cc2c2f8a7cf9219961ce6eef0f840758 usr/share/man/ja_JP.UTF-8/man3cfgadm/config_stat.3cfgadm
x 2d2ecb95cc2c2f8a7cf9219961ce6eef0f840758 usr/share/man/ja_JP.UTF-8/man3cfgadm/config_strerror.3cfgadm
x 2d2ecb95cc2c2f8a7cf9219961ce6eef0f840758 usr/share/man/ja_JP.UTF-8/man3cfgadm/config_test.3cfgadm
x 2d2ecb95cc2c2f8a7cf9219961ce6eef0f840758 usr/share/man/ja_JP.UTF-8/man3cfgadm/config_unload_libs.3cfgadm
x cc1e1a1c5634c5f61c23c1bec5aada00c223b0e5 usr/share/man/ja_JP.UTF-8/man3gen/advance.3gen
x cc1e1a1c5634c5f61c23c1bec5aada00c223b0e5 usr/share/man/ja_JP.UTF-8/man3gen/compile.3gen
x 4ab993739b144c9b27337a3ec8d656f5ec7c6a74 usr/share/man/ja_JP.UTF-8/man3gen/isencrypt.3gen
x fb4cedd65bd01c87550b000dd34214ef8b9b8702 usr/share/man/ja_JP.UTF-8/man3gen/regexpr.3gen
x cc1e1a1c5634c5f61c23c1bec5aada00c223b0e5 usr/share/man/ja_JP.UTF-8/man3gen/step.3gen
x 7d459f66da6f37264b6f88f236dca2f2230f7b7f usr/share/man/ja_JP.UTF-8/man3lib/libcfgadm.3lib
x c72e2d0fb5ba74cbac24e61118a0b02c8de6e939 usr/share/man/ja_JP.UTF-8/man3tsol/bcleartoh_r.3tsol
x c72e2d0fb5ba74cbac24e61118a0b02c8de6e939 usr/share/man/ja_JP.UTF-8/man3tsol/bcleartoh.3tsol
x 0a602a3e4936eecabd0c2d75fa4fbd276924ea21 usr/share/man/ja_JP.UTF-8/man3tsol/bcleartos.3tsol
x 60479c3c1ff9717b3bf0616fb933fcd2de57ab7f usr/share/man/ja_JP.UTF-8/man3tsol/blcompare.3tsol
x 98f93777db5443f9f68bbdb7ebd9ccc74080dca6 usr/share/man/ja_JP.UTF-8/man3tsol/bldominates.3tsol
x 98f93777db5443f9f68bbdb7ebd9ccc74080dca6 usr/share/man/ja_JP.UTF-8/man3tsol/blequal.3tsol
x 98f93777db5443f9f68bbdb7ebd9ccc74080dca6 usr/share/man/ja_JP.UTF-8/man3tsol/blinrange.3tsol
x 584b2eed0b66e6d05b46438b7a68c86227ad7271 usr/share/man/ja_JP.UTF-8/man3tsol/blmaximum.3tsol
x 584b2eed0b66e6d05b46438b7a68c86227ad7271 usr/share/man/ja_JP.UTF-8/man3tsol/blminimum.3tsol
x b275f0ea9b95cd0177f46b2b45117c0e7db53e2c usr/share/man/ja_JP.UTF-8/man3tsol/blminmax.3tsol
x 98f93777db5443f9f68bbdb7ebd9ccc74080dca6 usr/share/man/ja_JP.UTF-8/man3tsol/blstrictdom.3tsol
x 355794eeb1db49dc2ed278b3d0aa30ab9e44f7c9 usr/share/man/ja_JP.UTF-8/man3tsol/bltocolor_r.3tsol
x 468bcdb5b3b9e09c804c953a985510c8945bee66 usr/share/man/ja_JP.UTF-8/man3tsol/bltocolor.3tsol
x f3c6a3fe143d7d4aa466d88fbf40750f08d2075c usr/share/man/ja_JP.UTF-8/man3tsol/bltos.3tsol
x c72e2d0fb5ba74cbac24e61118a0b02c8de6e939 usr/share/man/ja_JP.UTF-8/man3tsol/bsltoh_r.3tsol
x c72e2d0fb5ba74cbac24e61118a0b02c8de6e939 usr/share/man/ja_JP.UTF-8/man3tsol/bsltoh.3tsol
x 0a602a3e4936eecabd0c2d75fa4fbd276924ea21 usr/share/man/ja_JP.UTF-8/man3tsol/bsltos.3tsol
x bb66c81175a10de7fd37e936dec35bfa446a9797 usr/share/man/ja_JP.UTF-8/man3tsol/btohex.3tsol
x f5e8716f3f2b2922363a8fc42479a950ec5fc472 usr/share/man/ja_JP.UTF-8/man3tsol/getdevicerange.3tsol
x be100259aa51885102a5f0289f96fd26fe421413 usr/share/man/ja_JP.UTF-8/man3tsol/getpathbylabel.3tsol
x f2883f9c4333a3d0c033362b946e90ba33bb80c2 usr/share/man/ja_JP.UTF-8/man3tsol/getplabel.3tsol
x 84ed5987409a6b11cb8ba107d69ba3adc6657c6d usr/share/man/ja_JP.UTF-8/man3tsol/getuserrange.3tsol
x 7a138f19707dbfe0ddce42ddc37a3bdacc77e5d3 usr/share/man/ja_JP.UTF-8/man3tsol/getzoneidbylabel.3tsol
x d90eaf6d2f9afda104e5ae75199c0485a59a5288 usr/share/man/ja_JP.UTF-8/man3tsol/getzonelabelbyid.3tsol
x 7a138f19707dbfe0ddce42ddc37a3bdacc77e5d3 usr/share/man/ja_JP.UTF-8/man3tsol/getzonelabelbyname.3tsol
x e616118f81cb43bbf27c8bd794e20c7b0df89789 usr/share/man/ja_JP.UTF-8/man3tsol/getzonerootbyid.3tsol
x 35d9464fc10300452c43fdf27e6ff905c43f84e7 usr/share/man/ja_JP.UTF-8/man3tsol/getzonerootbylabel.3tsol
x 35d9464fc10300452c43fdf27e6ff905c43f84e7 usr/share/man/ja_JP.UTF-8/man3tsol/getzonerootbyname.3tsol
x c72e2d0fb5ba74cbac24e61118a0b02c8de6e939 usr/share/man/ja_JP.UTF-8/man3tsol/h_alloc.3tsol
x c72e2d0fb5ba74cbac24e61118a0b02c8de6e939 usr/share/man/ja_JP.UTF-8/man3tsol/h_free.3tsol
x 0a99cb96de35b1c5ef91f13816b9974edbb057ec usr/share/man/ja_JP.UTF-8/man3tsol/hextob.3tsol
x 43618d64d923108d52c5833b022bdeed6c00af7b usr/share/man/ja_JP.UTF-8/man3tsol/htobclear.3tsol
x 43618d64d923108d52c5833b022bdeed6c00af7b usr/share/man/ja_JP.UTF-8/man3tsol/htobsl.3tsol
x 730edf2aa73b309012c1a4241fb81dbca5a90a78 usr/share/man/ja_JP.UTF-8/man3tsol/label_to_str.3tsol
x 21fb438e43fbafcef5cbe97892d4f82ca178e057 usr/share/man/ja_JP.UTF-8/man3tsol/labelbuilder.3tsol
x ebe5d8e804a128ecc9acd0bacfb374192759ba4a usr/share/man/ja_JP.UTF-8/man3tsol/labelclipping.3tsol
x d5f225d8290ff8e3e91801c1508519611a21a198 usr/share/man/ja_JP.UTF-8/man3tsol/m_label_alloc.3tsol
x d5f225d8290ff8e3e91801c1508519611a21a198 usr/share/man/ja_JP.UTF-8/man3tsol/m_label_dup.3tsol
x d5f225d8290ff8e3e91801c1508519611a21a198 usr/share/man/ja_JP.UTF-8/man3tsol/m_label_free.3tsol
x 764c135dcee8e4b92d538d320350063f196b4cb4 usr/share/man/ja_JP.UTF-8/man3tsol/m_label.3tsol
x 9ee53d50dd0f4db1d2e841dfbba1511c63309e33 usr/share/man/ja_JP.UTF-8/man3tsol/sbcleartos.3tsol
x dafe2499a5ca862f76a1602b49d34a69f36ce2ea usr/share/man/ja_JP.UTF-8/man3tsol/sbltos.3tsol
x 9ee53d50dd0f4db1d2e841dfbba1511c63309e33 usr/share/man/ja_JP.UTF-8/man3tsol/sbsltos.3tsol
x fc57bddd5e5889aa766444b9bccf63c942c326f7 usr/share/man/ja_JP.UTF-8/man3tsol/setflabel.3tsol
x d62abda0ad2b0bb75521aef6e76d6a5172231641 usr/share/man/ja_JP.UTF-8/man3tsol/stobclear.3tsol
x 79f081f075a206c0adf8d8c758dac74b068459e0 usr/share/man/ja_JP.UTF-8/man3tsol/stobl.3tsol
x d62abda0ad2b0bb75521aef6e76d6a5172231641 usr/share/man/ja_JP.UTF-8/man3tsol/stobsl.3tsol
x 72d7d5308e7bdb81ac95fc319027348112381b6e usr/share/man/ja_JP.UTF-8/man3tsol/str_to_label.3tsol
x c5ef013787a4c57b9e986698ea6e70c0a5e29666 usr/share/man/ja_JP.UTF-8/man3tsol/tsol_lbuild_create.3tsol
x c5ef013787a4c57b9e986698ea6e70c0a5e29666 usr/share/man/ja_JP.UTF-8/man3tsol/tsol_lbuild_destroy.3tsol
x c5ef013787a4c57b9e986698ea6e70c0a5e29666 usr/share/man/ja_JP.UTF-8/man3tsol/tsol_lbuild_get.3tsol
x c5ef013787a4c57b9e986698ea6e70c0a5e29666 usr/share/man/ja_JP.UTF-8/man3tsol/tsol_lbuild_set.3tsol
x 63a628a05cdeb8b28f02d55c5a79d4583657e333 usr/share/man/ja_JP.UTF-8/man3tsol/Xbcleartos.3tsol
x 63a628a05cdeb8b28f02d55c5a79d4583657e333 usr/share/man/ja_JP.UTF-8/man3tsol/Xbsltos.3tsol
x 09ca91545d3cd35f9bb94e7d306cf06596aa5ff7 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLgetClientAttributes.3xtsol
x 0d8a45211208d0925d925d7a42cdd5c0f21b8e7f usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLgetPropAttributes.3xtsol
x e5646b2f51aebf1826466ee1a0a1028b134517f6 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLgetPropLabel.3xtsol
x b0d4f34f8b33809daca6573e70ada5ef80267865 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLgetPropUID.3xtsol
x f9c5d1795b870cb88415fb7e0ef279aad3bef1ee usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLgetResAttributes.3xtsol
x 36a979af17e5ba4b312e10501c9bf40fff420fd7 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLgetResLabel.3xtsol
x a3fa4131a1216caad8b13b452a89a3bdecb3d79e usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLgetResUID.3xtsol
x 371c9ff70673fd2d3c7b4fe6589c2125d18abcd6 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLgetSSHeight.3xtsol
x 760064138baa41121f156d8f2990470b16c56cfa usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLgetWorkstationOwner.3xtsol
x 7518785583c15d28a2869ee9e2202f198f002bc8 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLIsWindowTrusted.3xtsol
x 939f869aae97eac6a0d11214264a4205b1d50339 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLMakeTPWindow.3xtsol
x c0bedf022525f5f336417c885ff94a8ccd651e42 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLsetPolyInstInfo.3xtsol
x 3477445e900e161a2d5c285777f68cf49c5e6f80 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLsetPropLabel.3xtsol
x 9020a1d9bead0aab0ce24d2c2edacb000a59cd01 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLsetPropUID.3xtsol
x a1a0ddca5de268ad803198c9ef613140d2881304 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLsetResLabel.3xtsol
x 6e2b8c4a8878f8f34c0251d6ffa8c388741075b0 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLsetResUID.3xtsol
x 6b1d36d13243c524a7dbf7f6d1d26a1dd2049320 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLsetSessionHI.3xtsol
x 77bfa97ea4040f1436cd5ca8413b87318adc9280 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLsetSessionLO.3xtsol
x 39cc3641b69ad03d9f247f58355963c7abd46d97 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLsetSSHeight.3xtsol
x d82b8a1f528613fb142b823303313e125660f309 usr/share/man/ja_JP.UTF-8/man3xtsol/XTSOLsetWorkstationOwner.3xtsol
x e7500a08d8e64a1902f249001f759ba9b134d16d usr/share/man/ja_JP.UTF-8/man4/contract.4
x 8afa092d93905b1e12ff1bbcaf7cacac5be612aa usr/share/man/ja_JP.UTF-8/man4/intro.4
x dcab91e698dd98f5283d9102931d6e912a1657d4 usr/share/man/ja_JP.UTF-8/man4/Intro.4
x 3e94eb2d10d9c7e9a6a693946a6897f70f05fc45 usr/share/man/ja_JP.UTF-8/man4/md.cf.4
x 53d9467906ecd95179c351f6963a8ac5e10da41c usr/share/man/ja_JP.UTF-8/man4/md.tab.4
x 3e1a408e933a4ad98ca5396e6879d08079fa10ec usr/share/man/ja_JP.UTF-8/man4/mddb.cf.4
x 9cfb25a8384bff6afebf80ad4ed1ee8394ba60f1 usr/share/man/ja_JP.UTF-8/man4/sel_config.4
x a5b95d6b4f34865b9ae16a64ef1fdb2fca88951e usr/share/man/ja_JP.UTF-8/man4/TrustedExtensionsPolicy.4
x 283bad0da8598d4d339faea89a9b2aecebcaacd7 usr/share/man/ja_JP.UTF-8/man5/attributes.5
x f900b47b5f72d5174b708c9ac4bb78311106a5ee usr/share/man/ja_JP.UTF-8/man5/environ.5
x de906d5754a70ad0a678dc97da1dd91db2f3e56a usr/share/man/ja_JP.UTF-8/man5/grub.5
x 7aa296ef3aa458eb5402a3ffb23589a653ede1df usr/share/man/ja_JP.UTF-8/man5/intro.5
x 72f0e1f01f4200eec759f45925bd6428bc1c3ca3 usr/share/man/ja_JP.UTF-8/man5/Intro.5
x 020fbbbc96fd96c2639ad511a64929116ac1bd22 usr/share/man/ja_JP.UTF-8/man5/isalist.5
x 5525b29fad1d453f1ea7efa4106e5b5bfa383d6a usr/share/man/ja_JP.UTF-8/man5/pam_tsol_account.5
x 83372676e95593457c962602c37c4bc2acbe6bdc usr/share/man/ja_JP.UTF-8/man5/smf_security.5
x 099f2ff0df51ce7d4959dd9e575bc56e056f9c8f usr/share/man/ja_JP.UTF-8/man5/smf.5
x 7cbb29b48ec55044b572cfed0cedef8f7b47facd usr/share/man/ja_JP.UTF-8/man5/zones.5
x 02321d5bf5ffb8df4bad014f826733e0b4e2c713 usr/share/man/ja_JP.UTF-8/man7/cpr.7
x 03ebd4bfb124a53a382295297d6d6e1f9641f8ad usr/share/man/ja_JP.UTF-8/man7d/cvc.7d
x 337344fcca675c7999373e97f85c6e1f4fe2aab1 usr/share/man/ja_JP.UTF-8/man7d/cvcredir.7d
x db4c888501f81a3315b43836195c272c39eee0ac usr/share/man/ja_JP.UTF-8/man7d/dtrace.7d
x 166fd8af87e869d820b5e92f4fce7098fe723cb8 usr/share/man/ja_JP.UTF-8/man7d/ecpp.7d
x 6838aa76c8daa4a52a431775a463fedf08f9982e usr/share/man/ja_JP.UTF-8/man7d/fas.7d
x 907129967228a473ccfc4399a4ad2e128d1645c0 usr/share/man/ja_JP.UTF-8/man7d/hme.7d
x 2b4c4bf7994a2db14caf75655a0db8598f1d3113 usr/share/man/ja_JP.UTF-8/man7d/md.7d
x 1f5286d77df0e067e95ca5660520ccee29bd64de usr/share/man/ja_JP.UTF-8/man7d/pm.7d
x 645160ff22c4e7da051e10a0ecd0bd7e16d95c72 usr/share/man/ja_JP.UTF-8/man9f/scsi_hba_attach_setup.9f
x 8cca34475b38faf81cca477241d3656f6e068a2e usr/share/man/ja_JP.UTF-8/man9f/scsi_hba_attach.9f
x 8cca34475b38faf81cca477241d3656f6e068a2e usr/share/man/ja_JP.UTF-8/man9f/scsi_hba_detach.9f

# hashes from pkg://opensolaris.org/text/doctools/ja@0.5.11,5.11-0.134.0.2:20100529T012303Z

ARCH=all

x a25e9139ca18a1fa30f2657a34dc8b02698113bd usr/share/lib/sgml/locale/ja_JP.PCK/dtds/solbookv2/solbook.dcl
x 671f74745a37af53d7c8048b46b3e11128ea1eb3 usr/share/lib/sgml/locale/ja_JP.PCK/transpec/docbook-to-man.ts
x e869497278dea1d30a1c300b426a1980060e7fb8 usr/share/lib/sgml/locale/ja_JP.UTF-8/dtds/solbookv2/solbook.dcl
x f7c7d87526094e471cd40c450296233e5b330b43 usr/share/lib/sgml/locale/ja_JP.UTF-8/transpec/docbook-to-man.ts
x fa270aaabfc5843500fbce7d99c2d22a94d68b6d usr/share/lib/sgml/locale/ja/dtds/solbookv2/solbook.dcl
x 8400e79060b6a28e2bb281798c2b8aa8c79c3923 usr/share/lib/sgml/locale/ja/transpec/docbook-to-man.ts
x 3eece6dc3597dc1ba03c5fc5c964d6ae89780b3b usr/share/lib/tmac/an.ja
x b9e0287730e537a5885b909ebfa2ac75b759a613 usr/share/lib/tmac/an.ja_JP.PCK
x 7466e337c85700529057327c4fe6878dc12325ea usr/share/lib/tmac/an.ja_JP.UTF-8
x b9e0287730e537a5885b909ebfa2ac75b759a613 usr/share/lib/tmac/an.ja_JP.PCK
x 7466e337c85700529057327c4fe6878dc12325ea usr/share/lib/tmac/an.ja_JP.UTF-8
x a91d2024a262d2fed0d0ad7ebcd6ae789c1bcf46 usr/share/lib/tmac/ansun.ja_JP.PCK
x 14537a808e230b7c1dffb8da158eb8a2c6202223 usr/share/lib/tmac/ansun.ja
x de6c6fe56ac2f2925cf1aedc83b4b60407e4944c usr/share/lib/tmac/ansun.ja_JP.UTF-8
x a91d2024a262d2fed0d0ad7ebcd6ae789c1bcf46 usr/share/lib/tmac/ansun.ja_JP.PCK
x de6c6fe56ac2f2925cf1aedc83b4b60407e4944c usr/share/lib/tmac/ansun.ja_JP.UTF-8
x a1bfedfa53aa1a8ae7e317d0bacae0f386ea51f9 usr/share/lib/tmac/tz.map.ja_JP.UTF-8
x 53f9f80dd1022adef428cff56eeb608bf95c210d usr/share/lib/tmac/tz.map.ja
x ebe8fd0c9263b36d97d42c829adca4e92792f203 usr/share/lib/tmac/tz.map.ja_JP.PCK
x ebe8fd0c9263b36d97d42c829adca4e92792f203 usr/share/lib/tmac/tz.map.ja_JP.PCK
x a1bfedfa53aa1a8ae7e317d0bacae0f386ea51f9 usr/share/lib/tmac/tz.map.ja_JP.UTF-8

exit 0


# hashes from pkg://opensolaris.org/system/locale/ko-extra@0.5.11,5.11-0.134.0.2:20100529T010628Z

ARCH=all

x 542f93fdd553b601f6a772395ac2986b5356aade usr/include/ko/xctype.h
x 8eae3c2242fcba4cbeed6c213683f20b5a2c3f8f usr/lib/mle/ko.UTF-8/keybind.dat
x 1eff91064ba131092ae229cc72b556429950b750 usr/lib/mle/ko.UTF-8/syshjd
x 1c591c9d28a8bfbb28e1909896c9abc55706e3de usr/lib/mle/ko/keybind.dat
x ecb39d690e092405b3994ea97165977066b5a2b5 usr/lib/mle/ko/syshjd

ARCH=i386

x 96ee7b8d9771210098efb8a1ab4db13c4bad6eac usr/kernel/strmod/amd64/kpack
x 24c79331ccbb063fcb70ce02cd5b701c61bdac30 usr/lib/amd64/libkle.a
x e03a8711a8677175bb38aea5fff49d2b2e6746ff usr/lib/amd64/libkle.so.1
x fb9dbbc138f7704e6fb4cb6b87aeb0c55357ad3c usr/lib/libkle.a
x d6e2c8a09374dca03e02acd6f267bef27823d140 usr/lib/libkle.so.1
x d926425b1250f994cb5d82557cd74bf1432a3057 usr/lib/lp/locale/ko.UTF-8/mp/xufallback_ksc.so
x 57e99bbbf7a5d2305d844d5234fd4cfbfda97073 usr/lib/lp/locale/ko/mp/xwcksc5601_1987_udc.so
x 3e0df5a18b5563d3f696b2c95658bc6919201f0c usr/lib/lp/locale/ko/mp/xwcksc5601_1987_unicode.so
x a925effa77de456801a8caab59b557e9b6bc20e3 usr/lib/mle/ko.UTF-8/libmle.a
x e42efaac1a42149c8bf13270f64777fe90bbdc1f usr/lib/mle/ko.UTF-8/mle.so
x 48c43f4825e38055463dd19e97936c8ded0c1362 usr/lib/mle/ko/libmle.a
x 3f2c9f595330ece870bdc0f8e1b3d813bb1127ac usr/lib/mle/ko/mle.so

ARCH=sparc

x d93d7066035e092ba524cd135e140517ef104947 usr/lib/libkle.a
x 69048e83916db77452a90321be31d0bf0fa041af usr/lib/libkle.so.1
x 108f3c198339621fe46d0db48f0db0380aa6b883 usr/lib/lp/locale/ko.UTF-8/mp/xufallback_ksc.so
x 69958e178041e87461655f2ecb57f2c13929090a usr/lib/lp/locale/ko/mp/xwcksc5601_1987_udc.so
x 5bc41affe5d1ec298306672bb2e1457dab8d954f usr/lib/lp/locale/ko/mp/xwcksc5601_1987_unicode.so
x fbbe7ecdb68b39b1328c55902e0ef2b408cc015c usr/lib/mle/ko.UTF-8/libmle.a
x af03130df5894cf6c0e20e9593bc993719eab0e7 usr/lib/mle/ko.UTF-8/mle.so
x f6fcd46a80d2d40b802b84a167f2aa5a41fac7be usr/lib/mle/ko/libmle.a
x eec1302fcf9afa6d02872b4412e024881778c31d usr/lib/mle/ko/mle.so

# hashes from pkg://opensolaris.org/system/locale/th-extra@0.5.11,5.11-0.134.0.2:20100529T011129Z

ARCH=all

x 8f0d3a968b0c43f3581de513ab33d8f0b1a07871 usr/lib/lp/files/TAC11x10.COD
x 095b3a58ba3dd0a84a3565652e5e01280a16ac9f usr/lib/lp/files/TAC11x11.COD
x c20900d34f7ecaba38c869be2f91ddb0aa293a55 usr/lib/lp/files/TAC11x12.COD
x 04c864b13d4d6a1000a56e8db0019e73a045f999 usr/lib/lp/files/TAC11x13.COD
x 4afcbc16e2b1d5808c8513b6385b7014a70c29f1 usr/lib/lp/files/TAC11x14.COD
x a902bcb8a3ab3543cd9405a5293f6849f44e2924 usr/lib/lp/files/TAC11x15.COD
x 491c8cdca50bfe1e06ec88ce3e1fec874f9d7fb7 usr/lib/lp/files/TAC11x16.COD
x 9b5244a4f4c870075695e17696b0e533a3c27e1a usr/lib/lp/files/TAC11x17.COD
x a7f7bcf7dd3069827d0f444e540218d63dce7424 usr/lib/lp/files/TAC11x18.COD
x 6653ad7808d95b5133e3b7853b175fa1830af4e0 usr/lib/lp/files/TAC11x19.COD
x 6e1ea37f6780ccbd0cfe07bd1340f4a88d14f233 usr/lib/lp/files/TAC11x20.COD
x a90858742a745ed419df490110ef0b8efb01431b usr/lib/lp/files/TAC11x21.COD
x 4694036217903f8bdd16447eabf0e79d7a95a354 usr/lib/lp/files/TAC11x22.COD
x bf9a7ed93ff98d0378e96193aa1b98d68da26a74 usr/lib/lp/files/TAC11x40.COD
x a8a1cf5de5d0a0b289e2b46bdb80cd77fb9169ef usr/lib/lp/files/TAC11x41.COD
x db817f1cdcea92662389b517fa4247a51f47c6e4 usr/lib/lp/files/TAC11x42.COD
x b776a0849979df0ccc33bbb70044f27881e5575e usr/lib/lp/files/TAC11x43.COD
x 0e6242413f2e2b6865e24da331ee43df7d6acb91 usr/lib/lp/files/TAC11xAA.COD

ARCH=i386

x 69b2963705fd28fb4f4b56f4c15005cc870f2244 usr/lib/lp/locale/th_TH/mp/ctlthai.so
x 6f347ef70446f346ee59cf04512256c817689a39 usr/lib/lp/locale/th_TH/thaifilter
x f0414cbd6889deb4d59ed960c3c0427b529aafea usr/lib/locale/th_TH.TIS620/LC_CTYPE/thaidic.dat
#x 3774734f3a4117378afa79709bec759be390734f usr/lib/locale/th_TH/LC_CTYPE/amd64/textboundary.so.1
#x 16a101c778d00f1dd9418e2ffedba513f150e1d6 usr/lib/locale/th_TH/LC_CTYPE/textboundary.so.1
#x 6240abeadb328628bb2ada9afd9873add9a646ab usr/lib/locale/th_TH/LC_CTYPE/amd64/wdresolve.so
#x a0024124fb6fa036d3937d4462a2f8c7f9855301 usr/lib/locale/th_TH/LC_CTYPE/wdresolve.so
x 3774734f3a4117378afa79709bec759be390734f usr/lib/locale/th_TH.TIS620/LC_CTYPE/amd64/textboundary.so.1
x 16a101c778d00f1dd9418e2ffedba513f150e1d6 usr/lib/locale/th_TH.TIS620/LC_CTYPE/textboundary.so.1
x 6240abeadb328628bb2ada9afd9873add9a646ab usr/lib/locale/th_TH.TIS620/LC_CTYPE/amd64/wdresolve.so
x a0024124fb6fa036d3937d4462a2f8c7f9855301 usr/lib/locale/th_TH.TIS620/LC_CTYPE/wdresolve.so
#x d22b548f417f40aa67390ba3f72d240d14490450 usr/lib/locale/th_TH/LO_LTYPE/amd64/th_TH.layout.so.1
#x 00eb3f7c8b7632c28d4c945e54541777d59a047c usr/lib/locale/th_TH/LO_LTYPE/th_TH.layout.so.1
x d22b548f417f40aa67390ba3f72d240d14490450 usr/lib/locale/th_TH.TIS620/LO_LTYPE/amd64/th_TH.TIS620.layout.so.1
x 00eb3f7c8b7632c28d4c945e54541777d59a047c usr/lib/locale/th_TH.TIS620/LO_LTYPE/th_TH.TIS620.layout.so.1

ARCH=sparc

x 09af648e2f905b05cc8f23e9149c44b22088c1c4 usr/lib/lp/locale/th_TH/mp/ctlthai.so
x 7dd63e6e2b44baa33fb5c5810f9a31f79bdf140f usr/lib/lp/locale/th_TH/thaifilter
x 96d658a21654da38b75ee65beec6f8c94d714f91 usr/lib/locale/th_TH.TIS620/LC_CTYPE/thaidic.dat
#x a0e7f470191c0b16e9653fd383ff1e815b8f2c2f usr/lib/locale/th_TH/LC_CTYPE/textboundary.so.1
#x 4a384c8ab4ca5a9f9518c1a0dcecd78860b71085 usr/lib/locale/th_TH/LC_CTYPE/wdresolve.so
x a0e7f470191c0b16e9653fd383ff1e815b8f2c2f usr/lib/locale/th_TH.TIS620/LC_CTYPE/textboundary.so.1
x 4a384c8ab4ca5a9f9518c1a0dcecd78860b71085 usr/lib/locale/th_TH.TIS620/LC_CTYPE/wdresolve.so
#x b714e999f330f27330e06bc8695778ce873732fd usr/lib/locale/th_TH/LO_LTYPE/sparcv9/th_TH.layout.so.1
#x 14e7e1c03f981b148135dc83979567ce56c60a35 usr/lib/locale/th_TH/LO_LTYPE/th_TH.layout.so.1
x b714e999f330f27330e06bc8695778ce873732fd usr/lib/locale/th_TH.TIS620/LO_LTYPE/sparcv9/th_TH.TIS620.layout.so.1
x 14e7e1c03f981b148135dc83979567ce56c60a35 usr/lib/locale/th_TH.TIS620/LO_LTYPE/th_TH.TIS620.layout.so.1

# hashes from pkg://opensolaris.org/system/locale/zh_cn-extra@0.5.11,5.11-0.134.0.2:20100529T011152Z

ARCH=all

x 0d6e19b8aa1cc758edea960a696bdedbc7f86659 usr/include/zh.GBK/xctype.h
x 4979fa561b546f70573fcf21de030ff83a29fdee usr/include/zh/xctype.h

# hash from pkg://opensolaris.org/system/locale/zh_cn@0.5.11,5.11-0.134.0.2:20100529T011149Z

ARCH=all

x 0d6e19b8aa1cc758edea960a696bdedbc7f86659 usr/include/zh.UTF-8/xctype.h

# hashes from pkg://opensolaris.org/system/locale/zh_tw-extra@0.5.11,5.11-0.134.0.2:20100529T011227Z

ARCH=all


x fbdd8e673d3f649140d671f72a948660f59d6713 usr/include/zh_TW/xctype.h

ARCH=i386

x 6acfc3105ea7019c59b367629c5c2c24203926ad usr/kernel/strmod/amd64/b5euc
x 27e68d4894582e708ef5843f5c48048217a693e9 usr/kernel/strmod/amd64/big5euc
x e1cbafd42ff0c30be7da7c735d2fac6f45d99c40 usr/lib/amd64/libhle.a
x 3253035565a7afd8af3aef8c57e01859482bac11 usr/lib/amd64/libhle.so.1

ARCH=sparc

x decb8eac3ea30bbcdd085ea700457d3193f087ee usr/kernel/strmod/sparcv9/b5euc
x c8f0c09ac5c514d1c1d55b4f46b9d426e5a1bd5e usr/kernel/strmod/sparcv9/big5euc
x 0174b9916f3cb9394dd3ffd2d2d012f9eef98978 usr/lib/sparcv9/libhle.a
x e8adf7d82162ad9ba757153cda2bb52ef887d8d5 usr/lib/sparcv9/libhle.so.1








# hashes from pkg://opensolaris.org/system/locale/af@0.5.11,5.11-0.134.0.2:20100529T005915Z

ARCH=all


x 8cb0564d7cef5e3bd5e2f21e6de204372e8c5289 usr/lib/locale/af_ZA.UTF-8/locale_description
x 3a91477464d9e276b4451ddf863ee9cb9e506956 usr/lib/locale/af_ZA.UTF-8/locale_map

ARCH=i386

x 0ed7e7ab5247b58e7b2dd6e688fdc4b7ec411114 usr/lib/locale/af_ZA.UTF-8/af_ZA.UTF-8.so.3
x 45656a9e0cc99e651036905af6d9600faad2ec89 usr/lib/locale/af_ZA.UTF-8/amd64/af_ZA.UTF-8.so.3

ARCH=sparc

x 3732638c5cfa9661a87e7f450b7a7a8f8425d1e5 usr/lib/locale/af_ZA.UTF-8/af_ZA.UTF-8.so.3
x f0c50e6de36f5528389ef1a7dc9779df0847f766 usr/lib/locale/af_ZA.UTF-8/sparcv9/af_ZA.UTF-8.so.3

# hashes from pkg://opensolaris.org/system/locale/ar_eg@0.5.11,5.11-0.134.0.2:20100529T010015Z

ARCH=all

x bd1ae0a081330d9edc8356c02edb5b3f54716baf usr/lib/locale/ar_EG.UTF-8/locale_description
x c2446d80ce6381920e774eae75431a1550ead331 usr/lib/locale/ar_EG.UTF-8/locale_map

ARCH=i386

x 75f78e32eea369a01c4e8a57b4ddc5192f2acf0c usr/lib/locale/ar_EG.UTF-8/amd64/ar_EG.UTF-8.so.3
x aef67be184ce853ce65b60f025b952ba477c4d8b usr/lib/locale/ar_EG.UTF-8/ar_EG.UTF-8.so.3

ARCH=sparc

x c4a69e765e4c32261481a13ccaaf5024c55b458b usr/lib/locale/ar_EG.UTF-8/ar_EG.UTF-8.so.3
x 36ba1440c98d002d809b2ddf30361230ca4a370b usr/lib/locale/ar_EG.UTF-8/sparcv9/ar_EG.UTF-8.so.3

# hashes from pkg://opensolaris.org/system/locale/ar-extra@0.5.11,5.11-0.134.0.2:20100529T010014Z

ARCH=all

x d20b80a045fd55e147c5906437b34b6ae4b2aa62 usr/lib/locale/ar_EG.ISO8859-6/locale_description
x 3cfd925e66f112ee7229d15b0ddf3f2810b1507e usr/lib/locale/ar_EG.ISO8859-6/locale_map

ARCH=i386

x b7ea675c6798869c3cac63899f10b51c4952547d usr/lib/locale/ar_EG.ISO8859-6/amd64/ar_EG.ISO8859-6.so.3
x 3438806ce398eb947236c709a08fde48cbabd9a4 usr/lib/locale/ar_EG.ISO8859-6/ar_EG.ISO8859-6.so.3

ARCH=sparc

x 818c2c9022a7c449ff504bc5671566469c12bb6d usr/lib/locale/ar_EG.ISO8859-6/ar_EG.ISO8859-6.so.3
x 486692e97b846a4a3f747cd3b4c47367904f8625 usr/lib/locale/ar_EG.ISO8859-6/sparcv9/ar_EG.ISO8859-6.so.3

# hashes from pkg://opensolaris.org/system/locale/ar@0.5.11,5.11-0.134.0.2:20100529T005920Z


ARCH=all

x 78f80c76a86065e256dce87e98a89c6ddfe7a530 usr/lib/locale/ar_AE.UTF-8/locale_description
x a7ee8bb58666cf50c6aea373dc8685da2bef8832 usr/lib/locale/ar_AE.UTF-8/locale_map
x a02bd3b18aa004b443685352ab032ff11ae775bb usr/lib/locale/ar_BH.UTF-8/locale_description
x 2cf42c983f77dbb1c3191dc3969dee1b97c37e78 usr/lib/locale/ar_BH.UTF-8/locale_map
x 01817c36c5ac258701b463efd0b558127a06de2b usr/lib/locale/ar_DZ.UTF-8/locale_description
x 261a2e85db472a186da7613134ab4b494ec230aa usr/lib/locale/ar_DZ.UTF-8/locale_map
x 6a422a86a1f569dd95afd5970e31cec86438cee5 usr/lib/locale/ar_IQ.UTF-8/locale_description
x 367bfa4c2d2db870395fde131d146009c14070df usr/lib/locale/ar_IQ.UTF-8/locale_map
x bc2b02b2b122ac38df364c0e1434125d3c242060 usr/lib/locale/ar_JO.UTF-8/locale_description
x c8267714f8b375355cfb16597d46e57b8ee8f335 usr/lib/locale/ar_JO.UTF-8/locale_map
x 8d0c309f68941e145218dee2f9a2b96fe0f4c46a usr/lib/locale/ar_KW.UTF-8/locale_description
x 34a5538b419bf02e48a80e06d342bda909f486ea usr/lib/locale/ar_KW.UTF-8/locale_map
x 7dde77a58af608a374c745567c21a393dc06b0b5 usr/lib/locale/ar_LY.UTF-8/locale_description
x d8ecb2f2a158e8ec763663f2030d22ea42205a07 usr/lib/locale/ar_LY.UTF-8/locale_map
x d9da7bade06118b5deb042d179217fe72578f424 usr/lib/locale/ar_MA.UTF-8/locale_description
x 20e37548202b594e39adc7d715d70d3c6031ddc7 usr/lib/locale/ar_MA.UTF-8/locale_map
x 836354ea82a48b2f336c6e05933ae36d3d1e54b9 usr/lib/locale/ar_OM.UTF-8/locale_description
x 5698aa3744bec156c1253c586b3d5be56681305a usr/lib/locale/ar_OM.UTF-8/locale_map
x 47c46e12686a46f00c6c8e5df901119eb7018e79 usr/lib/locale/ar_QA.UTF-8/locale_description
x 8da51eed40c8279b9c8c2d990e28dc41918e1bfa usr/lib/locale/ar_QA.UTF-8/locale_map
x 2a85d350b5d2b6b0c98fc9de9b5958c6d78045bd usr/lib/locale/ar_SA.UTF-8/locale_description
x 4fad8343a9b3adae631392bd45fdf08a6497d3f5 usr/lib/locale/ar_SA.UTF-8/locale_map
x 403f2c182598e3f6554f37c60e0a5fed2d0e87f4 usr/lib/locale/ar_TN.UTF-8/locale_description
x 61b7501e2e81a6329639cb9e5bcf979fe5769ea6 usr/lib/locale/ar_TN.UTF-8/locale_map
x 47ee2f94cbe91344aa36ca75e3d225b99fb1e530 usr/lib/locale/ar_YE.UTF-8/locale_description
x 25409d120f1ad27576e2bfa4ecc41d3b34622b2e usr/lib/locale/ar_YE.UTF-8/locale_map

ARCH=i386

x 2f1767960eafdda493da4b03b187e0bb4c0029e5 usr/lib/locale/ar_AE.UTF-8/amd64/ar_AE.UTF-8.so.3
x dd15eb5e62d107d11f4ac19ed2ec89830aaa86d8 usr/lib/locale/ar_AE.UTF-8/ar_AE.UTF-8.so.3
x 1387824a8a7a5214e2717a9aeede5469d54817fd usr/lib/locale/ar_BH.UTF-8/amd64/ar_BH.UTF-8.so.3
x c3f903c8a61bab5c8f0a42cebd2cbf84dea2199f usr/lib/locale/ar_BH.UTF-8/ar_BH.UTF-8.so.3
x adbd547efce0522ebdd7d14c3a429344b2e521a6 usr/lib/locale/ar_DZ.UTF-8/amd64/ar_DZ.UTF-8.so.3
x 7f1c6fca524e0de6acb4b4c73fcc2ea5bfd8bc49 usr/lib/locale/ar_DZ.UTF-8/ar_DZ.UTF-8.so.3
x e644b068d5cf21821af9b3f49fbe0e8fdeb25c20 usr/lib/locale/ar_IQ.UTF-8/amd64/ar_IQ.UTF-8.so.3
x 1dc646c45bcedac4a9aa05abf8c03094c88cbfec usr/lib/locale/ar_IQ.UTF-8/ar_IQ.UTF-8.so.3
x 4333a51631a0806986cd72270d7f99bc63286abd usr/lib/locale/ar_JO.UTF-8/amd64/ar_JO.UTF-8.so.3
x 33076b50cdf8bd1c6432cdd837292b31aaa5e676 usr/lib/locale/ar_JO.UTF-8/ar_JO.UTF-8.so.3
x 3159235de3423e0de81e376bab175fb3b47d93b6 usr/lib/locale/ar_KW.UTF-8/amd64/ar_KW.UTF-8.so.3
x 7ec72ed81bbafb8334e4a9691c1a0be1f30fad18 usr/lib/locale/ar_KW.UTF-8/ar_KW.UTF-8.so.3
x 106195360a965546074af61037c55b8573014da5 usr/lib/locale/ar_LY.UTF-8/amd64/ar_LY.UTF-8.so.3
x dbaebe4309c4e05e0f67f872e14df362b8ddb6a5 usr/lib/locale/ar_LY.UTF-8/ar_LY.UTF-8.so.3
x 580405f6189c870b5a7043bc5dea991f4cd8ffab usr/lib/locale/ar_MA.UTF-8/amd64/ar_MA.UTF-8.so.3
x 0ec0eee49af37cda8e9c1546430fbd6a4253c751 usr/lib/locale/ar_MA.UTF-8/ar_MA.UTF-8.so.3
x feeb9f709abd1a5891c4f548d51c13aec1bf9b54 usr/lib/locale/ar_OM.UTF-8/amd64/ar_OM.UTF-8.so.3
x b4aa6c5b8b879ab21f331795b64812190a86e4b6 usr/lib/locale/ar_OM.UTF-8/ar_OM.UTF-8.so.3
x b466572fd3cfe9596a4dd80419f09cf0c08f5cf1 usr/lib/locale/ar_QA.UTF-8/amd64/ar_QA.UTF-8.so.3
x f455ed16829781468cbe616300e9a677a7b1cc13 usr/lib/locale/ar_QA.UTF-8/ar_QA.UTF-8.so.3
x 2eb2b282e53f2e196aedcd8fc27dbf337b45b9f3 usr/lib/locale/ar_SA.UTF-8/amd64/ar_SA.UTF-8.so.3
x 176dd0d3410afe4a0ac7e883a6898a5be9358ee0 usr/lib/locale/ar_SA.UTF-8/ar_SA.UTF-8.so.3
x 5978197164382d1ec6be722ffb74de659398c146 usr/lib/locale/ar_TN.UTF-8/amd64/ar_TN.UTF-8.so.3
x 46ea82184213b83a498241e3ba4a25e586179115 usr/lib/locale/ar_TN.UTF-8/ar_TN.UTF-8.so.3
x f70e82290e4391ce7be24d0ab1297ec1965b259b usr/lib/locale/ar_YE.UTF-8/amd64/ar_YE.UTF-8.so.3
x b91333fde734f51598b871df8187610e114d1762 usr/lib/locale/ar_YE.UTF-8/ar_YE.UTF-8.so.3

ARCH=sparc

x 68deca7c3d05a46cea8921f4cc3a9dec761eb097 usr/lib/locale/ar_AE.UTF-8/ar_AE.UTF-8.so.3
x de729d5410fcb08c9a20be797b4853d70214f42d usr/lib/locale/ar_AE.UTF-8/sparcv9/ar_AE.UTF-8.so.3
x cd8b84f12f8fab8ed3ae66e8c8a6db76015448ec usr/lib/locale/ar_BH.UTF-8/ar_BH.UTF-8.so.3
x 3be4db5e43e0921faa92a4e8bd909dbd33fb20bf usr/lib/locale/ar_BH.UTF-8/sparcv9/ar_BH.UTF-8.so.3
x 3fb44c4fd59b2bff1c927368a860fe2b98761fc9 usr/lib/locale/ar_DZ.UTF-8/ar_DZ.UTF-8.so.3
x 56671a1d8a02c94ee614a5b3834f70f7019eae8a usr/lib/locale/ar_DZ.UTF-8/sparcv9/ar_DZ.UTF-8.so.3
x 7054493bdff09620f6cd2fdc246d5bc6059ce967 usr/lib/locale/ar_IQ.UTF-8/ar_IQ.UTF-8.so.3
x 1f806e44ff3fdbac9a632f98a099709022cecb9f usr/lib/locale/ar_IQ.UTF-8/sparcv9/ar_IQ.UTF-8.so.3
x e9fd986ec00cf85d0b1a4f366f9a0b2ebeba6d43 usr/lib/locale/ar_JO.UTF-8/ar_JO.UTF-8.so.3
x bbcad7d3f5474fb00fe9c4f5d1fe45b09a240d67 usr/lib/locale/ar_JO.UTF-8/sparcv9/ar_JO.UTF-8.so.3
x eb4a08ecbc098a12b0cbbd5ddc1a36190122e658 usr/lib/locale/ar_KW.UTF-8/ar_KW.UTF-8.so.3
x 572c1a6a820c722d5ec2771da7c1bb8187ca1085 usr/lib/locale/ar_KW.UTF-8/sparcv9/ar_KW.UTF-8.so.3
x 15a94544eb9936fa90e032946751c3ef96650c2d usr/lib/locale/ar_LY.UTF-8/ar_LY.UTF-8.so.3
x 90efcd32ca63e0fa543da5a4183a54791b67fb4e usr/lib/locale/ar_LY.UTF-8/sparcv9/ar_LY.UTF-8.so.3
x ff5e1e8d935aab242b6798a2840ad1b6edccba8e usr/lib/locale/ar_MA.UTF-8/ar_MA.UTF-8.so.3
x dbe7a17ec86ef8bea7828b5631ce01991a94e6a6 usr/lib/locale/ar_MA.UTF-8/sparcv9/ar_MA.UTF-8.so.3
x 28597057a4dad591583d1240a85cebee60de6462 usr/lib/locale/ar_OM.UTF-8/ar_OM.UTF-8.so.3
x 79c035d9ddbf8b82f7f5810d5ece959a40378db2 usr/lib/locale/ar_OM.UTF-8/sparcv9/ar_OM.UTF-8.so.3
x dad017c667187bc9f1f29bf083462ac13a232126 usr/lib/locale/ar_QA.UTF-8/ar_QA.UTF-8.so.3
x c11e50e8f425e81c5e877e5167a75dcf164ebc5c usr/lib/locale/ar_QA.UTF-8/sparcv9/ar_QA.UTF-8.so.3
x 2417886587745d434783af55db3cca0e3b88d9a3 usr/lib/locale/ar_SA.UTF-8/ar_SA.UTF-8.so.3
x d65f34ed9a02acc8cb644609d8346d6ad19c920d usr/lib/locale/ar_SA.UTF-8/sparcv9/ar_SA.UTF-8.so.3
x 335a1579cb1fd35bb7c9576a0456f72467bac3e5 usr/lib/locale/ar_TN.UTF-8/ar_TN.UTF-8.so.3
x 487c21a97d4d96da081273bccd79a7148b72a25b usr/lib/locale/ar_TN.UTF-8/sparcv9/ar_TN.UTF-8.so.3
x 866a8a50fac884d641056629c7434e333edb7f89 usr/lib/locale/ar_YE.UTF-8/ar_YE.UTF-8.so.3
x a5bb39400603e563d7c25bdc25ad2e85d9c2d70f usr/lib/locale/ar_YE.UTF-8/sparcv9/ar_YE.UTF-8.so.3

# hashes from pkg://opensolaris.org/system/locale/as@0.5.11,5.11-0.134.0.2:20100529T010020Z

ARCH=all

x 5a412653f376eeceaa8a32e3b9a45d3d082db469 usr/lib/locale/as_IN.UTF-8/locale_description
x 802befca3a0c03df880245035fc316c066ef14cb usr/lib/locale/as_IN.UTF-8/locale_map

ARCH=i386

x 8cdc08438525774c842e78c5bbf347ca076ffc59 usr/lib/locale/as_IN.UTF-8/amd64/as_IN.UTF-8.so.3
x 758a5e8fc76bd4d362d76221c332cd7b32df9dae usr/lib/locale/as_IN.UTF-8/as_IN.UTF-8.so.3

ARCH=sparc

x 85044b9ff952545b0f3c9b8079281d67d33054e3 usr/lib/locale/as_IN.UTF-8/as_IN.UTF-8.so.3
x 129ef5aa66390f0feac5c3bdb081aee74ea2598a usr/lib/locale/as_IN.UTF-8/sparcv9/as_IN.UTF-8.so.3

# hashes from http://pkg.opensolaris.org/release/manifest/0/system%2Flocale%2Faz%400.5.11%2C5.11-0.134.0.2%3A20100529T010025Z

ARCH=all

x af62328262cf78f71124030fdbac86207c5d2d09 usr/lib/locale/az_AZ.UTF-8/locale_description
x 06d20ad3271b1a856482356dafbaae52133d89dd usr/lib/locale/az_AZ.UTF-8/locale_map

ARCH=i386

x 19231b9843576e51a2d828017dfa2e644683419d usr/lib/locale/az_AZ.UTF-8/amd64/az_AZ.UTF-8.so.3
x de65d7be4571f16d0763bf4b94ed65dd2a2d0160 usr/lib/locale/az_AZ.UTF-8/az_AZ.UTF-8.so.3

ARCH=sparc

x ee6e99e0ed70f2440ffa3d96748f11ca06337f37 usr/lib/locale/az_AZ.UTF-8/az_AZ.UTF-8.so.3
x 008a49ad03956f4da285c8b94efc8291166e2563 usr/lib/locale/az_AZ.UTF-8/sparcv9/az_AZ.UTF-8.so.3
