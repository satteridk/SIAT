#ifndef OS_ICONS_H
#define OS_ICONS_H

#include "../os_globals.h"

void desenharIcone(int x, int y, int tipo);
int  iconePorNome(const String& nome, bool isDir);

#endif