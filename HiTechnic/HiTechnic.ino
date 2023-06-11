#define TWI_FREQ 44440L  // reduce communication rate of I2C bus to correct one
#include <Wire.h> 
/*
IMPORTANT: Go to C:\path\to\library\Wire\src\utility\twi.c 
line 96:

cbi(TWSR, TWPS0);

CHANGE TO:

sbi(TWSR, TWPS0);

And leave a comment to remmber that you changed that.
*/
#include <motor_controller.h>

MotorController Driver1 = MotorController(0x01);
MotorController Driver2 = MotorController(0x02);

const byte numChars = 64;
char receivedChars[numChars];
char tempChars[numChars]; // temporary array for use when parsing

// variables to hold the parsed data
char messageFromPC[numChars] = {0};

boolean newData = false;
int intValues[4] = {0};          // Modify the size according to the maximum number of integers expected
boolean booleanValues[18] = {0}; // Modify the size according to the maximum number of booleans expected

//============



void setup()
{
    Serial.begin(9600);
    Wire.begin();
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

}

//============

void recvWithStartEndMarkers()
{
    static boolean recvInProgress = false;
    static byte ndx = 0;
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
    char *strtokIndx;                    // this is used by strtok() as an index
    strtokIndx = strtok(tempChars, ","); // get the first token (model number)
    int modelNum = atoi(strtokIndx);     // convert the token to an integer

    // Use a switch case statement to handle different models (if needed)
    switch (modelNum)
    {
    case 0:
        // Parse the values for Model 0
        for (int i = 0; i < 4; i++)
        {
            strtokIndx = strtok(NULL, ",");
            intValues[i] = atoi(strtokIndx); // convert the token to an integer
            
            //Serial.print(i);
            //Serial.print(": ");
            //Serial.print(intValues[i]);
            //Serial.print(", ");

        }
        Serial.println();
        for (int i = 0; i < 18; i++)
        {
            strtokIndx = strtok(NULL, ",");
            booleanValues[i] = atoi(strtokIndx); // convert the token to an integer
        }
        break;
        // Add more cases for other models if needed

    default:
        // Handle unknown model or invalid data
        break;
    }
}

void useParsedData()
{   
    // Map the integer values

    int mappedVal0 = booleanValues[8]  ? 0 : (booleanValues[5] ? 100 : (booleanValues[4] ? -100 : (intValues[0] == 0) ? -128 : intValues[0]));  //if L  : brake, else if UP   : reverse, else if DOWN  : forward, else joystick_in
    int mappedVal1 = booleanValues[10] ? 0 : (booleanValues[7] ? 100 : (booleanValues[6] ? -100 : (intValues[1] == 0) ? -128 : intValues[1]));  //if ZL : brake, else if LEFT : reverse, else if RIGHT : forward, else joystick_in
    int mappedVal2 = booleanValues[9] ? 0 : (booleanValues[1] ? 100 : (booleanValues[2] ? -100 : (intValues[2] == 0) ? -128 : intValues[2]));   //if  R : brake, else if B    : reverse, else if X     : forward, else joystick_in
    int mappedVal3 = booleanValues[11]  ? 0 : (booleanValues[0] ? 100 : (booleanValues[3] ? -100 : (intValues[3] == 0) ? -128 : intValues[3])); //if ZR : brake, else if A    : reverse, else if Y     : forward, else joystick_in
    
    /*debug print
    Serial.print(mappedVal0);
    Serial.print(" ");
    Serial.print(mappedVal1);
    Serial.print(" ");
    Serial.print(mappedVal2);
    Serial.print(" ");
    Serial.println(mappedVal3);
    */
    Driver1.move_power(mappedVal0, mappedVal1);
    Driver2.move_power(mappedVal2, mappedVal3);
    
}

