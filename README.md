# laterAndForget
Controlling arduino with onChange() later() and forget()




Just like human life a large part of the life of an arduino consists of waiting. If you have a somewhat complex project  the arduino is waiting for many different I/O changes and timeouts. This is especially true if you are controlling a mechanical machine like a 3d printer.

The aim of this project is to centralize all the waiting so that the program is easier to read and to maintain. 

The first part is the onChange macro. When a input pin changes the code block after the macro is executed. In the code part there is a local boolean variable "hi" that indicates that the pin went to HIGH value. Example: turn led (output 13) on when pin 6 goes high:

    onChange(6) {
      if (hi)
        digitalWrite(13,HIGH) ; 
    }

The second part is the later function. The later function let you start another function a number of milliseconds later. Example: flash led  for 500 ms when input 6 goes high.

    onChange(6) {
      if (hi) {
        digitalWrite(13,HIGH) ;
        later(500,ledlow) ; } 
    }

    void ledlow(void) {
      digitalWrite(13,LOW) ; }

The forget function makes that a function that is scheduled by later() is not executed. For example key debounce code. I want to change the led when input 6 goes high but only if it stays high for more than 20ms. 

    onChange(6) {
    if (hi)
      later(20,debounced) ;
    else
      forget(debounced) ; }

    void debounced(void) {
      digtitalWrite(13,!digitalread(13)) ;}

The "addSetup" macro let you add some code to the setup part of the arduino code. The use of this function makes that you can keep all the code related to a group of I/O together. 
example: flash the led 2 times per second when program is running.

    #define LED 13
    void ledblink(void) {
      addSetup pinMode(LED,OUTPUT) ;
      addSetup later(1000,ledblink) ;
      digtitalWrite(LED,!digitalread(LED)) ;
      later(250,ledblink) ; }

The addSetup macro expands to "if(0)" so the code behind it is not executed in this function. The code is added to a EvStartMacro. This macro is used in the setup section of the sketch.

How does it work.

The project consists of some code in later.h and a prepreprocessor written in c. 
The code in later.h maintains a linked list of upcomming timed functions (also called events). In the loop section of the sketch there is a call to peek_event(). This function checks wether the next upcomming event should be executed.

The description here is for windows. I am sure the same kind of things can be done in linux or any other os.
The prepreprocessor (scanharvest.exe) must be run each time before you compile the sketch. It creates a header file called "harvested.h". It scans for the "onChange" and "addSetup" keywords. It creates a code that must be executed on setup or in the loop function.

You can use the prepreprocessor in two different ways you can run it in a batch file before you compile Or you can integrate it in the arduino system. 

Running from a batch file:

If your sketch is called test.ino copy scanharvest.exe to the project folder. Create a batchfile with the line "scanharvest test.ino" in the project folder.

I found a way to integrate it in the arduino system. I am not an arduino expert so it might not be the best way. This is for letting it work with the arduino uno.

find the file
  (...)\hardware\arduino\avr\platform.txt

In this file replace the line "compiler.cpp.cmd=avr-g++" in "compiler.cpp.cmd=avrcompil.bat"
find the folder:
(...)\hardware\tools\avr\bin

copy scanharvest.exe to this folder
create a batchfile avrcompil.bat with 2 lines int this folder:

(...)\hardware\tools\avr\bin\scanharvest.exe %*

(...)\hardware\tools\avr\bin\avr-g++.exe %*

The (...) part in this description is dependant on how you installed arduino and what version you are using.

If you want to use this system a ino file, an empty project looks like this:

    #include "Arduino.h"
    #include "later.h"  
    #include "harvested.h" 
    /** insert your code here */
    void setup(void) {
        EvStartMacro  
    }


    void loop() {
       EvLoopMacro ;
       peek_event() ;
    }

Some advanced topics:
 
You can use later() with micro seconds resolution  with the line:

#define microsLater

This line must be in your sketch before the line were you include later.h

With the line:

#define withpayload

The payload system is enabled. The payload variable is an integer. When later() is called the value of payload is stored in the time Queue. When after some time the the event function is started the value from the queue is copied back to payload.

When later() is called earlier references to the same event function are removed from the queue. The function alsoLater() works the same as later() but earlier references are not removed. Normally there are only 20 positions in the queue so alsoLater() should be used with care. 

The combination of alsoLater() and payload enables you to make very powerfull programs with few lines. The following code flashes 10 outputs each with a slightly different interval. On an arduino based industrial shields plc it creates a "war games" retro computer visualisation.

    // output pins of "industrial shields" plc
    int Q[10] = {13,11,10,9,6,5,3,7,1,0 } ;

    bool flips[10] ;

    void startmulti(void) {
      addSetup later(10,startmulti) ;
      for (payload=0;payload<=9;payload++) {
        pinMode(Q[payload],OUTPUT) ;
        alsoLater(200+20*payload,multiflash) ; }
    }

    void multiflash(void) {
      flips[payload] = !flips[payload] ;
      digitalWrite(Q[payload],flips[payload]) ;
      alsoLater(200+20*payload,multiflash) ;
    }





