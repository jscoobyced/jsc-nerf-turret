#include <stdio.h>
#include <glib.h>
#include "logger.h"

void info(char *message)
{
  _info(message, 1);
}

void ninfo(char *message)
{
  _info(message, 0);
}

void _info(char *message, int crlf)
{
  g_print("%s%s", message, (crlf ? "\n" : ""));
}