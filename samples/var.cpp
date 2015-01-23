#include "../squall/squall_vmstd.hpp"
#include "../squall/squall_klass.hpp"
#include <iostream>

class Foo {
public:
    Foo() : zot(32) {}

    void bar() {
    }

    std::int32_t baz;
    const std::int32_t zot;
};


int main() {
    try {
        squall::VMStd vm;
        vm.dofile("var.nut");

        squall::Klass<Foo> k(vm, "Foo");
        k.func("bar", &Foo::bar);
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



