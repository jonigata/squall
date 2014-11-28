#include "../squall/squall_vmstd.hpp"
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


