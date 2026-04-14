# consider different RLE encoding strategies!
$count= $nenc= 0;
while (<>) {
      while(/([0-9a-f]+)\+(\d+)/ig) {
          my ($c, $n)= ($1, $2);
          $count++;
          print sprintf("%02x+%4d\n", $c, $n);


          ++$nenc; # below may "reuse" this byte

          $s= 10;
          if ($s==0) {      # (1 314 480)
              # 2 bytes say repeat last N times more
              $nenc+= 2;
          } elsif ($s==1) { # (1 047 595)
              # dedicated 0x20 SAME char
              if ($c eq "20") {
                  $nenc+= 1;
              } else {
                  $nenc+= 2;
              }
          } elsif ($s==2) { # (691 689)
              # dedicated 0x20 SAME save 1 byte
              # and 2x16 for 2..18 saves much more!
              if ($n <= 16+2) {
                  if ($c eq "20") {
                      $nenc+= 0; # !
                  } else {
                      $nenc+= 1;
                  }
              } else {
                  # encodes max 16*256 = 4096, so many need two
                  while ($n>0) {
                      if ($c eq "20") {
                          $nenc+= 1;
                      } else {
                          $nenc+= 2;
                      }
                      $n-= 16*256; # max, lol
                  }
              }
          } elsif ($s==3) { # (678 392)
              # allow more savings for 0x20
              if ($c eq "20" && $n<=32+2) {
                  $nenc+= 0;
              } else {
                  # encodes max 16*256 = 4096, so many need two
                  while ($n>0) {
                      if ($n <= 16+2) {
                          $nenc+= 1;
                      } else {
                          $nenc+= 2;
                      }
                      $n-= 16*256; # max, lol
                  }
              }
          } elsif ($s==4) { # (907 580)
              # simpliest
              if ($n<=32+2) {
                  $nenc+= 1;
              } else {
                  $nenc+= 2;
              }
          } elsif ($s==5) { # (942 668)
              while ($n>0) {
                  if ($n < 2) {
                      $nenc+= $n;
                  } elsif ($n <= 64+1) {
                      $nenc+= 1; 
                  } else {
                      $nenc+= 1;
                  }
                  $n-= 64+1;
              }
          } elsif ($s==6) { # (892 928)
              #(* 64 64) = 4096
              if ($n < 64+1) { # using 63 values
                  $nenc+= 1;
              } else { # 64th says 2 bytes coming
                  $nenc+= 3;
              }
          } elsif ($s==7) { # (886 248)
              if ($n < 64+1-16) { # 48 values say repeat
                  $nenc+= 1;
              } else { # using 16 x 256
                  $nenc+= 2;
              }
          } elsif ($s==8) { # (657 566)
              # 32 47		= 32 47
              # 33 47		= 32 32 47
              # 34 47		= 32 32 32 47
              # ...
              # 62 47		= 32 x #32 47
              # AA BB           = 32 x # (* 64 32)= 2048
              # 32 32 47        = 32 
              # 33 32           = 
              # 32 33

              # 32..63 ...		= 32 x #1--32 (1 byte)
              # 32..63 32..63   	= 1..32 + 0..31*32 = 1024 (2 bytes)
              # 32..63... 32..63 + 128 	= 1..32*1024.... = 32K! (in 3 bytes)

              # cc 128+32..63           = cc x #
              if ($c eq "20") {
                  if ($n<31) { # 31 values 32#1 33#2 ... 32+30#31
                      $nenc+= 0;
                  } else { # if followed by repeat byte... (* 32 64)
                      $nenc+= 1;
                  }
              } elsif ($n < 32+2-1) { # 31 values
                  $nenc+= 1;
              } else { # 
                  $nenc+= 2;
              }
          } elsif ($s==9) { # (665 843)
              if ($c eq "20") {
                  $nenc+= 0;
              } else {
                  $nenc+= 1;
              }
              $n-= ($n & 31)+1;
              # basically 5 bits!
              # (for skip could do 6 bits..., but...)
              while($n>0) {
                  $nenc+= 1;
                  $n= $n>>5;
              }
          } elsif ($s==10) { # (615 868)
              # 96 chars RLE encoding...
              # not using 0-7, 16-23, 64-127 => 32+8+8= 48 ... x 2 (hibit) => 96
              if ($n<96-16) {
                  if ($c eq "20") {
                      $nenc+= 0;
                  } else {
                      $nenc+= 1;
                  }
              } else {
                  $nenc+= 1; # abs address? or skip#
              }

          } else {
              exit(3);
          }
      }
}
print "TOTAL($s) repeats=$count bytes=$nenc\n";
