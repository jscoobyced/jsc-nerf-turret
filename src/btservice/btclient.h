#include <glib.h>
#include <gio/gio.h>
#include "btconstant.h"


typedef struct BtDevice
{
  gchar address[4 * BLUETOOTH_ADDRESS_STRING_SIZE];
  gchar adapter[4 * BLUETOOTH_ADAPTER_STRING_SIZE];
  gchar name[4 * BLUETOOTH_NAME_MAX_LENGTH_BYTES];
  gchar uuid[4 * BLUETOOTH_UUID_STRING_SIZE];
  gboolean complete;
} btDevice;

typedef void (*BluetoothDeviceCallback)(btDevice *);

typedef void (*BluetoothConnectCallback)(GObject *, GAsyncResult *, gpointer);

typedef struct UserData
{
  GMainLoop *loop;
  gchar uuid[4 * BLUETOOTH_UUID_STRING_SIZE];
  btDevice *device;
  int counter;
  BluetoothDeviceCallback callback;
} userData;

int discover_service(BluetoothDeviceCallback callback, char *uuid, int timeout);
void device_found(GDBusConnection *connection, userData *data);
int adapter_connect_device(GDBusConnection *connection, char *address);