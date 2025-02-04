/*
        -- http://caca.zoy.org/browser/libpipi/trunk/pipi/codec/oric.c
        local out = assert(io.open(fullname,binmode("w")))
        
        if basic_loader>0 then
        -- write basic stub: 10 PAPER0:INK7:HIRES:CLOAD""
        out:write(string.char(0x16,0x16,0x16,0x16,0x24,
          0xff,0xff,0x00,0xc7,0x05,0x13,0x05,0x01,0xff,
          0x00,0x11,0x05,0x0a,0x00,0xb1,0x30,0x3a,0xb2,
          0x37,0x3a,0xa2,0x3a,0xb6,0x22,0x22,0x00,0x00,
          0x00,0x00)) end
        
        -- image data
        out:write(string.char(0x16,0x16,0x16,0x16,0x24,
            0x00,0xff,0x80,0x00,0xbf,0x3f,0xa0,0x00,0x00) .. 
            name .. 
            string.char(0) ..
            buf)
        out:close()
        
*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

char* hires(FILE* f) {
  char* h= calloc(1,8000);
  assert(h);
  if (8000==fread(h, 1, 8000, f)) return h;
  // fail
  assert(!"No file");
}

int main(int argc, char** argv) {
  // TODO: handle several? lol
  assert(argc==2);
  char* name= argv[1];
  FILE* f= fopen(name, "r");
  assert(0==fseek(f, -8000, SEEK_END));
  char* sc= hires(f);
  fclose(f);

  printf("P6\n240 200\n255\n");
  for(int r=0; r<200; ++r) {
    char bg= 0, fg= 7;

    for(int c=0; c<40; ++c) {
      char v= sc[r*40+c];

      switch(v) {
      case 0x00 ... 0x07: case 0x80 ... 0x87: fg= v&7; break;
      case 0x10 ... 0x17: case 0x90 ... 0x97: bg= v&7; break;
      }
      char f= v&128? 7-fg: fg, b= v&128? 7-bg: bg;
      for(int p=0; p<6; ++p) {
        char col= (v&127)<32? b: ((v&(32>>p))? f: b);
        // Show attributes
        if ((v&127)<8) { // ink
          // pink
          if (p&1 || v<128) printf("\xff\xcc\xcc"); else printf("%c%c%c", 0, 0, 0);
        } else if ((v&127)<16+8) { // paper
          // orange
          if (p&1 || v<128) printf("\xff\xb2\x66"); else printf("%c%c%c", 0, 0, 0);
        } else printf("%c%c%c", col&1? 255: 0, col&2? 255: 0, col&4? 255: 0); // rgb
      }
    }
  }
  return 0;
}
