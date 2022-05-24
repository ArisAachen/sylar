#ifndef __SYLAR_SRC_BYTEARRAY_H__
#define __SYLAR_SRC_BYTEARRAY_H__

#include "endian.h"

#include <cstddef>
#include <cstdint>
#include <memory>


namespace sylar {

class ByteArray {
public:
    typedef std::shared_ptr<ByteArray> ptr;

    /**
     * @brief Construct a new Byte Array object
     * @param[in] base_size base node size
     */
    ByteArray(size_t base_size = 4096);

    /**
     * @brief Destroy the Byte Array object
     */
    ~ByteArray();

public:
    /**
     * @brief write int8 to array
     * @param[in] value int8 value
     */
    void write_int8(int8_t value);

    /**
     * @brief write uint8 to array
     * @param[in] value uint8 value
     */
    void write_uint8(uint8_t value);

    /**
     * @brief write int16 to array
     * @param[in] value int16 value
     */
    void write_int16(int16_t value);

    /**
     * @brief write uint16 to array
     * @param[in] value uint16 value
     */
    void write_uint16(uint16_t value);

    /**
     * @brief write int32 to array
     * @param[in] value int32 value
     */
    void write_int32(int32_t value);

    /**
     * @brief write uint32 to array
     * @param[in] value uint32 value
     */
    void write_uint32(uint32_t value);

    /**
     * @brief write int64 to array
     * @param[in] value int64 value
     */
    void write_int64(int64_t value);

    /**
     * @brief write uint64 to array
     * @param[in] value uint64 value
     */
    void write_uint64(uint64_t value);

    /**
     * @brief write float to array
     * @param[in] value float value
     */
    void write_float(float value);

    /**
     * @brief write double to array
     * @param[in] value double value
     */
    void write_double(double value);

public:
    /**
     * @brief check current byte order
     */
    bool is_little_endian() { return endian_ == SYLAR_LITTLE_ENDIAN; }

    /**
     * @brief Set the endian object
     * @param[in] endian set endian
     */
    void set_endian(int8_t endian) { endian_ = endian; }

    /**
     * @brief Set the position object
     * @param[in] size position
     */
    void set_position(size_t size);

    /**
     * @brief Get the position object
     */
    size_t get_position() { return position_; }

private:
    /**
     * @brief write buf to byte array
     * @param[in] buf buf pointer
     * @param[in] size buf size
     */
    void write(const void* buf, size_t size);

    /**
     * @brief add byte array capacity
     * @param[in] size capacity size
     */
    void add_capacity(size_t size);

    /**
     * @brief Get the capacity object
     */
    size_t get_capacity() { return capacity_ - position_; }

private:
    struct Node {
        /**
         * @brief Construct a new Node object
         * @param[in] size node size
         */
        Node(size_t size);

        /**
         * @brief Construct a new Node object
         */
        Node();

        /**
         * @brief Destroy the Node object
         */
        ~Node();

        /// memory pointer
        char* ptr;
        /// memory size
        size_t size;
        /// next pointer
        Node* next;
    };


private:
    /// base node size
    size_t base_size_ {0};
    /// current memory position
    size_t position_ {0};
    /// current total capacity
    size_t capacity_ {0};
    /// current size
    size_t size_ {0};
    /// endian
    int8_t endian_ {0};
    /// root node
    Node* root_ {nullptr};
    /// current node
    Node* cur_ {nullptr};
};







}



#endif