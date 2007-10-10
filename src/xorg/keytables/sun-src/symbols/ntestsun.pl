#!/usr/bin/perl

if ($#ARGV<0 ){
        for(`ls /usr/lib/X11/X11/xkb/symbols/sun`){
                chomp;
		next if $_ eq "README";
                unless(-d "/usr/lib/X11/X11/xkb/symbols/sun/$_"){
                        print "$_:\n";
                        system("perl $0 /usr/lib/X11/X11/xkb/symbols/sun/$_");
                }
        }
}

$file=$ARGV[0];
open INPUT,"<$file" or die "Cannot open $file for reading. $!\n";

@errors=`/usr/X11/lib/X11/xkb/xkbcomp $file 2>&1`;
$realerrs=0;
for(@errors){
        $realerrs++ if /Error/;
}
if($realerrs==0){
        print "No errors. File is ok.\n";
        exit 0;
}

@file=<INPUT>;
close INPUT;

#check for missing semicolons. this is somewhat limited. the semicolons HAVE to come immedietly after the }
@missingsemicolonerrors=`grep -n '}' $file|grep -v ';'`;
if($#missingsemicolonerrors>=0){
        for(@missingsemicolonerrors){
                print "Missing semicolon: Line $_";
        }
}
#format the file so we can use regexps.
sub makeoneline{
        $onelinefile="";
        for(@file){
                chomp;
                $onelinefile.=$_;
        }
        return $onelinefile;
}

sub finderror{
        my $error=$_[0];
        my $msg=$_[1];
        $error=~s/([\[\]\{\}\(\)\$\^])/\$1/g;
        for($i=0;$i<=$#file and $file[$i]!~/$error/;$i++){}
        if($file[$i]=~/$error/){
                print "$msg: Line $i: $file[$i]";
        }
}

$onelinefile=&makeoneline();
while($onelinefile=~s/(\[[^\]]+[\{\}][^\]]+\])//){
        #replace a {} inside [] with nothing. There should never be a {} within []
        &finderror($1,"} or { inside []");
}
$onelinefile=&makeoneline();
while($onelinefile=~s/(\[[^\]]+\[)//){
        #all occurences of [ without a matching ]
        &finderror($1,"[ without matching ]");
}
$onelinefile=&makeoneline();
while($onelinefile=~s/(\{[^\}]+\{)//){
        #all occurences of { without a matching }
        &finderror($1,"{ without matching }");
}
$onelinefile=&makeoneline();
while($onelinefile=~s/(,\s*[\[\]])//){
        #all occurences of a [ or ] following a , (too many ,s)
        &finderror($1,"Too many commas");
}
$onelinefile=&makeoneline();
while($onelinefile=~s/(,\s*,)//){
        #all occurences of 2 ,s (too many)
        &finderror($1,"Too many commas");
}

$i=0;
for(@file){
        if(/"/ and not /".*"/){
                #check any lines that have " have a pair of them.
                print "Unmatched \": Line $i: $file[$i]";
        }
        $i++;
}


#check for sundeadkey info
$onelinefile=&makeoneline();
unless($onelinefile=~/sundeadkey/i){
        print "Error: No SunDeadKey info defined\n";
}

#check the last scanned symbol for a valid keyword. has to be done last.
@keywords=qw/\/\/ Name NoSymbol SetMods [ default degree hidden include key modifier_map name notakeyword override partial replace symbols type virtualMods virtual_modifiers xkb_symbols/;
$i=0;
for(;$i<=$#errors and $errors[$i]!~/last scanned/;$i++){}

sub notakeyword{
        my $word=$_[0];
        for(@keywords){
                if($word eq $_){
                        return 0;
                }
        }
        return 1;
}

$errors[$i]=~/symbol is: (.*)/;
$problem=$1;
if($problem eq ""){
        $i++;
        print "Unknown problem on line $i\n";
}
elsif(&notakeyword($problem)){
        $i++;
        print "$problem found when keyword expected: Line $i: $file[$i]\n";
}

