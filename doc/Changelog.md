# Changelog

# Release 0.3 (07-07-2015) {#Release03}

* [63](https://github.com/HBPVIS/zeq/pull/63):
  Replace boost by C++11 std equivalents
* [64](https://github.com/HBPVIS/zeq/pull/64):
  Allow specification of network announce protocols in Publisher
* [68](https://github.com/HBPVIS/zeq/pull/68):
  OPT: Filter messages for registered handlers
* [69](https://github.com/HBPVIS/zeq/pull/69):
  Replaced Lunchbox by Servus
* [74](https://github.com/HBPVIS/zeq/pull/74):
  Fix exception with broken DNS and zeroconf
* [75](https://github.com/HBPVIS/zeq/pull/75):
  Event for frame setting and animation

# Release 0.2 (01-05-2015) {#Release02}

* zeq::connection::Broker and zeq::connection::Service for subscription from a
  remote publisher.
* Concurrently dispatch events for multiple zeq::Subscriber and
  zeq::connection::Broker using shared zeq::Receiver groups.

# Release 0.1 (07-01-2015){#Release01}

* zeq::Publisher, zeq::Subscriber and zeq::Event for publish-subcribe event
  driven architectures.
