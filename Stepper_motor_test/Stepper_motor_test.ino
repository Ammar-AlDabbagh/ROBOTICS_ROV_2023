// Include the Arduino Stepper Library
#include <AccelStepper.h>

#define speedMultiplier 4
#define secondsToMaxSpeed 4
#define stepsToDegrees(degree) map(degree, 0, 360, 0, revolution)

// Define pin connections
const uint8_t s2p1 = 6;
const uint8_t s2p2 = 5;
const uint8_t s2p3 = 3;
const uint8_t s2p4 = 2;


// Define motor enable pin
#define enablePin 8

const byte microsteps = 1;
const int16_t revolution = 2038 * microsteps;

const float motorMaxSpeed = 4000/speedMultiplier; //Max relieble speed capable by arduino
float motorAccel = motorMaxSpeed/secondsToMaxSpeed; //maxspeed reach in s seconds


// Creates an instance
AccelStepper stepper1(AccelStepper::FULL4WIRE, s2p1, s2p2, s2p3, s2p4);
//AccelStepper stepper2(AccelStepper::DRIVER, stepPin2, dirPin2);


void setup()
{	
	pinMode(enablePin, OUTPUT);
	digitalWrite(enablePin, LOW);
	stepper1.setAcceleration(motorMaxSpeed);
	stepper1.setMaxSpeed(motorAccel);
	// initialize the serial port:
	stepper1.moveTo(stepsToDegrees(720));
	
//	stepper2.setAcceleration(motorMaxSpeed);
//	stepper2.setMaxSpeed(motorAccel);
	// initialize the serial port:
//	stepper2.moveTo(stepsToDegrees(720));
}

void loop() 
{
	if (stepper1.distanceToGo() == 0) 
		stepper1.moveTo(-stepper1.currentPosition());
	stepper1.run();
//	stepper2.run();


}