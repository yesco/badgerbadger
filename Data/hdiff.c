// diff of sequence of hires screens

#include <stdio.h>

int main() {
  FILE *f;
  char name[64]= {0};
  int i= 0;

  char scr[8000]= {0};
  char last[8000]= {0};

  int n;
  char c;

  int lastc, lasti;
  int ndiff, nenc, tenc= 0;

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
    c= 0; n= 0; lastc= -1; lasti= 0; ndiff= 0; nenc= 0;
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
    tenc+= nenc;
    printf("==> ndiff=%4d nenc=%4d tenc=%6d\n", ndiff, nenc, tenc);
    
  } while(1);

  printf("%% No such file: %s\n", name);
}
