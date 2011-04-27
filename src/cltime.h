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

#endif /* CLTIME_H */
