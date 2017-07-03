#ifndef __COMMON_HASHKIT_H
#define __COMMON_HASHKIT_H

#include <string>
#include <stdint.h>

#if 0
void md5_signature(const unsigned char *key, unsigned int length, unsigned char *result);
uint32_t hash_md5(const char *key, size_t key_length);
uint32_t hash_crc16(const char *key, size_t key_length);
uint32_t hash_crc32(const char *key, size_t key_length);
uint32_t hash_crc32a(const char *key, size_t key_length);
uint32_t hash_fnv1_64(const char *key, size_t key_length);
uint32_t hash_fnv1a_64(const char *key, size_t key_length);
uint32_t hash_fnv1_32(const char *key, size_t key_length);
uint32_t hash_fnv1a_32(const char *key, size_t key_length);
uint32_t hash_hsieh(const char *key, size_t key_length);
uint32_t hash_jenkins(const char *key, size_t length);
#endif

//typedef unsigned int uint32_t;

uint32_t hash_murmur(const char *key, size_t length);

inline uint32_t hash_murmur(const std::string& key)
{
	return hash_murmur(key.c_str(), key.size());
}

#endif // __COMMON_HASHKIT_H
