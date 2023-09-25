// Include the Arduino Stepper Library
#include <Arduino.h>
#include <AccelStepper.h>
#include <L298N.h>
#include <L298NX2.h>
// Motor 1 connections
const uint8_t m1e = 7;
const uint8_t m1a = 22;
const uint8_t m1b = 23;
// Motor 2 connections
const uint8_t m2e = 6;
const uint8_t m2a = 28;
const uint8_t m2b = 29;

// Motor C connections
const uint8_t m3e = 5;
const uint8_t m3a = 34;
const uint8_t m3b = 35;

// Stepper 1
const uint8_t enablePin = 13;
const uint8_t dirPin = 30;
const uint8_t stepPin = 2;

// Stepper 2
const uint8_t s2p1 = 36;
const uint8_t s2p2 = 37;
const uint8_t s2p3 = 38;
const uint8_t s2p4 = 39;

// Define arm speeds

const uint8_t armMicrosteps = 4;
const int16_t armRevolution = 200 * armMicrosteps;
const uint8_t armSpeedMultiplier = 4;
const uint8_t armSecondsToMaxSpeed = 1;

const float armMaxSpeed = armRevolution * armSpeedMultiplier; // 4000/armSpeedMultiplier; //Max relieble speed capable by arduino
const float armAccel = armMaxSpeed / armSecondsToMaxSpeed;    // maxspeed reach in s seconds

// Define forearm speeds

const int16_t forearmRevolution = 2038;
const uint8_t forearmSpeedMultiplier = 1;
const uint8_t forearmSecondsToMaxSpeed = 0.5;

const float forearmMaxSpeed = forearmRevolution * forearmSpeedMultiplier; // 4000/armSpeedMultiplier; //Max relieble speed capable by arduino
const float forearmAccel = forearmMaxSpeed / forearmSecondsToMaxSpeed;    // maxspeed reach in s seconds

#define stepsToDegrees(degree, revolution) map(degree, 0, 360, 0, revolution)

// Creates an instances

L298N claw(m3e, m3a, m3b);
L298NX2 car(m1e, m1a, m1b, m2e, m2a, m2b);

#define carSetSpeed(a, b) \
    car.setSpeedA(a);     \
    car.setSpeedB(b)
#define carRun(a, b) \
    car.runA(a);     \
    car.runB(b)

AccelStepper arm(AccelStepper::DRIVER, stepPin, dirPin);
AccelStepper forearm(AccelStepper::FULL4WIRE, s2p1, s2p2, s2p3, s2p4);

int_fast8_t clawDir = -1;
int_fast8_t m1Dir = -1;
int_fast8_t m2Dir = -1;

const uint8_t numChars = 64;
char receivedChars[numChars];
char tempChars[numChars]; // temporary array for use when parsing

// variables to hold the parsed data
char messageFromPC[numChars] = {0};

bool newData = false;
int_fast8_t intVals[6] = {0}; // Modify the size according to the maximum number of integers expected
bool boolVals[18] = {0};      // Modify the size according to the maximum number of bools expected

//============

void setup()
{
    Serial.begin(74880);
    Serial.println("Connected to arduino");
    setup_steppers();
}

//============

void loop()
{
    recvWithStartEndMarkers();
    if (newData == true)
    {
        strcpy(tempChars, receivedChars);
        // this temporary copy is necessary to protect the original data
        //   because strtok() used in parseData() replaces the commas with \0
        parseData();
        useParsedData();
        newData = false;
    };

    arm.run();
    forearm.run();

    claw.run(clawDir);
    // carRun(m1Dir, m2Dir);
    car.runA(m1Dir);
    car.runB(m2Dir);
}
//============

void setup_steppers()
{

    pinMode(enablePin, OUTPUT);
    digitalWrite(enablePin, LOW);

    arm.setAcceleration(armMaxSpeed);
    arm.setMaxSpeed(armAccel);

    forearm.setAcceleration(forearmMaxSpeed);
    forearm.setMaxSpeed(forearmAccel);
}

void recvWithStartEndMarkers()
{
    static bool recvInProgress = false;
    static uint8_t ndx = 0;
    char startMarker = '[';
    char endMarker = ']';
    char rc;

    while (Serial.available() > 0 && newData == false)
    {
        rc = Serial.read();

        if (recvInProgress == true)
        {
            if (rc != endMarker)
            {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars)
                {
                    ndx = numChars - 1;
                }
            }
            else
            {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }
        else if (rc == startMarker)
        {
            recvInProgress = true;
        }
    }
}

//============

void parseData()
{
    // Split the received data into tokens
    char *strtokIndx;                     // this is used by strtok() as an index
    strtokIndx = strtok(tempChars, ",");  // get the first token (model number)
    uint16_t modelNum = atoi(strtokIndx); // convert the token to an integer
    // Use a switch case statement to handle different models (if needed)
    switch (modelNum)
    {

    default:
        // Parse the values for Model 0
        for (int i = 0; i < 6; i++)
        {
            strtokIndx = strtok(NULL, ",");
            intVals[i] = atoi(strtokIndx); // convert the token to an integer

            // Serial.print(i);
            // Serial.print(": ");
            // Serial.print(intVals[i]);
            // Serial.print(", ");
        }
        // Serial.println();
        for (int i = 0; i < 18; i++)
        {
            strtokIndx = strtok(NULL, ",");
            boolVals[i] = atoi(strtokIndx); // convert the token to an integer
        }
        break;
        // Add more cases for other models if needed
    }
}

void useParsedData()
{

    int_fast8_t armVal = (boolVals[1] - boolVals[2]);
    int_fast8_t forearmVal = (boolVals[0] - boolVals[4]);

    arm.move(armVal * 99999);
    forearm.move(armVal * 99999);
    /*uint_fast8_t preArmVal = (arm.distanceToGo() == 0) ? 0 : (arm.distanceToGo() < 0) ? -1
                                                                                      : 1;
    if (preArmVal != armVal)
    {   
        Serial.println(armVal)
        arm.move(armVal * 99999);
    }

    uint_fast8_t preForearmVal = (forearm.distanceToGo() == 0) ? 0 : (forearm.distanceToGo() < 0) ? -1
                                                                                                  : 1;
    if (preForearmVal != forearmVal)
    {
        forearm.move(forearmVal * 99999);
    }*/

    clawDir = (intVals[2] == 0) ? -1 : (intVals[2] > 0) ? 0
                                                        : 1;
    int clawVal = map(abs(intVals[2]), 0, 100, 0, 255);
    claw.setSpeed(clawVal);

    /*if (clawVal == 0)
    {
        clawDir = (clawVal < 0) ? 1 : 0;
        claw.setSpeed(abs(clawVal));
    }
    else
    {
        clawDir = -1;
    }*/

    uint_fast8_t carM1 = map(intVals[4], -100, 100, 0, 255);
    uint_fast8_t carM2 = map(intVals[5], -100, 100, 0, 255);
    // carSetSpeed(carM1, carM2);
    car.setSpeedA(carM1);
    car.setSpeedB(carM2);
    if (boolVals[8])
    {
        m1Dir = !m1Dir;
    }
    if (boolVals[9])
    {
        m2Dir = !m2Dir;
    }
    // Serial.print(boolVals[0]);Serial.print(" ");
    // Serial.print(boolVals[1]);Serial.print(" ");
    // Serial.print(boolVals[2]);Serial.print(" ");
    // Serial.println(boolVals[3]);

    // forearm.run();
}
