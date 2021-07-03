#include <wiringPi.h>
#include "stepper.h"

void setupStepper()
{
  wiringPiSetup();
  delay(10);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void stopStepper()
{
  move(SLOW, LOW, LOW, LOW, LOW);
}

void shutdownStepper()
{
  stopStepper();
}

void move(int speed, int in1, int in2, int in3, int in4)
{
  digitalWrite(IN1, in1);
  digitalWrite(IN2, in2);
  digitalWrite(IN3, in3);
  digitalWrite(IN4, in4);
  delay(speed);
}

void clockwise(int speed, int steps)
{
  int counter = 0;
  while (counter < steps)
  {
    counter++;
    move(speed, HIGH, LOW, LOW, LOW);
    move(speed, HIGH, HIGH, LOW, LOW);
    move(speed, LOW, HIGH, LOW, LOW);
    move(speed, LOW, HIGH, HIGH, LOW);
    move(speed, LOW, LOW, HIGH, LOW);
    move(speed, LOW, LOW, HIGH, HIGH);
    move(speed, LOW, LOW, LOW, HIGH);
    move(speed, HIGH, LOW, LOW, HIGH);
  }
  stopStepper();
}

void counterClockwise(int speed, int steps)
{
  int counter = 0;
  while (counter < steps)
  {
    counter++;
    move(speed, HIGH, LOW, LOW, HIGH);
    move(speed, LOW, LOW, LOW, HIGH);
    move(speed, LOW, LOW, HIGH, HIGH);
    move(speed, LOW, LOW, HIGH, LOW);
    move(speed, LOW, HIGH, HIGH, LOW);
    move(speed, LOW, HIGH, LOW, LOW);
    move(speed, HIGH, HIGH, LOW, LOW);
    move(speed, HIGH, LOW, LOW, LOW);
  }
  stopStepper();
}

void centerTurret(int speed)
{
  int round = 600 * speed;
  clockwise(speed, round);
  round = 260 * speed;
  counterClockwise(speed, round);
}