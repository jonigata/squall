#ifndef SQUALL_KLASS_TABLE_HPP_
#define SQUALL_KLASS_TABLE_HPP_

#include <squirrel.h>
#include <memory>
#include <unordered_map>
#include "squall_utility.hpp"
#include "squall_exception.hpp"

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
    KlassImp(HSQUIRRELVM vm, const string& name)
        : vm_(vm), name_(name), closed_(false) {
        
        make_getter_table();
        make_setter_table();
        
        keeper k(vm);
        sq_newclass(vm, SQFalse);
        sq_getstackobj(vm, -1, &sqclass_);
        sq_addref(vm, &sqclass_);

        defmetamethod(_SC("_get"), getter_table_, delegate_get);
        defmetamethod(_SC("_set"), setter_table_, delegate_set);
    }
    
    KlassImp(HSQUIRRELVM vm, const string& name, const HSQOBJECT& baseclass)
        : vm_(vm), name_(name), closed_(false) {

        make_getter_table();
        make_setter_table();
        
        keeper k(vm);
        sq_pushobject(vm, baseclass);
        sq_newclass(vm, SQTrue);
        sq_getstackobj(vm, -1, &sqclass_);
        sq_addref(vm, &sqclass_);

        defmetamethod(_SC("_get"), getter_table_, delegate_get);
        defmetamethod(_SC("_set"), setter_table_, delegate_set);
    }
    ~KlassImp() {
        sq_release(vm_, &sqclass_);
        sq_release(vm_, &getter_table_);
        sq_release(vm_, &setter_table_);
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
    HSQOBJECT get_getter_table() { return getter_table_; }
    HSQOBJECT get_setter_table() { return setter_table_; }
    
private:
    HSQOBJECT make_accessor_table() {
        HSQOBJECT tableobj;
        sq_newtable(vm_);
        sq_getstackobj(vm_, -1, &tableobj);
        sq_addref(vm_, &tableobj);
        sq_pop(vm_, 1);
        return tableobj;
    }

    void make_getter_table() {
        getter_table_ = make_accessor_table();
    }

    static SQInteger delegate_get(HSQUIRRELVM vm) {
        sq_push(vm, 2);
        if (!SQ_SUCCEEDED(sq_get(vm, -2))) {
            const SQChar* s;
            sq_getstring(vm, 2, &s);
            return sq_throwerror(
                vm, ("member variable '" + string(s) + "' not found").c_str());
        }
		
        sq_push(vm, 1);

        sq_call(vm, 1, SQTrue, SQTrue);
        return 1;
    }

    void make_setter_table() {
        setter_table_ = make_accessor_table();
    }
    
    static SQInteger delegate_set(HSQUIRRELVM vm) {
        sq_push(vm, 2);
        if (!SQ_SUCCEEDED(sq_get(vm, -2))) {
            const SQChar* s;
            sq_getstring(vm, 2, &s);
            return sq_throwerror(
                vm, ("member variable '" + string(s) + "' not found").c_str());
        }
		
        sq_push(vm, 1);
        sq_push(vm, 3);

        sq_call(vm, 2, SQTrue, SQTrue);
        return 0;
    }

    void defmetamethod(const SQChar* mm, HSQOBJECT table, SQFUNCTION f) {
        sq_pushstring(vm_, mm, -1);
        sq_pushobject(vm_, table);
        sq_newclosure(vm_, f, 1);
        sq_newslot(vm_, -3, false);
    }

private:
    HSQUIRRELVM vm_;
    string      name_;
    bool        closed_;
    HSQOBJECT   sqclass_;
    HSQOBJECT   getter_table_;
    HSQOBJECT   setter_table_;
    
};

}

class KlassTable {
private:
    typedef
        std::unordered_map<size_t,
                           std::shared_ptr<detail::KlassImpBase>>
        klasses_type;
    

public:
    template <class C, class Base>
    struct KlassAdd {
        static
        std::shared_ptr<detail::KlassImp<C>>
        doit(const klasses_type& klasses, HSQUIRRELVM vm, const string& name) {
            auto bh = detail::KlassImp<Base>::hash();
            return std::make_shared<detail::KlassImp<C>>(
                vm, name,
                (*klasses.find(bh)).second->get_klass_object());
        }
    };

    template <class C>
    struct KlassAdd<C, void> {
        static
        std::shared_ptr<detail::KlassImp<C>>
        doit(const klasses_type& klasses, HSQUIRRELVM vm, const string& name) {
            return std::make_shared<detail::KlassImp<C>>(vm, name);
        }
    };


    template <class C, class Base>
    std::weak_ptr<detail::KlassImp<typename std::remove_cv<C>::type>>
    add_klass(HSQUIRRELVM vm, const string& name) {
        typedef typename std::remove_cv<C>::type CC;
        size_t h = detail::KlassImp<CC>::hash();
        auto i = klasses_.find(h);
        if (i == klasses_.end()) {
            auto p = KlassAdd<CC, Base>::doit(klasses_, vm, name);
            klasses_[h] = p;
            return p;
        } else {
            return std::dynamic_pointer_cast<detail::KlassImp<CC>>(
                (*i).second);
        }
    }

    template <class C>
    bool find_klass_object(HSQOBJECT& obj) {
        typedef typename std::remove_cv<C>::type CC;
        size_t h = detail::KlassImp<CC>::hash();
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

inline
KlassTable& klass_table(HSQUIRRELVM vm) {
    return *((KlassTable*)sq_getforeignptr(vm));
}

}

#endif // SQUALL_KLASS_TABLE_HPP_
