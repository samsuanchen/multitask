/*
 * multiTask.h 2017-08-13 10:10
 * derek@wifiboy.org & samsuanchen@gmail.com
 */
#include <arduino.h>
///////////////////////////////////////////////////////////////////////
//    Multi Tasking Blink, Beep, And Read/Write for WifiBoy esp32    //
///////////////////////////////////////////////////////////////////////
// 01. arduino serial input/output
//.......................................................................................
#define PRINTF    Serial.printf
#define PRINTLN   Serial.println
#define PRINT     Serial.print
#define WRITE     Serial.write
#define AVAILABLE Serial.available
#define READ      Serial.read
/////////////////////////////////////////////////////////////////////////////////////////
// 02. process at most 32 tasks, 32 output lines of len<128, 32 input lines of len<1024.
//.......................................................................................
#define mTask 32
#define mOut  32
#define mInp  32
#define mTib  1024
/////////////////////////////////////////////////////////////////////////////////////////
// 11. FuncP -- the type of pointer to run the function code.
//.......................................................................................
typedef void (* FuncP)();
/////////////////////////////////////////////////////////////////////////////////////////
// 12. Button -- the type of button having 5 fields and 2 functions.
//.......................................................................................
typedef struct Button {  // the type of button.
  char * name;           // the name of button.
  int pin;               // the gpio pin number of button. 
  FuncP onPressUp;       // the code to run on button press up.
  int level_1;           // the HIGH/LOW pin last level of button.
  int level_2;           // the HIGH/LOW pin last level of button.
  void check () {
    int x = digitalRead( pin );
    if ( level_1==LOW && level_2==LOW && x==HIGH )
      onPressUp();
    level_1 = x, level_2 = level_1;
  }
};
/////////////////////////////////////////////////////////////////////////////////////////
// 13. Task -- the task type haveing 7 fields and 1 function as follows.
//.......................................................................................
typedef struct Task {    // the type of Task.
  char*name;             // task name.
  int timeDelay; // time delay to wake up this task (in micro seconds).
  int times;             // number of times to run this task (NOTE! -1 forever, 0 to remove).
  unsigned long lastTime;// wait until micros()-lastTime > timeDelay (in micro seconds).
  FuncP code;            // function code to run this task.
  int data;
  int stop = 0;          // stop running of this task
  void toggle() {
    stop = 1 - stop;
    PRINTF( " %s %s\nat %06d", stop ? " stop " : "resume", name , millis() );
  }
};
/////////////////////////////////////////////////////////////////////////////////////////
// 21. tasks -- the task list
//.......................................................................................
Task   * tasks[mTask];   // the task list.
Task   * _task;          // the working task.
int      nTask = 0;      // number of tasks in the task list is 0 initially.
int      iTask;          // the index of working Task.
Button * _button;        // the working button.
/////////////////////////////////////////////////////////////////////////////////////////
// 22. outLines -- the output line circular buffer.
//.......................................................................................
char * outLines[mOut];
int    nOut = 0; // number of lines in the output line circular buffer.
int    sOut = 0; // index of the start line in the output line circular buffer.
Task * _writing;
/////////////////////////////////////////////////////////////////////////////////////////
// 23. inpLines -- the input line circular buffer.
//.......................................................................................
char * inpLines[mInp];
int    nInp =     0; // number of lines in input line circular buffer.
int    sInp =     0; // index of the start line in the input line circular buffer.
char * tib;          // the terminal input buffer.
int    nTib =     0; // number of characters in the terminal input buffer.
Task * _reading;     //
/////////////////////////////////////////////////////////////////////////////////////////
// 31. error -- report error and wait forever (in case a serious error happens)
//.......................................................................................
void error ( char * fmt, ... ) {
  char buf[128];
  va_list args;
  va_start (args, fmt);
  vsnprintf (buf, 128, fmt, args);
  va_end (args);
  PRINTF(buf);
  PRINTF("waiting for hardware reset (press back side RESET button)\n");
  while (true);
}
