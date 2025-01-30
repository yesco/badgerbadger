#include <stdio.h>

// full    - SUMMARY: frames= 1828 trlse=  1793 KB   1004 B/frame
//  black01  SUMMARY: frames= 1828 trlse=  1793 KB   1004 B/frame
// -r 12     SUMMARY: frames=  879 trlse=  1083 KB   1262 B/frame
// -r  6     SUMMARY: frames=  441 trlse=   717 KB   1665 B/frame

// quarter - SUMMARY: frames= 1828 trlse=   575 KB    322 B/frame
// -r  6     SUMMARY: frames=  441 trlse=   232 KB    540 B/frame

// --- sequence
// 3x (badger mushroom) badger arg snake
// 2x (badger mushroom) badger arg snake
// 4x (badger mushroom) badger arg snake
// 1x (badger mushroom)
//
// TOTAL: 13 badger, 10 mushroom, 3 arg snake = 1 793 KB 
//   badger: trlse= 134 868 Bytes
//   mushrm: trlse=   5 292 Bytes
//   snake :        179 634 Bytes
//          TOTAL   319 xxx Bytes
//
// (+ (* 13 134868) (* 10 5292) (* 3 179634))= 2.3MB
// 


#define FULL

#ifdef FULL
  #define W 240
  #define H 200
#else
  #define W 120
  #define H 100
#endif

int main() {
  FILE *f;
  char name[64]= {0};
  int i= 0;

  char rgb[3*W*H];
  char last[sizeof(rgb)]= {0};

  char head[100]={0};

  int trle= 0;

  do {
    sprintf(name, "oneps-%05d.ppm", ++i);
    if (!(f= fopen(name, "r+"))) break;

    printf("--- %s ", name);
    fread(head, 15, 1, f);
    if (sizeof(rgb)!=fread(rgb, 1, sizeof(rgb), f)) {
      printf("%% Error: not full screen?\n"); break;
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

    int tp= 0, tc= 0, rc= 0, rle=0, leq=0;

    // estimate size of RLE encoded image
    // comparing sequence of cells
    for(int j=1; j<H*W/6; ++j) {
      if (memcmp(&rgb[3*j*6-3*6], &rgb[3*j*6], 3*6)) {
        // one byte for each different
        ++rle;
        // one extra byte per repeat (clever encoding?)
        if (leq) ++rle;
        leq= 0;
      } else {
        // equal
        leq= 1;
      }
    }

    // write diff file?
    char diff[sizeof(rgb)];
    memcpy(diff, rgb, sizeof(diff));

    for(int r=0; r<H; ++r) {
      int p= 0, c= 0;
      for(int j=0; j<W; ++j) {
        int at= 3*(r*W+j);
        if (0==memcmp(rgb+at, last+at, 3)) {
          memset(diff+at, 0, 3);
        }
      }
    }
    size_t pos= 15;
    fsetpos(f, &pos);
    int wr= fwrite(diff, 1, sizeof(diff), f);
    //printf("wr=%d pos=%lu ferror=%d\n", wr, pos, ferror(f));

    // diff
    for(int r=0; r<H; ++r) {
      int p= 0, c= 0;
      for(int j=0; j<W; ++j)   if (memcmp(&rgb[3*(r*W+j)], &last[3*(r*W+j)], 3)) ++p;
      for(int j=0; j<W/6; ++j) if (memcmp(&rgb[3*(r*W+j*6)], &last[3*(r*W+j*6)], 3*6)) ++c;
      if (p || c)  {
        ++rc;
        //printf("%03d %5d %4d\n", r, p, c);
      }
      tp+= p; tc+= c;
    }

    if (tp || tc || rc)  printf("\tchange: rows=%3d pixels=%5d cells=%4d rle=%4d  trle=%8d\n", rc, tp, tc, rle, trle); else putchar('\n');

    trle+= rle;

    memcpy(last, rgb, sizeof(rgb));

    fclose(f);

  } while (1);

  printf("\nSUMMARY: frames=%5d trlse=%6d KB  %5d B/frame\n", i, trle/1024, trle/i);
  printf("%% No such file: %s\n", name);
}
