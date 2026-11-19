#pragma once

#include <winsock2.h>

#ifndef POLLIN
#define POLLIN 0x0001
#define POLLOUT 0x0004
#define POLLERR 0x0008
#define POLLHUP 0x0010
#endif

#define poll WSAPoll
