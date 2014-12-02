#include "../squall/squall_vmstd.hpp"
#include "../squall/squall_klass.hpp"
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

        k.func("qux", [](Foo* x, const std::string& y) {
                std::cerr << "**** qux called: " << y << std::endl;
                x->baz(y);
            });
        vm.call<void>("quux", &foo);
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}



