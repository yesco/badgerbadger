@lastf= (0,0,0);

while(<>) {
    $abc= ""; $vol= "";
    $n= 0;
    while (/(\d+)v\s*(\d+)Hz/g) {
        @newf= (0,0,0);
        my ($v, $f)= ($1, $2);
        # TODO: because skip, need to prolong last commmand if no tone generated at this beat?
        next if $f<100;

        if ($f==$lastf[0]) {
          $newf[0]= $f;
        } elsif ($f==$lastf[1]) {
          $newf[1]= $f;
        } elsif ($f==$lastf[1]) {
          $newf[2]= $f;
        } else {
          my $x= 0;
          while($x<3) {
            if (!$newf[$x]) { $newf[$x]= $f; last; }
            $x++;
          }
          if ($x>=3) { print "can't allocate counter!\n$_\n"; exit(1); }
        }

        $vol=$abc="" if !$n;
        $p= 1000000/16/$f;
        $abc.= sprintf("\\x%02x\\x%02x", $p&255, $p/256);

        # static ymu16 ymVolumeTable[16] =
        # {62,161,265,377,580,774,1155,1575,2260,3088,4570,6233,9330,13187,21220,32767};
        # $v seems we have values 0...127?
        $v= int($v/16); # lol
        $v= 15 if $v>15;
        # TODO: lol
        $v= 15;

        $newf[$n]= $f;
        $newv[$n]= $v;

        $vol.= sprintf("\\x%02x", $v);
        $n++;
        last if $n==3;
    }
    if ($n) {
        $abc= "";
        $p= !$newf[0]? 0xffff: 1000000/16/$newf[0]; $abc.= sprintf("\\x%02x\\x%02x", $p&255, $p/256);
        $p= !$newf[1]? 0xffff: 1000000/16/$newf[1]; $abc.= sprintf("\\x%02x\\x%02x", $p&255, $p/256);
        $p= !$newf[2]? 0xffff: 1000000/16/$newf[2]; $abc.= sprintf("\\x%02x\\x%02x", $p&255, $p/256);

        # TODO: vol
        $vol= "";
        $vol.= sprintf("\\x%02x", $newf[0]? 15: 0);
        $vol.= sprintf("\\x%02x", $newf[1]? 15: 0);
        $vol.= sprintf("\\x%02x", $newf[2]? 15: 0);

        # 0-31 NNNcba     (Mod)vvvv   T      0-15
        #  0   1   2   3   4   5   6   7   8   9  10  11  12  13
        # Al  Ah  Bl  Bh  Cl  Ch  N4  Ch  Va  Vb  Vc Env-freq ENV
        #  x   x   x   x   x   x   

        # music gen:
        # "\x9f\x00 \xe0\x01 \x77\x00  \x00\xf8 \x10\x0e\x0d \x42\x0f\xff"//  1 57  4  2

        #        0-5     6    7 8-10  11   12   13
        print "\"$abc\\x00\\xf8$vol\\x42\\x0f\\xff\"\n";
        # reg 13 $ff is instruction not to write as it resets counters/wave-forms
    }
}
