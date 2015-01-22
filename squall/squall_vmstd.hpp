#ifndef SQUALL_VMSTD_HPP_
#define SQUALL_VMSTD_HPP_

#include <squirrel.h>
#include <sqstdio.h>
#include <sqstdaux.h>

#include "squall_vm.hpp"
#include <stdarg.h>

namespace squall {

namespace detail {

template <class T>
void pf(HSQUIRRELVM v, const T* s, ...);

template <>
void inline pf<char>(HSQUIRRELVM v, const char* s, ...)  {
    va_list arglist;
    va_start(arglist, s);
    vprintf(s, arglist);
    va_end(arglist);
}

template <>
void inline pf<wchar_t>(HSQUIRRELVM v, const wchar_t* s, ...)  {
    va_list arglist;
    va_start(arglist, s);
    vwprintf(s, arglist);
    va_end(arglist);
}

}


class VMStd : public VM {
public:
    VMStd(int stack_size = 1024) : VM(stack_size) {
        sqstd_seterrorhandlers(handle());
        sq_setprintfunc(handle(), &detail::pf<SQChar>, &detail::pf<SQChar>);
    }

    void dofile(const SQChar* filename) {
        keeper k(handle());
        sq_pushroottable(handle());
        if (!SQ_SUCCEEDED(sqstd_dofile(handle(), filename, 0, 1))) {
            throw squirrel_error("dofile failed");
        }
    }

private:

};

}

#endif // SQUALL_VMSTD_HPP_
