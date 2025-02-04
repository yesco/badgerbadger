// diff of sequence of hires screens

#include <stdio.h>

int cgetc() { return 3; }
void gotoxy(char x, char y) { }
#define HIRESMODE 30
char curmode=HIRESMODE;
#define STATUS

#include "../../simple6502js/65lisp/compress.c"

// random useless hash-function
unsigned long hash(char* s, int len) {
  long h = 5381;
  int c;

  while(len-->0) {
    c= *s++;
    h= ((h << 5) + h) + c;
  }

  return h;
}

// last number of bytes diff during last rle
int ndiff;

int rle(char* last, char* scr, char print, char* modify) {
  int n= 0;
  char c= 0;

  int lastc= -1, lasti= 0;
  int nenc= 0;

  ndiff= 0;

  for(int i=0; i<8000; ++i) {
    c= scr[i];

    // collapse non-change to 32 ("unused graphics codes" 32--63, 128+32--63)
    if (c==last[i]) c= 32; else ++ndiff;

    if (modify) modify[i]= c;

    if (c==lastc) ++n;
    else {
      // RLE - encoding "estimate"
      // differ - report
      if (n>1) { nenc+=(lastc==32)?1:2; if (print) printf("+%d", n); } // use hibit to mean skip!
      else { int j= n; while(j-->0) { ++nenc; if (print) printf(" %02x", lastc); }} // print below threshold
      // print address just for debugging
      if (lasti!=i-n-1) { ++nenc; if (print) printf("\n%04X: ", lasti); }

      // newline for new 40byte line, just for debugging
      if (i%40==0 && print) printf("\n");

      // print current different character
      if (print) printf(" %02x", c);
      ++nenc;
      n= 0; lasti= i; lastc= c;
    }

    lastc= c;
  }
  // - report
  if (n) {
    if (c==32) --nenc;
    else { nenc+=2; if (print) printf("+%d", n); }
  }
  if (print) printf("\n");

  return nenc;
}

// TODO: 0 diff => 1 byte?
// TODO: encoding too big, 6 diff => 28
//                         1 diff =>  7 ???? should be max 5? maybe 

// This one outputs every unique followed by repeat a .... 31^n bits...., if n>1
int hiresRLE(char* last, char* scr, char print, char* modify) {
  int n= 0;
  char c= 0;

  int lastc= -1, lasti= 0;
  int nenc= 0;

  ndiff= 0;

  int i= 0;
  while(1) {
    c= scr[i];

    // collapse non-change to 32 ("unused graphics codes" 32--63, 128+32--63)
    if (c==last[i]) c= 32; else ++ndiff;
    ++i;

    if (modify) modify[nenc]= c;

    if (c==lastc && i<8000) {
      ++n;
    } else {
      // differ/eof - report

      // RLE - encoding 
      // (last no need skip!)
      if (lastc==32 && i>=8000) { break; }

      if (n>1) {
        // encode RLE: 33-63#31... (lo...hi)
        if (modify) modify[++nenc]= 32+1+(n%31); else nenc++;
        n-= n/31; // TODO: not optimal... lol
        while(n>0) {
          if (modify) modify[++nenc]= 32+1+(n%31); else nenc++;
          n/= 31;
        }
        //if (print) printf("+%d", n);
      }

      nenc++;
      if (i>=8000) { break; }

      // print address just for debugging
      //if (lasti!=i-n-1) { ++nenc; if (print) printf("\n%04X: ", lasti); }

      // newline for new 40byte line, just for debugging
      ///if (i%40==0 && print) printf("\n");

      // print current different character
      //if (print) printf(" %02x", c);
      n= 1; lasti= i; lastc= c;
    }

    lastc= c;
  }

  //if (print) printf("\n");
  
  return nenc;
}

int main() {
  FILE *f;
  char name[64]= {0};
  int i= 0;

  char scr[8000]= {0};
  char last[8000]= {0};
  char empty[8000]= {0};

  int tfull= 0, trle= 0, tnz=0, tnzrle= 0, tnzRLE= 0;

  do {
    sprintf(name, "oneps-%05d.tap", ++i);
    if (!(f= fopen(name, "r+"))) break;

    printf("\n\n--- %s\n", name);
    memcpy(last, scr, sizeof(last));

    fseek(f, -8000, SEEK_END);
    if (sizeof(scr)!=fread(scr, 1, sizeof(scr), f)) {
      printf("%% Error: not full screen?\n"); break;
    }

    long h= hash(scr, sizeof(scr));

    // diff
    int nfull= hiresRLE(empty, scr, 0, NULL);
    char nw[sizeof(last)]; // TODO: maybe not enough?
    int nrle= hiresRLE(last, scr, 1, nw);
    trle+= nrle;
    tfull+= nfull;

    Compressed* z= compress(scr, sizeof(scr));
    int nz= z->len;
    free(z);
    tnz+= nz;


    char rrr[sizeof(nw)];
    memcpy(rrr, nw, nrle);
    //int nRLE= RLE(rrr, nrle);

    Compressed* zrle= compress(rrr, nrle);
    int nzRLE= zrle->len;
    free(zrle);
    tnzRLE+= nzRLE;

    // TODO: make a copy with 
    // TODO: filepak()

    //printf("\n  :: %4d ==> nfull=%4d ndiff=%4d nrle=%4d trle=%6d nz=%4d tnz=%6d nRLE=%4d nzRLE=%4d tnzRLE=%6d\n",
    //i,            nfull,    ndiff,   nrle,    trle,  nz,     tnz,     nRLE,     nzRLE,     tnzRLE);
    printf("\n  %016lx :: %4d ==> nfull=%4d ndiff=%4d nrle=%4d trle=%6d nz=%4d tnz=%6d nzRLE=%4d tnzRLE=%6d %s\n",
           h,
                   i,            nfull,    ndiff,   nrle,    trle,  nz,     tnz,      nzRLE,     tnzRLE,
           nz<nzRLE? "nc<nzRLE!": ""
           );
    
  } while(1);

  printf("%% No such file: %s\n", name);

  printf("\n\n==> files=%d tfull=%6d trle=%6d\n", i, tfull, trle);
}
