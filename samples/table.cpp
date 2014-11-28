#include "../squall/squall_vmstd.hpp"
#include "../squall/squall_table.hpp"
#include <iostream>

int main() {
    try {
        squall::VMStd vm;

        squall::Table table(vm);
        table.set("hello", "world");

        std::cout << table.get<std::string>("hello") << std::endl;
    }
    catch(squall::squirrel_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}



