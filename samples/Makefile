.SUFFIXES: .cpp .o

LINKOPTS = -L../SQUIRREL3/lib -lsquirrel -lsqstdlib
CC = /usr/bin/clang++

all: simple export_object klass rawfunc table coroutine closure var prop

simple: simple.o
	$(CC) -o $@ $^ $(LINKOPTS)

export_object: export_object.o
	$(CC) -o $@ $^ $(LINKOPTS)

klass: klass.o
	$(CC) -o $@ $^ $(LINKOPTS)

rawfunc: rawfunc.o
	$(CC) -o $@ $^ $(LINKOPTS)

table: table.o
	$(CC) -o $@ $^ $(LINKOPTS)

coroutine: coroutine.o
	$(CC) -o $@ $^ $(LINKOPTS)

closure: closure.o
	$(CC) -o $@ $^ $(LINKOPTS)

var: var.o
	$(CC) -o $@ $^ $(LINKOPTS)

prop: prop.o
	$(CC) -o $@ $^ $(LINKOPTS)

.cpp.o:
	$(CC) -std=c++11 -I../SQUIRREL3/include -c -o $@ $^ 

clean:
	rm -f simple.o export_object.o klass.o rawfunc.o table.o coroutine.o closure.o var.o prop.o
	rm -f simple export_object klass rawfunc table coroutine closure var prop

rebuild:
	make clean && make
