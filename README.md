# multitask
### Multi Tasking Blink, Beep, And Serial Read/Write of WifiBoy esp32

####1. project created

2017-08-12 14:40 derek@wifiboy.org & samsuanchen@gmail.com

####2. first commit

2017-08-12 15:00 no compile error.
	
the tasks of keyboard reading, screen blinking, beeper huming, message writing, and 4 more buttons are initialized in **setup**.

The **RB** (Right blue) button can stop/resume beeping.
The **LG** (Left green) button can stop/resume blinking.
The **LB** (Left blue) button can rise a semitone of the beeping.
The **LY** (Left yellow) button can down a semitone of the beeping.

However, once screen blinking is stoped by the LG button, the beeper is no huming.  This is a **bug** needed to fix.

Not very ofen, it seems that RB button could no exactly stop/resume beeping.  This could also a **bug** needed to fix.