#import serial
import time
#import pymodbus
#from pymodbus.pdu import ModbusRequest
from pymodbus.client.sync import ModbusSerialClient as ModbusClient 
#from pymodbus.transaction import ModbusRtuFramer
#import sys
#import paho.mqtt.client as mqtt
#import msvcrt
#import getch
#import RPi.GPIO as GPIO

import sys
import select

mbclient= ModbusClient(method = "rtu", timeout=1, port='/dev/ttyUSB0',stopbits = 1, bytesize = 8, parity = 'N', baudrate = 9600)
connection = mbclient.connect()
print(connection)
time.sleep(3)

#ALL SENSOR DATA ARE IN INPUT REGISTERS

temp_adc_addr=3000          
voltage_adc_addr=3001
current_adc_addr=3002
ldr_addr=3003

#CHIP TEMPERATURE IN HOLDING REGISTER
chip_temp_adc_adr=3000

while (1):
    print("=====================================================================")
    result=mbclient.read_input_registers(temp_adc_addr, unit=1)
    temp_adc=result.registers[0]
    temp=(temp_adc*100)/4096
    print("Temperature: "+str(round(temp,2))+"C. ADC value: "+str(temp_adc)+".")
    result=mbclient.read_input_registers(voltage_adc_addr, unit=1)
    voltage_adc=result.registers[0]
    voltage=(voltage_adc*15.18)/4096
    print("Voltage: "+str(round(voltage,2))+"V. ADC value: "+str(voltage_adc)+".")
    result=mbclient.read_input_registers(current_adc_addr, unit=1)
    current_adc=result.registers[0]
    current=(current_adc*5.5)/4096
    print("Current: "+str(round(current,2))+"A. ADC value: "+str(current_adc)+".")
    result=mbclient.read_input_registers(ldr_addr, unit=1)
    ldr=result.registers[0]
    print("LDR ADC value: "+str(ldr)+".")
    result=mbclient.read_holding_registers(chip_temp_adc_adr, unit=1)
    chip_temp_adc=result.registers[0]
    chip_temp=(1475 - ((2475*chip_temp_adc)/4096))/10
    print("Chip Temperature: "+str(round(chip_temp,2))+"C. ADC value: "+str(chip_temp_adc)+".")
    if (sys.stdin in select.select([sys.stdin], [], [], 0)[0]):
        line=sys.stdin.readline()
        print("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
        br=input("Enter Brightness Value: ")
        br=int(br)
        #print(br)
        mbclient.write_register(2000, br, unit=1)
        time.sleep(3)
    else:
        time.sleep(3)
        continue

