
#include <Arduino.h>

template <byte leftEmitter, byte leftDetector, byte frontEmitter, byte frontDetector, byte rightEmitter, byte rightDetector>

class NanoMouseSensors
{
  private:
    // Variables used for smoothing (moving average)
    static const byte numReadings = 20;

    int leftTotal, frontTotal, rightTotal;
    byte leftIndex, frontIndex, rightIndex;
    int leftSmoothed, frontSmoothed, rightSmoothed;
    int leftReadings[ numReadings ];
    int frontReadings[ numReadings ];
    int rightReadings[ numReadings ];

  public:
    int left, front, right;

    void configure()
    {
      pinMode(leftEmitter, OUTPUT);
      pinMode(frontEmitter, OUTPUT);
      pinMode(rightEmitter, OUTPUT);
    }

    int filterAmbient( byte emitter, byte detector )
    {
      digitalWrite( emitter, HIGH );
      delay(1);
      int combined = analogRead(detector);
      digitalWrite( emitter, LOW );
      delay(1);
      int ambient = analogRead(detector);

      return (combined - ambient);
    }

    int smoothReflected( int* readings, int& total, byte& index, int reflected )
    {
      total -= readings[index % numReadings];
      readings[index % numReadings] = reflected;
      total += readings[index % numReadings];
      index++;

      return (total / numReadings);
    }

    void sense()
    {
      int reflected = filterAmbient( leftEmitter, leftDetector );
      leftSmoothed = smoothReflected( &leftReadings[0], leftTotal, leftIndex, reflected );
      left = leftSmoothed;

      reflected = filterAmbient( frontEmitter, frontDetector );
      frontSmoothed = smoothReflected( &frontReadings[0], frontTotal, frontIndex, reflected );
      front = frontSmoothed;

      reflected = filterAmbient( rightEmitter, rightDetector );
      rightSmoothed = smoothReflected( &rightReadings[0], rightTotal, rightIndex, reflected );
      right = rightSmoothed;
    }

    void view()
    {
      Serial.print("L: "); Serial.print( leftSmoothed );
      Serial.print(" | F: "); Serial.print( frontSmoothed );
      Serial.print(" | R: "); Serial.println( rightSmoothed );
    }

    void initialize()
    {
      for ( byte i = 0; i < numReadings; ++i )
      {
        sense();
      }
    }

};


