#ifndef __SYLAR_SRC_SCHEDULER_H__
#define __SYLAR_SRC_SCHEDULER_H__


#include "mutex.h"

#include <memory>

namespace sylar {


class Scheduler  {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;


};













}



#endif