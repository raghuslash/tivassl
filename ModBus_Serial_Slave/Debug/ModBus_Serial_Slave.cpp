#include "Energia.h"

#line 1 "C:/Users/admin/workspace_v8/ModBus_Serial_Slave/ModBus_Serial_Slave.ino"

#include <Modbus.h>
#include <ModbusSerial.h>

#include <avr/pgmspace.h>
#include "q3.h"
#include "cl1.h"
#include "cond1.h"
#include <Wire.h>

#define PRINTREGISTERS


#define BAUD 9600
#define ID 1             
#define TXPIN 5          
#define VLC_STR_LEN 100  

void setup();
void loop();

#line 19
ModbusSerial modbus;

const int ledPin = RED_LED;

unsigned long update_time = 0;
unsigned long inc_time = 0;

#define LAMP_I2C_ADDR 0x20
#define LAMP_BR_ADDR1 0x14
#define LAMP_BR_ADDR1 0x15



#define TEMP_IP 3000
#define TEMP_PIN A6             
#define VOLTAGE_IP 3001
#define VOLTAGE_PIN A1          
#define CURRENT_IP 3002
#define CURRENT_PIN A0          
#define LDR_IP 3003
#define LDR_PIN A5              


#define VLC_START_H 1000
#define BRIGHTNESS_H 2000
#define CHIP_TEMP_H 3000


void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  
  modbus.config(&Serial, BAUD, TXPIN); 

  
  modbus.setSlaveId(ID);


















  
  modbus.addIreg(TEMP_IP, 0);
  modbus.addIreg(VOLTAGE_IP, 0);
  modbus.addIreg(CURRENT_IP, 0);
  modbus.addIreg(LDR_IP, 0);


  

  modbus.addHreg(BRIGHTNESS_H, 20);
  modbus.addHreg(CHIP_TEMP_H, 0);

  for(int i=VLC_STR_LEN; i<(VLC_STR_LEN+VLC_START_H); i++)
      modbus.addHreg(i, '\0');
  Wire.setModule(1);
  Wire.begin();


}


int oldb=0;
void loop() {
  
    byte ioconf1[2]={0x00,0x00};
    byte ioconf2[2]={0x01,0x00};

    byte data1[2]={0x14,0x00};
    byte data2[2]={0x15,0x00};


    while(1){
          
          


          Wire.beginTransmission(LAMP_I2C_ADDR);
          Wire.write(ioconf1, 2);
          Wire.endTransmission();


          Wire.beginTransmission(LAMP_I2C_ADDR);
          Wire.write(ioconf2, 2);
          Wire.endTransmission();


          Wire.beginTransmission(LAMP_I2C_ADDR);
          Wire.write(data1, 2);
          Wire.endTransmission();


          Wire.beginTransmission(LAMP_I2C_ADDR);
          Wire.write(data2, 2);
          Wire.endTransmission();

          delay(1000);
    }




  if(modbus.task())
  {
      
      int b=modbus.Hreg(BRIGHTNESS_H);
      b=(b/10)*10;
      if(oldb!=b){
      oldb=b;



      }





      
      int i=VLC_START_H;
      while(modbus.Hreg(i)!='\0')
      {

      i++;
      }

  }

  


  if (millis() > update_time + 500) {
      update_time = millis();
      modbus.Ireg(LDR_IP, analogRead(LDR_PIN));
      modbus.Ireg(TEMP_IP, analogRead(TEMP_PIN));
      modbus.Ireg(VOLTAGE_IP, analogRead(VOLTAGE_PIN));
      modbus.Ireg(CURRENT_IP, analogRead(CURRENT_PIN));
      modbus.Hreg(CHIP_TEMP_H, analogRead(TEMPSENSOR));

  }
}




































































