#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include "btconstant.h"
#include "btserver.h"
#include "logger.h"

static const gchar introspection_xml[] =
		"<node>"
		"  <interface name='org.bluez.Profile1'>"
		"    <method name='Release' />"
		"    <method name='NewConnection'>"
		"      <arg type='o' name='device' direction='in' />"
		"      <arg type='h' name='fd' direction='in' />"
		"      <arg type='a{sv}' name='fd_properties' direction='in' />"
		"    </method>"
		"    <method name='RequestDisconnection'>"
		"      <arg type='o' name='device' direction='in' />"
		"    </method>"
		"  </interface>"
		"</node>";

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

static void signal_device_changed(GDBusConnection *conn,
																	const gchar *sender,
																	const gchar *path,
																	const gchar *interface,
																	const gchar *signal,
																	GVariant *params,
																	void *userdata)
{

	GVariantIter *properties = NULL;
	GVariantIter *unknown = NULL;
	const char *iface;
	const char *key;
	GVariant *value = NULL;
	const gchar *signature = g_variant_get_type_string(params);

	if (strcmp(signature, "(sa{sv}as)") != 0)
	{
		g_log(LOG_SERVER, G_LOG_LEVEL_INFO, "Invalid signature for %s: %s != %s", signal, signature, "(sa{sv}as)");
		goto done;
	}

	g_variant_get(params, "(&sa{sv}as)", &iface, &properties, &unknown);
	while (g_variant_iter_next(properties, "{&sv}", &key, &value))
	{
		if (!g_strcmp0(key, "Connected"))
		{
			if (!g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN))
			{
				g_log(LOG_SERVER, G_LOG_LEVEL_INFO, "Invalid argument type for %s: %s != %s", key,
							g_variant_get_type_string(value), "b");
				goto done;
			}
			g_log(LOG_SERVER, G_LOG_LEVEL_INFO, "Device is %s.", g_variant_get_boolean(value) ? "Connected" : "Disconnected");
			if (!g_variant_get_boolean(value))
			{
				g_main_loop_quit(((serverUserData *)userdata)->loop);
			}
		}
	}
done:
	if (properties != NULL)
		g_variant_iter_free(properties);
	if (value != NULL)
	{
		g_variant_unref(value);
	}
}

static void new_connection(GDBusMethodInvocation *inv)
{
	g_log(LOG_SERVER, G_LOG_LEVEL_INFO, "New connection.");

	GDBusMessage *msg = g_dbus_method_invocation_get_message(inv);
	gchar *content = g_dbus_message_print(msg, 2);
	g_log(LOG_SERVER, G_LOG_LEVEL_INFO, "Message is:\n%s", content);
	g_free(content);
	GVariant *params = g_dbus_method_invocation_get_parameters(inv);

	// DEBUG INFO
	const char *object;
	GVariant *properties;
	gint32 *handle;
	g_variant_get(params, "(oha{sv})", &object, &handle, &properties);
	g_log(LOG_SERVER, G_LOG_LEVEL_INFO, "Object is [%s]\nHandle is [%ls]", object, handle);
	GVariantIter iter;
	g_variant_iter_init(&iter, properties);
	display_properties(&iter);
	// DEBUG INFO
}

static void signal_method_call(GDBusConnection *conn, const char *sender,
															 const char *path, const char *interface, const char *method, GVariant *params,
															 GDBusMethodInvocation *invocation, void *userdata)
{

	g_log(LOG_SERVER, G_LOG_LEVEL_INFO, "Method invoked is [%s]\n\t on path [%s]\n\t with sender [%s]\n\t and interface [%s].", method, path, sender, interface);
	if (!g_strcmp0(method, "NewConnection"))
	{
		new_connection(invocation);
	}
}

int register_service(char *service_path,
										 char *service_name,
										 int service_channel,
										 char *service_uuid)
{
	GDBusProxy *proxy;
	GDBusConnection *conn, *pconn;
	GError *error = NULL;

	set_logger(LOG_SERVER);

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

	pconn = g_dbus_proxy_get_connection(proxy);

	serverUserData *userData = malloc(sizeof(serverUserData));
	userData->loop = g_main_loop_new(NULL, FALSE);

	guint sub_id = g_dbus_connection_signal_subscribe(pconn,
																										"org.bluez",
																										"org.freedesktop.DBus.Properties",
																										"PropertiesChanged",
																										NULL,
																										"org.bluez.Device1",
																										G_DBUS_SIGNAL_FLAGS_NONE,
																										signal_device_changed,
																										userData,
																										NULL);
	static const GDBusInterfaceVTable vtable = {
			.method_call = signal_method_call,
	};

	error = NULL;
	GDBusNodeInfo *info = g_dbus_node_info_new_for_xml(introspection_xml, &error);

	if (error != NULL)
	{
		g_error("Error obtaining interface due to [%s].", error->message);
	}
	else if (info == NULL)
	{
		g_log(LOG_SERVER, G_LOG_LEVEL_INFO, "Error obtaining interface (NULL interface).");
	}
	else
	{
		if (info->interfaces != NULL)
		{
			GDBusInterfaceInfo *interface = info->interfaces[0];
			if (interface != NULL)
			{
				gchar *interfaceName = interface->name;
				g_log(LOG_SERVER, G_LOG_LEVEL_INFO, "Interface name is [%s].", interfaceName);
			}

			g_dbus_connection_register_object(conn,
																				service_path, interface,
																				&vtable, NULL, NULL, &error);

			if (sub_id > 0)
			{
				g_log(LOG_SERVER, G_LOG_LEVEL_INFO, "Registration successful. Waiting for connection.");
			}

			g_main_loop_run(userData->loop);

			g_dbus_connection_signal_unsubscribe(pconn, sub_id);
		}
	}

	if (error != NULL)
	{
		g_error_free(error);
	}
	if (proxy != NULL)
	{
		g_object_unref(proxy);
	}
	if (conn != NULL)
	{
		g_object_unref(conn);
	}
	return RESULT_OK;
}
