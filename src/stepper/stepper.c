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

void shutdownStepper()
{
  move(SLOW, LOW, LOW, LOW, LOW);
}

void move(int speed, int in1, int in2, int in3, int in4)
{
  digitalWrite(IN1, in1);
  digitalWrite(IN2, in2);
  digitalWrite(IN3, in3);
  digitalWrite(IN4, in4);
  delay(speed);
}

void clockwise(int speed)
{
  move(speed, HIGH, LOW, LOW, LOW);
  move(speed, HIGH, HIGH, LOW, LOW);
  move(speed, LOW, HIGH, LOW, LOW);
  move(speed, LOW, HIGH, HIGH, LOW);
  move(speed, LOW, LOW, HIGH, LOW);
  move(speed, LOW, LOW, HIGH, HIGH);
  move(speed, LOW, LOW, LOW, HIGH);
  move(speed, HIGH, LOW, LOW, HIGH);
}

void counterClockWise(int speed)
{
  move(speed, HIGH, LOW, LOW, HIGH);
  move(speed, LOW, LOW, LOW, HIGH);
  move(speed, LOW, LOW, HIGH, HIGH);
  move(speed, LOW, LOW, HIGH, LOW);
  move(speed, LOW, HIGH, HIGH, LOW);
  move(speed, LOW, HIGH, LOW, LOW);
  move(speed, HIGH, HIGH, LOW, LOW);
  move(speed, HIGH, LOW, LOW, LOW);
}

void centerTurret(int speed)
{
  int round = 600 * speed;
  int counter = 0;
  while (counter < round)
  {
    clockwise(speed);
    counter++;
  }
  counter = 0;
  round = 260 * speed;
  while (counter < round)
  {
    counterClockWise(speed);
    counter++;
  }
}