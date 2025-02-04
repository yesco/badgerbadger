#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define FULL

#ifdef FULL
  #define W 240
  #define H 200
#else
  #define W 120
  #define H 100
#endif

int main(int argc, char** argv) {
  assert(argc==2);

  char* name= argv[1];
  FILE* f= fopen(name, "r+");
  int i= 0;

  char rgb[3*W*H];
  char last[sizeof(rgb)]= {0};

  char head[100]={0};

  int trle= 0;

  // very fixed, 240x200 lol
  fread(head, 15, 1, f);
  if (sizeof(rgb)!=fread(rgb, 1, sizeof(rgb), f)) {
    printf("%% Error: not full screen?\n"); exit(1);
  }

  // single bit rgb
  for(int r=0; r<H; ++r) {
    int p= 0, c= 0;
    for(int j=0; j<W; ++j) {
      int at= 3*(r*W+j);
      rgb[at+0]= rgb[at+0]>127? 255: 1;
      rgb[at+1]= rgb[at+1]>127? 255: 1;
      rgb[at+2]= rgb[at+2]>127? 255: 1;
    }
  }

  fpos_t pos= 15;
  fsetpos(f, &pos);
  int wr= fwrite(rgb, 1, sizeof(rgb), f);
  assert(wr==sizeof(rgb));
}



