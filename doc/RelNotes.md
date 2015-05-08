ZeroEQ API Documentation {#mainpage}
============

[TOC]

# Introduction {#Introduction}

Welcome to ZeroEQ, a cross-platform C++ library to publish and subscribe for
events.

ZeroEQ 0.1 is a prototype version. It can be retrieved by cloning the
[source code](https://github.com/HBPVIS/zeq). Please file a
[Bug Report](https://github.com/HBPVis/zeq/issues) if you find any
issues with this release.

## Features {#Features}

ZeroEQ provides the following major features:

* Publish events using zeq::Publisher
* Subscribe to events using zeq::Subscriber
* Asynchronous, reliable transport using ZMQ
* Automatic publisher discovery using Zeroconf
* Serialization of events using flatbuffers

- - -

## Changes {#Changes}

### git master

* Filter messages basec on registered handlers (Optimization)
* Allow specification of network announce protocols in zeq::Publisher

### Version 0.2

* zeq::connection::Broker and zeq::connection::Service for subscription
  from a remote publisher.
* Concurrently dispatch events for multiple zeq::Subscriber and
  zeq::connection::Broker using shared zeq::Receiver groups.

### Version 0.1

* zeq::Publisher, zeq::Subscriber and zeq::Event for publish-subcribe
  event driven architectures.

- - -

# About {#About}

ZeroEQ is a cross-platform toolkit, designed to run on any modern operating
system, including all Unix variants. ZeroEQ uses CMake to create a
platform-specific build environment. The following platforms and build
environments are tested:

* Linux: Ubuntu 14.04, RHEL 6 using gcc 4.8
* Windows: 8 using Visual Studio 12
* Mac OS X: 10.9 and 10.10 using clang 6

ZeroEQ requires the following external, pre-installed dependencies:

* ZeroMQ 4.0 or later
* Boost (tested with 1.54)

- - -

# Errata {#Errata}
