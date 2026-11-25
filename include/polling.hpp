#pragma once

#ifndef NETWORK_POLLING_H
#define NETWORK_POLLING_H

#if defined(_WIN32) || defined(WIN32)

#include <windows.h>

using pollfd_t = WSAPOLLFD;

int poll(pollfd_t* pfd, int n, int timeout)
{
    return WSAPoll(pfd, n, timeout);
}

#else

#include <sys/poll.h>

using pollfd_t = struct pollfd;

#endif // _WIN32

#endif // NETWORK_POLLING_H