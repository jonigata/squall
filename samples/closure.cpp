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
