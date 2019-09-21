/*
  SD card file dump

  This example shows how to read a file from the SD card using the
  SD library and send it over the serial port.

  The circuit:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created  22 December 2010
  by Limor Fried
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>
//#include <SD.h>
#define combine(low,high) (low)&((unsigned short) )

File dirFile;
uint16_t fileLength = 0;
const uint16_t nMax = 32;
uint16_t dirIndex[nMax];


const int chipSelect = 10;
SdFat SD;//SDFat Lib
File file; // File
unsigned short data[260]; // buffer
long fileSize; // FileSize
unsigned long frameNum;//num of frame 
unsigned short fps;//fps

unsigned long curFrame;// index of current frame
unsigned long curFrameBlock; // Block of the frame
unsigned long nextFrameBlock; // Block of the frame
unsigned long startTime;
unsigned short curFrameLen,curFrameCounter;//lenCounter
unsigned char roundPtr;//roundPtr

unsigned int timex[32];
unsigned int timeNow;
bool ENDPLAY=false;

unsigned short nowplayed = 0;
void up(){
  delay(1);
  if(curFrame>10&&!(PIND&B00001000)){
    nowplayed++;
    nowplayed%=fileLength;
    ENDPLAY=1;
    Serial.println(nowplayed);
  }
  return;
  
}

inline void set_XY(const unsigned short& valA, const unsigned short& valB) {
  PORTB ^= 0x03;
  SPI.transfer16(valB);//0x9000
  PORTB ^= 0x01;
  PORTB ^= 0x01;
  SPI.transfer16(valA);//0x1000
  PORTB ^= 0x03;
}
unsigned long curTime,readTime,nextOffset;
inline void playFrame(){
  file.seek(curFrameBlock*512);
  file.read(data,512);
  readTime=1;
  if(data[0]!=0xffff){
    Serial.print(F("error @"));
    Serial.println(curFrameBlock*512);
    Serial.println(data[0]);
    Serial.println(data[1]);
    while(1);
  }
  curFrameLen=data[1]+2,curFrameCounter=2,roundPtr=2;
  nextOffset=1+(curFrameLen+1>>8);
  nextFrameBlock = curFrameBlock+nextOffset;
  
  while(curFrameCounter<curFrameLen){
    if(!roundPtr){
      file.read(data,512);
      readTime++;
    }
    set_XY(data[roundPtr++],data[roundPtr++]);
    curFrameCounter+=2;
  }
  if(!(curFrameCounter & 0xff))
      readTime++;
  if(readTime!=nextOffset){
    Serial.print(F("error @ Frame"));
    Serial.println(curFrame);
    Serial.print(F("Offset"));
    Serial.println(curFrameBlock*512);
    Serial.println(readTime);
    Serial.println(nextOffset);
    Serial.println(curFrameCounter);
    Serial.println(curFrameLen);
    while(1);
  }
}
inline void play(unsigned short playx){
  if (!file.open(&dirFile, dirIndex[playx], O_RDONLY)) {
    Serial.println(F("Failed at open"));
    file.close();
    return;
  }
  Serial.print(F("start playing"));
  Serial.println(playx);
  ENDPLAY=false;
  curFrameBlock=0;
  nextFrameBlock=1;
  file.seek(curFrameBlock*512);
  file.read(data,512);
  if(data[0]==0xffff){
    frameNum=data[1];
    fps=data[2];
    Serial.print(frameNum);
    Serial.print(fps);
    startTime=millis();
    for(curFrame = 1;curFrame<=frameNum&&!ENDPLAY;++curFrame){
      Serial.println(curFrame);
      curTime=0;
      curFrameBlock=nextFrameBlock;
      do{
        curTime++;
        playFrame();
      }while((millis()-startTime)*fps < curFrame*1000);
      timeNow=millis();
      Serial.println(curTime);
      Serial.println(32000.0/(timeNow-timex[curFrame&31]));
      timex[curFrame&31]=timeNow;
      Serial.println(F(" "));
    }
  }else{
    Serial.println(F("Files get Wrong"));
    nowplayed++;
  }
  file.close();
}

void setup() {
  DDRD&=B11110111;
  PORTD|=B00001000;
  attachInterrupt(1, up, FALLING  );
  DDRB |= 0x03;
  PORTB |= 0x01;
  PORTB &= 0xfd;
  Serial.begin(115200);
  SPI.begin();
  if (!SD.begin(chipSelect, SD_SCK_MHZ(50))) {
    Serial.print(F("Failed Card"));
    while (1);
  }if (!dirFile.open("/", O_RDONLY)) {
    Serial.print(F("open root failed"));
    while(1);
  }

  while (fileLength < nMax && file.openNext(&dirFile, O_RDONLY)) {
    if (!file.isSubDir() && !file.isHidden()) {
      dirIndex[fileLength] = file.dirIndex();
      Serial.print(fileLength++);
      Serial.write(' ');
      file.printName(&Serial);
      Serial.println();
    }
    file.close();
  }
  Serial.println("Good");
}
void loop() {
  play(nowplayed);
}
