.SUFFIXES: .cpp .o

LINKOPTS = -L../SQUIRREL3/lib -lsquirrel -lsqstdlib

all: simple export_object klass rawfunc table coroutine closure var prop

simple: simple.o
	clang++ -o $@ $^ $(LINKOPTS)

export_object: export_object.o
	clang++ -o $@ $^ $(LINKOPTS)

klass: klass.o
	clang++ -o $@ $^ $(LINKOPTS)

rawfunc: rawfunc.o
	clang++ -o $@ $^ $(LINKOPTS)

table: table.o
	clang++ -o $@ $^ $(LINKOPTS)

coroutine: coroutine.o
	clang++ -o $@ $^ $(LINKOPTS)

closure: closure.o
	clang++ -o $@ $^ $(LINKOPTS)

var: var.o
	clang++ -o $@ $^ $(LINKOPTS)

prop: prop.o
	clang++ -o $@ $^ $(LINKOPTS)

.cpp.o:
	clang++ -std=c++11 -I../SQUIRREL3/include -c -o $@ $^ 

clean:
	rm -f simple.o export_object.o klass.o rawfunc.o table.o coroutine.o closure.o var.o prop.o
	rm -f simple export_object klass rawfunc table coroutine closure var prop

rebuild:
	make clean && make
