#include <glib.h>

#define LOG_SERVER "BTSERVER"
#define LOG_CLIENT "BTCLIENT"

#define TIME_BUF_LEN 256

void set_logger(const gchar *domain);
void display_value(const gchar *key, GVariant *value);
void display_properties(GVariantIter *properties);
void display_parameters_signature(GVariant *parameters);