#include <glib.h>
#include <gio/gio.h>
#include "btservice.h"

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
		return 1;

	g_variant_unref(result);
	return 0;
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

int register_profile(GDBusProxy *proxy,
										 gchar *service_path,
										 gchar *service_name,
										 gint service_channel,
										 gchar *service_uuid)
{
	GVariant *profile;
	GVariantBuilder profile_builder;
	GError *error = NULL;

	g_variant_builder_init(&profile_builder, G_VARIANT_TYPE("(osa{sv})"));

	if (!g_variant_is_object_path(service_path))
	{
		return ERR_CUSTOM_PATH_INVALID;
	}

	g_variant_builder_add(&profile_builder, "o", service_path);

	g_variant_builder_add(&profile_builder, "s", SERIAL_PORT_PROFILE_UUID);

	g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("a{sv}"));

	g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
	g_variant_builder_add(&profile_builder, "s", "Channel");
	g_variant_builder_add(&profile_builder, "v", g_variant_new_uint16(service_channel));
	g_variant_builder_close(&profile_builder);

	g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
	g_variant_builder_add(&profile_builder, "s", "Service");
	g_variant_builder_add(&profile_builder, "v",
												g_variant_new_string(service_uuid));
	g_variant_builder_close(&profile_builder);

	g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
	g_variant_builder_add(&profile_builder, "s", "Name");
	g_variant_builder_add(&profile_builder, "v",
												g_variant_new_string(service_name));
	g_variant_builder_close(&profile_builder);

	g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
	g_variant_builder_add(&profile_builder, "s", "Role");
	g_variant_builder_add(&profile_builder, "v",
												g_variant_new_string("server"));
	g_variant_builder_close(&profile_builder);

	g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
	g_variant_builder_add(&profile_builder, "s", "RequireAuthentication");
	g_variant_builder_add(&profile_builder, "v",
												g_variant_new_boolean(FALSE));
	g_variant_builder_close(&profile_builder);

	g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
	g_variant_builder_add(&profile_builder, "s", "RequireAuthorization");
	g_variant_builder_add(&profile_builder, "v",
												g_variant_new_boolean(FALSE));
	g_variant_builder_close(&profile_builder);

	g_variant_builder_open(&profile_builder, G_VARIANT_TYPE("{sv}"));
	g_variant_builder_add(&profile_builder, "s", "AutoConnect");
	g_variant_builder_add(&profile_builder, "v",
												g_variant_new_boolean(TRUE));
	g_variant_builder_close(&profile_builder);

	g_variant_builder_close(&profile_builder);
	profile = g_variant_builder_end(&profile_builder);

	GVariant *ret = g_dbus_proxy_call_sync(proxy,
																				 "RegisterProfile",
																				 profile,
																				 G_DBUS_CALL_FLAGS_NONE,
																				 -1,
																				 NULL,
																				 &error);
	g_assert_no_error(error);

	if (ret != NULL && error == NULL)
	{
		return RESULT_OK;
	}
	return ERR_CANNOT_REGISTER_PROFILE;
}

int register_service(char *service_path,
										 char *service_name,
										 int service_channel,
										 char *service_uuid)
{
	GDBusProxy *proxy;
	GDBusConnection *conn;
	GError *error = NULL;

	conn = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, &error);
	g_assert_no_error(error);

	proxy = g_dbus_proxy_new_sync(conn,
																G_DBUS_PROXY_FLAGS_NONE,
																NULL,
																"org.bluez",
																"/org/bluez",
																"org.bluez.ProfileManager1",
																NULL,
																&error);
	g_assert_no_error(error);
	error = NULL;
	int result = register_profile(proxy, service_path, service_name, service_channel, service_uuid);
	if (result != RESULT_OK)
	{
		return result;
	}
	printf("Registration successful. Press [ENTER] to exit.\n");
	char c;
	scanf("%c", &c);

	g_object_unref(proxy);
	g_object_unref(conn);

	return RESULT_OK;
}

int discover_service(BluetoothDeviceCallback callback, char *service_uuid, int timeout)
{
	GDBusConnection *connection;
	userData *userData = malloc(sizeof(userData));
	int resultCode, ret = RESULT_OK;
	guint iface_added;

	connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
	if (connection == NULL)
	{
		return ERR_CANNOT_GET_DBUS_CONNECTION;
	}

	userData->loop = g_main_loop_new(NULL, FALSE);

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
	if (resultCode)
	{
		ret = ERR_CANNOT_START_SCAN;
		goto out;
	}

	g_timeout_add(timeout * 1000, timeout_triggered, userData);
	g_main_loop_run(userData->loop);

	resultCode = adapter_call_method(connection, "StopDiscovery");
	if (resultCode)
	{
		ret = ERR_CANNOT_STOP_SCAN;
		goto out;
	}

	g_usleep(100);

	callback(userData->device);

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
