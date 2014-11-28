#ifndef SQUALL_VMSTD_HPP_
#define SQUALL_VMSTD_HPP_

#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdaux.h>

#include "vm.hpp"

#include <stdio.h>
#include <stdarg.h>
#include <iostream>

namespace squall {

class VMStd : public VM {
public:
    VMStd(int stack_size = 1024) : VM(stack_size) {
        sqstd_seterrorhandlers(handle());
        sq_setprintfunc(handle(), &VMStd::pf, &VMStd::pf);
    }

    void dofile(const char* filename) {
        keeper k(handle());
        sq_pushroottable(handle());
        if (!SQ_SUCCEEDED(sqstd_dofile(handle(), filename, 0, 1))) {
            throw squirrel_error("dofile failed");
        }
    }

private:
    static void pf(HSQUIRRELVM v, const char* s, ...)  {
        va_list arglist;
        va_start(arglist, s);
        vprintf(s, arglist);
        va_end(arglist);
    }

};

}

#endif // SQUALL_VMSTD_HPP_
