// diff of sequence of hires screens

#include <stdio.h>

// last number of bytes diff during last rle
int ndiff;

int rle(char* last, char* scr) {
  int n= 0;
  char c= 0;

  int lastc= -1, lasti= 0;
  int nenc= 0;

  ndiff= 0;

  for(int i=0; i<8000; ++i) {
    c= scr[i];

    // collapse non-change to 32 ("unused graphics codes" 32--63, 128+32--63)
    if (c==last[i]) c= 32; else ++ndiff;

    if (c==lastc) ++n;
    else {
      // differ - report
      if (n>1) printf("+%d", n),nenc+=(lastc==32)?1:2; // use hibit to mean skip!
      if (lasti!=i-n-1) printf("\n%04X: ", lasti);

      if (i%40==0) printf("\n");

      printf(" %02x", c);
      ++nenc;
      n= 0; lasti= i; lastc= c;
    }

    lastc= c;
  }
  // - report
  if (n) {
    if (c==32) --nenc;
    else printf("+%d", n),nenc+=2;
  }
  printf("\n");

  return nenc;
}

int main() {
  FILE *f;
  char name[64]= {0};
  int i= 0;

  char scr[8000]= {0};
  char last[8000]= {0};
  char empty[8000]= {0};

  int tfull= 0, trle= 0;

  do {
    sprintf(name, "oneps-%05d.tap", ++i);
    if (!(f= fopen(name, "r+"))) break;

    printf("\n\n--- %s\n", name);
    memcpy(last, scr, sizeof(last));

    fseek(f, -8000, SEEK_END);
    if (sizeof(scr)!=fread(scr, 1, sizeof(scr), f)) {
      printf("%% Error: not full screen?\n"); break;
    }

    // diff
    int nfull= rle(empty, scr);
    int nrle= rle(last, scr);
    trle+= nrle;
    tfull+= nfull;

    // TODO: md5! maybe some frames repeat? save? or save full for decompress?
    printf("%4d ==> nfull=%4d ndiff=%4d nrle=%4d trle=%6d\n", i, nfull, ndiff, nrle, trle);
    
  } while(1);

  printf("%% No such file: %s\n", name);

  printf("\n\n==> files=%d tfull=%6d trle=%6d\n", i, tfull, trle);
}
