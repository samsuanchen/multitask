/*
   multiTask.ino 2017-08-12 14:40
   derek@wifiboy.org & samsuanchen@gmail.com
*/
#include "multiTask.h"
void setup () {
  //.......................................................................................
  Serial.begin( 115200 );              // set baud rate to open the serial com port
  /*
  int go=1;
  int n,nMin=0,nMax=0;
  int d,dMin,dMax;
  int it=0;
  unsigned long currT;
  unsigned long T[200];
  unsigned long lastT=micros();
  for(int i=0;i<200;i++)T[200]=0xa5a5a5a5;
  do{
    n=0;
    while((currT=micros())==lastT)n++;
    d=currT-lastT;
    if(!nMin&&!nMax)nMin=nMax=n,dMin=dMax=d;
    if(nMin>n){
      PRINTF("\nat %d nMin %d > n %d",millis(),nMin,n);
      nMin=n;
    } else if(nMax<n){
      PRINTF("\nat %d nMax %d < n %d",millis(),nMax,n);
      nMax=n;
    }
    if(currT<lastT) {
      go=0;
      PRINTLN();
      for(int i=it-199, j=0;i<=it;i++){
        PRINTF("%12x ",T[i%200]);
        if(++j%5==0)PRINTLN();
      }
      error("\nat %d lastT 0x%x +1 0x%x +2 0x%x currT %d n %d .. %d d %d .. %d\n",
        millis(),lastT,lastT+1,lastT+2,currT,nMin,nMax,dMin,dMax);
    }
    T[(it++)%200]=currT;
    if(dMin>d) dMin=d;
    else if(dMax<d) dMax=d;
    lastT=currT;
  } while(go);
  */
  //.......................................................................................
  initReading ();                      // initialize readingKeyboard
  //initTiming  ();
  initBlinking();                      // initialize blinkingScreen
  initHumming ();                      // initialize hummingBeeper
  initWriting("Hello? World?\n");      // initialize writingMessage of "Hello? World?\n"
  initWriting("01234! 56789!\n");      // initialize writingMessage of "01234! 56789!\n"
  addButton("LR", 32, toggleHumming ); // initialize checkingButton for toggleHumming
  addButton("LG", 17, toggleBlinking); // initialize checkingButton for toggleBlinking
  addButton("LB", 33, riseSemiTone  ); // initialize checkingButton for riseSemiTone
  addButton("LY", 27, downSemiTone  ); // initialize checkingButton for downSemiTone
  addButton("RB", 34, riseOctave    ); // initialize checkingButton for riseOctave
  addButton("RY", 35, downOctave    ); // initialize checkingButton for downOctave
}
//.......................................................................................
void loop () {
  runTasks ();    // reading, blinking, humming, writing, and checkingButton
}









/////////////////////////////////////////////////////////////////////////////////////////
// 32. addTask -- add a Task into the Task List
//.......................................................................................
void addTask ( char*name, double timeDelay, void code(), int times, int data ) {
  PRINTF( "\nat %06d attach task: ", millis() );
  if ( code == checkingButton ) PRINT( "checkingButton " );
  PRINTF( "%s 0x%x ", name, code );
  if ( nTask >= mTask )
    error("addTask(\"%s\",%e,0x%x,%d) as task %d ?? the task list full\n",
          name, timeDelay, code, times, nTask);
  _task = (Task *) malloc( sizeof( Task ) );
  if ( ! _task )
    error("addTask(\"%s\",%e,0x%x,%d) as task %d ?? no space allocated\n",
          name, timeDelay, code, times, nTask);
  _task->stop      = 0;
  _task->name      = name;
  _task->times     = times;
  _task->lastTime  = micros();
  _task->timeDelay = timeDelay;
  _task->code      = code;
  _task->data      = data;
  tasks[nTask++]   = _task;
}
/////////////////////////////////////////////////////////////////////////////////////////
// 33. runTasks -- run all tasts (should be in the onTimer the Arduino loop).
//.......................................................................................
void runTasks () {
  for ( iTask = 0; iTask < nTask; iTask++ ) {
    _task = tasks[iTask];
    if ( _task->stop ) continue;
    long t = micros();
    long lastTime = _task->lastTime;
    int d = t>lastTime ? t - lastTime : 0x11111112 - lastTime + t;
    //PRINTF( "\nat %d lastTime %d 0x%x currTime %d 0x%x d %d 0x%x LONG_MAX 0x%x\n", millis(), lastTime, lastTime, t, t, d, d, LONG_MAX );
    //error( "\nat %d recalculate d %d %x", millis(), d, d );
    if ( d<0 ) {
      PRINTF( "\n(long)0x11111111-lastTime = %d lastTime %d 0x%x currTime %d 0x%x d %d 0x%x LONG_MAX 0x%x\n", millis(), lastTime, lastTime, t, t, d, d, LONG_MAX );
      error( "\nat %d lastTime %d 0x%x currTime %d 0x%x d %d 0x%x LONG_MAX 0x%x\n", millis(), lastTime, lastTime, t, t, d, d, LONG_MAX );
    }
    if ( d < _task->timeDelay ) continue;
    _task->lastTime = t;
    if ( _task->times >= 0 ) _task->times--;
    _task->code();
  }
  // remove all the tasks of times=0 from the task list
  int n;
  for ( n = 0; n < nTask; n++ ) {
    _task = tasks[n];
    if ( ! _task->times ) break;
  }
  if ( n == nTask ) return; // no task of times==0
  PRINTF("\nat %06d remove task: %s", millis(), tasks[n]->name);
  for ( int i = n + 1; i < nTask; i++ ) {
    if ( tasks[i]->times )
      tasks[n++] = tasks[i]; // keep all the tasks of times!=0
    else
      PRINTF(" %s", tasks[i]->name);
  }
  nTask = n;
}
/////////////////////////////////////////////////////////////////////////////////////////
// 40. timeing
//.......................................................................................
void showingTime () {
  PRINTF( "\nat %06d", millis() );
}
void initTiming () {
  addTask( "showingTime", 1000, showingTime, -1, 0 );
}
/////////////////////////////////////////////////////////////////////////////////////////
// 41. writing
//.......................................................................................
void writingMessage () {
  int iOut = sOut++ % mOut;
  PRINT( outLines[ iOut ] ); // print next line.
  if ( --nOut )  return; // if the next line is waiting
  _task->times = 0; // remove this writing task.
}
/////////////////////////////////////////////////////////////////////////////////////////
// 42. initWriting -- formated print args according to given format
//.......................................................................................
void initWriting ( char * fmt, ... ) {         // print args according to given format
  char buf[128];                                     // at most 127 characters !!!!!!
  if ( nOut >= mOut ) return;                        // ignore writingMessage if too many lines waiting
  va_list args;                                      // get variable argument list
  va_start (args, fmt);                              // set the start of variable argument list
  vsnprintf (buf, 128, fmt, args);                   // format the output string to buffer
  va_end (args);                                     // end of formating
  char * out = (char *) malloc( strlen(buf) + 1 );   // allocate space to save the string in buffer
  if ( ! out )                                       // if no space for output
    error("initWriting( \"%s\", ... ) ?? no more space is allocated\n", fmt);
  strcpy(out, buf);                                  // copy the string in buffer
  if ( ! nOut ) {                                    // if no exsiting message yet
    addTask( "writingMessage", 1, writingMessage, -1, 0 );     // activate the writing task
    _writing = _task;
  }
  outLines[ (sOut + nOut++) % mOut ] = out;   // save into the circular output buffer
}
/////////////////////////////////////////////////////////////////////////////////////////
// 51. readingKeyboard -- Fill each character into the terminal input buffer.
//.......................................................................................
void readingKeyboard () {
  if ( ! AVAILABLE() ) return;
  char c = READ();
  if ( c == '\b' ) {
    if ( ! nTib ) return;
    nTib--;
    PRINTF("\b \b");
    return;
  }
  if ( nTib == mTib || c == '\r' || c == '\n' ) {
    if ( nTib == mTib )
      PRINTF("readingKeyboard() !! break input line by len=%d\n", nTib);
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
  tib = (char *) malloc(mTib + 1);
  addTask( "readingKeyboard", 1, readingKeyboard, -1, 0 ); // activate the writing task
}
/////////////////////////////////////////////////////////////////////////////////////////
// 53. procLine -- Process the terminal input buffer.
//.......................................................................................
void procLine () {
  PRINTF(" %d characters filled in tib.\nat %06d ", millis(), nTib);
  for ( int i = 0; i < nTib; i++ ) PRINTF("%02x ", *(tib + i) );
  PRINTLN();
}
/////////////////////////////////////////////////////////////////////////////////////////
// 61. plinking.
//.......................................................................................
Task * _blinking;
int screen = 4; // gpio pin number 4 for screen (16 for led )
void blinkingScreen () { // toggle screen after each time delay
  digitalWrite( screen, ! digitalRead( screen ) ); // toggle the screen pin level
}
void initBlinking () { // activate blinking
  pinMode( screen, OUTPUT );
  digitalWrite( screen, HIGH );
  addTask( "blinkingScreen", 500000, blinkingScreen, -1, HIGH ); // add blinking forever task
  _blinking = _task;
}
void toggleBlinking () { // Toggle the blinking task.
  _blinking->toggle();
  if ( _blinking->stop ) digitalWrite( screen, HIGH );
}
/////////////////////////////////////////////////////////////////////////////////////////
// 62. humming.
//.......................................................................................
Task * _humming ;
int beeper = 25; // gpio numbers of beeper
void hummingBeeper () {
  digitalWrite( beeper, ! digitalRead( beeper ) );
}
void initHumming () { // activate humming
  pinMode( beeper, OUTPUT );     // set pin mode of the led direction as output
  digitalWrite( beeper, HIGH );  // turn off the beeper
  addTask( "hummingBeeper", 2272.7272727272725, hummingBeeper, -1, HIGH ); // add the humming of A4 task
  _humming = _task;
}
void toggleHumming () {
  _humming->toggle();
}
void riseSemiTone () {
  _humming->timeDelay = _humming->timeDelay / 1.059463;
  PRINTF( " humming riseSemiTone to duration %d us\nat %06d", _humming->timeDelay, millis() );
}
void downSemiTone () {
  _humming->timeDelay = _humming->timeDelay * 1.059463;
  PRINTF( " humming downSemiTone to duration %d us\nat %06d", _humming->timeDelay, millis() );
}
void riseOctave () {
  _humming->timeDelay = _humming->timeDelay / 2;
  PRINTF( " humming riseOctave to duration %d us\nat %06d", _humming->timeDelay, millis() );
}
void downOctave () {
  _humming->timeDelay = _humming->timeDelay * 2;
  PRINTF( " humming downOctave to duration %d us\nat %06d", _humming->timeDelay, millis() );
}
/////////////////////////////////////////////////////////////////////////////////////////
// 71. The buttons.
//.......................................................................................
void checkingButton () {
  _button = (Button *) _task->data;
  int x = digitalRead( _button->pin );
  if ( _button->level_1 == LOW && _button->level_2 == LOW && x == HIGH )
    _button->onPressUp();
  _button->level_1 = x;
  _button->level_2 = _button->level_1;
}
void addButton ( char*name, int pin, void onPressUp() ) {
  _button = (Button *) malloc( sizeof( Button ) );
  if ( ! _button )
    error( "addButton(\"%s\",%d,0x%x,%d) ?? no space allocated\n", name, pin, onPressUp );
  pinMode( pin, INPUT );
  _button->name      = name;
  _button->pin       = pin;
  _button->onPressUp = onPressUp;
  _button->level_1   = HIGH;
  _button->level_2   = HIGH;
  addTask( name, 10000, checkingButton, -1, (int)_button ); // every 10 milli second
}
