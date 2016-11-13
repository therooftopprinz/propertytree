# Property Tree

PropertyTree is a library for sharing objects data across a network. Property Tree is composed of PropertyTree Server Application and PropertyTree Client Library.

The PropertyTree Server Application handles the communication and storage of all the client connected to the PropertyTree.

The PropertyTree Client Library provides the interface in communicating the PropertyTree.

Please see the [wiki](https://github.com/rooftopprinz/propertytree/wiki).

Building
```
#!/bin/sh

mkdir gtest
git clone https://github.com/google/googletest.git gtest/git
python gtest/git/googlemock/scripts/fuse_gmock_files.py gtest/git/googlemock/ gtest/

echo "VExEID0gLi4KTElCT1VUID0gJChUTEQpL2xpYnMKQ\
0MgPSBnKysKSU5DTFVERSA9IC1JLgpDRkxBR1MgPSAtc3Rk\
PWMrKzExIC1XYWxsIC1XZXJyb3IgJChJTkNMVURFKQoKYWx\
sOgoJbWtkaXIgLXAgJChMSUJPVVQpCgkkKENDKSAkKENGTE\
FHUykgLWMgZ21vY2stZ3Rlc3QtYWxsLmNjIC1vICQoTElCT\
1VUKS9ndGVzdC5v"|base64 -d > gtest/Makefile

git clone https://github.com/rooftopprinz/propertytree.git PropertyTree

cd PropertyTree
make server_ut_run -j4

```
