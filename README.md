# eViewItf Library

eSoftThings is manufacturing eCUBE board populated with V3HSK and CAM main board.

This repository contains eViewItf source code that allows to build the user space library running on Cortex A53 and used to configure the video pipeline running on Cortex R7 (eView).

## Build from a linux host

### Prerequisites

Setup Yocto SDK to set target toolchain.

## Generate program

Use make to build the eViewItf library.
```
$ make clean && make
```

Or in debug mode

```
$ make clean && make DEBUG=1
```

## Deploy on eCube board

### Release mode

### Debug mode