#ifndef SQUALL_CLOSURE_DECL_HPP_
#define SQUALL_CLOSURE_DECL_HPP_

#include <memory>

namespace squall {

namespace detail {

template <class T>
class Closure;

template <class R, class... A>
class Closure<R (A...)> {
public:
    struct Imp {
        Imp(HSQUIRRELVM vm, SQInteger index) : vm_(vm) {
            sq_getstackobj(vm, index, &closure_);
            sq_addref(vm_, &closure_);
        }
        ~Imp() {
            sq_release(vm_, &closure_);
        }
        HSQUIRRELVM vm_;
        HSQOBJECT   closure_;
    };
    
public:
    Closure(HSQUIRRELVM vm, SQInteger index)
        : imp_(std::make_shared<Imp>(vm, index)) {}

    R operator()(A...);

private:
    std::shared_ptr<Imp>    imp_;
    
};

}

}

#endif // SQUALL_CLOSURE_DECL_HPP_
