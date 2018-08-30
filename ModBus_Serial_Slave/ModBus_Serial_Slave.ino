
#include <Modbus.h>
#include <ModbusSerial.h>
#include <avr/pgmspace.h>
#include "q3.h"
#include "cl1.h"
#include "cond1.h"
#include <Wire.h>
#include <vlcfunctions.h>

// ModBus Port information
#define BAUD 19200
#define ID 2
//MODBUS SLAVE ID

#define TXPIN 6 //send/receive enable for MAX485  //
#define REPIN 5 //active low

#define VLC_STR_LEN 100       //MAX number of characters to send in VLC data.
#define VLC_MODULATION_PIN 14 //PIN USED TO MODULATE VLC DATA

ModbusSerial modbus;

const int ledPin = RED_LED;

unsigned long update_time = 0;
unsigned long inc_time = 0;

#define LAMP_I2C_ADDR 0x20
#define LAMP_BR_ADDR1 0x14
#define LAMP_BR_ADDR1 0x15

//INPUT REGISTER ADDRESSES
#define TEMP_IP 3000
#define TEMP_PIN A6 // connected to PD1 - A6
#define VOLTAGE_IP 3001
#define VOLTAGE_PIN A1 //connected to PE2 - A1
#define CURRENT_IP 3002
#define CURRENT_PIN A0 //connected to PE3 - A0
#define LDR_IP 3003
#define LDR_PIN A5 //connected to PD2 - A5

//HOLDING REGISTER ADDRESSES
#define VLC_START_H 1000
#define BRIGHTNESS_H 2000
#define CHIP_TEMP_ADC_H 3000
#define CHIP_TEMP_H 3001

#define LAMP_ON_H 4000
#define VLC_ON_H 4001               //Enable to start VLC


bool sendVLC = true;               //Set to true to start VLC on startup
bool lampON = true;                 //Set to true to start lamp on startup


byte ioconf1[2] = {0x00, 0x00}; //GPIO expander configs
byte ioconf2[2] = {0x01, 0x00};

//BRIGHTNESS ENCODED VALUES staring from 0 to 100 in steps of 10
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
char data[VLC_STR_LEN] = "CPS"; //Initially transmit this string
char olddata[VLC_STR_LEN] = "CPS";
char interout[VLC_STR_LEN][8], manout[VLC_STR_LEN][16], final[VLC_STR_LEN][56];

HardwareSerial *ModbusSerialPort = &Serial1; //Serial Port that MODBUS network connects to. **Change to Serial1 before deployment

int modbus_data_available()
{
    return ModbusSerialPort->available();
}

void setup()
{

    // Enable ever receive

    modbus.config(ModbusSerialPort, BAUD, TXPIN);       // Config Modbus Serial (port, speed, byte format)

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

    modbus.addHreg(VLC_ON_H, sendVLC);      //Coil to control VLC transmission
    modbus.addHreg(LAMP_ON_H, lampON);

    for (int i = VLC_STR_LEN; i < (VLC_STR_LEN + VLC_START_H); i++)
        modbus.addHreg(i, '\0');

    ///Initialize to send CPS
    modbus.Hreg(1000, 'C');
    modbus.Hreg(1001, 'P');
    modbus.Hreg(1002, 'S');


    Wire.setModule(1);
    Wire.begin();
    setup_gpio();
    delay(1);
    startup_blink();
    led_brightness(50); //SET to 50% on startup

    pinMode(VLC_MODULATION_PIN, OUTPUT);            //Disable Modulation on Startup
    digitalWrite(VLC_MODULATION_PIN, LOW);
    modulate_vlc(); // Do VLC modulation

}

int adc_temp, chip_temp;            //variables for adc conversion

void loop()
{
    pinMode(VLC_MODULATION_PIN, OUTPUT);                //Reset Brightness to 100
    digitalWrite(VLC_MODULATION_PIN, LOW);              // Pull modulation low and turn on the LEDs. (!! It is dangerous to keep the LEDs off for > 500us)

    if (modbus_data_available())
    {

        if (!sendVLC) //Check if VLC is being sent
        {
            pinMode(VLC_MODULATION_PIN, OUTPUT);
            digitalWrite(VLC_MODULATION_PIN, LOW);      // Pull modulation low and turn on the LEDs. (!! It is dangerous to keep the LEDs off for > 500us)
        }


        else if (lampON)
        {
            pinMode(VLC_MODULATION_PIN, OUTPUT);
            analogWrite(VLC_MODULATION_PIN, 43);          //SETS TIME - LED IS OFF (Library has 490Hz PWM Frequency.)
        }


        modbus.task();
        //UPDATE BRIGHTNESS

        pinMode(VLC_MODULATION_PIN, OUTPUT);                 //Reset Brightness to 100
        digitalWrite(VLC_MODULATION_PIN, LOW);              // Pull modulation low and turn on the LEDs. (!! It is dangerous to keep the LEDs off for > 500us)
        if (modbus.Hreg(VLC_ON_H) > 0)
            sendVLC = true;
        else
            sendVLC = false;

        if (modbus.Hreg(LAMP_ON_H) > 0)
            lampON = true;
        else
            lampON = false;

        //UPDATE BRIGHTNESS OF LAMP
        int b = modbus.Hreg(BRIGHTNESS_H);
        if (lampON)
            led_brightness(b);
        else
            led_brightness(0);

        //UPDATE VLC STRING
        int i = 0;

        do
        {
            data[i] = char(modbus.Hreg(i + VLC_START_H));
            i++;
        } while (modbus.Hreg(i) != '\0'); //Read String from MODBUS Stack

        data[i] = '\0';

        if (data[0] == '\0')
        {
            sendVLC = 0;                // IF first char is NULL, Turn off VLC.
            modbus.Coil(VLC_ON_H, 0);   //Reset VLC status coil
        }

        if (olddata != data)             //Update modulation if data has changed.
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
        send_vlc_data(data, final, VLC_MODULATION_PIN); //SEND ENCODED VLC STRING
    }

    pinMode(VLC_MODULATION_PIN, OUTPUT);        //Reset the Analog Write
    digitalWrite(VLC_MODULATION_PIN, LOW);      // Pull modulation low and turn on the LEDs. (!! It is dangerous to keep the LEDs off for > 500us)

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

    if (oldb != b) //CHANGE BRIGHTNESS ONLY IF IT HAS CHANGED
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
