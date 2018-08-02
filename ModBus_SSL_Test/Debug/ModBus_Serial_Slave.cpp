#include "Energia.h"

#line 1 "C:/Users/admin/workspace_v8/ModBus_SSL_Test/ModBus_Serial_Slave.ino"

#include <Modbus.h>
#include <ModbusSerial.h>

#include <avr/pgmspace.h>
#include "q3.h"
#include "cl1.h"
#include "cond1.h"
#include <Wire.h>

#define PRINTREGISTERS


#define BAUD 19200
#define ID 1                    
#define TXPIN 5                 
#define VLC_STR_LEN 100         
#define VLC_MODULATION_PIN 14   

char* interleaver(char* s);
char* manchester(char* s, char interout[][8], char manout[][16]);
char* foo(char* s, char manout[][16], char final[][56]);
void send_vlc_data(char* data);
void setup();
void loop();

#line 20
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

#define VLC_ON_COIL 1000         



byte ioconf1[2]={0x00,0x00};  
byte ioconf2[2]={0x01,0x00};



byte data1[11][2]={{0x14,0xF0},{0x14,0xF0},{0x14,0xF0},{0x14,0x70},{0x14,0x70},{0x14,0x70},{0x14,0x70},{0x14,0x80},{0x14,0x00},{0x14,0x00},{0x14,0x00}};
byte data2[11][2]={{0x15,0xF9},{0x15,0xF1},{0x15,0xC9},{0x15,0xB8,},{0x15,0xB0},{0x15,0x88},{0x15,0x80},{0x15,0x41},{0x15,0x30},{0x15,0x08},{0x15,0x00}};



int oldb=0;
char data[VLC_STR_LEN]="CPS";              
char olddata[VLC_STR_LEN]="CPS";
char interout[VLC_STR_LEN][8], manout[VLC_STR_LEN][16], final[VLC_STR_LEN][56];
bool sendVLC=false;                    


char* interleaver(char* s){
    


    uint8_t a=0,b=0;

    while(*s !='\0'){

        for (b = 0; b < 8; b++) {
        interout [a][b] = !((*s << b) & 0x80);
        }
        
        s++;
        a++;
            }
    return s;
}


char* manchester(char* s, char interout[][8], char manout[][16]){
    


    int i=0,j=0;

    while(*s != '\0'){

        for (j=0; j<8; j++){
            if (interout[i][j]==1){
                manout[i][2*j]=1;
                manout[i][2*j+1]=0;
            }
            else{
                manout[i][2*j]=0;
                manout[i][2*j+1]=1;
            }
        }

        s++;
        i++;
    }

    return s;
}


char* foo(char* s, char manout[][16], char final[][56]){
    



    int i=0,j=0,k=0;


    while(*s != '\0'){
        k=0;
        for (j=0; j<16; j++){
            if (manout[i][j]==1){
                final[i][k]=1;
                final[i][k+1]=1;
                final[i][k+2]=1;
                final[i][k+3]=1;
                final[i][k+4]=1;
                final[i][k+5]=1;
                k+=6;
            }
            else{
                final[i][k]=0;
                k++;
            }
        }
        s++;
        i++;
    }

    return s;
}

void send_vlc_data(char* data)
{

    int i=0,j=0;

    digitalWrite(VLC_MODULATION_PIN, HIGH);
    delayMicroseconds(150);
    i=0;
    while(*data++ != '\0'){
        for (j = 0; j < 56; j++) {
            if (final[i][j]==1){
                        
                        digitalWrite(VLC_MODULATION_PIN, LOW);
                        delayMicroseconds(50);

                    }
                    else if(final[i][j]==0){

                        
                        digitalWrite(VLC_MODULATION_PIN, HIGH);
                        delayMicroseconds(50);


                    }
                }
            i++;
    }
}



void setup() {



  
  modbus.config(&Serial, BAUD, TXPIN); 

  
  modbus.setSlaveId(ID);


















  
  modbus.addIreg(TEMP_IP, 0);
  modbus.addIreg(VOLTAGE_IP, 0);
  modbus.addIreg(CURRENT_IP, 0);
  modbus.addIreg(LDR_IP, 0);


  

  modbus.addHreg(BRIGHTNESS_H, 50);
  modbus.addHreg(CHIP_TEMP_H, 0);

  

  modbus.addIreg(VLC_ON_COIL, sendVLC);

  for(int i=VLC_STR_LEN; i<(VLC_STR_LEN+VLC_START_H); i++)
      modbus.addHreg(i, '\0');
  
  modbus.Hreg(1000, 'c');
  modbus.Hreg(1001, 'p');
  modbus.Hreg(1002, 's');


  Wire.setModule(1);
  Wire.begin();
  Wire.beginTransmission(LAMP_I2C_ADDR);
  Wire.write(ioconf1, 2);
  Wire.endTransmission();


  Wire.beginTransmission(LAMP_I2C_ADDR);
  Wire.write(ioconf2, 2);
  Wire.endTransmission();

  Wire.beginTransmission(LAMP_I2C_ADDR);
  Wire.write(data1[5], 2);
  Wire.endTransmission();


  Wire.beginTransmission(LAMP_I2C_ADDR);
  Wire.write(data2[5], 2);
  Wire.endTransmission();

  pinMode(VLC_MODULATION_PIN, OUTPUT);

  interleaver(data);
  manchester(data, interout, manout);
  foo(data, manout, final);

}




void loop() {

    if(modbus.task())
  {
        sendVLC=modbus.Ireg(VLC_ON_COIL);
      
      
        int b=modbus.Hreg(BRIGHTNESS_H);
        b=(b/10);
        if (b>10) b=10;
        if(b<0) b=0;
        if(oldb!=b);                           
        {
        oldb=b;

        Wire.beginTransmission(LAMP_I2C_ADDR);
        Wire.write(data1[b], 2);
        Wire.endTransmission();


        Wire.beginTransmission(LAMP_I2C_ADDR);
        Wire.write(data2[b], 2);
        Wire.endTransmission();
        }



      int i=0;                          

      do{
          data[i]=char(modbus.Hreg(i+VLC_START_H));
          i++;
      }while(modbus.Hreg(i)!=0);   
      data[i]=0;
      if (olddata!=data)
      {
      interleaver(data);
      manchester(data, interout, manout);
      foo(data, manout, final);
      i=0;
      while(data[i]!=0){
            olddata[i]=data[i];
            i++;
      }
      }
  }

  if(sendVLC)
  send_vlc_data(data);

  if (millis() > update_time + 2000)
  {   update_time = millis();
      modbus.Ireg(LDR_IP, analogRead(LDR_PIN));
      modbus.Ireg(TEMP_IP, analogRead(TEMP_PIN));
      modbus.Ireg(VOLTAGE_IP, analogRead(VOLTAGE_PIN));
      modbus.Ireg(CURRENT_IP, analogRead(CURRENT_PIN));
      modbus.Hreg(CHIP_TEMP_H, analogRead(TEMPSENSOR));
  }

}
























