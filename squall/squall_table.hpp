#ifndef SQUALL_TABLE_HPP_
#define SQUALL_TABLE_HPP_

#include "squall_table_base.hpp"

namespace squall {

class Table : public TableBase {
public:
    Table(VM& vm) : TableBase(vm.handle(), make_table(vm.handle())) {}
    ~Table() { sq_release(handle(), &tableobj()); }        

private:
    static HSQOBJECT make_table(HSQUIRRELVM vm) {
        HSQOBJECT tableobj;
        sq_newtable(vm);
        sq_getstackobj(vm, -1, &tableobj);
        sq_addref(vm, &tableobj);
        sq_pop(vm, 1);
        return tableobj;
    }

};

}

#endif // SQUALL_TABLE_HPP_
