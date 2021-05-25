#include <glib.h>

#define BLUETOOTH_NAME_MAX_LENGTH_BYTES 248
#define BLUETOOTH_ADDRESS_STRING_SIZE 18
#define BLUETOOTH_UUID_STRING_SIZE 64
#define BLUETOOTH_DISCOVERY_TIMEOUT_SECONDS 1
#define BLUETOOTH_DISCOVERY_MAX_WAIT_SECONDS 60

#define SERIAL_PORT_PROFILE_UUID "0000110e-0000-1000-8000-00805f9b34fb"
// RFCOMM 00000003-0000-1000-8000-00805f9b34fb

#define RESULT_OK 200

#define ERR_CANNOT_GET_DBUS_CONNECTION 500
#define ERR_CANNOT_ENABLE_ADAPTER 501
#define ERR_CANNOT_DISABLE_ADAPTER 502
#define ERR_CANNOT_START_SCAN 503
#define ERR_CANNOT_STOP_SCAN 504

typedef struct BtDevice
{
  gchar address[4 * BLUETOOTH_ADDRESS_STRING_SIZE];
  gchar name[4 * BLUETOOTH_NAME_MAX_LENGTH_BYTES];
  gchar uuid[4 * BLUETOOTH_UUID_STRING_SIZE];
} btDevice;

typedef struct UserData
{
  GMainLoop *loop;
  gchar uuid[BLUETOOTH_UUID_STRING_SIZE];
  btDevice *device;
  int counter;
} userData;

typedef void (*BluetoothDeviceCallback)(btDevice *device);

int discoverService(BluetoothDeviceCallback callback);
