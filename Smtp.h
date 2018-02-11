#ifndef FILEGUARD_SMTP_H
#define FILEGUARD_SMTP_H

#include "common.h"

void SmtpConfig(const char *Setting, const char *Value);
int SmtpServer(const char *URL);

#endif
