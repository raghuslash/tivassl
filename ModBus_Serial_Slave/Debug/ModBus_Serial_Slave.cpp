#include "Energia.h"

#line 1 "C:/Users/admin/workspace_v8/ModBus_Serial_Slave/ModBus_Serial_Slave.ino"

#include <Modbus.h>
#include <ModbusSerial.h>
#include <avr/pgmspace.h>
#include "q3.h"
#include "cl1.h"
#include "cond1.h"
#include <Wire.h>
#include <vlcfunctions.h>


#define BAUD 19200
#define ID 2


#define TXPIN 6 
#define REPIN 5 

#define VLC_STR_LEN 100       
#define VLC_MODULATION_PIN 14 

int modbus_data_available();
void setup();
void loop();
void led_brightness(int b);
void setup_gpio();
void modulate_vlc();
void startup_blink();

#line 22
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
#define CHIP_TEMP_ADC_H 3000
#define CHIP_TEMP_H 3001

#define LAMP_ON_H 4000
#define VLC_ON_H 4001               


bool sendVLC = true;               
bool lampON = true;                 


byte ioconf1[2] = {0x00, 0x00}; 
byte ioconf2[2] = {0x01, 0x00};


byte data1[11][2] = {{0x14, 0xF0}, {0x14, 0xF0}, {0x14, 0xF0}, {0x14, 0x70}, {0x14, 0x70}, {0x14, 0x70}, {0x14, 0x70}, {0x14, 0x80}, {0x14, 0x00}, {0x14, 0x00}, {0x14, 0x00}};
byte data2[11][2] = {{0x15, 0xF9}, {0x15, 0xF1}, {0x15, 0xC9}, {0x15,0xB8},
                     {0x15, 0xB0},
                     {0x15, 0x88},
                     {0x15, 0x80},
                     {0x15, 0x41},
                     {0x15, 0x30},
                     {0x15, 0x08},
                     {0x15, 0x00}};

int oldb = 0;
char data[VLC_STR_LEN] = "CPS"; 
char olddata[VLC_STR_LEN] = "CPS";
char interout[VLC_STR_LEN][8], manout[VLC_STR_LEN][16], final[VLC_STR_LEN][56];

HardwareSerial *ModbusSerialPort = &Serial1; 

int modbus_data_available()
{
    return ModbusSerialPort->available();
}

void setup()
{

    

    modbus.config(ModbusSerialPort, BAUD, TXPIN);       

    
    modbus.setSlaveId(ID);

    

    

    

    

    
    

    
    
    

    
    modbus.addIreg(TEMP_IP, 0);
    modbus.addIreg(VOLTAGE_IP, 0);
    modbus.addIreg(CURRENT_IP, 0);
    modbus.addIreg(LDR_IP, 0);

    

    modbus.addHreg(BRIGHTNESS_H, 50);
    modbus.addHreg(CHIP_TEMP_H, 0);

    modbus.addHreg(VLC_ON_H, sendVLC);      
    modbus.addHreg(LAMP_ON_H, lampON);

    for (int i = VLC_STR_LEN; i < (VLC_STR_LEN + VLC_START_H); i++)
        modbus.addHreg(i, '\0');

    
    modbus.Hreg(1000, 'C');
    modbus.Hreg(1001, 'P');
    modbus.Hreg(1002, 'S');


    Wire.setModule(1);
    Wire.begin();
    setup_gpio();
    delay(1);
    startup_blink();
    led_brightness(50); 

    pinMode(VLC_MODULATION_PIN, OUTPUT);            
    digitalWrite(VLC_MODULATION_PIN, LOW);
    modulate_vlc(); 

}

int adc_temp, chip_temp;            

void loop()
{
    pinMode(VLC_MODULATION_PIN, OUTPUT);                
    digitalWrite(VLC_MODULATION_PIN, LOW);              

    if (modbus_data_available())
    {

        if (!sendVLC) 
        {
            pinMode(VLC_MODULATION_PIN, OUTPUT);
            digitalWrite(VLC_MODULATION_PIN, LOW);      
        }


        else if (lampON)
        {
            pinMode(VLC_MODULATION_PIN, OUTPUT);
            analogWrite(VLC_MODULATION_PIN, 43);          
        }


        modbus.task();
        

        pinMode(VLC_MODULATION_PIN, OUTPUT);                 
        digitalWrite(VLC_MODULATION_PIN, LOW);              
        if (modbus.Hreg(VLC_ON_H) > 0)
            sendVLC = true;
        else
            sendVLC = false;

        if (modbus.Hreg(LAMP_ON_H) > 0)
            lampON = true;
        else
            lampON = false;

        
        int b = modbus.Hreg(BRIGHTNESS_H);
        if (lampON)
            led_brightness(b);
        else
            led_brightness(0);

        
        int i = 0;

        do
        {
            data[i] = char(modbus.Hreg(i + VLC_START_H));
            i++;
        } while (modbus.Hreg(i) != '\0'); 

        data[i] = '\0';

        if (data[0] == '\0')
        {
            sendVLC = 0;                
            modbus.Coil(VLC_ON_H, 0);   
        }

        if (olddata != data)             
        {

            modulate_vlc();
            i = 0;
            while (data[i] != '\0')
            {
                olddata[i] = data[i];
                i++;
            }
            olddata[i]='\0';
        }

    }


    if (sendVLC && lampON)
    {
        pinMode(VLC_MODULATION_PIN, OUTPUT);
        send_vlc_data(data, final, VLC_MODULATION_PIN); 
    }

    pinMode(VLC_MODULATION_PIN, OUTPUT);        
    digitalWrite(VLC_MODULATION_PIN, LOW);      

    if (millis() > update_time + 2000)
    {
        update_time = millis();
        modbus.Ireg(LDR_IP, analogRead(LDR_PIN));
        modbus.Ireg(TEMP_IP, analogRead(TEMP_PIN));
        modbus.Ireg(VOLTAGE_IP, analogRead(VOLTAGE_PIN));
        modbus.Ireg(CURRENT_IP, analogRead(CURRENT_PIN));
        adc_temp=analogRead(TEMPSENSOR);
        modbus.Hreg(CHIP_TEMP_ADC_H, adc_temp);
        chip_temp=(1475 - ((2475*adc_temp)/4096))/10;
        modbus.Hreg(CHIP_TEMP_H, chip_temp);

    }
}

void led_brightness(int b)
{

    b = (b/10);

    if (b > 10)
        b = 10;
    if (b < 0)
        b = 0;

    if (oldb != b) 
    {
        oldb = b;
        Wire.beginTransmission(LAMP_I2C_ADDR);
        Wire.write(data1[b], 2);
        Wire.endTransmission();

        Wire.beginTransmission(LAMP_I2C_ADDR);
        Wire.write(data2[b], 2);
        Wire.endTransmission();
    }
}

void setup_gpio()
{

    Wire.beginTransmission(LAMP_I2C_ADDR);
    Wire.write(ioconf1, 2);
    Wire.endTransmission();

    Wire.beginTransmission(LAMP_I2C_ADDR);
    Wire.write(ioconf2, 2);
    Wire.endTransmission();
}

void modulate_vlc()
{
    interleaver(data, interout);
    manchester(data, interout, manout);
    foo(data, manout, final);
}

void startup_blink()
{
    pinMode(BLUE_LED,  OUTPUT);
    digitalWrite(BLUE_LED, HIGH);
    led_brightness(10);
    delay(1000);
    digitalWrite(BLUE_LED, LOW);
    led_brightness(20);
    delay(1000);
    digitalWrite(BLUE_LED, HIGH);
    led_brightness(30);
    delay(1000);
    digitalWrite(BLUE_LED, LOW);
    led_brightness(40);
    delay(1000);

}



