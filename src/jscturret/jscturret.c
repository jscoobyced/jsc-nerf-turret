#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include "btcommon.h"
#include "btserver.h"
#include "jscturret.h"
#include "stepper.h"
#include "logger.h"

void messageCallback(char *message)
{
  if (!strcmp(message, "center"))
  {
    centerTurret(SLOW);
  }
}

void init_stepper()
{
  setupStepper();
}

void init_bluetooth(GLogLevelFlags flag)
{
  serverConnectionData *data = malloc(sizeof(serverConnectionData));
  data->service_channel = BLUETOOTH_SERVICE_CHANNEL;
  data->service_name = BLUETOOTH_SERVICE_NAME;
  data->service_path = BLUETOOTH_SERVICE_PATH;
  data->service_uuid = BLUETOOTH_SERVICE_UUID;
  data->messageCallback = messageCallback;

  g_log(JSCBT, G_LOG_LEVEL_MESSAGE, "Starting service \"%s\".", BLUETOOTH_SERVICE_NAME);
  int result = register_service(data, flag);
  if (result != RESULT_OK)
  {
    g_log(JSCBT, G_LOG_LEVEL_ERROR, "Error %d occured while registering.", result);
  }
}

int main(int argc, char const *argv[])
{
  GLogLevelFlags flag = G_LOG_LEVEL_ERROR | G_LOG_LEVEL_WARNING | G_LOG_LEVEL_MESSAGE | G_LOG_LEVEL_CRITICAL;

  init_bluetooth(flag);
}