#include "../squall/vmstd.hpp"
#include "../squall/klass.hpp"
#include <cstdio>

class Foo {
public:
    Foo(){}

    void bar() {
        std::cerr << "**** bar called" << std::endl;
    }
    int baz(const std::string& s) {
        std::cerr << "**** baz called: s" << std::endl;
        return 4649;
    }
};

int main() {
    try {
        squall::VMStd vm;
        vm.dofile("klass.nut");

        squall::Klass<Foo> k("Foo", vm);
        k.func("bar", &Foo::bar);
        k.func("baz", &Foo::baz);

/*
        Foo foo;
        vm.call<void>("zot", &foo);
*/
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}



