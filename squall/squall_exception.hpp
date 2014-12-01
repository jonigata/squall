#ifndef SQUALL_EXCEPTION_HPP_
#define SQUALL_EXCEPTION_HPP_

#include <stdexcept>

namespace squall {

class squirrel_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
    squirrel_error(const std::string& s) : std::runtime_error(s) {}
};

}

#endif // SQUALL_EXCEPTION_HPP_
