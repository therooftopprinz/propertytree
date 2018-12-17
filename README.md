# Property Tree

| <img src="http://i.imgur.com/m1A8VQU.jpg" width="12"> Normal | <img src="http://i.imgur.com/VRQhgoh.png" width="14"> Memcheck | <img src="http://i.imgur.com/kysuYC1.png" width="12"> Coverage |
|---|:-:|---|
| [![Build Status](https://travis-ci.org/therooftopprinz/propertytree.svg?branch=master)](https://travis-ci.org/therooftopprinz/propertytree) | [![CircleCI](https://circleci.com/gh/therooftopprinz/propertytree.svg?style=svg)](https://circleci.com/gh/therooftopprinz/propertytree/tree/master) | [![Coverage Status](https://coveralls.io/repos/github/therooftopprinz/propertytree/badge.svg?branch=master)](https://coveralls.io/github/therooftopprinz/propertytree?branch=master) |

Trello https://trello.com/b/tuitIIq6/property-tree

PropertyTree is a library for sharing objects data across a network. Property Tree is composed of PropertyTree Server Application and PropertyTree Client Library.

The PropertyTree Server Application handles the communication and storage of all the client connected to the PropertyTree.

The PropertyTree Client Library provides the interface in communicating the PropertyTree.

Please see the [wiki](https://github.com/rooftopprinz/propertytree/wiki).

**New PropertyTree Specification!**

https://docs.google.com/document/d/1xvhpABWhiU1VUIQkq5ljw2uhR1o_IgIN0w-iVMXWba4/edit?usp=sharing

**IMPLEMENTATION ALIGNMENT IS GOING ON!!!**

## Building
```
Old way:

make server server_ut
make server_ut_gcov
make server_ut_run
make server_ut_valgrind_run

make client_ut
make client_ut_run
make client_ut_gcov
make client_ut_valgrind_run

New way:
mkdir buildNewWay
cd buildNewWay
../configure
make server_test
make client
```
