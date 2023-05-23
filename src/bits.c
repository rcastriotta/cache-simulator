#include "bits.h"
#include "cache.h"

int get_set(Cache *cache, address_type address) {
  unsigned int set_mask = (1 << cache->set_bits) - 1;
  return (address >> cache->block_bits) & set_mask;
}

int get_line(Cache *cache, address_type address) {
  return address >> (cache->block_bits + cache->set_bits);
}

int get_byte(Cache *cache, address_type address) {
  unsigned int byte_mask = (1 << cache->block_bits) - 1;
  return address & byte_mask;
}