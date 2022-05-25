#include "stream.h"
#include "bytearray.h"
#include <cstddef>

namespace sylar {


int Stream::read_fix_size(void* buf, size_t len) {
    size_t offset = 0;
    size_t left = len;
    while (left > 0) {
        // begin to read
        size_t size = read((char*)buf + offset, left);
        // check if read successfully
        if (size < 0) 
            return size;
        offset += size;
        left -= size;
    }
    return len;
}

int Stream::read_fix_size(ByteArray::ptr arr, size_t len) {
    size_t left = len;
    while (left > 0) {
        size_t size = read(arr, left);
        if (size < 0) 
            return size;
        left -= size;
    }
    return len;
}

int Stream::write_fix_size(const void* buf, size_t len) {
    size_t offset = 0;
    size_t left = len;
    while (left > 0) {
        size_t size = write((const char*)buf + offset, left);
        if (size < 0)
            return size;
        offset += len;
        left -= size;
    }
    return len;
}

int Stream::write_fix_size(ByteArray::ptr arr, size_t len) {
    size_t left = len;
    while (left > 0) {
        size_t size = write(arr, left);
        if (size < 0) 
            return size;
        left -= size;
    }
    return len;
}

}