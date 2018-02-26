#ifndef SQUALL_EXCEPTION_HPP_
#define SQUALL_EXCEPTION_HPP_

#include <stdexcept>
#include "squall_utility.hpp"

namespace squall {

class squirrel_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    squirrel_error(const string& s) : std::runtime_error(locale_converter::to_std_string(s)) {}
};

}

#endif // SQUALL_EXCEPTION_HPP_
