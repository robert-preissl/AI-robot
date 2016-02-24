
#include <Servo.h>
#include "NanoMouseMotors.h"
#include "NanoMouseSensors.h"

int durationPerField = 1900; // duration in ms spent to travel one field in the maze at average speed

const byte ledPin = 13;
const byte buttonPin = 9;

int targetFront;
int thresholdFront;

int targetSide; // govern distance from wall
int thresholdSide; // help to determine if a wall exist (if lower, not a lot of refl. no wall)

NanoMouseMotors motors;

// <leftEm, leftDet, frontEm, ... >
NanoMouseSensors<4, A7, 3, A6, 2, A5> sensors;


void setup()
{
  // D5 .. right. D6 .. left
  motors.attach( 6, 5 );

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  sensors.configure();

  Serial.begin(9600);

  while ( digitalRead( buttonPin ) )
  {}
  delay(500);

  calibrate();
  Serial.print("Calibrate done. targetFront = "); Serial.println( targetFront );
  Serial.println("");



  // hard coded for known maze:
  forwardWhiskers();
  motors.turn(RIGHT, 90);
  sensors.initialize();

  forwardWhiskers();
  motors.turn(RIGHT, 90);
  sensors.initialize();

  forwardWhiskers();
  motors.turn(LEFT, 90);
  sensors.initialize();

  forwardWhiskers();
  // motors.turn(LEFT, 90);
  sensors.initialize();

  forwardWhiskers();
  motors.turn(LEFT, 90);
  sensors.initialize();

  forwardWhiskers();
  // motors.turn(LEFT, 90);
  sensors.initialize();

  forwardWhiskers();
  motors.turn(LEFT, 90);
  sensors.initialize();

  forwardWhiskers();
  motors.turn(LEFT, 90);
  sensors.initialize();

  forwardWhiskers();
  // motors.turn(LEFT, 90);
  sensors.initialize();

}

void loop() {
  // put your main code here, to run repeatedly:
  //digitalWrite(ledPin, HIGH);

  byte st = state();
  Serial.print("State: "); Serial.println( st );
  // avoid( st );

  // navigateLabyrinth( st );
  
}

void calibrate()
{
  sensors.initialize();
  // measures value for sensors when seeing a wall.
  targetSide = (sensors.left + sensors.right) / 2;

  motors.turn(RIGHT, 90);
  sensors.initialize();
  sensors.sense();
  targetFront = sensors.front / 2; // corridor is narrow. -> front is high like around 400. when robot moves there is a delay until it gets the high front value and i need to make sure to stop it sooner

  // if turn right 90. left points into empty corridor. resulting into low refl number
  //  and targetSide is expected to be higher value ("there is a wall")
  // goal is to say. x > thresholdSide ... there is a wall.
  //                   <               ... there is no wall
  thresholdSide = ( targetSide + sensors.left ) / 2 ;

/*
 * If robot does not appear to detect walls consistently, we could replace 
 * the average between the target and ambient values used in the previous 
 * lecture with values that are weighted towards the ambient values.
 * For example, if the side sensors miss walls on occasion, try calculating 
 * the threshold value for the side sensors as follows:
       thresholdSide = (targetSide + 2*sensors.left)/3;
 */

  motors.turn(LEFT, 90);
  sensors.initialize();

  // original /2 . with darker surrounding light /4.
  thresholdFront = (targetFront + sensors.front) / 4; // value for wall, and average with ambient value of current front sensor

}

void forwardWhiskers()
{

  unsigned long startingTime = millis();
  
  while ( sensors.front < targetFront && millis() - startingTime < durationPerField )
  {
    sensors.sense();

    // check if walls on both sides of robot.
    if ( sensors.left > thresholdSide && sensors.right > thresholdSide )
    {
      int error = sensors.right - sensors.left;
      motors.forwardProportional( error );
      sensors.initialize();
    }
    // check if wall on right side only. not on left side.
    // -> since we have no left one. we use the "targetSide" which stores what a value for a wall should be like
    else if( sensors.right > thresholdSide )
    {
      int error = sensors.right - targetSide;
      motors.forwardProportional( error );
      sensors.initialize();
    }
    else if( sensors.left > thresholdSide )
    {
      int error = targetSide - sensors.left;
      motors.forwardProportional( error );
      sensors.initialize();
    }
    else {
      motors.forward();
    }

    Serial.print("S-F: "); Serial.println( sensors.front );
    
    sensors.sense();
  }

  motors.stop();
}

byte state()
{
  byte event = 0;

  sensors.sense();

  if ( sensors.front > thresholdFront )
    event += 1;

  if ( sensors.left > thresholdSide )
    event += 2;

  if ( sensors.right > thresholdSide )
    event += 4;

  return event;
}

void avoid(byte event)
{
  switch (event)
  {
    case 1: // front sensor is triggered
      // if( random(2) ) ..
      motors.turn( LEFT, 90 ); // motors.turn( LEFT, random(90, 181) );
      sensors.initialize();
      break;

    case 2: // left sensor is triggered
      motors.turn( RIGHT, 45 );
      sensors.initialize();
      break;

    case 3: // front & left sensor is triggered
      motors.turn( RIGHT, 45 );
      sensors.initialize();
      break;

    case 4: // right sensor is triggered
      motors.turn( LEFT, 45 );
      sensors.initialize();
      break;

    case 5: // front & right sensor is triggered
      motors.turn( LEFT, 45 );
      sensors.initialize();
      break;

    case 6: // left & right sensor is triggered
      motors.turn( LEFT, 180 );
      sensors.initialize();
      break;

    case 7: // left & right & front sensor is triggered
      motors.turn( LEFT, 180 );
      sensors.initialize();
      break;

    default:
      motors.forward();
      break;
  }
}

void navigateLabyrinth(byte event)
{
  switch(event)
  {
    case 0:
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
      delay(1000);
      break;
    case 1:
      digitalWrite(ledPin, HIGH);
      delay(500);
      digitalWrite(ledPin, LOW);
      delay(500);
      break;
    
    case 2:
      forwardWhiskers();
      break;

    case 3:
      motors.turn(RIGHT, 90);
      sensors.initialize();
      break;

    case 4:
      forwardWhiskers();
      break;

    case 5:
      motors.turn(LEFT, 90);
      sensors.initialize();
      break;

    case 6:
      forwardWhiskers();
      break;

    case 7:
      digitalWrite(ledPin, HIGH);
      break;
      
  }
}




