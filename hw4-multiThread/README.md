# CS344-assignment4-counter

Both threads must share one mutex and two condition variables to control and protect the counting of a number. The number must count from a starting value of 0 to 10, by ones, at which point the program will end. You get to decide which parts of the incrementation and printing go in each thread.

Your variables must be named as follows:

* Your mutex must be named "myMutex".
* Your two conditions variables must be named "myCond1" and "myCond2".
* Your counting variable must be named "myCount".

# Output
Your program must output lines that contain the following text exactly as written, at the following times. No other lines are allowed to be in the output:

* When your program begins:

  `PROGRAM START`
* When thread 2 is created:

  `CONSUMER THREAD CREATED`
* When myCount changes value:

  `myCount: <PREVIOUS#> -> <NEW#>`
* Example:

  `myCount: 1 -> 2`
* When myMutex is unlocked:

  `<THREAD>: myMutex unlocked`
* Example:

  `CONSUMER: myMutex unlocked`
* When myMutex is locked:

  `<THREAD>: myMutex locked`
* Example:

  `CONSUMER: myMutex locked`
* When myCond1 or myCond2 has pthread_cond_wait() called on it:

  `<THREAD>: waiting on <CONDITION VAR>`
* Example:

  `PRODUCER: waiting on myCond1`
* When myCond1 or myCond2 has pthread_cond_signal() called on it:

  `<THREAD>: signaling <CONDITION VAR>`
* Example:

  `CONSUMER: signaling myCond1`
* When your program ends:

  `PROGRAM END`
