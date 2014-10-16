
#include <SoftwareSerial.h>
/* HC-SR04 Sensor

This sketch reads a HC-SR04 ultrasonic rangefinder and returns the distance to the closest object in range. To do this, it sends a pulse to the sensor to initiate a reading, then listens for a pulse to return. The length of the returning pulse is proportional to the distance of the object from the sensor.

The circuit:
* VCC connection of the sensor attached to +5V
* GND connection of the sensor attached to ground
* TRIG connection of the sensor attached to digital pin 6
* ECHO connection of the sensor attached to digital pin 7

Original code for Ping))) example was created by David A. Mellis
Adapted for HC-SR04 by Tautvidas Sipavicius

This example code is in the public domain.
*/

/* Define trigger pin and echo pins */

const int trigPin = 6;
const int echoPin = 7;
const int laserpin = 12;                                // the pin interface for the laser diode designator
 
char buf[6];
/* Initialize serial comms to a baud rate of 9600 */
SoftwareSerial myserial = SoftwareSerial(10, 11);
void setup()

{
 Serial.begin(19200);
 myserial.begin(19200);
 pinMode(trigPin, OUTPUT);
 pinMode(echoPin, INPUT);
 pinMode(laserpin, OUTPUT);
}

void loop()
{
/* Define variables for ping duration and distance in inches and cm */

 int duration, inches, cm;
 char recvbyte = 0;

/* The sensor is triggered by a HIGH pulse of at least 10us 
Giving a short LOW pulse initially ensures a sharp HIGH pulse */

 
 digitalWrite(trigPin, LOW);
 delayMicroseconds(2);
 digitalWrite(trigPin, HIGH);
 delayMicroseconds(10);
 digitalWrite(trigPin, LOW);

 

// Set the echo pin as an input

 

 

/* Read the signal from the sensor. A HIGH pulse whose
duration is the time (in microseconds) from the sending
of the ping to the reception of its echo from an object */

duration = pulseIn(echoPin, HIGH);

//num_to_arr(duration);

while(1)
{
if(myserial.available() > 0) 

{

//recvbyte = Serial.read();
recvbyte = myserial.read();
myserial.write(duration);

digitalWrite(laserpin, HIGH);
delay(10);
digitalWrite(laserpin, LOW);
break;
}
}




}







 

/* There are 73.746 microseconds per inch, sound travels at 1130 feet  per second. The distance travelled by the ping, outbound and return, is divided by 2 to get the distance of the obstacle. */

int microsecondsToInches(int microseconds)
{

 return microseconds/74/2;

}

 

/* The speed of sound is 340 m/s or 29 microseconds per centimetre. The ping travels out and back, so to find the distance of the object we take half of the distance travelled. */

int microsecondsToCentimeters(int microseconds)

{

 return microseconds/29/2;

}

void num_to_arr(int b) 
{
 
 
 
 int i = 0;
 
 while(i < 6)
 {
  
      buf[i] = (b % 10);
      b = b/10;
      i++;
 }

}
