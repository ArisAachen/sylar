#ifndef __SYLAR_SRC_SINGLETON_H__
#define __SYLAR_SRC_SINGLETON_H__

#include "noncopyable.h"

#include <memory>

namespace sylar {

template<typename T>
class Singleton : Noncopyable{
public:
    template<typename... Args>
    /**
     * @brief create type singleton 
     * @param[in] arg type elem's construct params
     */
    static T* get_instance(Args... args) {
        static T* instance_ = new T(args...);
        return instance_;
    }
};

template <typename T>
class SingletonPtr : Noncopyable {
public:
    typedef std::shared_ptr<T> ptr;
    template<typename... Args>
    /**
     * @brief create share ptr type singleton 
     * @param[in] args type elem's construct params
     */
    static SingletonPtr<T>::ptr get_instance(Args... args) {
        static SingletonPtr<T>::ptr instance_ = std::make_shared<T>(args...);
        return instance_;
    }
};
}
#endif