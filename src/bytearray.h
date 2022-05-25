#ifndef __SYLAR_SRC_BYTEARRAY_H__
#define __SYLAR_SRC_BYTEARRAY_H__

#include "endian.h"


#include <bits/types/struct_iovec.h>
#include <memory>
#include <string>
#include <cstddef>
#include <cstdint>
#include <vector>



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
     * @brief use zigzag to encode value 
     * @param[in] value before zigzag value
     */
    void write_zigzag_int32(int32_t value);

    /**
     * @brief use zigzag to encode value 
     * @param[in] value before zigzag value
     */
    void write_zigzag_uint32(uint32_t value);

    /**
     * @brief use zigzag to encode value 
     * @param[in] value before zigzag value
     */
    void write_zigzag_int64(int64_t value);

    /**
     * @brief use zigzag to encode value 
     * @param[in] value before zigzag value
     */
    void write_zigzag_uint64(uint64_t value);

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

    /**
     * @brief write string with u16 length
     * @param[in] message write message 
     */
    void write_string_u16(const std::string& message);

    /**
     * @brief write string with u32 length
     * @param[in] message write message 
     */
    void write_string_u32(const std::string& message);

    /**
     * @brief write string with u64 length
     * @param[in] message write message 
     */
    void write_string_u64(const std::string& message);

    /**
     * @brief write string without length
     * @param[in] message write message 
     */
    void write_string_nolength(const std::string& message);

public:
    /**
     * @brief read int8
     */
    int8_t read_int8();

    /**
     * @brief read uint8
     */
    uint8_t read_uint8();

    /**
     * @brief read int16
     */
    int16_t read_int16();

    /**
     * @brief read uint16
     */
    uint16_t read_uint16();

    /**
     * @brief read int32
     */
    int32_t read_int32();

    /**
     * @brief read uint32
     */
    uint32_t read_uint32();

    /**
     * @brief read int64
     */
    int64_t read_int64();

    /**
     * @brief read uint64
     */
    uint64_t read_uint64();

    /**
     * @brief read zigzag int32
     */
    int32_t read_zigzag_int32();

    /**
     * @brief read zigzag uint32
     */
    uint32_t read_zigzag_uint32();

    /**
     * @brief read zigzag int64
     */
    int64_t read_zigzag_int64();

    /**
     * @brief read zigzag uint64
     */
    uint64_t read_zigzag_uint64();

    /**
     * @brief read float
     */
    float read_float();

    /**
     * @brief read double
     */
    double read_double();

    /**
     * @brief read string u16
     */
    std::string read_string_u16();

    /**
     * @brief read string u32
     */
    std::string read_string_u32();

    /**
     * @brief read string u16
     */
    std::string read_string_u64();

    /**
     * @brief Get the read buffer object
     * @param[out] buffers io vec
     * @param[in] len len
     */
    uint64_t get_read_buffer(std::vector<iovec>& buffers, uint64_t len = ~0ull) const;

    /**
     * @brief Get the read buffer object
     * @param[out] buffers io vec
     * @param[in] len read len
     * @param[in] position read position
     */
    uint64_t get_read_buffer(std::vector<iovec>& buffers, uint64_t len, uint64_t position) const;

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

    /**
     * @brief Get the readable size object
     */
    size_t get_readable_size() { return size_ - position_; }

    /**
     * @brief clear all array
     */
    void clear();

public: 
    /**
     * @brief to string
     */
    const std::string to_string();

    /**
     * @brief get hex string
     */
    const std::string to_hex_string();

    /**
     * @brief write byte array to file
     * @param[in] filename save file name
     */
    bool write_to_file(const std::string& filename);

    /**
     * @brief read byte array from file
     * @param[in] filename read file name
     */
    bool read_from_file(const std::string& filename);

private:
    /**
     * @brief write buf to byte array
     * @param[in] buf buf pointer
     * @param[in] size buf size
     */
    void write(const void* buf, size_t size);

    /**
     * @brief read buf from array
     * @param[in] buf buf pointer
     * @param[in] size buf size
     */
    void read(void* buf, size_t size);

    /**
     * @brief read buf from array
     * @param[in] buf buf pointer
     * @param[in] size buf size
     * @param[in] position read position
     */    
    void read(void* buf, size_t size, size_t position);

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
    /// total size
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