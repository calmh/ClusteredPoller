/*
 *  ClusteredPoller
 *
 *  Created by Jakob Borg.
 *  Copyright 2011 Nym Networks. See LICENSE for terms.
 */

#ifndef CLTIME_H
#define CLTIME_H

/** @file cltime.h Basic timekeeping. */

/** A type for milliseconds since the epoch. */
typedef long long curms_t;

/**
 * Current time in milliseconds since the epoch.
 * @return Milliseconds since the epoch.
 */
curms_t curms(void);

/**
 * Calculate the start of the next polling time, give now and the interval length.
 * @param now The current time, in milliseconds since the epoch.
 * @param poll_interval The polling interval, in seconds.
 * @return The start of the next polling interval, in milliseconds.
 */
curms_t next_interval(curms_t now, unsigned poll_interval);

#endif /* CLTIME_H */
