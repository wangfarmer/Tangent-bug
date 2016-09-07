#include "BugAlgorithms.hpp"

BugAlgorithms::BugAlgorithms(Simulator * const simulator)
{
    m_simulator = simulator;   
    //add your initialization of other variables
    //that you might have declared

    m_mode = STRAIGHT;
    m_hit[0] = m_hit[1] = HUGE_VAL;
    m_leave[0] = m_leave[1] = 0;
    m_distLeaveToGoal = HUGE_VAL;    
}

BugAlgorithms::~BugAlgorithms(void)
{
    //do not delete m_simulator  
}

Move BugAlgorithms::Bug0(Sensor sensor)
{
    //add your implementation
  //First, get my position.
  double xCurr = m_simulator->GetRobotCenterX();
  double yCurr = m_simulator->GetRobotCenterY();
  //The goal's position.
  double xGoal = m_simulator->GetGoalCenterX();
  double yGoal = m_simulator->GetGoalCenterY();

  //Variables to store future move values.
  double moveX;
  double moveY;


  if(amITooClose(sensor)){
    //Perpendicular vector of (a,b) = (-b, a) (for left turn)
    
    double xDistObstacle = sensor.m_xmin - xCurr;
    double yDistObstacle = sensor.m_ymin - yCurr;
    Move perpendVector = getPerpendUnitVector(xDistObstacle, yDistObstacle);
    moveX = perpendVector.m_dx;
    moveY = perpendVector.m_dy;
    
  }

  else{
    moveX = (xGoal - xCurr)/m_simulator->GetDistanceFromRobotToGoal();
    moveY = (yGoal - yCurr)/m_simulator->GetDistanceFromRobotToGoal();
  }

  Move move ={moveX,moveY};
  

  return move;
}

Move BugAlgorithms::Bug1(Sensor sensor)
{
    //add your implementation
    Move move ={0,0};

    return move;
}

Move BugAlgorithms::Bug2(Sensor sensor)
{
  
    //add your implementation
    Move move ={0,0};
    
    return move;
}



bool BugAlgorithms::amITooClose(Sensor sensor)
{
  if(sensor.m_dmin <= m_simulator->GetWhenToTurn()){
    return true;
  }

  return false;
}

//Calculates the left-turning vector for a given vector (like the nearest
//obstacle vector, for instance) in the following way:

//X_perp = -y/|vector|
//Y_perp = x/|vector|

//Returns a Move struct.
Move BugAlgorithms::getPerpendUnitVector(double x, double y){
  //Do some vector arithmetic to find the unit vector, adapting magnitude
  //to our step size.
  
  double newX = -y/getMagnitude(-y,x); 
  double newY = x/getMagnitude(-y,x);

  Move newVector = {newX, newY};

  return newVector;
}

//Calculates the magnitude of the given vector.
double BugAlgorithms::getMagnitude(double x, double y){

  return sqrt((x * x)+(y * y));
}
       

