// Include the library:
#include "SparkFunMMA8452Q.h"
int led = D6;
int button = D4;
String ledStatus = "off";

// Create an MMA8452Q object, used throughout the rest of the sketch.
MMA8452Q accel; // Default constructor, SA0 pin is HIGH


void setup() 
{
    Serial.begin(9600);
    
	// Initialize the accelerometer with begin():
	// begin can take two parameters: full-scale range, and output data rate (ODR).
	// Full-scale range can be: SCALE_2G, SCALE_4G, or SCALE_8G (2, 4, or 8g)
	// ODR can be: ODR_800, ODR_400, ODR_200, ODR_100, ODR_50, ODR_12, ODR_6 or ODR_1
    accel.begin(SCALE_2G, ODR_1); // Set up accel with +/-2g range, and slowest (1Hz) ODR
    
    
    pinMode(button, INPUT_PULLUP);
    pinMode(led, OUTPUT);

    // turn off light at start to match ledStatus
    digitalWrite(led, LOW);
}

void loop() 
{
	// accel.available() will return 1 if new data is available, 0 otherwise
    if (accel.available())
    {
		// To update acceleration values from the accelerometer, call accel.read();
        accel.read();
		
		// After reading, six class variables are updated: x, y, z, cx, cy, and cz.
		// Those are the raw, 12-bit values (x, y, and z) and the calculated
		// acceleration's in units of g (cx, cy, and cz).
		
		// use the printAccelGraph funciton to print the values along with a bar
		// graph, to see their relation to eachother:
        printAccelGraph(accel.cx, "X", 20, 2.0);
        printAccelGraph(accel.cy, "Y", 20, 2.0);
        printAccelGraph(accel.cz, "Z", 20, 2.0);
        Serial.println();
        
    }
	
	// No need to delay, since our ODR is set to 1Hz, accel.available() will only return 1
	// about once per second.
	
	   // check button
    int buttonState = digitalRead(button);

    // LOW means button is being pushed
    if(buttonState == LOW) {

        // if light is currently off, switch to on
        if(ledStatus == "off") {
            ledStatus = "on";
            digitalWrite(led, HIGH);
        }
        // else light must be on, so switch to off
        else {
            ledStatus = "off";
            digitalWrite(led, LOW);
        }
    }
     // wait 0.2 seconds before checking button again
    delay(200);
}

// printAccelGraph prints a simple ASCII bar graph for a single accelerometer axis value.
// Examples: 
//	printAccelGraph(-0.1, "X", 20, 2.0) will print:
// 		X:                    =|                     (0.1 g)
//	printAccelGraph(1.0, "Z", 20, 2.0) will print:
//		Z:                     |==========           (1.0 g)
// Input:
//	- [value]: calculated value of an accelerometer axis (e.g accel.cx, accel.cy)
//	- [name]: name of the axis (e.g. "X", "Y", "Z")
//	- [numBarsFull]: Maximum number of bars either right or left of 0 point.
//	- [rangeAbs]: Absolute value of the maximum acceleration range
void printAccelGraph(float value, String name, int numBarsFull, float rangeAbs)
{
	// Calculate the number of bars to fill, ignoring the sign of numBars for now.
	int numBars = abs(value / (rangeAbs / numBarsFull));
    
    Serial.print(name + ": "); // Print the axis name and a colon:
	
	// Do the negative half of the graph first:
    for (int i=0; i<numBarsFull; i++)
    {
        if (value < 0) // If the value is negative
        {
			// If our position in the graph is in the range we want to graph
            if (i >= (numBarsFull - numBars))
                Serial.print('='); // Print an '='
            else
                Serial.print(' '); // print spaces otherwise
        }
        else // If our value is positive, just print spaces
            Serial.print(' ');
    }
    
    Serial.print('|'); // Print a pipe (|) to represent the 0-point
    
    
	// Do the positive half of the graph last:
    for (int i=0; i<numBarsFull; i++)
    {
        if (value > 0)
        {	// If our position in the graph is in the range we want to graph
            if (i <= numBars)
                Serial.print('='); // Print an '='
            else
                Serial.print(' '); // otherwise print spaces
        }
        else // If value is negative, just print spaces
            Serial.print(' ');
    }
    

    static int lastEvent = 0;
	static int activeTime = 0;
	static int inactiveTime = 0;
	
    //value between 0.98 and 0.99 because while resting on the surface value is around that
    //value > 0 to ignore wrong negative values
    if(ledStatus != "off" && (value > 0.98 && value < 0.99) && value > 0 )
    {
        Particle.publish("Status:", "Embedded device is running but washing machine inactive.", PRIVATE);
        inactiveTime++;
        Particle.publish("InActive time:", String(inactiveTime), PRIVATE);
        delay(1000);
    }
    if(ledStatus != "off" && (value > 0.99 || value < 0.98) && value > 0 )
    {
        
        Particle.publish("Status:", "Washing machine is active.", PRIVATE);
        inactiveTime = 0;
        lastEvent = 0;
        activeTime++;
        Particle.publish("Active time:", String(activeTime), PRIVATE);
        delay(1000);
          // wait a second so as not to send massive amounts of data
    }
    
    //lastEvent used so that alert is published only once.
    if(lastEvent != 1 && ledStatus != "off" && inactiveTime > 60)
    {
        lastEvent = 1;
        Particle.publish("Alert", "Washing machine inactive for more than 1 minute. Please check.", PRIVATE);
        //the alert will be sent to the user after every 5 minutes until or unless he starts the machine.
    }
    
    //resetting the inactive timer
    if(ledStatus == "off")
    {
        inactiveTime = 0;
    }
    
}

