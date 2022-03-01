#ifndef __SYLAR_SRC_NONCOPYABLE_H__
#define __SYLAR_SRC_NONCOPYABLE_H__

namespace sylar {

// Noncopyable indicate class or subclass is not 
// allow copy
class Noncopyable {
public:
    // default constructor
    Noncopyable() = default;
    virtual~Noncopyable() = default;

    // dont allow copy
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
};

}

#endif