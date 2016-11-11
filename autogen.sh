#!/bin/bash
autoreconf -vi

while (( "$#" )); do
  if [ $1 == "-c" ]; then
    ./configure --with-jpeg-prefix=/usr/local \
                --with-curl-config=`which curl-config`
  fi
  shift
done
