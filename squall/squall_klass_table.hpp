#ifndef SQUALL_KLASS_TABLE_HPP_
#define SQUALL_KLASS_TABLE_HPP_

#include <squirrel.h>
#include <memory>
#include <unordered_map>
#include "squall_utility.hpp"

namespace squall {

namespace detail {

////////////////////////////////////////////////////////////////
// klass base
class KlassImpBase {
public:
    virtual ~KlassImpBase() {}

    virtual void close() = 0;
    virtual HSQOBJECT get_klass_object() = 0;
};

template <class C>
class KlassImp : public KlassImpBase {
public:
    static size_t hash() { return typeid(KlassImp<C>).hash_code(); }

public:
    KlassImp(HSQUIRRELVM vm, const std::string& name)
        : vm_(vm), name_(name), closed_(false) {

        keeper k(vm);
        sq_newclass(vm, SQFalse);
        sq_getstackobj(vm, -1, &sqclass_);
        sq_addref(vm, &sqclass_);
    }
    ~KlassImp() {
        sq_release(vm_, &sqclass_);
    }

    void close() {
        if (closed_) { return; }

        keeper k(vm_);
        sq_pushroottable(vm_); // TODO: root固定になってる
        sq_pushstring(vm_, name_.c_str(), -1);
        sq_pushobject(vm_, sqclass_);
        sq_createslot(vm_, -3);

        closed_ = true;
    }

    HSQOBJECT get_klass_object() { return sqclass_; }

private:
    HSQUIRRELVM vm_;
    std::string name_;
    bool        closed_;
    HSQOBJECT   sqclass_;
    
};

}

class KlassTable {
public:
    template <class C>
    std::weak_ptr<detail::KlassImp<C>>
    add_klass(HSQUIRRELVM vm, const std::string& name) {
        size_t h = detail::KlassImp<C>::hash();
        auto i = klasses_.find(h);
        if (i == klasses_.end()) {
            auto p = std::make_shared<detail::KlassImp<C>>(vm, name);
            klasses_[h] = p;
            return p;
        } else {
            return std::dynamic_pointer_cast<detail::KlassImp<C>>((*i).second);
        }
    }

    template <class C>
    bool find_klass_object(HSQOBJECT& obj) {
        size_t h = detail::KlassImp<C>::hash();
        auto i = klasses_.find(h);
        if (i == klasses_.end()) {
            return false;
        } else {
            auto p = (*i).second;
            p->close();
            obj = p->get_klass_object();
            return true;
        }
    }

private:
    std::unordered_map<size_t, std::shared_ptr<detail::KlassImpBase>> klasses_;

};

}

#endif // SQUALL_KLASS_TABLE_HPP_
