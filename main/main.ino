#include <Sabertooth.h>

// Create SoftwareSerial objects for communication with the Sabertooth motor controller
// Create Sabertooth objects for controlling the motors using SoftwareSerial
Sabertooth ST1(128);
Sabertooth ST2(129);

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
    Sabertooth::autobaud(Serial);
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
        showParsedData();
        newData = false;
    }
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
        }
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

void showParsedData()
{
    // Map the integer values
    int mappedVal1 = map(intValues[0], -100, 100, -127, 127);
    int mappedVal2 = map(intValues[1], -100, 100, -127, 127);
    int mappedVal3 = map(intValues[2], -100, 100, -127, 127);
    int mappedVal4 = map(intValues[3], -100, 100, -127, 127);


    // Motor 1 and Motor 2 (using mappedVal1 and mappedVal2)
    ST1.motor(1, (booleanValues[4] ? -127 : (booleanValues[5] ? 127 : mappedVal1)));
    ST1.motor(2, (booleanValues[6] ? -127 : (booleanValues[7] ? 127 : mappedVal2)));

    // Motor 3 and Motor 4 (using mappedVal3 and mappedVal4)
    ST2.motor(1, (booleanValues[2] ? -127 : (booleanValues[1] ? 127 : mappedVal3)));
    ST2.motor(2, (booleanValues[3] ? -127 : (booleanValues[0] ? 127 : mappedVal4)));
}
