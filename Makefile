all:
	make -C CppUnitLite
	make -C src

clean:
	make -C CppUnitLite clean
	make -C src clean
	make -C test clean

longtest: all
	make -C test test

quicktest:
	make -C test quicktest

