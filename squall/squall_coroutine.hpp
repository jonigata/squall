#ifndef SQUALL_COROUTINE_HPP_
#define SQUALL_COROUTINE_HPP_

#include <squirrel.h>

namespace squall {

class Coroutine {
public:
    Coroutine() : vm_(nullptr) {}
    Coroutine(HSQUIRRELVM vm, SQInteger top) : vm_(vm), top_(top) {
        check_suspended();
    }
    Coroutine(Coroutine&& c)
        : vm_(c.vm_), top_(c.top_), suspended_(c.suspended_) {
        c.vm_ = nullptr;
    }
    ~Coroutine() { if (vm_) { sq_settop(vm_, top_); } }

    Coroutine& operator=(Coroutine&& c) {
        vm_ = c.vm_;
        top_ = c.top_;
        suspended_ = c.suspended_;
        return *this;
    }

    bool suspended() const { validate_vm(); return suspended_; }

    template <class R>
    R yielded() {
        validate_vm();
        return detail::fetch<R, detail::FetchContext::YieldedValue>(vm_, -1);
    }

    template <class R>
    R result() {
        validate_vm();
        RestoreStack r(vm_, top_);
        return detail::call_teardown<R>(vm_);
    }

    template <class T>
    void resume(T v) {
        validate_vm();
        detail::push(vm_, v);
        if (!SQ_SUCCEEDED(sq_wakeupvm(vm_, SQTrue, SQTrue, SQTrue, SQFalse))) {
            throw squirrel_error("wake up vm failed");
        }
        check_suspended();
    }

    void resume() {
        validate_vm();
        if (!SQ_SUCCEEDED(sq_wakeupvm(vm_, SQFalse, SQTrue, SQTrue, SQFalse))) {
            throw squirrel_error("wake up vm failed");
        }
        check_suspended();
    }

private:
    struct RestoreStack {
        RestoreStack(HSQUIRRELVM& vm, SQInteger top) : vm_(vm), top_(top) {}
        ~RestoreStack() {
            sq_settop(vm_, top_);
            vm_ = nullptr;
        }
        HSQUIRRELVM&    vm_;
        SQInteger       top_;
    };

    void check_suspended() {
        suspended_ = sq_getvmstate(vm_) == SQ_VMSTATE_SUSPENDED;
    }

    void validate_vm() const { 
        if (!vm_) {
            throw squirrel_error("apply invalid coroutine");
        }
    }

private:
    HSQUIRRELVM     vm_;
    SQInteger       top_;
    bool            suspended_;

};

}

#endif // SQUALL_COROUTINE_HPP_
