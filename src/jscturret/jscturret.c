#include <stdio.h>
#include "btservice.h"

void deviceCallback(btDevice *device)
{
  if (strlen(device->uuid) > 0)
  {
    printf("Device %s [%s] has service %s.\n",
           device->name,
           device->address,
           device->uuid);
  }
  else
  {
    printf("No device found.\n");
  }
}

int main(int argc, char const *argv[])
{
  discoverService(deviceCallback);
}