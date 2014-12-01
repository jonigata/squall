#include "../squall/squall_vmstd.hpp"
#include <iostream>

void foo(int) {
}

int main() {
    try {
        squall::VMStd vm;
        vm.dofile("coroutine.nut");

        vm.co_call("bar", 1, 2, 3, 4);

        {
            auto co = vm.co_call("zot");
            while (co.suspended()) {
                co.resume();
            }
            std::cerr << "**** zot result: " << co.result<int>() << std::endl;
        }

        {
            auto co = vm.co_call("baz");
            while (co.suspended()) {
                int n = co.yielded<int>();
                std::cerr << "**** baz yielded: " << n << std::endl;
                co.resume();
            }
            std::cerr << "**** baz result: " << co.result<int>() << std::endl;
        }

        {
            auto co = vm.co_call("foo");
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


