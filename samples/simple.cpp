#include "../squall/vmstd.hpp"
#include <cstdio>

void foo(int) {
}

int main() {
    try {
        squall::VMStd vm;
        vm.dofile("simple.nut");

        int n0 = vm.call<int>("foo0");
        std::cout << "**** return value: " << n0 << std::endl;

        int n1 = vm.call<int>("foo1", 7);
        std::cout << "**** return value: " << n1 << std::endl;

        vm.defun("foo", &foo);

        vm.defun("bar", [=](int x)->int {
                std::cout << "**** lambda: " << x << std::endl;
                return 7777;
            });
        vm.call<void>("baz");
        
        int n2 = vm.call<int>("baz");
        std::cout << "**** return value: " << n2 << std::endl;
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}


