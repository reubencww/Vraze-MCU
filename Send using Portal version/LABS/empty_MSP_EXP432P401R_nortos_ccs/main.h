#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ESP8266.h>
#include <UART_Driver.h>
#include <math.h>
#include <Initialise.h>
#include <CAR_Controls.h>
#include <HCSR04.h>

/*
 * Macro Definitions
*/
/* Car's ESP8266 State*/
#define CAR_WAITING_INSTRUCTION     0x1
#define CAR_RECEIVED_INSTRUCTION    0x2

/* Car's Engine State*/
#define CAR_ENGINE_ON               0x1
#define CAR_ENGINE_OFF              0x0

// -------------------------------------------------------------------------------------------------------------------/

/******************************************************************************
 * main.h function prototypes
 *
 * getPIDOutput             - Implementation of PID controller to control DC motor's PWM
 * clearCounters            - Reset global variable and counters for next PID's loop
 * moveCar                  - Calls getPID to generate pwm to move car
 * stopCar                  - If collision detected after each character instruction, off the car's engine
 * turnCar                  - instead of using pid to turn car, because of limitations, we just turn it normally using notches
 *******************************************************************************/
void getPIDOutput(uint32_t targetNotch, char dir);
void clearCounters(void);
void moveCar(char dir);
void stopCar(void);
void turnCar(uint32_t targetNotch);
// -------------------------------------------------------------------------------------------------------------------/

/******************************************************************************
 * Global Variables
 *
 * engineState              - Stores the State of the Car Engine
 * wifiState                - Stores the State of ESP8266
 * instructionBuffer        - Stores Instructions received from ESP8266
 * rightNotchesDetected     - Stores the number of notches detected on the right wheel
 * leftNotchesDetected      - Stores the number of notches detected on the left wheel
 * desiredNotches           - Stores the number of notches per instruction we desire
 * PIDDesiredNotches        - Stores the number of notches per instruction we desire for PID loop
 * turnDesiredNotches       - Stores the number of notches per instruction we desire for turning instructions
 * turnPIDDesiredNotches    - Stores the number of notches per instruction we desire for PID loop for turning instructions
 * timeSeconds              - Stores seconds elapsed per instruction character
 * leftSpeed                - Stores the speed of the left wheel per instruction character
 * rightSpeed               - Stores the speed of the right wheel per instruction character
 * leftRPM                  - Stores the RPM of the left wheel per instruction character
 * rightRPM                 - Stores the RPM of the right wheel per instruction character
 * leftError                - Stores the left wheel error for PID controller
 * rightError               - Stores the right wheel error for PID controller
 * errorTl                  - Stores the left wheel total error for PID controller
 * errorTr                  - Stores the right wheel total error for PID controller
 * lastErrorL               - Stores the previous error for left wheel for PID controller
 * lastErrorR               - Stores the previous error for right wheel for PID controller
 * proportionL              - Stores the proportional gain for left wheel for PID controller
 * proportionR              - Stores the proportional gain for right wheel for PID controller
 * integralL                - Stores the integral gain for left wheel for PID controller
 * integralR                - Stores the integral gain for right wheel for PID controller
 * derivativeL              - Stores the derivative for left wheel for PID controller
 * derivativeR              - Stores the derivative for right wheel for PID controller
 *******************************************************************************/
volatile uint32_t engineState = CAR_ENGINE_OFF;
volatile uint8_t wifiState;
char *instructionBuffer;
char RPM[] = "12.5";
char Distance[] = "30";
volatile bool collision = 0;
volatile uint32_t rightNotchesDetected = 0;
volatile uint32_t leftNotchesDetected = 0;
volatile uint32_t desiredNotches = 40;
volatile uint32_t PIDDesiredNotches = 45;
const uint32_t turnDesiredNotches = 10;
const uint32_t turnPIDDesiredNotches = 12;
volatile uint32_t timeSeconds = 0;
volatile float leftSpeed = 0;
volatile float rightSpeed = 0;
volatile float leftRPM = 0;
volatile float rightRPM = 0;
volatile float leftError = 0;
volatile float rightError = 0;
float errorTl;
float errorTr;
float lastErrorL;
float lastErrorR;
float proportionL;
float proportionR;
float integralL;
float integralR;
float derivativeL;
float derivativeR;
