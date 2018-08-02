
#include <Modbus.h>
#include <ModbusSerial.h>
//#include "functions.h"
#include <avr/pgmspace.h>
#include "q3.h"
#include "cl1.h"
#include "cond1.h"
#include <Wire.h>

#define PRINTREGISTERS

// ModBus Port information
#define BAUD 9600
#define ID 1                    //MODBUS SLAVE ID
#define TXPIN 5                 //send/receive enable for RS485
#define VLC_STR_LEN 100         //MAX number of characters to send in VLC data.
#define VLC_MODULATION_PIN 14   //PIN USED TO MODULATE VLC DATA

ModbusSerial modbus;

const int ledPin = RED_LED;

unsigned long update_time = 0;
unsigned long inc_time = 0;

#define LAMP_I2C_ADDR 0x20
#define LAMP_BR_ADDR1 0x14
#define LAMP_BR_ADDR1 0x15


//INPUT REGISTER ADDRESSES
#define TEMP_IP 3000
#define TEMP_PIN A6             // connected to PD1 - A6
#define VOLTAGE_IP 3001
#define VOLTAGE_PIN A1          //connected to PE2 - A1
#define CURRENT_IP 3002
#define CURRENT_PIN A0          //connected to PE3 - A0
#define LDR_IP 3003
#define LDR_PIN A5              //connected to PD2 - A5

//HOLDING REGISTER ADDRESSES
#define VLC_START_H 1000
#define BRIGHTNESS_H 2000
#define CHIP_TEMP_H 3000


byte ioconf1[2]={0x00,0x00};  //GPIO expander configs
byte ioconf2[2]={0x01,0x00};


//BRIGHTNESS ENCODED VALUES staring from 0 to 100 in steps of 10
byte data1[11][2]={{0x14,0xF0},{0x14,0xF0},{0x14,0xF0},{0x14,0x70},{0x14,0x70},{0x14,0x70},{0x14,0x70},{0x14,0x80},{0x14,0x00},{0x14,0x00},{0x14,0x00}};
byte data2[11][2]={{0x15,0xF9},{0x15,0xF1},{0x15,0xC9},{0x15,0xB8,},{0x15,0xB0},{0x15,0x88},{0x15,0x80},{0x15,0x41},{0x15,0x30},{0x15,0x08},{0x15,0x00}};



int oldb=0;
char data[VLC_STR_LEN]="CPS";              //Initially transmit this string
char olddata[VLC_STR_LEN]="CPS";
char interout[VLC_STR_LEN][8], manout[VLC_STR_LEN][16], final[VLC_STR_LEN][56];


char* interleaver(char* s){
    /* The Interleaver block rearranges the binary bits. Each character in the string is represented by 8 bit binary.
     Input to this block is string array(message) and the output will be stored in interout[][8]. We have to pass a string array
     another 2D array say for example interout[][8] while calling this function. This function returns a character pointer. */
    uint8_t a=0,b=0;

    while(*s !='\0'){

        for (b = 0; b < 8; b++) {
        interout [a][b] = !((*s << b) & 0x80);
        }
        //UARTprintf("%c",*s);
        s++;
        a++;
            }
    return s;
}


char* manchester(char* s, char interout[][8], char manout[][16]){
    /*The manchester encoding block represents binary "1" as "10" and binary 0 as "01". Input to this block is the string array(message)
    interout[][8] that is the output of interleaver block and manout[][16] to hold the output. We have to pass a string array, output of
    interleaver and an another 2D array say for example manout[][16] while calling this function. This function returns a character pointer.*/
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
    /*This block is for increasing the number of ones. each binary "1" is represented as "111" and binary "0" is represented as "0".
     Input to this block is the string array(message),manout[][16] that is the output of manchester encoding block and final[][32] to hold the output.
     We have to pass a string array, output of manchester and an another 2D array say for example final[][32] while calling this function.
     This function returns a character pointer.*/
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
//        GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_6, GPIO_PIN_6);
    digitalWrite(VLC_MODULATION_PIN, LOW);
    delayMicroseconds(150);
    i=0;
    while(*data++ != '\0'){
        for (j = 0; j < 56; j++) {
            if (final[i][j]==1){
                        //GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_6, 0);
                        digitalWrite(VLC_MODULATION_PIN, LOW);
                        delayMicroseconds(50);
//                        delay(500);
                    }
                    else if(final[i][j]==0){

                        //GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_6, GPIO_PIN_6);
                        digitalWrite(VLC_MODULATION_PIN, HIGH);
                        delayMicroseconds(50);
//                        delay(500);

                    }
                }
            i++;
    }
}



void setup() {
//  pinMode(ledPin, OUTPUT);
//  digitalWrite(ledPin, HIGH);

  // Config Modbus Serial (port, speed, byte format)
  modbus.config(&Serial, BAUD, TXPIN); // Change to Serial1 before deployment

  // Set the Slave ID (1-247)
  modbus.setSlaveId(ID);

//  // Use addIsts() for digital inputs - Discrete Input - Master Read-Only

//    modbus.addIsts(i);

//  // Use addIreg() for analog Inputs - Input Register - Master Read-Only

//    modbus.addIreg(i);

//  // Use addCoil() for digital outputs -  Coil - Master Read-Write
//    modbus.addCoil(i);

//
//  // Use addHreg() for analog outpus - Holding Register - Master Read-Write
//    modbus.addHreg(i, i)



  //CREATE INPUT REGISTERS
  modbus.addIreg(TEMP_IP, 0);
  modbus.addIreg(VOLTAGE_IP, 0);
  modbus.addIreg(CURRENT_IP, 0);
  modbus.addIreg(LDR_IP, 0);


  //CREATE HOLDING REGISTERS

  modbus.addHreg(BRIGHTNESS_H, 50);
  modbus.addHreg(CHIP_TEMP_H, 0);

  for(int i=VLC_STR_LEN; i<(VLC_STR_LEN+VLC_START_H); i++)
      modbus.addHreg(i, '\0');
  ///Initialize to send CPS
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
      //UPDATE BRIGHTNESS
      //UPDATE BRIGHTNESS OF LAMP
        int b=modbus.Hreg(BRIGHTNESS_H);
        b=(b/10);
        if (b>10) b=10;
        if(b<0) b=0;
        if(oldb!=b);                           //CHANGE BRIGHTNESS ONLY IF IT HAS CHANGED
        {
        oldb=b;

        Wire.beginTransmission(LAMP_I2C_ADDR);
        Wire.write(data1[b], 2);
        Wire.endTransmission();


        Wire.beginTransmission(LAMP_I2C_ADDR);
        Wire.write(data2[b], 2);
        Wire.endTransmission();
        }



      int i=0;                          //UPDATE VLC STRING

      do{
          data[i]=char(modbus.Hreg(i+VLC_STR_LEN));
          i++;
      }while(modbus.Hreg(i)!=0);   //Read String from MODBUS Stack

      interleaver(data);
      manchester(data, interout, manout);
      foo(data, manout, final);
  }

  send_vlc_data(data);//SEND ENCODED VLC STRING

  if (millis() > update_time + 2000)
  {   update_time = millis();
      modbus.Ireg(LDR_IP, analogRead(LDR_PIN));
      modbus.Ireg(TEMP_IP, analogRead(TEMP_PIN));
      modbus.Ireg(VOLTAGE_IP, analogRead(VOLTAGE_PIN));
      modbus.Ireg(CURRENT_IP, analogRead(CURRENT_PIN));
      modbus.Hreg(CHIP_TEMP_H, analogRead(TEMPSENSOR));
  }

}





















