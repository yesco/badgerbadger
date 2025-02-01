@foo= (726, 23, 117, 2226, 750, 11, 738, 2238);
print "o:@foo\n";
@res= sort { $a <=> $b } @foo;
print "q:@res\n";

$lastfreq= 4711;
%freq= ();

@urk= ();
while(<>) {
    if (/^(\d+)\.\d+\s+(\d+)\./) {
        my ($f, $a) = ($1, $2);

        # new dump?
        if ($f<$lastfreq) {

            # perl sort broken? - WTF?
            #@a= keys %freq;
            #print "@x\n";
            #@b= sort { $b <=> $a } @a;
            #print "b:@b\n";

            # WTF? doesn't sort correctly!
            if (0) {
            @res= sort { $b <=> $a } keys %freq;
            #@res= sort { $freq{$b} <=> $freq{$a} } keys %freq;
            print "r:@res\n";
            foreach $k (@res) {
                printf("---%5d %4d\n", $k, $freq{$k});
            }
            }

            @res= reverse sort @urk;
            print @res[0 .. 5], "\n";
            #print (reverse sort @urk)[0 .. 4];
            print "\n[H[2J[3J\n";

            %freq= ();
            @urk= ();
        }
        $lastfreq= $f;

        # too small
        next if !$a;
        next if $a<1024;

        # scale 0-128? lol, actually should be 2^31/1024/4/2 = [0, 262144]....
        $x= $a/1024/4/2;
        next if $x<1;

        next if !$f;
        # ok, got something
        $freq{$f}= 0+$x;
        #print sprintf("%5d %4d %8d\n", $f, $x, $a);
        push(@urk, sprintf("%3dv%4dHz\t", $x, $f));
    }
}
