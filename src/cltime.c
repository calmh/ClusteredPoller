/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#include "cltime.h"

#include <sys/time.h>

curms_t curms(void)
{
        struct timeval now;
        gettimeofday(&now, NULL);
        return now.tv_sec * 1000 + now.tv_usec / 1000;
}
