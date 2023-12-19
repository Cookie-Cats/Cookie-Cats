#!/bin/bash

# 用于自动签名
# Example:
# ./compress_and_sign.sh PIONEER_alpha_prerelease_002 esp8266.esp8266.d1_mini

VERSION=$1
FQBN=$2

PATH=$PWD:$PATH
WORK_PATH=../build/$FQBN

cd "$WORK_PATH"
mv "Cookie-Cats.ino.bin" "CookieCats-$VERSION.bin"
# gzip -9 -c CookieCats-$VERSION.bin > CookieCats-$VERSION.bin.gz
signing.py --mode sign --privatekey "../../keys/private.pem" --bin "CookieCats-$VERSION.bin" --out "CookieCats-$VERSION.bin.signed"

