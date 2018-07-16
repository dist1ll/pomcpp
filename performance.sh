#! /bin/sh
make -s main
make -s test
if [ "$#" -eq 0 ]; then
	(cd bin/ && ./test "[performance]")
else
	if [ "$1" = "-t" ]; then
		(cd bin/ && ./test "[performance]" --threads $2)
	else
		echo "Didn't recognize argument. Use -t x for concurrent testing."
	fi
fi
