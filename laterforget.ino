#include "Arduino.h"
#include "later.h"
#include "harvested.h"

/************************************************************
*                                                           *
*  Examples of event based programming on an arduino        *
*  all this code can run together in a single arduino uno   *
*                                                           *
************************************************************/


/************ debouncing a button *************/
int push_counter ;
#define push_contact 6

onChange(push_contact) {
  if (hi)
    later(20,debounced) ;
  else
    forget(debounced) ;
}

void debounced(void) {
  push_counter++ ;
}

/******* tracking a quadrature encoder **************/
#define phaseA 3
#define phaseB 4

onChange(phaseA) {
  if (hi == digitalRead(phaseB)) 
    glob_int position++ ;
  else
    position-- ; } 
  
onChange(phaseB) {
  if (hi != digitalRead(phaseA)) 
    position++ ;
  else
    position-- ; } 

/******** controlling a heated bed  *******/

#define pt1000 A1
#define heater 5

int setpoint = 60 ;

void heatcontrol(void) {
  float r = analogRead(pt1000)/1024.0 ;
  r = 1000.0*r/(1-r) ;
  float tempr = (r-1000)*100/385.0 ;
  int heatertime = (tempr-20)*20+ (tempr-setpoint)*30 ;
  digitalWrite(heater,heatertime > 0) ;
  if (heatertime < 2000)
    later(heatertime,heaterOff) ;
  later(2000,heatcontrol) ;
  addSetup pinMode(heater,OUTPUT) ;
  addSetup later(0,heatcontrol) ; 
  }
  
void heaterOff(void) {
  digitalWrite(heater,LOW) ; }  
  
/***********************************************/

void setup(void) {
  EvStartMacro
}

void loop() {
  EvLoopMacro ;
  peek_event() ;
}
