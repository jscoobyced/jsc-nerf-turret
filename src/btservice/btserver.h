#include <glib.h>
int register_service(char *service_path,
                     char *service_name,
                     int service_channel,
                     char *service_uuid,
                     GLogLevelFlags flag);

typedef struct ServerUserData
{
  GMainLoop *loop;
} serverUserData;