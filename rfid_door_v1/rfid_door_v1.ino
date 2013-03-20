/*
RFID Door Reader version 0.1
Copyright (c) 2013, Cosmo Borsky
All rights reserved.

FreeBSD License

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies, 
either expressed or implied, of the FreeBSD Project.
*/
/*
Copyright(c) Adafruit Industries regarding readMifare.pde
Credit to Bill Greiman regarding the SDFat Library
*/
#include <Adafruit_PN532.h>
#include <SdFat.h>

#define SCK  (2)
#define MOSI (3)
#define SS   (9)
#define MISO (5)
#define relay (8)
#define writepin (7)
#define speaker (6)
#define tone1 (1760)
#define tone2 (340)

static const char nybble_chars[]="0123456789ABCDEF";

boolean rfid_lock = false;
boolean writestate = 0;
boolean masterstate = 0;

uint8_t master_card[] = { 0x00, 0x00, 0x00, 0x00 }; //due to change to a database
uint8_t master_card_pass[] = { 'i', 'h', 'e', 'a', 'r', 't', 'r', 'o', 'b', 'o', 't', 'i', 'c', 's', 0, 0 }; //depricated, but good for example
uint8_t data_pass_1[] = { 'm', 'i', 'f', 'a', 'r', 'e', 'c', 'a', 'r', 'd', 0, 0, 0, 0, 0, 0 };
uint8_t data_pass_clear[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int cardnum = 0;

SdFat sd;
SdFile myFile;
char uidhex[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
char datahex[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

Adafruit_PN532 nfc(SCK, MISO, MOSI, SS);

/**************************************************************************/

char* hextoarray(const uint8_t *data, const uint32_t number, char *buffer){
  uint32_t szPos;
  uint32_t count;
  char* manipulate = buffer;
  for (szPos=0; szPos < number; szPos++){
    manipulate[szPos+count] = nybble_chars[(data[szPos]>>4)&0x0F];
    count++;
    manipulate[szPos+count] = nybble_chars[(data[szPos])&0x0F];
  }
  return manipulate;
}
void opendoor(void){//get on the floor, everybody walk the dinosaur
  //unlock
  digitalWrite(relay, HIGH);
  Serial.println("Access Granted");
  //beep twice, high tone
  tone(speaker,tone1,250);
  delay(300);
  tone(speaker,tone1,250);
  delay(2100);
  //lock
  digitalWrite(relay, LOW);
}
//deny entry, play low tone
void denyentrance(void){
  Serial.println("Access Denied");
  tone(speaker,tone2,500);
  delay(1000);
}
void rfid_loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    if (uidLength == 4){
      uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
      success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya);
      
      if (success){
        uint8_t data[16];
        success = nfc.mifareclassic_ReadDataBlock(4, data);
                
        if (success){
          char* card = hextoarray(uid, 4, uidhex);
          if(masterstate && memcmp(master_card, uid, 4) != 0){
            if(sd.exists(card)){
              sd.remove(card);
            }
            if(writestate){
              if(nfc.mifareclassic_WriteDataBlock(4, data_pass_clear)){
                tone(speaker,tone1,500);
                masterstate = false;
                delay(1000);
                return;
              }
            }
            if(nfc.mifareclassic_WriteDataBlock(4, data_pass_1)){
              if(myFile.open(card, O_RDWR | O_CREAT | O_AT_END)){
                myFile.print((char*)data_pass_1);
                myFile.close();
              }else{
                tone(speaker,tone2,500);
              }
            }
            masterstate = false;
            delay(200);
            return;
          }
            
          if(sd.exists(card)){
            if(myFile.open(card, O_READ)){
              uint8_t filedata[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
              int dataint;
              int count = 0;
              while ((dataint = myFile.read()) > 0){
                if(count < 16){
                  filedata[count] = dataint;
                  count++;
                }else{
                  break;
                }
              }
              if(memcmp(master_card, uid, 4) == 0 && memcmp(master_card_pass, data, 16) == 0){
                
                if(masterstate){
                  Serial.println("!masterstate");
                  masterstate = false;
                  tone(speaker,tone2,250);
                }else{
                  Serial.println("masterstate");
                  masterstate = true;
                  tone(speaker,tone1,250);
                }
                delay(1500);
                myFile.close();
                return;
              }
              if(memcmp(filedata, data, 16) == 0){
                opendoor();
              }else{
                denyentrance();
              }
            }else{
              
            }
          }else{
            denyentrance();
          }
          myFile.close();
        }
      }
    }
  }
}



void setup(void) {
  Serial.begin(9600);

  pinMode(relay, OUTPUT);
  pinMode(speaker, OUTPUT);
  pinMode(writepin, INPUT);
  pinMode(10, OUTPUT);
  
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    tone(speaker,tone2,1000);
    Serial.print("No RFID board");
    while (1);
  }
  nfc.SAMConfig();

  if (!sd.begin(4, SPI_HALF_SPEED)) sd.initErrorHalt();
  
  delay(10);
  tone(speaker,tone1,350);
  delay(350);
}
void loop() {
  writestate = !digitalRead(writepin);
  if(rfid_lock && !writestate)
    rfid_lock = false;
  rfid_loop();
}
