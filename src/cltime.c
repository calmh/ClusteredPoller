/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#include "cltime.h"
#include <stddef.h>

#include <sys/time.h>

curms_t curms(void)
{
        struct timeval now;
        gettimeofday(&now, NULL);
        return (curms_t) now.tv_sec * 1000 + (curms_t) now.tv_usec / 1000;
}

curms_t next_interval(curms_t now, unsigned poll_interval)
{
        return 1000 * ((now / 1000) / poll_interval + 1) * poll_interval;
}
