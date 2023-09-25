// Include the Arduino Stepper Library
#include <AccelStepper.h>

#define speedMultiplier 1
#define secondsToMaxSpeed 4
#define stepsToDegrees(degree) map(degree, 0, 360, 0, revolution)

// Define pin connections
#define dirPin1 5
#define stepPin1 2
#define dirPin2 6
#define stepPin2 3

// Define motor enable pin
#define enablePin 8

const byte microsteps = 4;
const int16_t revolution = 200 * microsteps;

const float motorMaxSpeed = 8000/speedMultiplier; //Max relieble speed capable by arduino
float motorAccel = motorMaxSpeed/secondsToMaxSpeed; //maxspeed reach in s seconds


// Creates an instance
AccelStepper stepper1(AccelStepper::DRIVER, stepPin1, dirPin1);
//AccelStepper stepper2(AccelStepper::DRIVER, stepPin2, dirPin2);


void setup()
{	
	pinMode(enablePin, OUTPUT);
	digitalWrite(enablePin, LOW);
	stepper1.setAcceleration(motorMaxSpeed);
	stepper1.setMaxSpeed(motorAccel);
	// initialize the serial port:
	//stepper1.moveTo(stepsToDegrees(720));
	
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