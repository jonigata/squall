# squall

A squirrel C++11 binding

License: Modified BSD

## To make VM but do nothing

C++
```
#include "squall/squall_vm.hpp"

int main() {
    squall::VM vm; // may throw squall::squirrel_error
    return 0;
}
```

## To make VM with standard library but do nothing

C++
```
#include "squall/squall_vmstd.hpp"

int main() {
    squall::VMStd vm; // may throw squall::squirrel_error
    return 0;
}
```

## To call squirrel function from C++

Squirrel
```test.nut
function foo() {
  print("==== foo0 called\n");
  return 0;
}
```

C++
```
#include "squall/squall_vmstd.hpp"
#include <iostream>

int main() {
    try {
        squall::VMStd vm;
        vm.dofile("test.nut");

        int n1 = vm.call<int>("foo", 7);
        std::cout << "**** return value: " << n1 << std::endl;
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## To call C++ function from squirrel

Squirrel
```test.nut
function baz() {
  print("==== baz called\n");
  return bar(4649);
}
```

C++
```
#include "squall/squall_vmstd.hpp"
#include <iostream>

int main() {
    try {
        squall::VMStd vm;
        vm.dofile("test.nut");

        vm.defun("bar", [=](int x)->int {
                std::cout << "**** lambda: " << x << std::endl;
                return 7777;
            });

        int n2 = vm.call<int>("baz");
        std::cout << "**** return value: " << n2 << std::endl;
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## To export C++ object and call member function

Squirrel
```
function zot(foo) {
  print("==== zot called\n");
  //print(foo);
  foo.bar();
  foo.bar();
  foo.bar();
  foo.bar();
  foo.bar();
  foo.bar();
  foo.baz("hello");
}
```

C++
```
#include "squall/squall_vmstd.hpp"
#include "squall/squall_klass.hpp"
#include <iostream>

class Foo {
public:
    Foo() : n_(0) {}
    int n_;

    void bar() {
        n_++;
        std::cerr << "**** bar called: " << n_ << std::endl;
    }
    int baz(const std::string& s) {
        std::cerr << "**** baz called: " << s << std::endl;
        return 4649;
    }
};

int main() {
    try {
        squall::VMStd vm;
        vm.dofile("klass.nut");

        squall::Klass<Foo> k(vm, "Foo");
        k.func("bar", &Foo::bar);
        k.func("baz", &Foo::baz);

        Foo foo;
        vm.call<void>("zot", &foo);
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## To register raw global function

Squirrel
```
function bar() {
  foo();
}
```

C++
```
#include "squall/squall_vmstd.hpp"
#include <iostream>

SQInteger foo(HSQUIRRELVM vm) {
    std::cout << "**** foo called" << std::endl;
    return 0;
}

int main() {
    try {
        squall::VMStd vm;
        vm.dofile("rawfunc.nut");

        vm.defraw("foo", foo);
        vm.call<void>("bar");
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
```

## To use coroutine

Squirrel
```
function foo() {
  print("==== foo called\n");
  local n = 4649;
  n = suspend(n);
  print("==== foo resumed\n");
  n = suspend(n);
  print("==== foo resumed\n");
  n = suspend(n);
  print("==== foo resumed\n");
  return n;
}
```

C++
```
#include "../squall/squall_vmstd.hpp"
#include <iostream>

void foo(int) {
}

int main() {
    try {
        squall::VMStd vm;
        vm.dofile("coroutine.nut");

        // You have better to make a scope because Coroutine dtor cleans up
        // Squirrel stack. co.result<T>() has the same effect, but if you
        // forget to call this, the stack might be completely broken.
        // The ownership of this clean-up behavior is transferred through
        // move semantics.
        {
            squall::Coroutine co = vm.co_call("foo");
            while (co.suspended()) {
                int n = co.yielded<int>();
                std::cerr << "**** foo yielded: " << n << std::endl;
                co.resume(n*2);
            }
            std::cerr << "**** foo result: " << co.result<int>() << std::endl;
        }
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
```

## To import/export closure

Squirrel
```
function bar() {
  foo(@(s)s.len(), "hello world");
  return 7;
}
```

C++
```
#include "../squall/squall_vmstd.hpp"
#include <iostream>

void foo(std::function<int (const std::string&)> f, const std::string& s) {
    std::cout << "**** foo called: " << s << std::endl;
    std::cout << "**** closure return value: " << f(s) << std::endl;
}

int main() {
    try {
        squall::VMStd vm;
        vm.dofile("closure.nut");

        vm.defun("foo", &foo);
        int n = vm.call<int>("bar");
        std::cout << "**** return value: " << n << std::endl;
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
```


## To derive

Squirrel
```
function zot(foo2) {
  foo2.bar2();
  foo2.bar();
}
```

C++
```
#include "../squall/squall_vmstd.hpp"
#include <iostream>

class Foo {
public:
    Foo() {}

    void bar() {
        std::cerr << "**** bar called: " << std::endl;
    }
};

class Foo2 : public Foo {
public:
    void bar2() {
        std::cerr << "**** bar2 called: " << std::endl;
    };
};

int main() {
    try {
        {
            squall::Klass<Foo> k(vm, "Foo");
            k.func("bar", &Foo::bar);
        }

        {
            squall::Klass<Foo2, Foo> k(vm, "Foo2");
            k.func("bar2", &Foo2::bar2);
        }

        Foo2 foo2;
        vm.call<void>("zot", &foo2);
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
```


## To export member variables

Squirrel
```
function zot(foo) {
  print(foo.baz); print("\n");
  print(foo.zot); print("\n");
  foo.baz = 49;
  print(foo.baz); print("\n");
  // foo.zot = 34; // error, because zot is const member variable
  // print(foo.zot); print("\n");
}
```

C++
```
#include "../squall/squall_vmstd.hpp"
#include "../squall/squall_klass.hpp"
#include <iostream>

class Foo {
public:
    Foo() : zot(32) {}

    std::int32_t baz;
    const std::int32_t zot;
};

int main() {
    try {
        squall::VMStd vm;
        vm.dofile("var.nut");

        squall::Klass<Foo> k(vm, "Foo");
        k.var("baz", &Foo::baz);
        k.var("zot", &Foo::zot);
        
        Foo foo;
        foo.baz = 47;
        vm.call<void>("zot", &foo);
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
```

## To define property

Squirrel
```
function baz(foo) {
  print(foo.bar1); print("\n");
  foo.bar2 = 42;
  print(foo.bar1); print("\n");
  print(foo.bar2); print("\n");
}
```

C++
```
#include "../squall/squall_vmstd.hpp"
#include "../squall/squall_klass.hpp"
#include <iostream>

class Foo {
public:
    Foo() { zot = 37; }

    int get_zot() const {
        return zot;
    }
    void set_zot(int n) {
        zot = n * 2;
    }
    
    int zot;
};


int main() {
    try {
        squall::VMStd vm;
        vm.dofile("prop.nut");

        squall::Klass<Foo> k(vm, "Foo");
        k.prop("bar1", &Foo::get_zot);
        k.prop("bar2", &Foo::get_zot, &Foo::set_zot);
        
        Foo foo;
        vm.call<void>("baz", &foo);
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
```
