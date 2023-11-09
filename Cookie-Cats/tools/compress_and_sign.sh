#!/bin/bash

# 用于自动签名
# Example:
# ./compress_and_sign.sh PIONEER_0.1_alpha_signed esp8266.esp8266.nodemcu

VERSION=$1
FQBN=$2

PATH=$PWD:$PATH
WORK_PATH=../build/$FQBN

cd $WORK_PATH
mv Cookie-Cats.ino.bin CookieCats-$VERSION.bin
gzip -9 CookieCats-$VERSION.bin
signing.py --mode sign --privatekey ../../keys/private.pem --bin CookieCats-$VERSION.bin.gz --out CookieCats-$VERSION.bin.gz.signed
