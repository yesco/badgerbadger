while(<>) {
    $n= 0;
    while (/(\d+)v\s*(\d+)Hz/g) {
        my ($v, $f)= ($1, $2);
        # TODO: because skip, need to prolong last commmand if no tone generated at this beat?
        next if $f<100;
#        print "synth 0.2" if !$n; # clear tones but too slow
        print "synth 0.02" if !$n;
        #print "\t0.032 sin $f";
	#        print "\t0.02 sin $f";
	print " sin $f";
	# continue at phase - nodiff
#	print " sin +$f";
        $n++;
        last if $n==3;
    }

    if ($n) {
	# doesn't help really - muddles
	print " fade q 0.001 0.02 0.0003";
	print "\n";
    }
}
