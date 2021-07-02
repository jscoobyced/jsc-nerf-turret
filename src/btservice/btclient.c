#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include "btconstant.h"
#include "btclient.h"
#include "logger.h"

char *get_path(char *address)
{
	int i = 0;
	while (address[i] != '\0')
	{
		if (address[i] == ':')
		{
			address[i] = '_';
		}
		i++;
	}
	char base_path[] = "/org/bluez/hci0/dev_";
	char *path = malloc(sizeof(char) * 64);
	path[0] = '\0';
	strcat(path, base_path);
	strcat(path, address);
	g_log(JSCBT, G_LOG_LEVEL_INFO, "Path: %s", path);
	return path;
}

static void get_device_info(const gchar *key, GVariant *value, btDevice *device, gchar *uuidToFind)
{
	if (g_strcmp0(key, "Name") == 0)
	{
		const gchar *name = g_variant_get_string(value, NULL);
		g_print("Name: %s\n", name);

		g_strlcpy(device->name, name, sizeof(device->name));
		return;
	}

	if (g_strcmp0(key, "Address") == 0)
	{
		const gchar *address = g_variant_get_string(value, NULL);
		g_strlcpy(device->address, address, sizeof(device->address));
		return;
	}
	if (g_strcmp0(key, "Adapter") == 0)
	{
		const gchar *adapter = g_variant_get_string(value, NULL);
		g_strlcpy(device->adapter, adapter, sizeof(device->adapter));
		return;
	}
	if (g_strcmp0(key, "UUIDs") == 0)
	{
		const gchar *uuid;
		GVariantIter i;
		g_variant_iter_init(&i, value);
		while (g_variant_iter_next(&i, "s", &uuid))
		{
			g_print("UUID: %s\n", uuid);
			gchar *toFind = g_ascii_strdown(uuidToFind, -1);
			gchar *current = g_ascii_strdown(uuid, -1);

			if (g_strstr_len(toFind, -1, current))
			{
				g_strlcpy(device->uuid, uuid, sizeof(device->uuid));
			}
		}
		return;
	}
	display_value(key, value);
}

static void device_appeared(GDBusConnection *connection,
														const gchar *sender_name,
														const gchar *object_path,
														const gchar *interface,
														const gchar *signal_name,
														GVariant *parameters,
														gpointer user_data)
{
	GVariantIter *interfaces;
	const char *object;
	const gchar *interface_name;
	GVariant *properties;
	userData *data = ((userData *)user_data);

	g_variant_get(parameters, "(&oa{sa{sv}})", &object, &interfaces);
	while (g_variant_iter_next(interfaces, "{&s@a{sv}}", &interface_name, &properties))
	{
		btDevice *device = malloc(sizeof(btDevice));
		if (g_strstr_len(g_ascii_strdown(interface_name, -1), -1, "device"))
		{
			const gchar *property_name;
			GVariantIter i;
			GVariant *prop_val;
			g_variant_iter_init(&i, properties);
			while (g_variant_iter_next(&i, "{&sv}", &property_name, &prop_val))
			{
				get_device_info(property_name, prop_val, device, data->uuid);
			}

			if (strlen(device->name) > 0 && strlen(device->address) > 0 && strlen(device->uuid) > 0 && strlen(device->adapter) > 0)
			{
				g_strlcpy(data->device->name, device->name, sizeof(device->name));
				g_strlcpy(data->device->address, device->address, sizeof(device->address));
				g_strlcpy(data->device->uuid, device->uuid, sizeof(device->uuid));
				g_strlcpy(data->device->adapter, device->adapter, sizeof(device->adapter));
				device_found(connection, user_data);
			}
			free(device);
			g_variant_unref(prop_val);
		}
		g_variant_unref(properties);
	}
}

static int call_method(GDBusConnection *connection,
											 const gchar *path,
											 const gchar *interface,
											 const char *method,
											 GVariant *parameters,
											 const BluetoothConnectCallback callback,
											 gpointer data)
{
	if (callback == NULL)
	{
		GVariant *result;
		GError *error = NULL;

		result = g_dbus_connection_call_sync(connection,
																				 BLUEZ,
																				 path,
																				 interface,
																				 method,
																				 parameters,
																				 NULL,
																				 G_DBUS_CALL_FLAGS_NONE,
																				 -1,
																				 NULL,
																				 &error);
		if (error != NULL)
		{
			g_error("Error calling %s on %s due to [%s].", method, interface, error->message);
			return ERR_CANNOT_CALL_METHOD;
		}

		if (result != NULL)
		{
			g_variant_unref(result);
		}
	}
	else
	{
		g_dbus_connection_call(connection,
													 BLUEZ,
													 path,
													 interface,
													 method,
													 parameters,
													 NULL,
													 G_DBUS_CALL_FLAGS_NONE,
													 5000,
													 NULL,
													 callback,
													 data);
	}

	return RESULT_OK;
}

static int adapter_call_method(GDBusConnection *connection, const char *method)
{
	return call_method(connection, BLUETOOTH_PATH_DEFAULT, BLUETOOTH_ADAPTER_DEFAULT, method, NULL, NULL, NULL);
}

static int device_call_method(GDBusConnection *connection,
															const gchar *path,
															const char *method,
															BluetoothConnectCallback callback,
															gpointer data)
{
	return call_method(connection, path, BLUETOOTH_DEVICE_DEFAULT, method, NULL, callback, data);
}

void device_paired_callback(GObject *object, GAsyncResult *result, gpointer user_data)
{
	GDBusConnection *connection = (GDBusConnection *)object;
	userData *data = ((userData *)user_data);
	g_log(JSCBT, G_LOG_LEVEL_MESSAGE, "Paired successfully with %s.", data->device->name);
	gchar *path = get_path(data->device->address);
	device_call_method(connection, path, "Connect", NULL, NULL);
	g_main_loop_quit(data->loop);
}

static int adapter_set_property(GDBusConnection *connection, const char *prop, GVariant *value)
{
	GVariant *result;
	GError *error = NULL;

	result = g_dbus_connection_call_sync(connection,
																			 BLUEZ,
																			 BLUETOOTH_PATH_DEFAULT,
																			 "org.freedesktop.DBus.Properties",
																			 "Set",
																			 g_variant_new("(ssv)", "org.bluez.Adapter1", prop, value),
																			 NULL,
																			 G_DBUS_CALL_FLAGS_NONE,
																			 -1,
																			 NULL,
																			 &error);
	if (error != NULL)
		return 1;

	g_variant_unref(result);
	return 0;
}

static gboolean timeout_triggered(gpointer user_data)
{
	userData *data = ((userData *)user_data);
	int counter = data->counter;
	counter++;
	data->counter = counter;
	if (counter < BLUETOOTH_DISCOVERY_MAX_WAIT_SECONDS)
	{
		int found = (strlen(data->device->uuid) > 0);
		if (found)
		{
			return FALSE;
		}
	}
	else
	{
		g_log(JSCBT, G_LOG_LEVEL_MESSAGE, "Device not found. Cleaning up and stopping program.");
		g_main_loop_quit(data->loop);
		return FALSE;
	}
	return TRUE;
}

void device_found(GDBusConnection *connection, userData *data)
{
	data->device->complete = TRUE;
	data->callback(data->device);
	int resultCode = adapter_call_method(connection, "StopDiscovery");
	g_usleep(100);
	if (resultCode == RESULT_OK)
	{
		gchar *path = get_path(data->device->address);
		resultCode = device_call_method(connection, path, "Pair", device_paired_callback, data);
		if (resultCode == RESULT_OK)
		{
			g_log(JSCBT, G_LOG_LEVEL_MESSAGE, "Pairing requested.");
		}
		else
		{
			g_log(JSCBT, G_LOG_LEVEL_ERROR, "Cannot connect to device.");
			g_main_loop_quit(data->loop);
		}
	}
}

int discover_service(BluetoothDeviceCallback callback, char *service_uuid, int timeout)
{
	GDBusConnection *connection;
	int resultCode, ret = RESULT_OK;
	guint iface_added;
	userData *userData = malloc(sizeof(userData));

	connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
	if (connection == NULL)
	{
		return ERR_CANNOT_GET_DBUS_CONNECTION;
	}

	userData->loop = g_main_loop_new(NULL, FALSE);
	userData->callback = callback;

	userData->device = malloc(sizeof(btDevice));
	userData->device->complete = FALSE;
	userData->counter = 0;
	g_strlcpy(userData->uuid, service_uuid, sizeof(userData->uuid));

	iface_added = g_dbus_connection_signal_subscribe(connection,
																									 BLUEZ,
																									 "org.freedesktop.DBus.ObjectManager",
																									 "InterfacesAdded",
																									 NULL,
																									 NULL,
																									 G_DBUS_SIGNAL_FLAGS_NONE,
																									 device_appeared,
																									 userData,
																									 NULL);

	resultCode = adapter_set_property(connection, "Powered", g_variant_new("b", TRUE));
	if (resultCode)
	{
		ret = ERR_CANNOT_ENABLE_ADAPTER;
		goto out;
	}

	resultCode = adapter_call_method(connection, "StartDiscovery");
	if (resultCode != RESULT_OK)
	{
		ret = ERR_CANNOT_START_SCAN;
		goto out;
	}

	g_timeout_add(timeout * 1000, timeout_triggered, userData);
	g_main_loop_run(userData->loop);

	if (!userData->device->complete)
	{
		adapter_call_method(connection, "StopDiscovery");
	}
	g_usleep(100);

	resultCode = adapter_set_property(connection, "Powered", g_variant_new("b", FALSE));
	if (resultCode)
	{
		ret = ERR_CANNOT_DISABLE_ADAPTER;
		goto out;
	}

out:
	g_dbus_connection_signal_unsubscribe(connection, iface_added);
	g_object_unref(connection);
	return ret;
}
