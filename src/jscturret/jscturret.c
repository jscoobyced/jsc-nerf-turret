#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include "btserver.h"
#include "btclient.h"
#include "jscturret.h"
#include "logger.h"

void deviceCallback(btDevice *device)
{
  if (device->complete == TRUE)
  {
    g_log(JSCBT, G_LOG_LEVEL_MESSAGE, "Device %s [%s] is available.",
          device->name,
          device->address);
  }
  else
  {
    g_log(JSCBT, G_LOG_LEVEL_MESSAGE, "No device found.");
  }
}

void usage(char const *name)
{
  printf("Usage: %s [register|discover]", name);
}

int main(int argc, char const *argv[])
{
  GLogLevelFlags flag = G_LOG_LEVEL_ERROR | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_CRITICAL;
  if (argc > 2)
  {
    if (strcmp("-v", argv[2]) == 0)
    {
      flag = G_LOG_LEVEL_MASK;
    }
  }
  if (argc > 1)
  {
    if (strcmp("register", argv[1]) == 0)
    {
      g_log(JSCBT, G_LOG_LEVEL_MESSAGE, "Starting registration of service \"%s\" of UUID [%s].", BLUETOOTH_SERVICE_NAME, BLUETOOTH_SERVICE_UUID);

      int result = register_service(
          BLUETOOTH_SERVICE_PATH,
          BLUETOOTH_SERVICE_NAME,
          BLUETOOTH_SERVICE_CHANNEL,
          BLUETOOTH_SERVICE_UUID,
          flag);
      if (result != RESULT_OK)
      {
        g_log(JSCBT, G_LOG_LEVEL_ERROR, "Error %d occured while registering.", result);
      }
    }
    else if (strcmp("discover", argv[1]) == 0)
    {
      set_logger(JSCBT, flag);

      g_log(JSCBT, G_LOG_LEVEL_MESSAGE, "Starting discovery of UUID [%s].", BLUETOOTH_SERVICE_UUID);
      int result = discover_service(deviceCallback, BLUETOOTH_SERVICE_UUID, BLUETOOTH_DISCOVERY_TIMEOUT_SECONDS);
      if (result != RESULT_OK)
      {
        g_log(JSCBT, G_LOG_LEVEL_ERROR, "Error %d occured while searching for device.", result);
      }
      else
      {
        g_log(JSCBT, G_LOG_LEVEL_MESSAGE, "Completed successfully.");
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