#!/bin/bash

$XGETTEXT `find . -name '*.cpp' | grep -v '/tests/'` -o $podir/ffmpegthumbs.pot
