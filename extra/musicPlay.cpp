
#include <SimpleSDAudio.h>
#define MUSIC_CH 3
// Callback target, prints output to serial
void DirCallback(char *buf) { 
  Serial.println(buf);
}

char AudioFileName[16];

// Create static buffer 
#define BIGBUFSIZE (2*512)      
uint8_t bigbuf[BIGBUFSIZE];
uint8_t flag;
uint8_t num=1;
bool x=1;
// helper function to determine free ram at runtime
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void playNext(){
  delay(1);
  if(SdPlay.isPlaying()&&digitalRead(3)==0){
      SdPlay.stop();
      Serial.println(F("Stop."));
      num++;
      if(num==5)num=1;
      flag=0;
      x=1;
  }
}

void setup()
{
  
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  attachInterrupt(1,playNext,FALLING);
  Serial.print(F("Free Ram: "));
  Serial.println(freeRam());
  
  // Setting the buffer manually for more flexibility
  SdPlay.setWorkBuffer(bigbuf, BIGBUFSIZE); 
  
  Serial.print(F("\nInitializing SimpleSDAudio V" SSDA_VERSIONSTRING " ..."));  
    
  // Select between SSDA_MODE_FULLRATE or SSDA_MODE_HALFRATE (62.5kHz or 31.25kHz)
  // and the output modes SSDA_MODE_MONO_BRIDGE, SSDA_MODE_MONO or SSDA_MODE_STEREO
  if (!SdPlay.init(SSDA_MODE_FULLRATE | SSDA_MODE_MONO)) {
    Serial.println(F("initialization failed. Things to check:"));
    Serial.println(F("* is a card is inserted?"));
    Serial.println(F("* Is your wiring correct?"));
    Serial.println(F("* maybe you need to change the chipSelect pin to match your shield or module?"));
    Serial.print(F("Error code: "));
    Serial.println(SdPlay.getLastError());
    while(1);
  } else {
    Serial.println(F("Wiring is correct and a card is present.")); 
  }

  pinMode(MUSIC_CH, INPUT_PULLUP);
  

}
void loop(void) {
  uint8_t count=0, c;
 
  Serial.println(F("Files on card:"));
  SdPlay.dir(&DirCallback);

ReEnter: 
  count = 0;
  switch (num)
  {
  case 1: strcpy(AudioFileName,"1.AFM");break;
  case 2: strcpy(AudioFileName,"2.AFM");break;
  case 3: strcpy(AudioFileName,"3.AFM");break;
  case 4: strcpy(AudioFileName,"4.AFM");break;
  }

  
  Serial.print(F("Looking for file... "));
  if(!SdPlay.setFile(AudioFileName)) {
    Serial.println(F(" not found on card! Error code: "));
    Serial.println(SdPlay.getLastError());
    goto ReEnter;
  } else {
   Serial.println(F("found.")); 
  }    


  flag = 1;
  
while(flag) {
    SdPlay.worker();
    //digitalRead(2)==LOW
    if(x||SdPlay.isStopped())
    {
      SdPlay.play();
      Serial.println(F("Play."));
      x=0;
    }
  }
}