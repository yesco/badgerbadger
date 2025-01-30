#include <stdio.h>

int main() {
  FILE *f;
  char name[64]= {0};
  int i= 0;

  char rgb[3*240*200];
  char last[sizeof(rgb)]= {0};

  char head[100]={0};

  do {
    sprintf(name, "oneps-%05d.ppm", ++i);
    if (!(f= fopen(name, "r+"))) break;

    printf("--- %s ", name);
    fread(head, 15, 1, f);
    if (sizeof(rgb)!=fread(rgb, 1, sizeof(rgb), f)) {
      printf("%% Error: not full screen?\n"); break;
    }

    // single bit rgb
    for(int r=0; r<200; ++r) {
      int p= 0, c= 0;
      for(int j=0; j<240; ++j) {
        int at= 3*(r*240+j);
        rgb[at+0]= rgb[at+0]>127?255:0;
        rgb[at+1]= rgb[at+1]>127?255:0;
        rgb[at+2]= rgb[at+2]>127?255:0;
      }
    }

    // estimate size of RLE encoded image
    // comparing sequence of cells
    for(int j=1; j<200*240/6; ++j) {
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

    for(int r=0; r<200; ++r) {
      int p= 0, c= 0;
      for(int j=0; j<240; ++j) {
        int at= 3*(r*240+j);
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
    int tp= 0, tc= 0, rc= 0, rle=0, leq=0;

    for(int r=0; r<200; ++r) {
      int p= 0, c= 0;
      for(int j=0; j<240; ++j)   if (memcmp(&rgb[3*(r*240+j)], &last[3*(r*240+j)], 3)) ++p;
      for(int j=0; j<240/6; ++j) if (memcmp(&rgb[3*(r*240+j*6)], &last[3*(r*240+j*6)], 3*6)) ++c;
      if (p || c)  {
        ++rc;
        //printf("%03d %5d %4d\n", r, p, c);
      }
      tp+= p; tc+= c;
    }
    if (tp || tc || rc)  printf("\tchange: rows=%3d pixels=%5d cells=%4d rle=%4d\n", rc, tp, tc, rle); else putchar('\n');

    memcpy(last, rgb, sizeof(rgb));

    fclose(f);

  } while (1);

  printf("%% No such file: %s\n", name);
}
