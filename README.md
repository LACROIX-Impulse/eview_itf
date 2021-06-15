# eViewItf Library

LACROIX Impulse is manufacturing eCube board populated with V3HSK and CAM main board.

This repository contains eViewItf source code that allows to build the user space library running on Cortex A53 and used to configure the video pipeline running on Cortex R7 (eView).

## Build from a linux host

### Prerequisites

Setup Yocto SDK to set target toolchain.
```
$ LD_LIBRARY_PATH= source /path/to/poky/2.4.2/environment-setup-aarch64-poky-linux
```
Note: unset LD_LIBARY_PATH may be necessary while sourcing environment

## Generate library

Use make to build the eViewItf library.
```
$ make clean && make
```

## Deploy on eCube board

Both program and lib are deployed in the RootFS of eCube:
* /usr/bin/eviewitf
* /usr/lib/libeviewitf.a
* /usr/include/eviewitf.h

To deploy eViewItf in the eCube, the following commands can be used:
```
$ export TARGETIP=192.168.0.80
$ make deploy
```

"192.168.0.80" is to be replaced by the IP of the eCube connected through SSH.


To get the version of the lib deployed in your board:
```
$ eviewitf -V
```
