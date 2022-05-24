#ifndef __SYLAR_SRC_ENDIAN_H__
#define __SYLAR_SRC_ENDIAN_H__

#define SYLAR_LITTLE_ENDIAN 1
#define SYLAR_BIG_ENDIAN 2

#include <cstdint>
#include <type_traits>

#include <byteswap.h>

namespace sylar {

/**
 * @brief convert uint64
 * @tparam T 
 * @param value 
 */
template<typename T>
typename std::enable_if<sizeof(T) == sizeof(uint64_t), T>::type 
byteswap(T value) {
    return (T)bswap_64((uint64_t) value);
}

/**
 * @brief convert uint32
 * @tparam T 
 * @param value 
 */
template<typename T>
typename std::enable_if<sizeof(T) == sizeof(uint32_t), T>::type 
byteswap(T value) {
    return (T)bswap_32((uint32_t) value);
}

/**
 * @brief convert uint16
 * @tparam T 
 * @param value 
 */
template<typename T>
typename std::enable_if<sizeof(T) == sizeof(uint16_t), T>::type 
byteswap(T value) {
    return (T)bswap_16((uint64_t) value);
}

#if BYTE_ORDER == BIG_ENDIAN 
#define SYLAR_BYTE_ORDER SYLAR_BIG_ENDIAN
#else
#define SYLAR_BYTE_ORDER SYLAR_LITTLE_ENDIAN
#endif


#if SYLAR_BYTE_ORDER == SYLAR_BIG_ENDIAN

template<typename T>
T byteswapOnLittleEndian(T t) {
    return t;
}

template<typename T>
T byteswapOnBigEndian(T t) {
    return byteswap(t);
}

#else 

template<typename T>
T byteswapOnLittleEndian(T t) {
    return byteswap(t);
}

template<typename T>
T byteswapOnBigEndian(T t) {
    return t;
}

#endif

}


#endif