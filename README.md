RTG Clustered Poller
====================

**Database friendly poller for the [RTG](http://rtg.sourceforge.net/)
traffic statistics system.**

rtgpoll
-------

RTG:s standard poller, `rtgpoll`, is a fast and stable multithreaded
SNMP poller. Unfortunately it isn't completely optimized for large
deployments with hundreds or thousands of devices and many different
counters. `rtgpoll` uses a worker thread model where all targets
(Host+OID combination) are put in a large pool and then handled
separately. This doesn't take advantage of the fact that all results
of the same type (for example all `ifInOctets` values) from one device
are going to end up in the same database table. Since all queries are
made individually, `rtgpoll` will do open, insert, and close many
times for each table.

clpoll
------

The clustered poller, `clpoll`, treats each host as a unit, and takes
care to only ever do one insert per table per interval. This reduces
the amount of table opens and inserts by about 90% in our environment,
with a corresponding effect on server load.

Differences against standard `rtgpoll` are:

* Clustered inserts reduces database load.

* Database rows are dated based on when the result was read, not when
  it was inserted into the database.

* It only supports the MySQL backend. (Support for other database
  backends would probably amount to 10-20 lines of code. I would
  accept such patches if they were made.)

* Clustered polls potentially increases load on monitored devices.

In all other aspects, `clpoll` strives to be backwards compatible with
`rtgpoll`.  In particular, it should be able to read the same
configuration files and produce measurement data in an identical
format. It **should** be a simple drop-in replacement.

Installation
------------

`clpoll` depends on `libmysql++` and `net-snmp`. Make sure these are
available.

Run `make`. It should produce a binary called `clpoll` in the `src`
directory. Use this in the same way you would use `rtgpoll`.

Jakob Borg <jakob@nym.se>
2009-12-25
