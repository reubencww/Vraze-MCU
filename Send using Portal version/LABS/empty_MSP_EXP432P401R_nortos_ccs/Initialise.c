#include <Initialise.h>

/* UART Configuration for ESP8266*/
eUSCI_UART_ConfigV1 UART2Config =
{
    EUSCI_A_UART_CLOCKSOURCE_SMCLK, 13, 0, 37,
    EUSCI_A_UART_NO_PARITY,
    EUSCI_A_UART_LSB_FIRST,
    EUSCI_A_UART_ONE_STOP_BIT,
    EUSCI_A_UART_MODE,
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};

/* 1 second Timer Configuration, to calculate speed/ RPM*/
const Timer_A_UpModeConfig upConfigEncoder =
{
    TIMER_A_CLOCKSOURCE_ACLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_1,
    32678,
    TIMER_A_TAIE_INTERRUPT_DISABLE,
    TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
    TIMER_A_DO_CLEAR
};

/* Initialize GPIO for LED and 3v3 outputs*/
void Initialise_IO(void)
{
    /* GPIO configuration for LEDs*/
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1);
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN2);

    /* Initialize all LEDs to low*/
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

    /* Extra 3v3 outputs for encoder's VCC*/
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN7);
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN7);
}

/* Initialize Car GPIO output pins*/
void Initialise_CarMotor(void)
{
    /* Configuring P4.4 and P4.5 as Output. P2.4 as peripheral output for PWM and P1.1 for button interrupt */
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN4);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4,
    GPIO_PRIMARY_MODULE_FUNCTION);

    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN2);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN5,
    GPIO_PRIMARY_MODULE_FUNCTION);
}

/* Initialize UART pins*/
void Initialise_EspUART(void)
{
    /* Ensure MSP432 is Running at 24 MHz*/
    FlashCtl_setWaitState(FLASH_BANK0, 2);
    FlashCtl_setWaitState(FLASH_BANK1, 2);
    PCM_setCoreVoltageLevel(PCM_VCORE1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_24);

    /* Initialize UART pins for ESP8266*/
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P3, GPIO_PIN2 | GPIO_PIN3,
            GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_UART_initModule(EUSCI_A2_BASE, &UART2Config);
    MAP_UART_enableModule(EUSCI_A2_BASE);
    MAP_UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA2);

    /* Reset GPIO of the ESP8266*/
    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN1);

    /* Reset ESP8266 to prevent timeout errors*/
    ESP8266_HardReset();
    __delay_cycles(48000000);
    UART_Flush(EUSCI_A2_BASE);

    /* Configure to ESP8266's connection*/
    ESP8266_ChangeMode1();
    //ESP8266_DisconnectToAP();
    ESP8266_ConnectToAP("Reuben","lol12345678");
    __delay_cycles(48000000);
    /* For testing of wifi module using home private ip*/
    //if(ESP8266_EstablishConnection('0', TCP, "192.168.157.22", "8080"))
    //    GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN2);
    /* Actual deployment using phone's 5g private ip*/

    if(ESP8266_EstablishConnection('0', TCP, "172.20.10.2", "8080"))
        GPIO_toggleOutputOnPin(GPIO_PORT_P2, GPIO_PIN2);

}

/* Initialize optical encoder Input pins*/
void Initialise_Encoder(void)
{
    /* Initialize input pins*/
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN6);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_interruptEdgeSelect(GPIO_PORT_P2, GPIO_PIN6, GPIO_LOW_TO_HIGH_TRANSITION);
    GPIO_interruptEdgeSelect(GPIO_PORT_P2, GPIO_PIN7, GPIO_LOW_TO_HIGH_TRANSITION);

    /* Initialize Interrupt for optical encoders*/
    GPIO_clearInterruptFlag(GPIO_PORT_P2, GPIO_PIN6);
    GPIO_clearInterruptFlag(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN6);
    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN7);
}

/* Initialize Timer to calculate speed/ RPM*/
void Initialise_TimerA1(void)
{
    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfigEncoder);
    Interrupt_enableInterrupt(INT_TA1_0);
    Timer_A_clearTimer(TIMER_A1_BASE);
}

void enableInterrupts(void)
{
    Interrupt_enableInterrupt(INT_TA1_0);
    Interrupt_enableInterrupt(INT_PORT1);
    Interrupt_enableInterrupt(INT_PORT2);
    Interrupt_enableMaster();
}
