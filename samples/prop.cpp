#include "../squall/squall_vmstd.hpp"
#include "../squall/squall_klass.hpp"
#include <iostream>

class Foo {
public:
    Foo() { zot = 37; }

    int get_zot() const {
        std::cerr << "**************** get_zot\n";
        return zot;
    }
    void set_zot(int n) {
        std::cerr << "**************** set_zot\n";
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



