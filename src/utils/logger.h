#include <glib.h>

#define JSCBT "JSCBT"

#define TIME_BUF_LEN 256

void set_logger(const gchar *domain, GLogLevelFlags flag);
void display_value(const gchar *key, GVariant *value);
void display_properties(GVariantIter *properties);
void display_parameters_signature(GVariant *parameters);