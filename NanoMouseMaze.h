
#define DEBUG

#include <Arduino.h>

#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

const int neighborCells[4][2] = {
  { -1, 0},
  { 0, 1},
  { 1, 0},
  { 0, -1}
};

const byte neighborWalls[4][2] = {
  { 0, 0},
  { 0, 1},
  { 1, 0},
  { 0, 0}
};

template <byte ROWS, byte COLUMNS>
class NanoMouseMaze
{
  private:
    // vertical walls array
    boolean verticalWalls[ROWS][COLUMNS + 1];

    // horizontal walls array
    boolean horizontalWalls[ROWS + 1][COLUMNS];

  public:
    // value array
    byte values[ROWS][COLUMNS];

    byte mouseRow;
    byte mouseColumn;
    byte mouseHeading;

    byte targetRow;
    byte targetColumn;

    byte maxValue = 255;

    //Constructor method (called when the maze is created)
    NanoMouseMaze()
    {
      //initialize verticalWalls (add exterior walls)
      for (byte i = 0; i < ROWS; i++)
      {
        for (byte j = 0; j < COLUMNS + 1; j++)
        {
          if (j == 0 || j == COLUMNS)
          {
            verticalWalls[i][j] = true;
          }
        }
      }

      //initialize horizontalWalls (add exterior walls)
      for (byte i = 0; i < ROWS + 1; i++)
      {
        for (byte j = 0; j < COLUMNS; j++)
        {
          if (i == 0 || i == ROWS)
          {
            horizontalWalls[i][j] = true;
          }
        }
      }
    }

    void solve()
    {
      // initialize array with max values
      for (byte i = 0; i < ROWS; i++)
      {
        for (byte j = 0; j < COLUMNS; j++)
        {
          values[i][j] = maxValue;
        }
      }

      // set target cell
      values[targetRow][targetColumn] = 0;

      boolean continueSolving = true;
      while ( continueSolving ) {

        continueSolving = false;
        // filling surrounding cells of target cells
        for (byte i = 0; i < ROWS; i++)
        {
          for (byte j = 0; j < COLUMNS; j++)
          {
            if ( values[i][j] < maxValue )
            {
              for (byte k = 0; k < 4; ++k)
              {
                int neighborCellRow = i + neighborCells[k][0];
                int neighborCellColumn = j + neighborCells[k][1];

                byte neighborWallRow = i + neighborWalls[k][0];
                byte neighborWallColumn = j + neighborWalls[k][1];

                bool wallExists = false;

                if (k == NORTH || k == SOUTH)
                  wallExists = horizontalWalls[neighborWallRow][neighborWallColumn];
                else // must be looking at an EAST or WEST wall
                  wallExists = verticalWalls[neighborWallRow][neighborWallColumn];

                // only update elements which haven't been updated yet. and which are not blocked by a wall
                if ( values[neighborCellRow][neighborCellColumn] == maxValue && !wallExists )
                {
                  values[neighborCellRow][neighborCellColumn] = values[i][j] + 1;
                  continueSolving = true;
                }
              }
            }
          }
        }
      }
    }

    byte findBestNeighbor()
    {
      byte valueBestNeighbor = maxValue;
      byte desiredHeading = NORTH;
      
      for (byte kk = 0; kk < 4; ++kk)
      {
        byte k = (mouseHeading + kk) % 4; // start with the current mouseHeading direction. motivation: if current mouseHeading is the lowest value and other directions are same, stick to current mouse heading to avoid costly turns 
        int neighborCellRow = mouseRow + neighborCells[k][0];
        int neighborCellColumn = mouseColumn + neighborCells[k][1];

        byte neighborWallRow = mouseRow + neighborWalls[k][0];
        byte neighborWallColumn = mouseColumn + neighborWalls[k][1];

        bool wallExists = false;

        if (k == NORTH || k == SOUTH)
          wallExists = horizontalWalls[neighborWallRow][neighborWallColumn];
        else // must be looking at an EAST or WEST wall
          wallExists = verticalWalls[neighborWallRow][neighborWallColumn];

        if ( values[neighborCellRow][neighborCellColumn] < valueBestNeighbor && !wallExists )
        {
          valueBestNeighbor = values[neighborCellRow][neighborCellColumn];
          desiredHeading = k;
        }
      }
      return desiredHeading;
    }

    void addWalls( byte cardinalDirection )
    {
#ifdef DEBUG
      Serial.println(""); 
      Serial.print("AW -- cardinalDirection = "); Serial.println( cardinalDirection );
#endif
      switch( cardinalDirection )
      {
        case NORTH:
          horizontalWalls[mouseRow][mouseColumn] = true;
          break;
        case EAST:
          verticalWalls[mouseRow][mouseColumn + 1] = true;
          break;
        case SOUTH:
          horizontalWalls[mouseRow + 1][mouseColumn] = true;
          break;
        case WEST:
          verticalWalls[mouseRow][mouseColumn] = true;
          break;
      }
    }

    void addVirtualWalls()
    {
      horizontalWalls[1][2] = true;
      horizontalWalls[1][3] = true;
      horizontalWalls[1][4] = true;
      horizontalWalls[2][3] = true;
      horizontalWalls[3][4] = true;
      
      verticalWalls[1][1] = true;
      verticalWalls[3][1] = true;
      verticalWalls[1][2] = true;
      verticalWalls[2][2] = true;
      verticalWalls[2][3] = true;
      verticalWalls[2][3] = true;
      verticalWalls[2][4] = true;
      verticalWalls[2][5] = true;
    }


    /*Do not change or add code below this line

      NanoMouseMaze Print Function Version 2
      This version of the print function has been modified to print
      any size maze (the previous version could not print large
      mazes) and to work with the btMonitor Android App I wrote,
      which is available through my free online course at:
      http://udemy.com/nanomouse
      Scroll down to "Wireless Debugging with the Bluetooth Module"
      and go to the Downloadable Materials section of the lecture.*/

    void print()
    {
      for (byte i = 0; i < 2 * ROWS + 1; i++)
      {
        for (byte j = 0; j < 2 * COLUMNS + 1; j++)
        {
          //Add Horizontal Walls
          if (i % 2 == 0 && j % 2 == 1)
          {
            if (horizontalWalls[i / 2][j / 2] == true)
            {
              Serial.print(" ---");
            }
            else
            {
              Serial.print("    ");
            }
          }

          //Add Vertical Walls
          if (i % 2 == 1 && j % 2 == 0)
          {
            if (verticalWalls[i / 2][j / 2] == true)
            {
              Serial.print("|");
            }
            else
            {
              Serial.print(" ");
            }
          }

          //Add Flood Fill Values
          if (i % 2 == 1 && j % 2 == 1)
          {
            if ((i - 1) / 2 == mouseRow && (j - 1) / 2 == mouseColumn)
            {
              if (mouseHeading == NORTH)
              {
                Serial.print(" ^ ");
              }
              else if (mouseHeading == EAST)
              {
                Serial.print(" > ");
              }
              else if (mouseHeading == SOUTH)
              {
                Serial.print(" v ");
              }
              else if (mouseHeading == WEST)
              {
                Serial.print(" < ");
              }
            }
            else
            {
              byte value = values[(i - 1) / 2][(j - 1) / 2];
              if (value >= 100)
              {
                Serial.print(value);
              }
              else
              {
                Serial.print(" ");
                Serial.print(value);
              }
              if (value < 10)
              {
                Serial.print(" ");
              }
            }
          }
        }
        Serial.print("\n");
      }
      Serial.print("\n");
    }
};
