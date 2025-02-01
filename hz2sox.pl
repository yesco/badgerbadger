while(<>) {
    $n= 0;
    while (/(\d+)v\s*(\d+)Hz/g) {
        my ($v, $f)= ($1, $2);
        # TODO: because skip, need to prolong last commmand if no tone generated at this beat?
        next if $f<100;
        print "synth " if !$n;
        #print "\t0.032 sin $f";
        print "\t0.02 sin $f";
        $n++;
        last if $n==3;
    }
    print "\n" if $n;
}
