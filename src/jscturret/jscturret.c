#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include "btserver.h"
#include "btclient.h"
#include "jscturret.h"
#include "logger.h"

void deviceCallback(btDevice *device)
{
  if (strlen(device->uuid) > 0)
  {
    g_print("\n");
    g_log(LOG_CLIENT, G_LOG_LEVEL_INFO, "Device %s [%s] has service %s on path [%s].",
           device->name,
           device->address,
           device->uuid,
           device->adapter);
  }
  else
  {
    g_log(LOG_CLIENT, G_LOG_LEVEL_INFO, "\nNo device found.");
  }
}

void usage(char const *name)
{
  printf("Usage: %s [register|discover]", name);
}

int main(int argc, char const *argv[])
{
  if (argc > 1)
  {
    if (strcmp("register", argv[1]) == 0)
    {
      set_logger(LOG_SERVER);
      g_log(LOG_SERVER, G_LOG_LEVEL_INFO,"Starting registration of service \"%s\" of UUID [%s].", BLUETOOTH_SERVICE_NAME, BLUETOOTH_SERVICE_UUID);

      int result = register_service(
          BLUETOOTH_SERVICE_PATH,
          BLUETOOTH_SERVICE_NAME,
          BLUETOOTH_SERVICE_CHANNEL,
          BLUETOOTH_SERVICE_UUID);
      if (result != RESULT_OK)
      {
        g_log(LOG_SERVER, G_LOG_LEVEL_ERROR, "Error %d occured while registering.", result);
      }
    }
    else if (strcmp("discover", argv[1]) == 0)
    {
      set_logger(LOG_CLIENT);

      g_log(LOG_CLIENT, G_LOG_LEVEL_INFO, "Starting discovery of UUID [%s].", BLUETOOTH_SERVICE_UUID);
      int result = discover_service(deviceCallback, BLUETOOTH_SERVICE_UUID, BLUETOOTH_DISCOVERY_TIMEOUT_SECONDS);
      if (result != RESULT_OK)
      {
        g_log(LOG_CLIENT, G_LOG_LEVEL_ERROR, "Error %d occured while searching for device.", result);
      }
      else
      {
        g_log(LOG_CLIENT, G_LOG_LEVEL_INFO, "Discovery completed successfully.");
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