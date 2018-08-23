#import serial
import time
#import pymodbus
#from pymodbus.pdu import ModbusRequest
from pymodbus.client.sync import ModbusSerialClient as ModbusClient 
#from pymodbus.transaction import ModbusRtuFramer
#import sys
#import paho.mqtt.client as mqtt
import msvcrt

# from multiprocessing.dummy import Pool
# pool = Pool(2)

import time
# import RPi.GPIO as GPIO
# max485enPin = 12
# GPIO.setmode(GPIO.BOARD)
# GPIO.setup(max485enPin, GPIO.OUT)


# def max485txen():
#     state=GPIO.HIGH
#     GPIO.output(max485enPin, state)
#     time.sleep(0.3)
#     state=GPIO.LOW
#     GPIO.output(max485enPin, state)

mbclient= ModbusClient(method = "rtu", timeout=1, port='COM5',stopbits = 1, bytesize = 8, parity = 'N', baudrate = 19200)
connection = mbclient.connect()
print(connection)
time.sleep(5)

#ALL SENSOR DATA ARE IN INPUT REGISTERS

temp_adc_addr=3000          
voltage_adc_addr=3001
current_adc_addr=3002
ldr_addr=3003

#CHIP TEMPERATURE IN HOLDING REGISTER
chip_temp_adc_adr=3000
brightness_reg_adr=2000
vlc_data_reg_ard=1000

#VLC ENABLE 
lamp_enable_addr=4000
vlc_enable_addr=4001

while (1):
    
    result=mbclient.read_input_registers(temp_adc_addr, unit=1)
    temp_adc=result.registers[0]
    result=mbclient.read_input_registers(voltage_adc_addr, unit=1)
    voltage_adc=result.registers[0]
    result=mbclient.read_input_registers(current_adc_addr, unit=1)
    current_adc=result.registers[0]
    result=mbclient.read_input_registers(ldr_addr, unit=1)
    ldr=result.registers[0]
    result=mbclient.read_holding_registers(chip_temp_adc_adr, unit=1)
    chip_temp_adc=result.registers[0]
    temp=(temp_adc*75)/4096
    voltage=(voltage_adc*15.18)/4096
    current=(current_adc*5.5)/4096
    chip_temp=(1475 - ((2475*chip_temp_adc)/4096))/10
    print("=====================================================================")
    print("Temperature: "+str(round(temp,2))+"°C. ADC value: "+str(temp_adc)+".")
    print("Voltage: "+str(round(voltage,2))+"V. ADC value: "+str(voltage_adc)+".")
    print("Current: "+str(round(current,2))+"A. ADC value: "+str(current_adc)+".")
    print("LDR ADC value: "+str(ldr)+".")
    print("Chip Temperature: "+str(round(chip_temp,2))+"°C. ADC value: "+str(chip_temp_adc)+".")
    print("Time: ", time.time())
    time.sleep(3)
    if msvcrt.kbhit():
        print("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
        op=input("1 - Change brightness\n2 - Change VLC status\n3 - Change VLC data\nSelect option number: ")
        op=int(op)
        if op==1:
            br=input("Enter Brightness Value: ")
            br=int(br)
            # print(br)
            print(mbclient.write_registers(brightness_reg_adr, br, unit=1))
        elif op==2:
            vlcstaus=input("Enter 1 to enable VLC or 0 to disable vlc: ")
            vlc_en=1
            if int(vlcstaus)<1:
                vlc_en=0
            print(mbclient.write_register(vlc_enable_addr, vlc_en, unit=1))

        elif op==3:
            vlcdata=input("Enter new VLC data string: ")
            arrdata=list(vlcdata)
            for i in range(len(arrdata)):
                intdata=int((ord(arrdata[i])))
                print(arrdata[i])
                mbclient.write_register(vlc_data_reg_ard+i, intdata, unit=1)
            mbclient.write_register(vlc_data_reg_ard+i+1, 0, unit=1)
        else:
            print("Invalid option!!")
            continue



            
#def send_cmd(x):
#    result=mbclient.write_register(0, x, unit=1)
#    print(result)
#    
#def on_message(client, userdata, message):
#    msg=str(message.payload.decode("utf-8"))
#    print("message received " ,msg)
#    print("message topic=",message.topic)
#    print("message qos=",message.qos)
#    print("message retain flag=",message.retain)
#    try:msg=int(msg)
#    except: return
#    send_cmd(msg)
#    
#
#broker_address="localhost"
#print("creating new instance")
#client = mqtt.Client("P1") #create new instance
#client.on_message=on_message #attach function to callback
#print("connecting to broker")
#client.connect(broker_address) #connect to broker
##client.loop_start() #start the loop
#print("Subscribing to topic","actuate/modbus/ac")
#client.subscribe("actuate/modbus/ac")
##time.sleep(1) # wait
#client.loop_forever() #stop the loop
##mbclient.close()
