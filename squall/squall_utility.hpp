#ifndef UTILITY_HPP_
#define UTILITY_HPP_

#include <squirrel.h>

namespace squall {

////////////////////////////////////////////////////////////////
// stack keeper
struct keeper {
    keeper(HSQUIRRELVM v) { vm = v; top = sq_gettop(vm); }
    ~keeper() { sq_settop(vm, top); }
    HSQUIRRELVM vm;
    int top;
};

}

#endif // UTILITY_HPP_

