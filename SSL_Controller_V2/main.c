//*****************************************************************************
// To control the SSL V2.0
//      - Brightness control over I2C GPIO Expander
//      - VLC data
//      - Current, Voltage, Temperature, Brightness measurement
//      - Communication over UART 2 - via MAX485
//
//*****************************************************************************

#include <stdarg.h>
//#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/adc.h"
#include "inc/hw_timer.h"
#include "inc/hw_types.h"
#include "inc/hw_sysctl.h"

uint8_t flg1=0,flg2=0;

uint8_t div1add=0x20;

uint32_t pui32ADC0Value[4];
uint32_t pui32ADC1Value[4];
uint32_t chiptemp=0;
uint64_t Period=80; // for delay of 1uS with 16MHz clock

uint8_t interout[30][8], manout[30][16], final[30][56];
char rbuf[10];



//void power_on(void);
//void InitI2C0(void);
//void InitConsole_A(void);
//void LED_startup(void);

char *data_1="CPS\0";

void timer_init(void){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0))
    {
    }
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);// TIMER_CFG_ONE_SHOT
    //TimerPrescaleSet(TIMER0_BASE,TIMER_A,1);
    //TimerEnable(TIMER0_BASE, TIMER_A);


}

void us_delay(uint64_t i){

    //TimerIntClear(TIMER0_BASE,TIMER_TIMA_TIMEOUT);

    //Period = (SysCtlClockGet());
    //TimerLoadSet(TIMER0_BASE,TIMER_A,((Period*i) -1));
    HWREG(TIMER0_BASE + TIMER_O_TAILR) = ((80*i) -1);
    HWREG(TIMER0_BASE + TIMER_O_IMR) |= TIMER_TIMA_TIMEOUT;
   // TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    HWREG(TIMER0_BASE + TIMER_O_CTL) |= TIMER_A & (TIMER_CTL_TAEN |
                                                     TIMER_CTL_TBEN);
   // TimerEnable(TIMER0_BASE, TIMER_A);
    //TimerIntStatus(TIMER0_BASE,false);

    HWREG(TIMER0_BASE + TIMER_O_ICR) = TIMER_TIMA_TIMEOUT;

    while(!HWREG(TIMER0_BASE + TIMER_O_RIS))
           {

           }
    //TimerIntClear(TIMER0_BASE,TIMER_TIMA_TIMEOUT);
    //}
    //TimerDisable(TIMER0_BASE, TIMER_A);
}

//void us_delay(uint32_t i){
//
//    SysCtlDelay(i * (SysCtlClockGet() / 3 / 1000000));
//}

void power_on(void){

    // for on board LEDs

            GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1, GPIO_PIN_1); // red
    SysCtlDelay(SysCtlClockGet() / 10);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3, GPIO_PIN_3); // green
    SysCtlDelay(SysCtlClockGet() / 10);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2, GPIO_PIN_2); // blue
    SysCtlDelay(SysCtlClockGet() / 10);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1, 0);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3, 0);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_2, 0);
}

void UART_handler(){

    uint32_t x=0;

       // rbuf=UARTCharGetNonBlocking(UART0_BASE);

//        if (rbuf=="b"){
//            UARTprintf("Enter Brightness from 1 - 10 \n");

//            uint8_t val = UARTCharGet(UART0_BASE);
//            LED_brightness(val*10);
//            UARTprintf("Brightness set to %d  \n", val);

//        }
        //UARTprintf("got something = %d \n",rbuf);
        //UARTCharPutNonBlocking(UART0_BASE,UARTCharGetNonBlocking(UART0_BASE));
    while(UARTCharsAvail(UART1_BASE)) //loop while there are chars
    {

        rbuf[x]=UARTCharGetNonBlocking(UART1_BASE);
    //UARTCharPutNonBlocking(UART0_BASE, UARTCharGetNonBlocking(UART0_BASE)); //echo character
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2); //blink LED
    //SysCtlDelay(SysCtlClockGet() / (1000 * 3)); //delay ~1 msec
    us_delay(1000);
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0); //turn off LED
    x++;
    }

    //UARTprintf("got something = %s \n",rbuf);


    if (flg1==1){
        //UARTprintf("Brightness is set to %s  \n",rbuf);
        uint8_t b = atoi (rbuf);
        if (b==99){b=100;}
        LED_brightness(b);
        flg1=0;
    }


    if (rbuf[0]=='b' & rbuf[1]=='r'){

//
//        GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, GPIO_PIN_5); // DE made High to start sending data
//        GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4, GPIO_PIN_4); // RE made High to stop receiving data
//
//        //UARTprintf("Enter Brightness from 10 - 100 \n");
//        while(UARTBusy(UART1_BASE)){
//
//            }
//
//        GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, 0); // DE made Low to stop sending data
//        GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4, 0); // RE made Low to start receiving data


        flg1=1;

    }

if (flg2== 1){

    uint8_t c=0;


//    GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, GPIO_PIN_5); // DE made High to start sending data
//    GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4, GPIO_PIN_4); // RE made High to stop receiving data
//
//
//    UARTprintf("String \" %s \" is beeing transmitted \n", rbuf);
//    while(UARTBusy(UART1_BASE)){
//
//        }
//
//    GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, 0); // DE made Low to stop sending data
//    GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4, 0); // RE made Low to start receiving data


    for (c=0;c<strlen(rbuf);c++){

    //data_1 = rbuf[c];
    strcpy(data_1,rbuf);
    //data_1++;
    //rbuf++;
    }
    UARTIntDisable(UART1_BASE,UART_INT_RX);
                    UARTFIFOLevelSet(UART1_BASE,UART_FIFO_TX7_8,UART_FIFO_RX1_8);
                    UARTIntEnable(UART1_BASE,UART_INT_RX);

    flg2=0;
}

    if (rbuf[0]=='v' & rbuf[1]=='l'){
        //UARTIntRegister(UART0_BASE,UART_handler);
                UARTIntDisable(UART1_BASE,UART_INT_RX);
                UARTFIFOLevelSet(UART1_BASE,UART_FIFO_TX7_8,UART_FIFO_RX4_8);
                UARTIntEnable(UART1_BASE,UART_INT_RX);

//
//                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, GPIO_PIN_5); // DE made High to start sending data
//                GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4, GPIO_PIN_4); // RE made High to stop receiving data
//
//
//           UARTprintf("Type the String for VLC \n");
//           while(UARTBusy(UART1_BASE)){
//
//               }
//
//           GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, 0); // DE made Low to stop sending data
//           GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4, 0); // RE made Low to start receiving data

           flg2=1;

       }

    if (rbuf[0]=='d' & rbuf[1]=='t'){

            //flg3=1;
            //char buf[50],i[10],t[10],l[10];
            get_adc_data();
//            chiptemp= 147.5 - ((75 * (3.3 - 0) * pui32ADC0Value[3]) / 4096);
//            float volt=(pui32ADC0Value[1]*15.18) / (4096); //(ADC/4096)*(Vcc/Gain)*(15.18/3.3)
//            float curr=(pui32ADC0Value[0]*5.5) / (4096); // (ADC/4096)*(Vcc/Gain)*(1/0.05)
//            float temp1=(pui32ADC1Value[0]*100) / (4096); // (ADC/4096)*(Vcc/Gain)*(1/10mV)
//            float ldr= pui32ADC1Value[1];
//            // pui32ADC0Value[2] -- LDR-2
//            // pui32ADC1Value[0] -- Temp
//            // pui32ADC1Value[1] -- LDR-1
//            // pui32ADC1Value[2] -- PD
//
//            //gcvt(volt, 2, v);
//
//            sprintf(buf,"%.2f,%.2f,%.2f,%.2f,%.2f",chiptemp,volt,curr,temp1,ldr);
//            UARTprintf("%s \n",buf);
            us_delay(1000);
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3, GPIO_PIN_3);
            GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, GPIO_PIN_5); // DE made High to start sending data
            GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4, GPIO_PIN_4); // RE made High to stop receiving data
            us_delay(10000);
            UARTprintf("%d,%d,%d,%d,%d, \n", pui32ADC0Value[3],pui32ADC0Value[1],pui32ADC0Value[0],pui32ADC1Value[0],pui32ADC1Value[1] );
            //UARTprintf("%d,%d,%d,%d \n", pui32ADC1Value[0],pui32ADC1Value[1],pui32ADC1Value[2],pui32ADC1Value[3] );
            while(UARTBusy(UART1_BASE)){

                }

            GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, 0); // DE made Low to stop sending data
            GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4, 0); // RE made Low to start receiving data
            GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3, 0);
            //UARTprintf("ADC data sent \n");

    }


    UARTIntClear(UART1_BASE,UART_INT_RX);

        //rbuf=0;
}

void
InitConsole_A(void)
{
    //
    // Enable GPIO port A which is used for UART0 pins.
    // TODO: change this to whichever GPIO port you are using.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Configure the pin muxing for UART0 functions on port A0 and A1.
    // This step is not necessary if your part does not support pin muxing.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);

    //
    // Enable UART0 so that we can configure the clock.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Select the alternate (UART) function for these pins.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
    //UARTIntRegister(UART0_BASE,UART_handler);
}


void
InitConsole_B(void)
{
    //
    // Enable GPIO port A which is used for UART0 pins.
    // TODO: change this to whichever GPIO port you are using.
    //


    //
    // Configure the pin muxing for UART0 functions on port A0 and A1.
    // This step is not necessary if your part does not support pin muxing.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);

    //
    // Enable UART0 so that we can configure the clock.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);

    //
    // Select the alternate (UART) function for these pins.
    // TODO: change this to select the port/pin you are using.
    //
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(1, 115200, 16000000);
}


//initialize I2C module 0
//Slightly modified version of TI's example code
void InitI2C0(void)
{
    //enable I2C module 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);

    //
    // Wait for the I2C0 module to be ready.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_I2C1))
    {
    }
    //reset module
    //SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    //enable GPIO peripheral that contains I2C 1


    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    GPIOPinConfigure(GPIO_PA6_I2C1SCL);
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);

    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);

    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);

    //clear I2C FIFOs
    //HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
}

//sends an I2C command to the specified slave
void I2CSend(uint8_t slave_addr, uint8_t num_of_args, ...)
{
    uint8_t i;
    // Tell the master module what address it will place on the bus when
    // communicating with the slave.
    I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, false);

    //stores list of variable number of arguments
    va_list vargs;

    //specifies the va_list to "open" and the last fixed argument
    //so vargs knows where to start looking
    va_start(vargs, num_of_args);

    //put data to be sent into FIFO
    I2CMasterDataPut(I2C1_BASE, va_arg(vargs, uint32_t));

    //if there is only one argument, we only need to use the
    //single send I2C function
    if(num_of_args == 1)
    {
        //Initiate send of data from the MCU
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);

        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C1_BASE));

        //"close" variable argument list
        va_end(vargs);
    }

    //otherwise, we start transmission of multiple bytes on the
    //I2C bus
    else
    {
        //Initiate send of data from the MCU
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);

        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C1_BASE));

        //send num_of_args-2 pieces of data, using the
        //BURST_SEND_CONT command of the I2C module
        for( i = 1; i < (num_of_args - 1); i++)
        {
            //put next piece of data into I2C FIFO
            I2CMasterDataPut(I2C1_BASE, va_arg(vargs, uint32_t));
            //send next data that was just placed into FIFO
            I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

            // Wait until MCU is done transferring.
            while(I2CMasterBusy(I2C1_BASE));
        }

        //put last piece of data into I2C FIFO
        I2CMasterDataPut(I2C1_BASE, va_arg(vargs, uint32_t));
        //send next data that was just placed into FIFO
        I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C1_BASE));

        //"close" variable args list
        va_end(vargs);
    }
}

//read specified register on slave device
uint32_t I2CReceive(uint32_t slave_addr, uint8_t reg)
{
    //specify that we are writing (a register address) to the
    //slave device
    I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, false);

    //specify register to be read
    I2CMasterDataPut(I2C1_BASE, reg);

    //send control byte and register address byte to slave device
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C1_BASE));

    //specify that we are going to read from slave device
    I2CMasterSlaveAddrSet(I2C1_BASE, slave_addr, true);

    //send control byte and read from the register we
    //specified
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C1_BASE));

    //return data pulled from the specified register
    return I2CMasterDataGet(I2C1_BASE);
}

// initialize  ADC0 and ADC1 modules
void adc_init(){
    //
     // The ADC0 are enabled for use.
     //
     SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);  // pin PE3
     SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);  // pin PE2

     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); // for adc input and gpio inputs
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); // for adc input and gpio inputs
     //
        // Select the analog ADC function for these pins.
        // Consult the data sheet to see which functions are allocated per pin.
        // TODO: change this to select the port/pin you are using.
        //
        //GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_0);
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_1);  // LDR  2                 -- AIN2
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);  // voltage                -- AIN1
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);  // current                -- AIN0
        GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_1);  // Temperature - LM35     -- AIN6
        GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_2);  // LDR   1                -- AIN5
        GPIOPinTypeADC(GPIO_PORTD_BASE, GPIO_PIN_3);  // BPW34                  -- AIN4



        ADCSequenceDisable(ADC0_BASE, 2);  // disable the sequencer before configuring
        ADCSequenceDisable(ADC1_BASE, 2);
        //
        // Enable sample sequence 3 with a PWM signal trigger.  Sequence 3
        // will do a single sample when the processor sends a signal to start the
        // conversion.  Each ADC module has 4 programmable sequences, sequence 0
        // to sequence 3.  This example is arbitrarily using sequence 3.
        //
        //PWMGenIntTrigEnable(PWM0_BASE,PWM_GEN_0,PWM_TR_CNT_ZERO);
        ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 1);
        //ADCSequenceConfigure(ADC1_BASE, 2,ADC_TRIGGER_PWM0 |  ADC_TRIGGER_PWM_MOD0, 1);
        ADCSequenceConfigure(ADC1_BASE, 2,ADC_TRIGGER_PROCESSOR, 1);
        //
        // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
        // single-ended mode (default) and configure the interrupt flag
        // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
        // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
        // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
        // sequence 0 has 8 programmable steps.  Since we are only doing a single
        // conversion using sequence 3 we will only configure step 0.  For more
        // information on the ADC sequences and steps, reference the datasheet.
        //
        //ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_CH0 | ADC_CTL_CH2 | ADC_CTL_CH3 | ADC_CTL_IE |
         //                        ADC_CTL_END);
        ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH0);
        ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_CH1);
        ADCSequenceStepConfigure(ADC0_BASE, 2, 2, ADC_CTL_CH2);
        ADCSequenceStepConfigure(ADC0_BASE, 2, 3, ADC_CTL_TS| ADC_CTL_IE |
                                 ADC_CTL_END);    // data from on chip temp sensor




        //ADCSequenceStepConfigure(ADC1_BASE, 2, 2, ADC_CTL_CH1 |ADC_CTL_CH8| ADC_CTL_CH9 | ADC_CTL_IE |
          //                       ADC_CTL_END);
        ADCSequenceStepConfigure(ADC1_BASE, 2, 0, ADC_CTL_CH6);
        ADCSequenceStepConfigure(ADC1_BASE, 2, 1, ADC_CTL_CH5);
        ADCSequenceStepConfigure(ADC1_BASE, 2, 2, ADC_CTL_CH4);
 //       ADCSequenceStepConfigure(ADC1_BASE, 2, 3,  ADC_CTL_TS | ADC_CTL_IE |
 //                                           ADC_CTL_END);


        //SysCtlADCSpeedSet(SYSCTL_ADCSPEED_250KSPS);

        // Since sample sequence 3 is now configured, it must be enabled.
        //
        ADCSequenceEnable(ADC0_BASE, 2);
        ADCSequenceEnable(ADC1_BASE, 2);

        ADCSequenceDataGet(ADC0_BASE, 2, pui32ADC0Value);
        ADCSequenceDataGet(ADC1_BASE, 2, pui32ADC1Value);


        ADCIntClear(ADC0_BASE, 2);
        ADCIntClear(ADC1_BASE, 2);

        ADCIntEnable(ADC0_BASE,2);
        ADCIntEnable(ADC1_BASE,2);
        IntMasterEnable();

        //ADCProcessorTrigger(ADC0_BASE, 3);

}


void get_adc_data(void){

    //
           // Trigger the ADC conversion.
           //
           ADCProcessorTrigger(ADC0_BASE, 2);
           ADCProcessorTrigger(ADC1_BASE, 2);

           //
           // Wait for conversion to be completed.
           //
           while(!(ADCIntStatus(ADC0_BASE, 2, false) | ADCIntStatus(ADC1_BASE, 2, false)))
           {

           }


           //
           // Clear the ADC interrupt flag.
           //
           ADCIntClear(ADC0_BASE, 2);
           ADCIntClear(ADC1_BASE, 2);

           //
           // Read ADC Value.
           //
           ADCSequenceDataGet(ADC0_BASE, 2, pui32ADC0Value);
           ADCSequenceDataGet(ADC1_BASE, 2, pui32ADC1Value);



}


void LED_brightness(uint8_t b){

    /*
     * Brightness levels
     *
     * 10%  -- string  1
     * 20%  -- strings 2,3
     * 30%  -- strings 4,5,6 OR 1,2,3
     * 40%  -- strings 7,8,9,10 OR 1,4,5,6
     * 50%  -- strings 1,7,8,9,10 OR 2,3,4,5,6
     * 60%  -- strings 2,3,7,8,9,10 OR 1,2,3,4,5,6
     * 70%  -- strings 1,2,3,7,8,9,10
     * 80%  -- strings 1,4,5,6,7,8,9,10
     * 90%  -- strings 2,3,4,5,6,7,8,9,10
     * 100% -- strings 1 to 10
     */

    /*LED string No. --- MCP23017 pin No.
     * string 1  ---D12 --- GPB3
     * string 2  ---D13 --- GPB4
     * string 3  ---D14 --- GPB5
     * string 4  ---D9  --- GPB0
     * string 5  ---D8  --- GPA7
     * string 6  ---D15 --- GPB6
     * string 7  ---D16 --- GPB7
     * string 8  ---D5  --- GPA4
     * string 9  ---D7  --- GPA6
     * string 10 ---D6  --- GPA5
     */
    if(b<100){
    b=((b/10)%10)*10;
    }
    else
        b=100;



    GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, GPIO_PIN_5); // DE made High to start sending data
    GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4, GPIO_PIN_4); // RE made High to stop receiving data


   // UARTprintf("Brightness is set to %d  \n",b);
    b=100-b;

    GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, 0); // DE made Low to stop sending data
    GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4, 0); // RE made Low to start receiving data

    I2CSend(div1add, 2,0x00,0x00); // Configure port A as output
    I2CSend(div1add, 2,0x01,0x00); // Configure port B as output
    switch(b){

    case 0:
        // all are disabled

        I2CSend(div1add, 2,0x14,0x00);
        I2CSend(div1add, 2,0x15,0x00);
        break;

    case 10:
        // stirng 1 ON  GPB3

        I2CSend(div1add, 2,0x14,0x00);
        I2CSend(div1add, 2,0x15,0x08);
        break;

    case 20:
        // stirng 1 ON  GPB 4,5

        I2CSend(div1add, 2,0x14,0x00);
        I2CSend(div1add, 2,0x15,0x30);
        break;

    case 30:
            // stirng 1 ON  GPB 0,6,GPA7

            I2CSend(div1add, 2,0x14,0x80); // A7
            I2CSend(div1add, 2,0x15,0x41); // B0,6
            break;

    case 40:
            // stirng 1 ON  GPB 7,GPA 4,6,5

            I2CSend(div1add, 2,0x14,0x70); // A4,,5,6
            I2CSend(div1add, 2,0x15,0x80);  // B7
            break;

    case 50:
            // stirng 1 ON  GPB 3,7,GPA 4,6,5

            I2CSend(div1add, 2,0x14,0x70);
            I2CSend(div1add, 2,0x15,0x88);
            break;

    case 60:
            // stirng 1 ON  GPB 4,5,7 GPA 4,6,5

            I2CSend(div1add, 2,0x14,0x70);
            I2CSend(div1add, 2,0x15,0xB0);
            break;

    case 70:
               // stirng 1 ON  GPB 3,4,5,7 GPA 4,6,5

            I2CSend(div1add, 2,0x14,0x70);
            I2CSend(div1add, 2,0x15,0xB8);
            break;

    case 80:
            // stirng 1 ON  GPB 0,3,6,7 GPA 4,5,6,7

            I2CSend(div1add, 2,0x14,0xF0);
            I2CSend(div1add, 2,0x15,0xC9);
            break;

    case 90:
            // stirng 1 ON  GPB 0,4,5,6,7 GPA 4,5,6,7

            I2CSend(div1add, 2,0x14,0xF0);
            I2CSend(div1add, 2,0x15,0xF1);
            break;

    case 100:
            // stirng 1 ON  GPB 0,3,4,5,6,7 GPA 4,5,6,7

            I2CSend(div1add, 2,0x14,0xF0);
            I2CSend(div1add, 2,0x15,0xF9);
            break;
    default:
        // all are disabled -- Negative logic

        I2CSend(div1add, 2,0x14,0xF0);
        I2CSend(div1add, 2,0x15,0xF9);
        break;

    }


}

void LED_startup(){
// to initialise the the LEDs in increasing brightness levels
    //SysCtlDelay(SysCtlClockGet()/10);
    uint8_t k;
    for (k=0;k<110;k=k+10){
    //UARTprintf("Brighness value %d \n",k );
    LED_brightness(k);
    us_delay(500000);
    }
}




// LIFI part

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


char* manchester(char* s, uint8_t interout[][8], uint8_t manout[][16]){
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


char* foo(char* s, uint8_t manout[][16], uint8_t final[][56]){
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
    //uint8_t interout[80][8], manout[80][16], final[80][56];
    interleaver(data);
    manchester(data, interout, manout);
    foo(data, manout, final);
    int i=0,j=0;
    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_6, GPIO_PIN_6);
    us_delay(150);
    i=0;
     while(*data++ != '\0'){
        for (j = 0; j < 56; j++) {
                if (final[i][j]==1){

                    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_6, 0);
                    us_delay(50);
                }
                else if(final[i][j]==0){

                    GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_6, GPIO_PIN_6);
                    us_delay(50);
                }
            }
        i++;
    }
     GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_6, GPIO_PIN_6);
     us_delay(150);
     GPIOPinWrite(GPIO_PORTB_BASE,GPIO_PIN_6, 0);
}
// Main
void main(void){

    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    timer_init();
    power_on();
    InitConsole_B();
    InitI2C0();
    LED_startup();

    adc_init();


    UARTIntRegister(UART1_BASE,UART_handler);
    UARTFIFOLevelSet(UART1_BASE,UART_FIFO_TX7_8,UART_FIFO_RX1_8);
    UARTIntEnable(UART1_BASE,UART_INT_RX);//UART_INT_RX);
    IntMasterEnable();


              GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_6);

              GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_4 |GPIO_PIN_5); // DE -- Active High
                  // RE -- Active Low


   //us_delay(1000);
  LED_brightness(100);
while(1){

   us_delay(1000);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1, GPIO_PIN_1); // red
   us_delay(1000);
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1, 0); // red
    send_vlc_data(data_1);

//    GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5, GPIO_PIN_5); // DE made High to start sending data
//    GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4, GPIO_PIN_4); // RE made High to stop receiving data
//
//    UARTprintf("Clock %d \n",SysCtlClockGet() );
//    while(UARTBusy(UART1_BASE)){
//
//    }
//    //us_delay(1000);
//    GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_5,0); // DE made High to start sending data
//    GPIOPinWrite(GPIO_PORTE_BASE,GPIO_PIN_4, 0); // RE made High to stop receiving data

}

}
