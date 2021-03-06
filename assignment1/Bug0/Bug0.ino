#include <stdlib.h>
#include <unistd.h>

#include <FlockBotMotion.h>
#include <FlockBotLCD.h>
#include <FlockBotBasics.h>

#define DIGILEN 10

#define X_GOAL 7
#define Y_GOAL 0

//Sensor constants (mm), based on what we've seen to be 'good' values for stopping

#define RIGHT_CLEARANCE 350
#define FRONT_CLEARANCE 350

//Rotation constants (rad/sec), to ensure that the bot doesn't bank too sharply and mess itself up.

#define RIGHT_BANK -0.2
#define LEFT_BANK 0.05

char buf[DIGILEN];
char s[DIGILEN];
char s1[DIGILEN];
char state;
int clearCounter;

int16_t irCenterWindow[5];
int16_t irRightWindow[5];
int16_t irLeftWindow[5];
int16_t irFrontLeftWindow[5];
int16_t irFrontRightWindow[5];

char hasCalledDoAchPoint;
char hasCalledRotate;

//WALL FOLLOWING
char WallFollowState;

//FLAGS, for various things.

boolean amICorrectingLeft;
boolean amICorrectingRight;
boolean amIGoingForward;

float x_goal = X_GOAL * 304.8;
float y_goal = Y_GOAL * 304.8;

int cmpfunc (const void * a, const void * b)
{
  return ( *(int*)a - * (int*)b );
}

int goToGoal()
{


}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  initializeFlockBot();
  initializeMotion();
  initializeLCD();
  clearLCD();
  //Init state variables.
  state = 0;
  WallFollowState = 0;
  hasCalledDoAchPoint = 0;
  hasCalledRotate = 0;
  //Flag init
  amICorrectingLeft = false;
  amICorrectingRight = false;
  amIGoingForward = false;
  //Delay for 3 seconds, to allow for setting robot down and such.
  delay(3000);
}

void loop() {
  //  robotStateToSerial();
  ++clearCounter;
  //int8_t batVolt;
  //batVolt = getBattery();
  //printLCD(batVolt, 3, 100);
  char buf[DIGILEN];
  char buf1[DIGILEN];
  char buf2[DIGILEN];

    sprintf(s, "%d", state);
    printLCD(s, 1, 10);

    sprintf(s1, "%d", WallFollowState);
    printLCD(s1, 2, 10);
  //int16_t dis, dis2, dis3;
  // dis = getIR(pinout.irCenter);
  //  dis2 = getIR(pinout.irLeft);
  //  dis3 = getIR(pinout.irRight);


  //Here's a rolling median filter that we're using to filter out some of the sensor noise. Every five readings,
  // we take the middle value. Should be alright.
  unsigned counter;
  counter = 0;
  while (counter < 5)
  {
    irCenterWindow[counter] = getIR(pinout.irCenter);
    irRightWindow[counter] = getIR(pinout.irRight);
    irLeftWindow[counter] = getIR(pinout.irLeft);
    irFrontLeftWindow[counter] = getIR(pinout.irFrontLeft);
    irFrontRightWindow[counter] = getIR(pinout.irFrontRight);
    ++counter;
  }
  qsort(irCenterWindow, 5, sizeof(int16_t), cmpfunc);
  qsort(irRightWindow, 5, sizeof(int16_t), cmpfunc);
  qsort(irLeftWindow, 5, sizeof(int16_t), cmpfunc);
  qsort(irFrontLeftWindow, 5, sizeof(int16_t), cmpfunc);
  qsort(irFrontRightWindow, 5, sizeof(int16_t), cmpfunc);

  Serial.println("Center IR Reading:" );
  Serial.println(irCenterWindow[2]);
  Serial.flush();
  Serial.println("Front Right IR Reading: " );
  Serial.println(irFrontLeftWindow[2]);
  Serial.flush();
  sprintf(buf, "%d", irCenterWindow[3]);
  sprintf(buf1, "%d", irRightWindow[3]);
  sprintf(buf2, "%d", irLeftWindow[3]);


  // put your main code here, to run repeatedly:
  switch (state) {
    // move to goal case
    case 0:
     
    sprintf(buf2, "%d", irLeftWindow[3]);
      if ((irCenterWindow[2] > FRONT_CLEARANCE))
      {
        Serial.println("Going to state 1");
        
        Serial.flush();
        state = 1;
      }
      else
      {
        doIdle(); 
        Serial.println("There is a stupid object in my way");
        Serial.flush();
        state = 2;
      }
      printLCD(buf, 3, 10);
      printLCD(buf1, 4, 10);
      printLCD(buf2, 5, 10);
       //delay(3000);
      break;

    case 1:
   
      Serial.println("Moving forward in case 1");
      Serial.flush();
      if (!hasCalledDoAchPoint)
      {
        Serial.println("Starting aching point");
        doAchievePoint(x_goal, y_goal, GOOD_ROTATE, GOOD_FORWARD);
        hasCalledDoAchPoint = 1;
      }
      state = 0;
      break;

    // Rotate 90 case
    case 2:
    
      Serial.println("Following a wall in Case 2!");
      Serial.println("IR Sensor is ");
      Serial.println(irRightWindow[2]);
      Serial.flush();
      switch (WallFollowState) {
        case 0: //TURN PARALLEL STATE
          if (!hasCalledRotate)
          {
            Serial.println("calling rotate");
            Serial.flush();
            
            doRotate(GOOD_ROTATE - 1);
            hasCalledRotate = 1;
          
          }
          ++WallFollowState;
          break;


        case 1: // TURNING PARALLEL STATE
  
        //After seeing a wall, choose what action to take.
          Serial.println("Saw a wall choosing action");
          Serial.flush();
          if (irCenterWindow[2] < FRONT_CLEARANCE) { //See something to my front?
            WallFollowState = 1;
          //  hasCalledRotate = 1;
            Serial.println("Still rotating!");
            Serial.flush();

          }

          else if ((irCenterWindow[2] >= FRONT_CLEARANCE) && (irRightWindow[2] < RIGHT_CLEARANCE)) { //If I don't see anything in front, but see something to my right...
            doIdle();
            hasCalledRotate = 0;
            WallFollowState = 2;
            Serial.println("Nothing to front something to right,");

          }
          break;

        case 2: //COURSE CORRECTION STATE 
     
        //When I'm following the wall, check if I need to correct course.

            if ((irCenterWindow[2] < FRONT_CLEARANCE))
            {
              Serial.println("*************** Corner Detected!");
              doIdle();
              if(!hasCalledRotate){
                Serial.println("fricken rotate");
                doRotate(GOOD_ROTATE/2);
                hasCalledRotate = 1;
              }
              state = 2;
              WallFollowState = 0;
              amICorrectingLeft = false;
              amICorrectingRight = false;
              amIGoingForward = false;
              //break;
            }
        
          else if ((irRightWindow[2] < 150)) { //If I'm too close to the wall...
            if ((amICorrectingLeft == false)) {
              Serial.println("Too close, correcting left");
              doCurve(LEFT_BANK, GOOD_FORWARD);
              //doForward(GOOD_FORWARD);
              hasCalledRotate = 0;
              amICorrectingLeft = true;
              amICorrectingRight = false;
              amIGoingForward = false;
            }
            Serial.println("Too close, correcting left on outside");
            WallFollowState = 2;
          }
          else if((150 <= irRightWindow[2]) && (irRightWindow[2] <= 250)){
            if(amIGoingForward == false){
              doForward(GOOD_FORWARD);
              Serial.println("Within the strip, going forward.");
              amICorrectingLeft = false;
              amICorrectingRight = false;
              hasCalledRotate = 0;
              amIGoingForward = true;
            }
            WallFollowState = 2;
            Serial.println("right place, correcting left on outside");
          }  
          else if (((250 < irRightWindow[2])) && ((irRightWindow[2] <= 400))) { //Am I slowly veering away from the wall?
            if (amICorrectingRight == false) {
              Serial.println("Too far, correcting right.");
              doCurve(RIGHT_BANK, GOOD_FORWARD);
              //doForward(GOOD_FORWARD);
              amICorrectingLeft = false;
             hasCalledRotate = 0;
              amIGoingForward = false;
            amICorrectingRight = true;
   
            }
            WallFollowState = 2;
            Serial.println("Too far, correcting left on outside");
          }

          else { //Have I suddenly lost visual with the wall?
            //doIdle();
            amICorrectingLeft = false;
            amICorrectingRight = false;
            amIGoingForward = false;
            hasCalledRotate = 0;
            Serial.println("Lost visual with the wall, stopping.");
            WallFollowState = 3; //Case 4 doesn't exist, so we've broken out of the Wall Following state.

          }
          break;

          case 3: //Done wall following

            Serial.println("left the wall");
            Serial.flush();
           // if(irCenterWindow[2] > FRONT_CLEARANCE){ // Do I no longer see anything to the front?
             //  doIdle();
              //}
              doIdle();
                //doCurve(RIGHT_BANK, GOOD_FORWARD);
               doCurveAboutPoint(-150, 100);
               //doForwardToDistance(GOOD_FORWARD, 100);          
             WallFollowState = 0;
             state = 0;
             
            break;
            
      }//end inner switch   
      hasCalledDoAchPoint = 0;
      //delay(3000);
      break;  
      // end case in the outer switch
    case 3:
     
      //delay(3000);
      if (isDone()) {
        Serial.println("In case 3!");
        doIdle();
        state++;
        exit(0);
      }
      break;
  }

  if (!(clearCounter % 1000))
  {
    clearLCD();
  }
}


