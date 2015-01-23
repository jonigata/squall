// 2015/01/23 Naoyuki Hirayama

/*!
	@file	  squall_defvar.hpp
	@brief	  <概要>

	<説明>
*/

#ifndef SQUALL_DEFVAR_HPP_
#define SQUALL_DEFVAR_HPP_

#include <squirrel.h>
#include "squall_stack_operation.hpp"

namespace squall {

namespace detail {

template <class C, class V> inline
SQInteger var_getter(HSQUIRRELVM vm) {
    C* p = nullptr;
    sq_getinstanceup(vm, 1, (SQUserPointer*)&p, nullptr);
		
    typedef V C::*M;
    M* mp = nullptr;
    sq_getuserdata(vm, -1, (SQUserPointer*)&mp, nullptr);
    M m = *mp;

    push(vm, p->*m);

    return 1;
}

template <class C, class V>
void defvar_local(
    HSQUIRRELVM     vm,
    HSQOBJECT       getter_table,
    const string&   name,
    V C::*          var) {

    sq_pushobject(vm, getter_table);
    sq_pushstring(vm, name.data(), name.length());

    SQUserPointer vp = sq_newuserdata(vm, sizeof(var));
    memcpy(vp, &var, sizeof(var));

    sq_newclosure(vm, var_getter<C, V>, 1);
    sq_newslot(vm, -3, false);
    sq_pop(vm, 1);
}

}

}

#endif // SQUALL_DEFVAR_HPP_
