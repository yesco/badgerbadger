/*
  YM file register extractor and interleaver
  by Daniel Tufvesson 2014
  Based on YM file format specification
  by Arnaud Carré
 */

#include <stdlib.h>
#include <stdio.h>

//typedef char byte; //byte
typedef unsigned char u_byte; //unsigned byte
typedef short int16; //little-endian signed 16-bit integer
typedef unsigned short u_int16; //little-endian unsigned 16-bit integer
typedef int int32; //little-endian signed 32-bit integer
typedef unsigned int u_int32; //little-endian unsigned 32-bit integer
typedef float float32; //IEEE-754 32-bit floating-point
typedef double float64; //IEEE-754 64-bit floating-point
typedef int bool; //bool

// File types
#define YM2YM3 1
#define YM5YM6 2

u_byte *ymFileData;

u_int32 readMotorola32(u_byte **ptr){
  u_int32 n;
  u_byte *p = *ptr;  
  n = (p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3];
  p+=4;
  *ptr = p;
  return n;
}

u_int16 readMotorola16(u_byte **ptr){
  u_int16 n;
  u_byte *p = *ptr;
  n = (p[0]<<8)|p[1];
  p+=2;
  *ptr = p;
  return n;
}

u_byte *readString(u_byte **ptr){
  u_byte *p;
  p = *ptr;
  (*ptr) += strlen(*ptr)+1;
  return p;
}


int32 readLittleEndian32(u_byte *pLittle){
  int32 v = ( (pLittle[0]<<0) |
	      (pLittle[1]<<8) |
	      (pLittle[2]<<16) |
	      (pLittle[3]<<24)); 
  return v;
}

int32 readBigEndian32(u_char *pBig){
  int32 v = ( (pBig[0]<<24) |
	      (pBig[1]<<16) |
	      (pBig[2]<<8) |
	      (pBig[3]<<0)); 
  return v;
}


int getFileSize(FILE *fp){
  int fSize;
  int oPos; 
  oPos = ftell(fp);
  fseek(fp,0,SEEK_END);
  fSize = ftell(fp);
  fseek(fp,oPos,SEEK_SET);
  return fSize;
}

int main(int argc, char **argv){
  FILE *ymFile, *outFile;
  unsigned int fileSize;

  if(argc < 3){
    printf("Usage: %s <in:uncompressed YM file> <out:YM dump file>\n",argv[0]);
    exit(-1);
  }

  if( (ymFile = fopen( argv[1], "rb")) == NULL ){
    printf("Unable to open source file\n");
    exit(-1);
  }

  if( (outFile = fopen( argv[2], "wb")) == NULL ){
    printf("Unable to open destination file\n");
    exit(-1);
  }

  // Read YM file into memory
  fileSize = getFileSize(ymFile);
  printf("Reading %d bytes\n", fileSize);
  ymFileData = (u_byte*)malloc(fileSize);
  if(!ymFileData){
    printf("Unable to allocate memory\n");
    fclose(ymFile);
    exit(-2);
  }
  if(fread(ymFileData,1,fileSize,ymFile)!=(size_t)fileSize){
    free(ymFileData);
    printf("Unable to read file\n");
    fclose(ymFile);
    exit(-3);
  }
  fclose(ymFile);

  // Parse YM file
  u_byte ymType;
  u_int32 ymFrames;
  u_int32 ymAttributes;
  u_int16 ymDrums;
  u_int32 ymClock;
  u_int16 ymRate;
  u_int32 ymLoopFrame;
  u_int16 ymSkip;
  u_byte *ymRegData;
  u_char *ymStrName;
  u_char *ymStrAuthor;
  u_char *ymStrComment;
  u_byte *cPtr;
  u_int32 ymID = readBigEndian32(ymFileData);
  switch(ymID){
  case 'YM2!':
    ymType = YM2YM3;
    printf("File is YM2\n");
    ymFrames = (fileSize - 4) / 14;
    ymRegData = ymFileData + 4;
    break;
  case 'YM3!':
    ymType = YM2YM3;
    printf("File is YM3\n");
    ymFrames = (fileSize-4)/14;
    ymRegData = ymFileData + 4;
    break;
  case 'YM3b':
    ymType = YM2YM3;
    printf("File is YM3b\n");
    ymFrames = (fileSize-4)/14;
    ymRegData = ymFileData + 4;
    break;
  case 'YM4!':
    printf("File is YM4 - not supported\n");
    free(ymFileData);
    exit(-4);
    break;
  case 'YM5!':
  case 'YM6!':
    ymType = YM5YM6;
    printf("File is YM5/YM6\n");
    if(strncmp((u_char*)(ymFileData+4),"LeOnArD!",8)){
      printf("File is not a valid YM5/YM6 file\n");
      free(ymFileData);
      exit(-4);
    }
    cPtr = ymFileData + 12;
    ymFrames = readMotorola32(&cPtr);
    ymAttributes = readMotorola32(&cPtr);
    ymDrums = readMotorola16(&cPtr);
    ymClock = readMotorola32(&cPtr);
    ymRate = readMotorola16(&cPtr);
    ymLoopFrame = readMotorola32(&cPtr);
    ymSkip = readMotorola16(&cPtr);
    cPtr = cPtr + ymSkip;
    if(ymDrums != 0){
      printf("Digi Drums not supported. Sorry.\n");
      free(ymFileData);
      exit(-4);
    }
    ymStrName = readString(&cPtr);
    ymStrAuthor = readString(&cPtr);
    ymStrComment = readString(&cPtr);
    ymRegData = cPtr;
    break;
  default:
    printf("File is unknown\n");
    free(ymFileData);
    exit(-4);
  }
  
  // Print file info
  printf("  Frames: %d\n", ymFrames);
  if(ymType == YM5YM6){
    printf("  Attrib: %#.4x\n", ymAttributes);
    printf("   Clock: %d Hz\n", ymClock);
    printf("    Rate: %d Hz\n", ymRate);
    printf("    Name: %s\n", ymStrName);
    printf("  Author: %s\n", ymStrAuthor);
    printf(" Comment: %s\n", ymStrComment);
  }

  // Save interleaved YM register data to file
  u_int32 frame;
  u_byte reg;
  for(frame = 0; frame < ymFrames; frame++){
    for(reg = 0; reg < 14; reg++)
      fputc(ymRegData[reg*ymFrames+frame],outFile);
  }
  fclose(outFile);
  free(ymFileData);
}
