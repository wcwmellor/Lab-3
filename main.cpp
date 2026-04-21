#include "mbed.h"
#include "arm_book_lib.h"

DigitalIn enterButton(BUTTON1);
DigitalIn gasDetector(D2);
DigitalIn overTempDetector(D3);
DigitalIn aButton(D4);
DigitalIn bButton(D5);
DigitalIn cButton(D6);
DigitalIn dButton(D7);

DigitalOut alarmLed(LED1);
DigitalOut incorrectCodeLed(LED3);
DigitalOut systemBlockedLed(LED2);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);


bool alarmState = OFF;
int numberOfIncorrectCodes = 0;

bool gasalarm = false; //
bool tempalarm = false; //
bool continuousMonitoring = false; //
int monitorCounter = 0; //

void inputsInit();
void outputsInit();

void alarmActivationUpdate();
void alarmDeactivationUpdate();

void uartTask();
void availableCommands();
void monitoringTaskUpdate(); //

int main()
{
    inputsInit();
    outputsInit();
    while (true) {
        alarmActivationUpdate();
        alarmDeactivationUpdate();
        uartTask();
        monitoringTaskUpdate(); //
        ThisThread::sleep_for(10ms);  //
    }
}

void inputsInit()
{
    gasDetector.mode(PullDown);
    overTempDetector.mode(PullDown);
    aButton.mode(PullDown);
    bButton.mode(PullDown);
    cButton.mode(PullDown);
    dButton.mode(PullDown);
}

void outputsInit()
{
    alarmLed = OFF;
    incorrectCodeLed = OFF;
    systemBlockedLed = OFF;
}

void alarmActivationUpdate()
{
    if ( gasDetector || overTempDetector || gasalarm || tempalarm ) { //
        alarmState = ON;
    }
    alarmLed = alarmState;
}

void alarmDeactivationUpdate()
{
    if ( numberOfIncorrectCodes < 5 ) {
        if ( aButton && bButton && cButton && dButton && !enterButton ) {
            incorrectCodeLed = OFF;
        }
        if ( enterButton && !incorrectCodeLed && alarmState ) {
            if ( aButton && bButton && !cButton && !dButton ) {
                alarmState = OFF;
                gasalarm = false;  //
                tempalarm = false; //
                numberOfIncorrectCodes = 0;
            } else {
                incorrectCodeLed = ON;
                numberOfIncorrectCodes = numberOfIncorrectCodes + 1;
            }
        }
    } else {
        systemBlockedLed = ON;
    }
}

void uartTask()
{
    char receivedChar = '\0';
    if( uartUsb.readable() ) {
        uartUsb.read( &receivedChar, 1 );
          uartUsb.write("\r\n", 2); 
        
        switch (receivedChar) {
            case '1':
                gasalarm = !gasalarm;
                if (gasalarm){
                    uartUsb.write("Gas detected\r\n", 14);
                } else {
                    uartUsb.write("Gas alarm off\r\n", 15);
                    }
                    break;
            case '2':
                if (gasalarm){
                    uartUsb.write("Gas alarm active\r\n", 18);
                } else {
                    uartUsb.write("Gas alarm off\r\n", 15);}
                    uartUsb.write("\r\n", 2); 
                break;
            case '3':
                if (tempalarm){
                    uartUsb.write("Temperature alarm active\r\n", 25);
                } else{
                    uartUsb.write("Temperature alarm off\r\n", 22);
                }
                uartUsb.write("\r\n", 2); 
                break;
            case '4':
                tempalarm = !tempalarm;
                if (tempalarm){
                 uartUsb.write("Temperature too high\r\n", 22);
                 incorrectCodeLed = ON;  
                } else {
                  uartUsb.write("Temperature alarm off\r\n", 22);
                  incorrectCodeLed = OFF; 
                }
             break;
            case '5':
                gasalarm = false;
                tempalarm = false;
                alarmState = OFF; 
                incorrectCodeLed = OFF; 
                uartUsb.write("Alarms Reset\r\n", 14);
                break;
           case '6':
                 continuousMonitoring = !continuousMonitoring;
                 if (continuousMonitoring){
                 uartUsb.write("Continuous monitoring on\r\n", 25);
                 monitorCounter = 0;
                 uartUsb.write("\r\n", 2); 
    } else {
        uartUsb.write("Continuous monitoring off\r\n", 26);
    } uartUsb.write("\r\n", 2);  
    break;
            default:
                availableCommands();
                break;
        }
    }
}

void availableCommands()
{
    uartUsb.write("Command List:\r\n", 15);
    uartUsb.write("1: Activate Gas Alarm\r\n", 23);
    uartUsb.write("2: Gas State\r\n", 14);
    uartUsb.write("3: Temp State\r\n", 15);
    uartUsb.write("4: Activate Temperature Alarm\r\n", 31);
    uartUsb.write("5: Reset Alarms\r\n", 17);
    uartUsb.write("6: Activate Monitoring\r\n", 24);
}

void monitoringTaskUpdate()
{
    if (continuousMonitoring){
        monitorCounter++;
        if (monitorCounter >= 200){
            monitorCounter = 0;

            if (gasalarm){
                uartUsb.write("Gas Alarm active\r\n", 18);
            } else {
                uartUsb.write("Gas Alarm off\r\n", 15);
            }
            if (tempalarm){
                uartUsb.write("Temperature alarm active\r\n", 25);
            } else {
                uartUsb.write("Temperature alarm off\r\n", 22);
            }
            uartUsb.write("\r\n", 2);
        }
    } else {
        monitorCounter = 0;
    }
}
