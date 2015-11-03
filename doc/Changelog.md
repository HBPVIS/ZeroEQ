# Changelog {#Changelog}

# git master

# Release 0.4 (02-11-2015)

* [98](https://github.com/HBPVIS/zeq/pull/98):
  Disable zeroconf subscriptions to publishers in the same process
* [97](https://github.com/HBPVIS/zeq/pull/97):
  Added a small command line tool to send events parsed from a script file.
* [94](https://github.com/HBPVIS/zeq/pull/94):
  Added CellSetBinaryOp to HBP vocabulary
* [81](https://github.com/HBPVIS/zeq/pull/81):
  Implement (optional) ZeroBuf support

# Release 0.3 (07-07-2015)

* [75](https://github.com/HBPVIS/zeq/pull/75):
  Event for frame setting and animation
* [74](https://github.com/HBPVIS/zeq/pull/74):
  Fix exception with broken DNS and zeroconf
* [69](https://github.com/HBPVIS/zeq/pull/69):
  Replaced Lunchbox by Servus
* [68](https://github.com/HBPVIS/zeq/pull/68):
  OPT: Filter messages for registered handlers
* [64](https://github.com/HBPVIS/zeq/pull/64):
  Allow specification of network announce protocols in Publisher
* [63](https://github.com/HBPVIS/zeq/pull/63):
  Replace boost by C++11 std equivalents

# Release 0.2 (01-05-2015)

* zeq::connection::Broker and zeq::connection::Service for subscription from a
  remote publisher.
* Concurrently dispatch events for multiple zeq::Subscriber and
  zeq::connection::Broker using shared zeq::Receiver groups.

# Release 0.1 (07-01-2015)

* zeq::Publisher, zeq::Subscriber and zeq::Event for publish-subcribe event
  driven architectures.
