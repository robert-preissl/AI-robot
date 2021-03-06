
// WIFI: https://www.youtube.com/watch?v=ylyS29DC8xY

#define DEBUG

#include <Servo.h>
#include "NanoMouseMotors.h"
#include "NanoMouseSensors.h"
#include "NanoMouseMaze.h"

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

NanoMouseMaze<3, 4> maze;

void setup()
{
  // D5 .. right. D6 .. left
  motors.attach( 6, 5 );

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  sensors.configure();

  maze.mouseRow = 2;
  maze.mouseColumn = 0;
  maze.mouseHeading = NORTH;

  maze.targetRow = 1;
  maze.targetColumn = 2;

  Serial.begin(9600);

  while ( digitalRead( buttonPin ) )
  {}
  delay(500);

  calibrate();
#ifdef DEBUG
  Serial.print("Calibrate done. targetFront = "); Serial.print( targetFront );
  Serial.print(" / thresholdFront = "); Serial.print( thresholdFront );
  Serial.print(" / targetSide = "); Serial.print( targetSide );
  Serial.print(" / thresholdSide = "); Serial.println( thresholdSide );
#endif

  do
  {
    sensors.initialize();
    scanWalls();

    maze.solve();

#ifdef DEBUG
    maze.print();
    while ( digitalRead(buttonPin) )
    {}
    delay(500);
#endif

    turnTowardBestNeighbor();

    forwardWhiskers();
  } while ( maze.values[maze.mouseRow][maze.mouseColumn] != 0 );

#ifdef DEBUG
  scanWalls();
  maze.solve();
  maze.print();
#endif

}

void loop() {
  // put your main code here, to run repeatedly:
  //digitalWrite(ledPin, HIGH);
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
     If robot does not appear to detect walls consistently, we could replace
     the average between the target and ambient values used in the previous
     lecture with values that are weighted towards the ambient values.
     For example, if the side sensors miss walls on occasion, try calculating
     the threshold value for the side sensors as follows:
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
#ifdef DEBUG
    Serial.print("FW -- S-F: "); Serial.print( sensors.front );
    Serial.print(" / S-R: "); Serial.print( sensors.right );
    Serial.print(" / S-L: "); Serial.println( sensors.left );
#endif
    // check if walls on both sides of robot.
    if ( sensors.left > thresholdSide && sensors.right > thresholdSide )
    {
      int error = sensors.right - sensors.left;
      motors.forwardProportional( error );
      sensors.initialize();
    }
    // check if wall on right side only. not on left side.
    // -> since we have no left one. we use the "targetSide" which stores what a value for a wall should be like
    else if ( sensors.right > thresholdSide )
    {
      int error = sensors.right - targetSide;
      motors.forwardProportional( error );
      sensors.initialize();
    }
    else if ( sensors.left > thresholdSide )
    {
      int error = targetSide - sensors.left;
      motors.forwardProportional( error );
      sensors.initialize();
    }
    else {
      motors.forward();
    }

    sensors.sense();
  }

  motors.stop();

  // update mouse position after movement
  maze.mouseRow += neighborCells[maze.mouseHeading][0];
  maze.mouseColumn += neighborCells[maze.mouseHeading][1];
}

void scanWalls()
{
#ifdef DEBUG  
  Serial.println("");
  Serial.print("SW -- S-F = "); Serial.print( sensors.front );
  Serial.print(" / S-R = "); Serial.print( sensors.right );
  Serial.print(" / S-L = "); Serial.println( sensors.left );
#endif
  if ( sensors.front > thresholdFront )
    maze.addWalls( maze.mouseHeading );

  if ( sensors.right > thresholdSide )
    maze.addWalls( (maze.mouseHeading + 1) % 4 );

  if ( sensors.left > thresholdSide )
    maze.addWalls( (maze.mouseHeading + 3) % 4 );
}

void turnTowardBestNeighbor()
{
  byte desiredHeading = maze.findBestNeighbor();
  int difference = maze.mouseHeading - desiredHeading;

  // e.g, if mouse points to EAST (1) and best neighbor is NORTH (0)
  //      -> difference = 1 -> turn left
  if ( difference == 1 || difference == -3 )
    motors.turn( LEFT, 90 );
  else if ( difference == 3 || difference == -1 )
    motors.turn( RIGHT, 90 );
  else if ( difference == 2 || difference == -2 )
    motors.turn( RIGHT, 180 );

  sensors.initialize();
  maze.mouseHeading = desiredHeading;
}


