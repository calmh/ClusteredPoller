RTG Clustered Poller
====================

**Database friendly poller for the [RTG](http://rtg.sourceforge.net/)
traffic statistics system.**

The clustered poller, `clpoll`, is a replacement poller for the RTG
system. It stems from a need for a faster, more intelligent poller
that minimizes polling time and database load.

The `clpoll` process treats each host as a unit, and takes care to
only ever do one insert per table per interval. This reduces the
amount of table opens and inserts by about 90% in our environment,
with a corresponding effect on server load.

Differences against standard `rtgpoll` are:

* Clustered inserts drastically reduces database load.

* Dead host detection avoids blocking the polling system when devices
  are unreachable.

* Database rows are dated based on when the result was read, not when
  it was inserted into the database.

* MySQL is the only supported database backend. (Support for other
  database backends would probably amount to 10-20 lines of code. I
  would accept such patches if they were made.)

In all other aspects, `clpoll` strives to be backwards compatible with
`rtgpoll`.  In particular, it should be able to read the same
configuration files and produce measurement data in an identical
format. It **should** be a simple drop-in replacement.

Installation
------------

`clpoll` depends on `libmysqlclient` and `net-snmp`. Make sure these are
available.

Run `make`. It should produce a binary called `clpoll`. Use this in
the same way you would use `rtgpoll`.

Make sure that your database is using a schema with a `rate` column or
no data will be inserted into the database. This is an `rtg` default since
2004 or thereabout, but there are old installations without this column.

Jakob Borg <jakob@nym.se>
