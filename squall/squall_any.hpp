#ifndef SQUALL_ANY_HPP_
#define SQUALL_ANY_HPP_

#include <memory>

namespace squall {

namespace detail {

struct AnyImpBase {
    virtual ~AnyImpBase() {}
};

template <class T>
struct AnyImp : public AnyImpBase {
    AnyImp(const T& x) : value(x) {}
    ~AnyImp(){}
    T value;
};

}

class Any {
public:
    template <class T>
    Any(const T& x) : imp_(new detail::AnyImp<T>(x)){}
    ~Any() = default;

    Any(const Any&) = delete;
    Any(const Any&&) = delete;
    void operator=(const Any&) = delete;
    void operator=(const Any&&) = delete;

    template <class T>
    const T& cast() const {
        return dynamic_cast<detail::AnyImp<T>&>(*imp_).value;
    }
        
private:
    std::unique_ptr<detail::AnyImpBase> imp_;
    
};

}

#endif // SQUALL_ANY_HPP_
