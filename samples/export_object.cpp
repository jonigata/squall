#include "../squall/squall_vmstd.hpp"
#include <iostream>

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

        vm.defun("baz_back", [=](Foo* x)->void {
                std::cout << "**** lambda: " << x->foo() << std::endl;
            });

        Foo foo;
        vm.call<void>("baz", &foo);

        vm.defun("zot_back", [=](std::shared_ptr<Foo> x)->void {
                std::cout << "**** lambda: " << x->foo() << std::endl;
            });

        auto foo2 = std::make_shared<Foo>();
        vm.call<void>("zot", foo2);
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}


