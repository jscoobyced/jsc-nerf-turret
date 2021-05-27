#include <stdio.h>
#include "btserver.h"
#include "btclient.h"
#include "jscturret.h"

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

void usage(char const *name)
{
  printf("Usage: %s [register|discover]\n", name);
}

int main(int argc, char const *argv[])
{
  if (argc > 1)
  {
    if (strcmp("register", argv[1]) == 0)
    {
      printf("Starting registration of service \"%s\" of UUID [%s].\n", BLUETOOTH_SERVICE_NAME, BLUETOOTH_SERVICE_UUID);

      int result = register_service(
          BLUETOOTH_SERVICE_PATH,
          BLUETOOTH_SERVICE_NAME,
          BLUETOOTH_SERVICE_CHANNEL,
          BLUETOOTH_SERVICE_UUID);
      if (result != RESULT_OK)
      {
        printf("Error %d occured while registering.\n", result);
      }
    }
    else if (strcmp("discover", argv[1]) == 0)
    {
      printf("Starting discovery of UUID [%s].\n", BLUETOOTH_SERVICE_UUID);
      int result = discover_service(deviceCallback, BLUETOOTH_SERVICE_UUID, BLUETOOTH_DISCOVERY_TIMEOUT_SECONDS);
      if (result != RESULT_OK)
      {
        printf("Error %d occured while searching for device.\n", result);
      }
      else
      {
        printf("Discovery completed successfully.\n");
      }
    }
    else
    {
      usage(argv[0]);
    }
  }
  else
  {
    usage(argv[0]);
  }
}