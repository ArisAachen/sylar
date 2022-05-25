#ifndef __SYLAR_SRC_TCP_SERVER_H__
#define __SYLAR_SRC_TCP_SERVER_H__

#include "iomanager.h"
#include "noncopyable.h"

#include <memory>


namespace sylar {


class TcpServer : Noncopyable, 
            public std::enable_shared_from_this<TcpServer> {
public:
    typedef std::shared_ptr<TcpServer> ptr;

    TcpServer();


};


}

#endif