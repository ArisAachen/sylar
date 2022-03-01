#ifndef __SYLAR_SRC_MACRO_H__
#define __SYLAR_SRC_MACRO_H__

#include <cassert>


namespace sylar {

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define SYLAR_ASSERT(x) \
    if (unlikely(x)) \
        assert(x)
}

#endif