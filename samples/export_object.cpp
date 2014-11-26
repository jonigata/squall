#include "../squall/vmstd.hpp"
#include <cstdio>

struct Foo {
    int foo() {
        std::cout << "**** foo() called" << std:: endl;
        return 4649;
    }
};


int main() {
    try {
        squall::VMStd vm;
        vm.dofile("export_object.nut");

        vm.defun("bar", [=](Foo* x)->void {
                std::cout << "**** lambda: " << x->foo() << std::endl;
            });

        Foo foo;
        vm.call<void>("baz", &foo);
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}


