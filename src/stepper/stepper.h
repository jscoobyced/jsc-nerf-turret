
#define IN1 0
#define IN2 1
#define IN3 2
#define IN4 3

#define SLOW 5
#define FAST 1

void setupStepper();
void stopStepper();
void shutdownStepper();
void move(int speed, int in1, int in2, int in3, int in4);
void clockwise(int speed, int steps);
void counterClockwise(int speed, int steps);
void centerTurret(int speed);