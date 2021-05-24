#include "stepper.h"

int main(void)
{
  setupStepper();
  centerTurret(FAST);
  shutdownStepper();
  return 0;
}