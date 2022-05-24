#include "bytearray.h"
#include "log.h"

#include <cmath>
#include <cstddef>
#include <cstring>

namespace sylar {


ByteArray::Node::Node(size_t size): 
ptr(new char[size]), next(nullptr), size(size) {
    
}

ByteArray::Node::Node():
ptr(nullptr), next(nullptr), size(0) {

}

ByteArray::Node::~Node() {
    if (ptr)
        delete []ptr;
    next = nullptr;
    size = 0;
}

ByteArray::ByteArray(size_t base_size): 
    base_size_(base_size), position_(0), capacity_(base_size), 
    size_(0), endian_(SYLAR_BIG_ENDIAN), root_(new Node(base_size)), 
    cur_(root_) {
    SYLAR_FMT_DEBUG("create byte array, base size: %d", base_size);
}

ByteArray::~ByteArray() {
    // release node
    Node* tmp = root_; 
    while (tmp) {
        cur_ = tmp;
        tmp = tmp->next;
        delete cur_;
    }
}

void ByteArray::write(const void* buf, size_t size) {
    // check size
    if (size == 0)
        return;
    // add capacity
    add_capacity(size);
    // node occupation
    size_t node_ocp = position_ % base_size_;
    // node rest
    size_t node_rest = cur_->size - node_ocp;
    // rem position
    size_t rem_pos = 0;
    // begin to save
    while (size > 0) {
        // current node can store size, 
        // store rest part of buf to node rest space
        if (node_rest > size) {
            // copy memory
            memcpy(cur_->ptr + node_ocp, (const char*)buf + rem_pos, size);
            // check if current node is full
            // if is, set current node to next
            if (cur_->size == (node_ocp + size)) 
                cur_ = cur_->next;
            // record new position
            position_ += size;
            // record buf pos
            rem_pos += size;
            size = 0;
        } else {
            
        }
    }
}

void ByteArray::add_capacity(size_t size) {
    // check if need realloc capacity
    if (size == 0) 
        return;
    // get origin cap
    size_t origin_cap = get_capacity();
    if (origin_cap > size) 
        return;
    // get size
    size = size - origin_cap;
    // check create node count
    size_t count = ceil(1.0 * size / base_size_);
    // find last node
    Node* tmp = root_;
    while (tmp->next) {
        tmp = tmp->next;
    }

    Node* first = nullptr;
    for (size_t i = 0; i < count; i++) {
        // create node
        tmp->next = new Node(base_size_);
        if (first == nullptr)
            first = tmp->next;
        tmp = tmp->next;
        // add size
        capacity_ += base_size_;
    }
    // save first node
    if (origin_cap == 0) 
        cur_ = first;
    SYLAR_FMT_DEBUG("byte array has realloc, capacity: %d", capacity_);
}



}