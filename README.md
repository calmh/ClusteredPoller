RTG Clustered Poller
====================

**Database friendly poller for the [RTG](http://rtg.sourceforge.net/) traffic
statistics system.**

The clustered poller, `clpoll`, is a replacement poller for the RTG system. It
stems from a need for a faster, more intelligent poller that minimizes polling
time and database load.

Features
--------

  * Clustered inserts

    Clpoll takes care to only do one insert query per table and polling round.
    That query contains the data for all ports on the polled device. This
    reduces the amount of table opens and inserts by about 90-95 % and reduces
    load on the database server.

  * Database queueing

    The polling is decoupled from database writing, so polling can continue
    unhindered without data loss even in the case of a database restart or
    temporary overload. It also means we can poll faster than we can do
    database inserts, with the polling closely aligned to a five-minute
    boundary (or other configured interval).

  * Dead host detection

    Clpoll keeps track of SNMP errors per host and if the number of errors
    exceeds certain threshold that host is ignored for the rest of the polling
    interval. This lets `clpoll` focus on the hosts that actually respond and
    not spend unnecessary time waiting for hosts that are down. The check is
    done every polling interval, so as soon as a dead host is recovered it
    will be polled again.

  * `rtgpoll` compatibility

    Clpoll tries to be a drop-in replacement for `rtgpoll`, and thus uses

      *  the same database schema (with or without `rate` column),
      *  the same configuration file syntax (`rtg.conf`),
      *  the same target configuration (old table-like or new grouped style).

  * MySQL support

    Currently MySQL is the only supported database backend. Support for other
    database backends would probably amount to 10-20 lines of code. I would
    accept such patches if they were made, and might consider writing it
    myself if there is demand.

General Installation
--------------------

`clpoll` depends on `libmysqlclient` and `net-snmp`. Make sure these are
available.

Run `make`. It should produce a binary called `clpoll`. Use this in the same
way you would use `rtgpoll`.

If your database schema doesn't use the `rate` column from RTG 0.8 you need to
use the `-O` option ("old schema").

OS-specific Installation Instructions
-------------------------------------

These are installation instructions for tested environments. Other version of
the same distributions will probably work much the same. If you have it
working on another OS or distribution I'd be happy to know about it and add
the instructions here. Instructions assume a regular development environment,
i.e. a working GCC and make.

### Ubuntu 10.04 LTS or Debian 5

  1. Install the development tools, if you haven't already.

     `apt-get install build-essential`

  2. Install Net-SNMP and the MySQL libraries.

     `apt-get install libmysqlclient-dev libsnmp-dev`

  3. Run make.

  4. Use the created `clpoll` binary.

### Centos 5.5

  1. Install the development tools, if you haven't already.

     `yum groupinstall 'Development Tools'`

     You can probably trim this down to a slimmer requirement if you know what
     you're doing, but that will at least give you a sane development
     environment.

  2. Install Net-SNMP and the MySQL libraries.

     `yum install net-snmp-devel mysql-devel`

  3. Run make.

  4. Use the created `clpoll` binary.

### FreeBSD 8.1

  1. Install GNU make, Net-SNMP and the MySQL libraries.

     `pkg_add -r gmake net-snmp mysql51-client`

  2. Run gmake.

     `/usr/local/bin/gmake`

  3. Use the created `clpoll` binary.

### Mac OS X Snow Leopard (10.6.7)

  1. Install the development environment (Xcode).

  2. Install MySQL (from MySQLs website).

  3. Run make.

  4. Use the created `clpoll` binary.

### Solaris 11 Express (snv_151a)

  1. Install the development environment.

     `pkg install gcc-3 gnu-make`

  2. Install the match MySQL libraries (actually a lot more MySQL than that).
     The SNMP libraries seem present by default.

     `pkg install mysql-51 header-math`

  3. Run gmake.

     `gmake`

  4. Use the created `clpoll` binary.


Jakob Borg <jakob@nym.se>
