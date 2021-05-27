#include <stdio.h>
#include <glib.h>
#include <gio/gio.h>
#include "btconstant.h"
#include "btserver.h"
#include "logger.h"

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
	info("Registration successful. Press [ENTER] to exit.");
	char c;
	result = scanf("%c", &c);

	g_object_unref(proxy);
	g_object_unref(conn);

	return RESULT_OK;
}
