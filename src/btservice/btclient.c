#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include "btconstant.h"
#include "btclient.h"
#include "logger.h"

static void get_device_info(const gchar *key, GVariant *value, btDevice *device, gchar *uuidToFind)
{
	if (g_strcmp0(key, "Name") == 0)
	{
		const gchar *name = g_variant_get_string(value, NULL);
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

static int adapter_call_method(GDBusConnection *connection, const char *method)
{
	GVariant *result;
	GError *error = NULL;

	result = g_dbus_connection_call_sync(connection,
																			 "org.bluez",
																			 "/org/bluez/hci0",
																			 "org.bluez.Adapter1",
																			 method,
																			 NULL,
																			 NULL,
																			 G_DBUS_CALL_FLAGS_NONE,
																			 -1,
																			 NULL,
																			 &error);
	if (error != NULL)
		return ERR_CANNOT_CALL_METHOD;

	g_variant_unref(result);
	return RESULT_OK;
}

static int adapter_set_property(GDBusConnection *connection, const char *prop, GVariant *value)
{
	GVariant *result;
	GError *error = NULL;

	result = g_dbus_connection_call_sync(connection,
																			 "org.bluez",
																			 "/org/bluez/hci0",
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
		else
		{
			g_print(".");
		}
	}
	else
	{
		g_print("\n");
		g_log(LOG_CLIENT, G_LOG_LEVEL_INFO, "Device not found. Cleaning up and stopping program.");
		g_main_loop_quit(data->loop);
		return FALSE;
	}
	return TRUE;
}

void device_found(GDBusConnection *connection, userData *data)
{
	data->callback(data->device);
	int resultCode = adapter_call_method(connection, "StopDiscovery");
	g_usleep(100);
	if (resultCode == RESULT_OK)
	{
		g_main_loop_quit(data->loop);
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
	userData->counter = 0;
	g_strlcpy(userData->uuid, service_uuid, sizeof(userData->uuid));

	iface_added = g_dbus_connection_signal_subscribe(connection,
																									 "org.bluez",
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

	adapter_call_method(connection, "StopDiscovery");
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
