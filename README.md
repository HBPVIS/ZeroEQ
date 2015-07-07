[TOC]

# Introduction {#Introduction}

A cross-platform C++ library to publish and subscribe for events. Applications
can discover each other automatically through the integrated ZeroConf protocol,
or through explicit addressing using hostname and port. A defined vocabulary
defines semantics for the published events.

## Features {#Features}

ZeroEQ provides the following major features:

* Publish events using zeq::Publisher
* Subscribe to events using zeq::Subscriber
* Asynchronous, reliable transport using ZMQ
* Automatic publisher discovery using Zeroconf
* Serialization of events using flatbuffers

# Building {#Building}

ZeroEQ is a cross-platform toolkit, designed to run on any modern operating
system, including all Unix variants. ZeroEQ uses CMake to create a
platform-specific build environment. The following platforms and build
environments are tested:

* Linux: Ubuntu 14.04, RHEL 6 using gcc 4.8
* Windows: 8 using Visual Studio 12
* Mac OS X: 10.9 and 10.10 using clang 6

ZeroEQ requires the following external, pre-installed dependencies:

* ZeroMQ 4.0 or later
* Boost (tested with 1.54) for unit tests

~~~
git clone https://github.com/HBPVIS/zeq.git
cd zeq
mkdir build
cd build
cmake ..
make
~~~
