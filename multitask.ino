/*
 * multiTask.ino 2017-08-12 14:40
 * derek@wifiboy.org & samsuanchen@gmail.com
 */
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
  void init () {
    pinMode( pin, INPUT );
    level_1 = HIGH, level_2 = HIGH;
  }
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
  int timeDelay;         // time delay to wake up this task (in micro seconds).
  int times;             // number of times to run this task (NOTE! -1 forever, 0 to remove).
  unsigned long lastTime;// wait until micros()-lastTime > timeDelay (in micro seconds).
  FuncP code;            // function code to run this task.
  int data;
  int stop = 0;          // stop running of this task
  void toggle() {
    stop = 1 - stop;
  }
};
/////////////////////////////////////////////////////////////////////////////////////////
// 21. tasks -- the task list
//.......................................................................................
Task * tasks[mTask];     // the task list.
Task * _task;            // the running task.
int    nTask = 0;        // number of tasks in the task list is 0 initially.
int    iTask;            // the index of running Task.
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
/////////////////////////////////////////////////////////////////////////////////////////
// 32. addTask -- add a Task to the Task List
//     To add a tast to the task list is defined as follows.
//.......................................................................................
void addTask ( char*name, int timeDelay, void code(), int times, int data ) {
  PRINTF("at %d ms task begin: %s\n",millis(),name);
  if ( nTask >= mTask )
    error("addTask(\"%s\",%d,0x%x,%d) as task %d ?? the task list full\n",
      name,timeDelay,code,times,nTask);
  _task = (Task *) malloc( sizeof( Task ) );
  if ( ! _task )
    error("addTask(\"%s\",%d,0x%x,%d) as task %d ?? no space allocated\n",
      name,timeDelay,code,times,nTask); 
  _task->stop = 0;
  _task->name = name;
  _task->times = times;
  _task->lastTime = (unsigned long) micros();
  _task->timeDelay = timeDelay;
  _task->code = code;
  _task->data = data;
  tasks[nTask++] = _task;
}
/////////////////////////////////////////////////////////////////////////////////////////
// 33. runTasks -- run all tasts (should be in the onTimer the Arduino loop).
//.......................................................................................
void runTasks () {
  for ( iTask = 0; iTask < nTask; iTask++ ) {
    _task = tasks[iTask];
    if ( _task->stop ) return;
    unsigned long t = micros();
    unsigned long lastTime = _task->lastTime;
    unsigned long d;
    if ( t > lastTime ) d = t - lastTime;                              // if t not overflow
    else                d = t + 1 + ( (unsigned long) -1 - lastTime ); // if t     overflow
    if ( d < _task->timeDelay ) continue;
    _task->lastTime = t;
    if ( _task->times >= 0 ) _task->times--;
    _task->code();
  }
  // remove all the tasks of times=0 from the task list
  int n;
  for ( n = 0; n < nTask; n++ ) {
    if ( ! tasks[n]->times ) break;
  }
  if ( n == nTask ) return; // no task of times==0
  PRINTF("at %d ms task done: %s", millis(), tasks[n]->name);
  for ( int i = n+1; i < nTask; i++ ) {
    if ( tasks[i]->times )
      tasks[n++] = tasks[i]; // keep all the tasks of times!=0
    else
      PRINTF(" %s", tasks[i]->name);
  }
  PRINTLN();
  nTask = n;
}
/////////////////////////////////////////////////////////////////////////////////////////
// 41. writing
//.......................................................................................
void writing () {
  int iOut = sOut++ % mOut;
  PRINT( outLines[ iOut ] ); // print next line.
  if ( --nOut )  return; // if the next line is waiting
  _task->times = 0; // remove this writing task.
}
/////////////////////////////////////////////////////////////////////////////////////////
// 42. formatedWrite -- formated print args according to given format
//.......................................................................................
void formatedWrite ( char * fmt, ... ) {          // print args according to given format
  char buf[128];                                     // at most 127 characters !!!!!!
  if ( nOut >= mOut ) return;                        // ignore formatedWrite if too many lines waiting
  va_list args;                                      // get variable argument list
  va_start (args, fmt);                              // set the start of variable argument list
  vsnprintf (buf, 128, fmt, args);                   // format the output string to buffer
  va_end (args);                                     // end of formating
  char * out = (char *) malloc( strlen(buf) + 1 );   // allocate space to save the string in buffer
  if ( ! out )                                       // if no space for output
    error("formatedWrite( \"%s\", ... ) ?? no more space is allocated\n", fmt);
  strcpy(out, buf);                                  // copy the string in buffer
  if ( ! nOut ) {
    addTask( "formatedWrite", 1, writing, -1, 0 ); // activate the writing task
    _writing = _task;
  }
  outLines[ (sOut + nOut++) % mOut ] = out;   // save into the circular output buffer
}
/////////////////////////////////////////////////////////////////////////////////////////
// 51. readingKey -- Fill each character into the terminal input buffer.
//.......................................................................................
void readingKey () {
  if ( ! AVAILABLE() ) return;
  char c = READ();
  if ( c == '\b' ) {
    if ( ! nTib ) return;
    nTib--;
    formatedWrite("\b \b");
    return;
  }
  if ( nTib == mTib || c == '\r' || c == '\n' ) {
    if ( nTib == mTib )
      formatedWrite("readingKey() !! break input line by len=%d\n",
        nTib);
    * (tib + nTib) = '\0';
    procLine();
    return;
  }
  * (tib + nTib) = c;
  nTib++;
  WRITE(c);
}
/////////////////////////////////////////////////////////////////////////////////////////
// 52. initReading -- Init the terminal input buffer reading task.
//.......................................................................................
void initReading () {
  nTib = 0;
  tib = (char *) malloc(mTib+1);
  addTask( "readingKey", 1, readingKey, -1, 0 ); // activate the writing task
}
/////////////////////////////////////////////////////////////////////////////////////////
// 53. procLine -- Process the terminal input buffer.
//.......................................................................................
void procLine () {
  PRINTF("\nat %d ms %d characters filled in tib.\n", millis(), nTib);
  for ( int i = 0; i<nTib; i++ ) PRINTF("%02x ", *(tib+i) );
  PRINTLN();
}
/////////////////////////////////////////////////////////////////////////////////////////
// 61. plinking.
//.......................................................................................
Task * _blinking;
int led = 4; // gpio pin number 4 for screen (16 for led )
void ledToggle () { // toggle led after each time delay
  digitalWrite( led, ! digitalRead( led ) ); // toggle the led
}
void initBlinking () { // activate blinking
  pinMode( led, OUTPUT );
  digitalWrite( led, HIGH );
  addTask( "blink", 500000, ledToggle, -1, HIGH ); // add blinking forever task
  _blinking=_task;
}
void toggleBlinking () { // Toggle the blinking task.
  if ( _blinking->stop ) digitalWrite( led, HIGH );
  _blinking->stop = 1 - _blinking->stop;
}
/////////////////////////////////////////////////////////////////////////////////////////
// 62. beeping.
//.......................................................................................
Task * _beeping ;
int beeper = 25; // gpio numbers of beeper
void beeperToggle () { // toggle beeper
  digitalWrite( beeper, ! digitalRead( beeper ) );
}
void initBeeping () { // activate beeping
  pinMode( beeper, OUTPUT );     // set pin mode of the led direction as output
  digitalWrite( beeper, HIGH );  // turn off the beeper
  addTask( "beep", 2500, beeperToggle, -1, HIGH ); // add beeping forever task
  _beeping =_task;
}
void toggleBeeping () {
  _beeping->stop = 1 - _beeping->stop;
}
void riseBeeping () {
  long x = _beeping->timeDelay * 100000;
  _beeping->timeDelay = x / 105946;
  PRINTF("at %d ms task riseBeeping %d -- beep\n",
    millis(), _beeping->timeDelay);
}
void downBeeping () {
  long x = _beeping->timeDelay * 105946;
  _beeping->timeDelay = x / 100000;
  PRINTF("at %d ms task downBeeping %d -- beep\n",
    millis(), _beeping->timeDelay);
}
/////////////////////////////////////////////////////////////////////////////////////////
// 71. LG -- The left green button.
//.......................................................................................
Button LG = { name:"LG", pin:17, onPressUp:toggleBlinking };
/////////////////////////////////////////////////////////////////////////////////////////
// 72. RB -- The right blue button.
//.......................................................................................
Button RB = { name:"RB", pin:34, onPressUp:toggleBeeping };
/////////////////////////////////////////////////////////////////////////////////////////
// 73. LB -- The left blue button.
//.......................................................................................
Button LB = { name:"LB", pin:33, onPressUp:riseBeeping };
/////////////////////////////////////////////////////////////////////////////////////////
// 74. LY -- The left yellow button.
//.......................................................................................
Button LY = { name:"LY", pin:27, onPressUp:downBeeping };
/////////////////////////////////////////////////////////////////////////////////////////
void setup () {
//.......................................................................................
  Serial.begin( 115200 );           // set baud rate to open the serial com port
//.......................................................................................
  initReading ();                   // activate reading
  initBlinking();                   // activate blinking
  initBeeping ();                   // activate beeping
  formatedWrite("Hello? World?\n"); // activate writing of "Hello? World?\n"
  formatedWrite("01234! 56789!\n"); // activate writing of "01234! 56789!\n"
  (&RB)->init();                    // activate toggling of stop/resume  beeping
  (&LG)->init();                    // activate toggling of stop/resume blinking
  (&LB)->init();                    // activate rising  semitone of beeping
  (&LY)->init();                    // activate downing semitone of beeping
}
//.......................................................................................
void loop () {
  runTasks (),    // run tasks of blinking,  beeping,  and writing
  (&RB)->check(), // right  blue button -- stop/resume      beeping
  (&LG)->check(), // left  green button -- stop/resume     blinking
  (&LB)->check(), // left   blue button -- rise semitone of beeping
  (&LY)->check(); // left yellow button -- down semitone of beeping
}
