# multitask
### Multi Tasking Blink, Beep, And Serial Read/Write of WifiBoy esp32

####1. project created

2017-08-12 14:40 derek@wifiboy.org & samsuanchen@gmail.com

####2. the first commit

2017-08-12 15:00 no compile error.
	
the tasks of keyboard reading, screen blinking, beeper huming, message writing, and 4 more buttons are initialized in **setup**.

The **RB** (Right blue) button can stop/resume beeping.
The **LG** (Left green) button can stop/resume blinking.
The **LB** (Left blue) button can rise a semitone of the beeping.
The **LY** (Left yellow) button can down a semitone of the beeping.

However, once screen blinking is stoped by the LG button, the beeper is no huming.  This is a **bug** needed to fix (fixed in the second commit).

Not very ofen, it seems that RB button could no exactly stop/resume beeping.  This could also a **bug** needed to fix (fixed in the second commit).

####3. the second commit

2017-08-14 00:34 fixed bug in runTask (using continue instead of return), changed naming, fixed button checking (duration of 10 ms).  Seems now every thing is okay.

However, to split into 2 files by moving codes to multitask.h, for example the code addTask, is not that easy.

####4. redefine button actions

2017-08-17 10:46 use double instead of long int for timeDelay, redefine button actions

**LR** button (pin 32 Left red) to stop/resume blinking.
**LG** button (pin 17 Left green) to stop/resume humming.
**LB** button (pin 33 Left blue) to rise humming semitone.
**LY** button (pin 27 Left yellow) to down humming semitone.
**RB** button (pin 32 Right blue) to rise humming octave.
**RY** button (pin 35 Right yellow) to down humming octave.

####5. use uint16_t notes[12]