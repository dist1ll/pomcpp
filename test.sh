#! /bin/sh
make -s main
make -s test
(cd bin/ && ./test -r console)
