
#define DEBUG

#include <Arduino.h>
#include <Servo.h>

#define RIGHT 1
#define LEFT -1

class NanoMouseMotors
{

  private:

    Servo leftServo;
    Servo rightServo;

    static const byte power = 250;

    // --------------------------------

  public:

    void attach(byte leftMotor, byte rightMotor)
    {
      leftServo.attach(leftMotor);
      rightServo.attach(rightMotor);
    }

    void forward()
    {
      /*
         If you are using the SM-S4303R servos, set your power level to 250
         in order to run the servos at half speed, since these servos reach
         their top speed at a pulse width of 500 microseconds above and
         below the stop signal.
      */
      // L = 1250 -> f  1000 -> ff  750  -> b  500  -> bb
      // R = 1750 -> f  2000 -> ff  1250 -> b  1000 -> bb
      leftServo.writeMicroseconds(1500 - power);
      rightServo.writeMicroseconds(1500 + power * 1.06 ); // > 1 .. more to the left
    }

    void stop(int time = 200)
    {
      leftServo.writeMicroseconds(1500);
      rightServo.writeMicroseconds(1500);
      delay(time);
    }

    void forwardTime( unsigned int time ) {
      forward();
      delay(time);
      stop();
    }

    // L = 1000 -> ff. 1250 -> f || R = 1750 -> f. R = 2000 -> ff
    void turn(int direction, int degrees) {
      // turning RIGHT: -> direction = 1
      //   lS = 1500 - 250 = 1250 ... turns forward
      //   rS = 1500 - 250 = 1250 ... turns backw  => turn RIGHT
      //
      // turn LEFT: -> direction = -1
      //   lS = 1500 + 250 = 1750 ... turns backw
      //   rS = 1500 + 250 = 1750 ... turns forw  => turn LEFT
      leftServo.writeMicroseconds(1500 - power * direction);
      rightServo.writeMicroseconds(1500 - power * direction);
      delay(degrees * 5.5);
      stop();
    }

    void square(unsigned int time)
    {
      for (int i = 0; i < 4; ++i) {
        forwardTime(time);
        turn(LEFT, 90);
      }
    }

    // ensure that left is always going forward. hence staying within [1000 and 1500]
    int maxL( int power )
    {
      if( power < 1000 )
        return 1000;
      if( power > 1500 )
        return 1500;
      else
        return power;
    }

    // ensure that right is always going forward. hence staying within [1500 and 2000]
    int maxR( int power )
    {
      if( power < 1500 )
        return 1500;
      if( power > 2000 )
        return 2000;
      else
        return power;
    }

    // L = R = 1500 -> STOP
    // L = 1000 -> ff. 1250 -> f   ||   1750 -> b   2000 -> bb
    // R = 2000 -> ff  1750 -> f   ||   1250 -> b   1000 -> bb
    void forwardProportional(int error)
    {
      const byte kp = 1.4;

      // e.g. error = 400 .. close to right wall. i want to steer left. means more right power
      // -> lP = 1500 - 250 - 2*400 = 1250 + 800 = 2050 .. l backw   for error > 0 :  values can be within [1250, 3000]  -> more b
      // -> rP = 1500 + 250 + 2*400 = 1750 + 800 = 2550 .. r forward

      int leftPower  = maxL( 1500 - power + kp * error );
      int rightPower = maxR( 1500 + power * 1.06 + kp * error );
#ifdef DEBUG
      Serial.print("leftPower: "); Serial.print( leftPower );
      Serial.print(" / rightPower: "); Serial.println( rightPower );
      Serial.println(" ");
#endif      
      leftServo.writeMicroseconds(leftPower);
      rightServo.writeMicroseconds(rightPower);
      
    }

};

