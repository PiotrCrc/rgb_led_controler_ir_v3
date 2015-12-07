
#include <JeeLib.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Arduino.h"
#include "Ramp.h"

#define DEBUG 1
        //#if DEBUG
        //#endif
        
#define BAND RF12_868MHZ 
#define GROUP 5     
#define NODEID 29  
#define NVALUES 6  
#define GATEID 1

// Temperature
#define ONE_WIRE_BUS A0
OneWire ds(ONE_WIRE_BUS);
DallasTemperature sensors(&ds);
long tempTimer;

// RF radio variables
byte rf_cmd;
byte rf_msg_id;
byte rf_snd_ack = 0;
byte rf_color[4]; // R,G,B,Time
byte rf_packet[NVALUES];
byte rf_ack_retry = 0;
byte eeprom[10];
long sendTimer;
long recvTimer;

long rampTimer;

Ramp ch_rgb(true);
Ramp ch_w(true);

boolean timeout(long *timer, int ms_delay) {
  if (millis() - (*timer) > ms_delay) {
    *timer = millis();
    return HIGH;  
  } else {
    return LOW;
  }
}

void loadSettings() 
{
    for (byte i = 0; i < 10; ++i) eeprom[i] = EEPROM.read(i);
}

void saveSettings() 
{
    for (byte i = 0; i < 10; ++i) EEPROM.write(i, eeprom[i]);
}

void setup () {
    Serial.begin(9600);

    loadSettings(); 

    sensors.begin();
    timeout(&tempTimer,0);

    ch_rgb.set_sp(eeprom[1],eeprom[2],eeprom[3],eeprom[4]);
    ch_w.set_sp(eeprom[5],eeprom[6]);
    
    rf12_initialize(NODEID, BAND, GROUP);    
    
    timeout(&sendTimer,0);
    timeout(&recvTimer,0);
    
}

void loop() {
//  handling changing of the colors (ramp)
    if (timeout(&rampTimer,12)) {
      ch_rgb.do_step();
      ch_w.do_step();
      }
      if (!ch_rgb.at_sp()) {
          analogWrite(5, ch_rgb.get_val(0));
          analogWrite(9, ch_rgb.get_val(1));
          analogWrite(6, ch_rgb.get_val(2));
      }
      if (!ch_w.at_sp()) {
          analogWrite(3, ch_w.get_val(0));
      }
   
    
 
//  handling radio packets - receive   
    if ( rf12_recvDone() && timeout(&recvTimer,50) ) 
    {           
      if (rf12_hdr == ( RF12_HDR_DST | NODEID) && rf12_crc == 0 && rf12_len == NVALUES) 
      {
        rf_cmd     = rf12_data[0]; 
        rf_msg_id  = rf12_data[5];
        rf_snd_ack = 1;
        for (byte i = 0; i < 4; ++i) rf_color[i] = rf12_data[i+1];
         
        #if DEBUG
        Serial.print("RCV");
        Serial.print("C:"); Serial.print(rf_cmd);
        Serial.print("R:"); Serial.print(rf_color[1]);
        Serial.print("G:"); Serial.print(rf_color[2]);
        Serial.print("B:"); Serial.print(rf_color[3]);
        Serial.print("T:"); Serial.println(rf_color[4]); 
        #endif
       
        switch (rf_cmd) {
         case 1:               
           ch_rgb.set_sp(rf_color[0],rf_color[1],rf_color[2],rf_color[3]);
           break;
         case 2:               
           ch_w.set_sp(rf_color[0],rf_color[3]);
           break;
         case 3:
           ch_rgb.set_sp(eeprom[1],eeprom[2],eeprom[3],eeprom[4]);  
           ch_w.set_sp(eeprom[5],eeprom[6]); 
           break;
         case 4:
           ch_rgb.set_sp(0,0,0,eeprom[4]); 
           ch_w.set_sp(0,eeprom[6]);
           break;
         case 5:
           for (byte i = 0; i < 3; ++i) eeprom[i+1] = ch_rgb.get_sp(i);
           eeprom[4] = ch_rgb.get_steps();
           saveSettings();             
           break;  
         case 6:
           eeprom[5] = ch_w.get_sp(0);
           eeprom[6] = ch_w.get_steps();
           saveSettings();             
           break;  
         case 96:
           eeprom[9] = rf_color[3]; 
           saveSettings();  
           break;
         case 98:
           eeprom[8] = rf_color[3]; 
           saveSettings();  
           break;
        }
      }     
    }

if (timeout(&tempTimer,5000)) {sensors.requestTemperatures();}
// handling  radio transmition - sending acknowlage
      if (rf_snd_ack && timeout(&recvTimer,15) && rf12_canSend()) {
        rf_packet[0] = rf_cmd;
        rf_packet[3] = 0;
        rf_packet[4] = 0;
        rf_packet[5] = rf_msg_id;
        if (rf_cmd == 9) {
          rf_packet[1] = sensors.getTempCByIndex(0);
          rf_packet[2] = round(sensors.getTempCByIndex(0)*100)-round(sensors.getTempCByIndex(0))*100;
        } else if (rf_cmd == 97) {
          rf_packet[1] = eeprom[9];
        } else if (rf_cmd == 99) {
          rf_packet[1] = eeprom[8];
        } else {        
          rf_packet[1] = 0;
          rf_packet[2] = 0;
        }
        rf12_sendStart((RF12_HDR_DST | GATEID), &rf_packet , NVALUES);
        rf_ack_retry++;
        if (rf_ack_retry > 3) {                 // Send ack 3 times
        rf_ack_retry = 0;
        rf_snd_ack = 0;
        }
      }


}
