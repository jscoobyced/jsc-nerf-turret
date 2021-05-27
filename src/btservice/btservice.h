#include <glib.h>
#include <gio/gio.h>

#define BLUETOOTH_NAME_MAX_LENGTH_BYTES 248
#define BLUETOOTH_ADDRESS_STRING_SIZE 18
#define BLUETOOTH_UUID_STRING_SIZE 64
#define BLUETOOTH_DISCOVERY_MAX_WAIT_SECONDS 60

#define BLUETOOTH_BASE_128_UUID "00000000-0000-1000-8000-00805F9B34FB"
#define SERIAL_PORT_PROFILE_UUID "0000110e-0000-1000-8000-00805f9b34fb"

#define RESULT_OK 200

#define ERR_CANNOT_GET_DBUS_CONNECTION 500
#define ERR_CANNOT_ENABLE_ADAPTER 501
#define ERR_CANNOT_DISABLE_ADAPTER 502
#define ERR_CANNOT_START_SCAN 503
#define ERR_CANNOT_STOP_SCAN 504
#define ERR_CUSTOM_PATH_INVALID 505
#define ERR_CANNOT_REGISTER_PROFILE 506
#define ERR_CANNOT_CONNECT_DEVICE 507

typedef struct BtDevice
{
  gchar address[4 * BLUETOOTH_ADDRESS_STRING_SIZE];
  gchar name[4 * BLUETOOTH_NAME_MAX_LENGTH_BYTES];
  gchar uuid[4 * BLUETOOTH_UUID_STRING_SIZE];
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
int register_service(char *service_path, char *service_name, int service_channel, char *service_uuid);
void device_found(GDBusConnection *connection, userData *data);
int adapter_connect_device(GDBusConnection *connection, char *address);