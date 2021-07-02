#include <glib.h>
#include <time.h>
#include "logger.h"

void display_parameters_signature(GVariant *parameters)
{
  const gchar *signature = g_variant_get_type_string(parameters);
  g_log(JSCBT, G_LOG_LEVEL_INFO, "Signature is [%s].", signature);
}

void display_value(const gchar *key, GVariant *value)
{
  const gchar *signature = g_variant_get_type_string(value);
  if (g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN))
  {
    g_log(JSCBT, G_LOG_LEVEL_INFO, "Key: [%s]\tValue: [%s]", signature, key, g_variant_get_boolean(value) ? "true" : "false");
  }
  else if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING))
  {
    g_log(JSCBT, G_LOG_LEVEL_INFO, "Key: [%s]\tValue: [%s]", signature, key, g_variant_get_string(value, NULL));
  }
  else if (g_variant_is_of_type(value, G_VARIANT_TYPE_INT16))
  {
    g_log(JSCBT, G_LOG_LEVEL_INFO, "Key: [%s]\tValue: [%d]", signature, key, g_variant_get_int16(value));
  }
  else if (g_variant_is_of_type(value, G_VARIANT_TYPE_INT32))
  {
    g_log(JSCBT, G_LOG_LEVEL_INFO, "Key: [%s]\tValue: [%d]", signature, key, g_variant_get_int32(value));
  }
  else if (g_strcmp0(signature, "as") == 0)
  {
    GVariantIter *iterator;
    gchar *c;
    g_variant_get(value, "as", &iterator);
    while (g_variant_iter_loop(iterator, "s", &c))
    {
      g_log(JSCBT, G_LOG_LEVEL_INFO, "Key: [%s]\tValue: [%s]", signature, key, c);
    }
    g_variant_iter_free(iterator);
  }
  else if (g_strcmp0(signature, "ay") == 0)
  {
    GVariantIter *iterator;
    guchar *c;
    g_variant_get(value, "ay", &iterator);
    int b = 1;
    while (g_variant_iter_loop(iterator, "y", &c))
    {
      unsigned char *uc = (unsigned char *)c;
      if (b == 2)
      {
        g_log(JSCBT, G_LOG_LEVEL_DEBUG, ":");
      }
      b = 2;
      g_log(JSCBT, G_LOG_LEVEL_DEBUG, "%x", *uc);
    }
    g_variant_iter_free(iterator);
  }
  else if (g_strcmp0(signature, "a{qv}") == 0)
  {
    GVariantIter *iterator;
    GVariant *variant;
    guint16 *number;
    g_variant_get(value, "a{qv}", &iterator);

    while (g_variant_iter_loop(iterator, "{qv}", &number, &variant))
    {
      g_log(JSCBT, G_LOG_LEVEL_INFO, "Key: [%s]\tDetails: ", key);
      display_value(key, variant);
      g_log(JSCBT, G_LOG_LEVEL_DEBUG, "\n");
    }
    g_variant_iter_free(iterator);
  }
  else
  {
    g_log(JSCBT, G_LOG_LEVEL_INFO, "Signature: [%s]\tKey: [%s].", signature, key);
  }
}

void display_properties(GVariantIter *properties)
{
  const gchar *key;
  GVariant *value = NULL;
  if (properties == NULL)
  {
    g_log(JSCBT, G_LOG_LEVEL_INFO, "There are no property to display.\n");
    return;
  }

  while (g_variant_iter_next(properties, "{&sv}", &key, &value))
  {
    const gchar *signature = g_variant_get_type_string(value);
    g_log(JSCBT, G_LOG_LEVEL_INFO, "Value signature is [%s].\n", signature);

    display_value(key, value);

    if (value != NULL)
    {
      g_variant_unref(value);
    }
  }
}

static void _log_handler(const gchar *log_domain,
                         GLogLevelFlags log_level,
                         const gchar *message,
                         gpointer user_data)
{
  if (log_level == G_LOG_LEVEL_DEBUG)
  {
    g_print("%s", message);
    return;
  }
  time_t now = time(NULL);
  struct tm *ptm = localtime(&now);
  char time_display[TIME_BUF_LEN] = {0};
  strftime(time_display, TIME_BUF_LEN, "%Y-%m-%d %H:%M:%S", ptm);
  g_print("%s - %s - %s\n", time_display, log_domain, message);
}

void set_logger(const gchar *domain, GLogLevelFlags flag)
{
  g_log_set_handler(domain, flag, _log_handler, NULL);
}