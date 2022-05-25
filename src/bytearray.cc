#include "bytearray.h"
#include "endian.h"
#include "log.h"



#include <cstdint>
#include <sstream>
#include <string>
#include <cmath>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <exception>
#include <stdexcept>


namespace sylar {

static uint32_t EncodeZigzag32(const int32_t& v) {
    if(v < 0) {
        return ((uint32_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

static uint64_t EncodeZigzag64(const int64_t& v) {
    if(v < 0) {
        return ((uint64_t)(-v)) * 2 - 1;
    } else {
        return v * 2;
    }
}

static int32_t DecodeZigzag32(const uint32_t& v) {
    return (v >> 1) ^ -(v & 1);
}

static int64_t DecodeZigzag64(const uint64_t& v) {
    return (v >> 1) ^ -(v & 1);
}

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

void ByteArray::write_int8(int8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::write_uint8(uint8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::write_int16(int16_t value) {
    // check byte order
    if (endian_ != SYLAR_BYTE_ORDER) 
        value = byteswap(value);
    write(&value, sizeof(value));
}

void ByteArray::write_uint16(uint16_t value) {
    // check byte order
    if (endian_ != SYLAR_BYTE_ORDER) 
        value = byteswap(value);
    write(&value, sizeof(value));    
}

void ByteArray::write_int32(int32_t value) {
    // check byte order
    if (endian_ != SYLAR_BYTE_ORDER) 
        value = byteswap(value);
    write(&value, sizeof(value));    
}

void ByteArray::write_uint32(uint32_t value) {
    // check byte order
    if (endian_ != SYLAR_BYTE_ORDER) 
        value = byteswap(value);
    write(&value, sizeof(value));    
}

void ByteArray::write_int64(int64_t value) {
    // check byte order
    if (endian_ != SYLAR_BYTE_ORDER) 
        value = byteswap(value);
    write(&value, sizeof(value));    
}

void ByteArray::write_uint64(uint64_t value) {
    // check byte order
    if (endian_ != SYLAR_BYTE_ORDER) 
        value = byteswap(value);
    write(&value, sizeof(value));    
}

void ByteArray::write_zigzag_int32(int32_t value) {
    write_zigzag_uint32(EncodeZigzag32(value));
}

void ByteArray::write_zigzag_uint32(uint32_t value) {
    uint8_t tmp[5];
    uint8_t i = 0;
    while(value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);    
}

void ByteArray::write_zigzag_int64(int64_t value) {
    write_zigzag_uint64(EncodeZigzag64(value));
}

void ByteArray::write_zigzag_uint64(uint64_t value) {
    uint8_t tmp[10];
    uint8_t i = 0;
    while(value >= 0x80) {
        tmp[i++] = (value & 0x7F) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::write_float(float value) {
    uint32_t result;
    memcpy(&result, &value, sizeof(value));
    write_uint32(result);
}

void ByteArray::write_double(double value) {
    uint64_t result;
    memcpy(&result, &value, sizeof(value));
    write_uint32(result);
}

void ByteArray::write_string_u16(const std::string &message) {
    // write size
    write_uint16(message.size());
    write(message.c_str(), message.size());
}

void ByteArray::write_string_u32(const std::string &message) {
    // write size
    write_uint32(message.size());
    write(message.c_str(), message.size());
}

void ByteArray::write_string_u64(const std::string &message) {
    // write size
    write_uint64(message.size());
    write(message.c_str(), message.size());
}

void ByteArray::write_string_nolength(const std::string &message) {
    write(message.c_str(), message.size());
}

#define XX(type) \
    type value; \
    read(&value, sizeof(value)); \
    if (endian_ != SYLAR_BYTE_ORDER) \
        value = byteswap(value); \
    return value

int8_t ByteArray::read_int8() {
    int8_t value;
    read(&value, sizeof(value));
    return value;
}

uint8_t ByteArray::read_uint8() {
    uint8_t value;
    read(&value, sizeof(value));
    return value;
}

int16_t ByteArray::read_int16() {
    XX(int16_t);
}

uint16_t ByteArray::read_uint16() {
    XX(uint16_t);
}

int32_t ByteArray::read_int32() {
    XX(int32_t);
}

uint32_t ByteArray::read_uint32() {
    XX(int16_t);
}

int64_t ByteArray::read_int64() {
    XX(int64_t);
}

uint64_t ByteArray::read_uint64() {
    XX(uint64_t);
}

#undef XX

int32_t ByteArray::read_zigzag_int32() {
    return DecodeZigzag32(read_zigzag_uint32());
}

uint32_t ByteArray::read_zigzag_uint32() {
    uint32_t result = 0;
    for(int i = 0; i < 32; i += 7) {
        uint8_t b = read_uint8();
        if(b < 0x80) {
            result |= ((uint32_t)b) << i;
            break;
        } else {
            result |= (((uint32_t)(b & 0x7f)) << i);
        }
    }
    return result;
}

int64_t ByteArray::read_zigzag_int64() {
    return DecodeZigzag64(read_zigzag_uint64());
}

uint64_t ByteArray::read_zigzag_uint64() {
    uint64_t result = 0;
    for(int i = 0; i < 64; i += 7) {
        uint8_t b = read_uint8();
        if(b < 0x80) {
            result |= ((uint64_t)b) << i;
            break;
        } else {
            result |= (((uint64_t)(b & 0x7f)) << i);
        }
    }
    return result; 
}

float ByteArray::read_float() {
    uint32_t value = read_uint32();
    float result;
    memcpy(&result, &value, sizeof(value));
    return result;
}

double ByteArray::read_double() {
    uint64_t value = read_uint32();
    double result;
    memcpy(&result, &value, sizeof(value));
    return result;
}

std::string ByteArray::read_string_u16() {
    uint16_t len = read_uint16();
    std::string result;
    result.resize(len);
    read(&result[0], len);
    return result;
}

std::string ByteArray::read_string_u32() {
    uint32_t len = read_uint32();
    std::string result;
    result.resize(len);
    read(&result[0], len);
    return result;
}

std::string ByteArray::read_string_u64() {
    uint64_t len = read_uint64();
    std::string result;
    result.resize(len);
    read(&result[0], len);
    return result;
}


uint64_t ByteArray::get_read_buffer(std::vector<iovec>& buffers, uint64_t len) const {
    // TODO: should be done here
    return 0;
}

uint64_t ByteArray::get_read_buffer(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const {
    // TODO: should be done here
    return 0;
}

const std::string ByteArray::to_string() {
    std::string str;
    str.resize(get_readable_size());
    if (str.empty()) 
        return "";
    read(&str[0], str.size(), position_);
    return str;
}

const std::string ByteArray::to_hex_string() {
    // set to hex
    std::stringstream result;
    result << std::ios::hex;
    result << to_string();
    return result.str();
}

void ByteArray::clear() {
    position_ = size_ = 0;
    capacity_ = base_size_;
    Node* tmp = root_->next;
    while (tmp) {
        cur_ = tmp;
        tmp = tmp->next;
        delete cur_;
    }
    cur_ = root_;
    root_->next = nullptr;
}

void ByteArray::set_position(size_t size) {
    // check valid
    if (size > capacity_) 
        throw std::out_of_range("set position out of range");
    // save size
    position_ = size;
    // adjust size
    if (position_ > size_) 
        size_ = position_;
    cur_ = root_;
    // find cur
    while (size > cur_->size) {
        size -= cur_->size;
        cur_ = cur_->next;
    }
    if (size == cur_->size) 
        cur_ = cur_->next;
}

bool ByteArray::write_to_file(const std::string &filename) {
    // open file
    std::ofstream ofs;
    ofs.open(filename, std::ios::out | std::ios::trunc | std::ios::binary);
    // check if file open successfully
    if (!ofs.is_open()) {
        SYLAR_FMT_ERR("cannot open file to write, file name: %s, err: %s", filename.c_str(), 
            strerror(errno));
        return false;
    }
    // get result
    std::string result(to_string());
    try {
        ofs.write(result.c_str(), result.size());
    } catch (std::exception& err) {
        SYLAR_FMT_ERR("write byte array failed, filename: %s, err: %s", filename.c_str(), err.what());
        return false;
    }
    SYLAR_FMT_ERR("write byte array success, filename: %s", filename.c_str());
    return true;
}

bool ByteArray::read_from_file(const std::string &filename) {
    std::ifstream ifs;
    ifs.open(filename, std::ios::in | std::ios::binary);
    if (ifs.is_open()) {
        SYLAR_FMT_ERR("cannot open file to read, file name: %s, err: %s", filename.c_str(), 
            strerror(errno));
        return false;
    }
    try {
        std::string result;
        ifs >> result;
        write(result.c_str(), result.size());
    } catch (std::exception& err) {
        SYLAR_FMT_ERR("write byte array failed, file name: %s, err: %s", filename.c_str(), 
            strerror(errno));        
    }
    SYLAR_FMT_ERR("read byte array success, filename: %s", filename.c_str());
    return true;
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
            // current node rest part can not store size,
            // should store 
            memcpy(cur_->ptr + node_ocp, (const char*)buf + rem_pos, node_rest);
            // add position
            position_ += node_rest;
            // add buf pointer
            rem_pos += node_rest;
            // buf left
            size -= node_rest;
            // next node
            cur_ = cur_->next;
            // next node has full space
            node_rest = cur_->size;
            // next space from begin
            node_ocp = 0;
        }
    }
    // add position
    if (position_ > size_) {
        size_ = position_;
    }
    SYLAR_DEBUG("write buf to byte array successfully");
}

void ByteArray::read(void* buf, size_t size) {
    // check readable size
    if (size > get_readable_size()) 
        throw std::out_of_range("read buf len not enough");
    // node occupation
    size_t node_ocp = position_ % base_size_;
    // node rest
    size_t node_rest = cur_->size - node_ocp;
    // rem pos
    size_t rem_pos = 0;
    // TODO: here may be not correct
    // begin to read
    while (size > 0) {
        // node rest
        if (node_rest >= size) {
            // copy 
            memcpy((char*) buf + rem_pos , cur_->ptr + node_ocp, size);
            if (cur_->size == (node_ocp + size)) 
                cur_ = cur_->next;
            position_ += size;
            rem_pos += size;
            size = 0;
        } else {
            memcpy((char*) buf + rem_pos, cur_->ptr + node_ocp, node_rest);
            position_ += node_rest;
            rem_pos += node_rest;
            size -= node_rest;
            cur_ = cur_->next;
            node_rest = cur_->size;
            node_ocp = 0;
        }
    }
}

void ByteArray::read(void* buf, size_t size, size_t position) {
    // check readable size
    if (position + size > get_readable_size()) 
        throw std::out_of_range("read buf position not enough");
    // node occupation
    size_t node_ocp = position % base_size_;
    // node rest
    size_t node_rest = cur_->size - node_ocp;
    // rem pos
    size_t rem_pos = 0;
    // temp cur 
    Node* cur = cur_;
    while ( size > 0) {
        // node rest
        if (node_rest >= size) {
            // copy 
            memcpy((char*) buf + rem_pos , cur->ptr + node_ocp, size);
            if (cur->size == (node_ocp + size)) 
                cur = cur->next;
            position_ += size;
            rem_pos += size;
            size = 0;
        } else {
            memcpy((char*) buf + rem_pos, cur->ptr + node_ocp, node_rest);
            position_ += node_rest;
            rem_pos += node_rest;
            size -= node_rest;
            cur = cur->next;
            node_rest = cur_->size;
            node_ocp = 0;
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