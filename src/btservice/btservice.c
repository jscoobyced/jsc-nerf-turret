#include <glib.h>
#include <gio/gio.h>
#include "btservice.h"

GDBusConnection *con;

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
		g_print("Address: %s.\n", address);
		return;
	}
	if (g_strcmp0(key, "UUIDs") == 0)
	{
		const gchar *uuid;
		GVariantIter i;
		g_variant_iter_init(&i, value);
		while (g_variant_iter_next(&i, "s", &uuid))
		{
			if (g_strcmp0(uuidToFind, uuid) == 0)
			{
				g_strlcpy(device->uuid, uuid, sizeof(device->uuid));
			}
		}
		return;
	}
}

static void device_appeared(GDBusConnection *sig,
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
				get_device_info(property_name, prop_val, device, ((userData *)user_data)->uuid);
			}

			if (strlen(device->uuid) > 0)
			{
				g_strlcpy(((userData *)user_data)->device->name, device->name, sizeof(device->name));
				g_strlcpy(((userData *)user_data)->device->address, device->address, sizeof(device->address));
				g_strlcpy(((userData *)user_data)->device->uuid, device->uuid, sizeof(device->uuid));
				g_main_loop_quit(((userData *)user_data)->loop);
			}
			free(device);
			g_variant_unref(prop_val);
		}
		g_variant_unref(properties);
	}
}

static int adapter_call_method(const char *method)
{
	GVariant *result;
	GError *error = NULL;

	result = g_dbus_connection_call_sync(con,
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
		return 1;

	g_variant_unref(result);
	return 0;
}

static int adapter_set_property(const char *prop, GVariant *value)
{
	GVariant *result;
	GError *error = NULL;

	result = g_dbus_connection_call_sync(con,
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
	int counter = ((userData *)user_data)->counter;
	counter++;
	((userData *)user_data)->counter = counter;
	if (counter < BLUETOOTH_DISCOVERY_MAX_WAIT_SECONDS)
	{
		if (strlen(((userData *)user_data)->device->uuid) > 0)
		{
			g_main_loop_quit(((userData *)user_data)->loop);
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		g_main_loop_quit(((userData *)user_data)->loop);
		return FALSE;
	}
	return TRUE;
}

int discoverService(BluetoothDeviceCallback callback)
{
	userData *userData = malloc(sizeof(userData));
	int resultCode, ret = RESULT_OK;
	guint iface_added;

	con = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
	if (con == NULL)
	{
		return ERR_CANNOT_GET_DBUS_CONNECTION;
	}

	userData->loop = g_main_loop_new(NULL, FALSE);

	userData->device = malloc(sizeof(btDevice));
	userData->counter = 0;
	g_strlcpy(userData->uuid, SERIAL_PORT_PROFILE_UUID, sizeof(SERIAL_PORT_PROFILE_UUID));

	iface_added = g_dbus_connection_signal_subscribe(con,
																									 "org.bluez",
																									 "org.freedesktop.DBus.ObjectManager",
																									 "InterfacesAdded",
																									 NULL,
																									 NULL,
																									 G_DBUS_SIGNAL_FLAGS_NONE,
																									 device_appeared,
																									 userData,
																									 NULL);

	resultCode = adapter_set_property("Powered", g_variant_new("b", TRUE));
	if (resultCode)
	{
		ret = ERR_CANNOT_ENABLE_ADAPTER;
		goto out;
	}

	resultCode = adapter_call_method("StartDiscovery");
	if (resultCode)
	{
		ret = ERR_CANNOT_START_SCAN;
		goto out;
	}

	g_timeout_add(BLUETOOTH_DISCOVERY_TIMEOUT_SECONDS * 1000, timeout_triggered, userData);
	g_main_loop_run(userData->loop);

	resultCode = adapter_call_method("StopDiscovery");
	if (resultCode)
	{
		ret = ERR_CANNOT_STOP_SCAN;
		goto out;
	}

	g_usleep(100);

	callback(userData->device);

	resultCode = adapter_set_property("Powered", g_variant_new("b", FALSE));
	if (resultCode)
	{
		ret = ERR_CANNOT_DISABLE_ADAPTER;
		goto out;
	}

out:
	g_dbus_connection_signal_unsubscribe(con, iface_added);
	g_object_unref(con);
	return ret;
}
