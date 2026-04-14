// BadgerBadger
//
// This is a try to create a BadgerBadgerBadger app that
// plays the famous meme, including music, mushroom, and snake!
//
// This may not be possible, as we need to play (highly repetetive)
// video, as well as sampled music. We barely have 37KB for program
// and data!

      
#define TEXT ((char*)0xBB80)
#define TSIZE (28*40)

#define HIRES ((char*)0xA000)
#define HSIZE (200*40)

#define TEXTMODE  26 // and 24-39 (2x 60 Hz, 2x 50 Hz)
#define HIRESMODE 30 // and 28-31 (2x 60 Hz, 2x 50 Hz)

// poor man's hires mode
void hires() {
  *(TEXT+TSIZE-1)= HIRESMODE;
//  memset(HIRES, 64, HSIZE);
}

#include <string.h>

 // dummys
 int T, nil, print, doapply1;

void waitms(long w) {
  w <<= 3;
  while(--w>=0);
}

#include <stdio.h>

#include "../simple6502js/65lisp/conio-raw.c"
#include "../simple6502js/65lisp/compress.c"

#include "Data/frames.c"

//struct { unsigned int size; char* z; } frames[]={ {7,NULL},{7,NULL},{7,NULL},};

// TODO: 8000 makes the file not start... no warning
char scr[8000]= {64};

void main() {
  int i= 0, n= sizeof(frames)/sizeof(*frames), ln;

  //hires();

  clrscr();
  gotoxy(0, 25);
  printf("Hello badger... %d %d\n", i, n);
  while(i<n) {
    Compressed *z= (Compressed*)(frames[i].z);
    //gotoxy(0, 25);
    printf("Badger Badger %4u->%4u %d/%d \n", z->len, z->origlen, i, n);

    puts("dez...");
    decompress(z, scr);
    puts("unrle...");
    ln= unRLE(scr, z->len, z->origlen);
    printf(" --> %d\n", ln);
    //while(ln) {
    //--ln;
    //}

    cgetc();
    ++i;
  }
}
