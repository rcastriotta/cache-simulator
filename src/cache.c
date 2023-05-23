#include "cache.h"
#include "bits.h"
#include "cpu.h"
#include "lru.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

char *make_block(int block_size) {
  char *accessed = (char *)calloc(block_size, sizeof(char));
  return accessed;
}

Line *make_lines(int line_count, int block_size) {
  Line *linesArr = (Line *)malloc(line_count * sizeof(Line));

  for (int i = 0; i < line_count; i++) {
    linesArr[i].valid = 0;
    linesArr[i].tag = 0;
    linesArr[i].accessed = make_block(block_size);
    linesArr[i].block_size = block_size;
  }

  return linesArr;
}

Set *make_sets(int set_count, int line_count, int block_size) {
  Set *setsArr = (Set *)malloc(set_count * sizeof(Set));

  for (int i = 0; i < set_count; i++) {
    setsArr[i].lines = make_lines(line_count, block_size);
    setsArr[i].line_count = line_count;
    setsArr[i].lru_queue = NULL;
  }

  return setsArr;
}

Cache *make_cache(int set_bits, int line_count, int block_bits) {
  Cache *cache = (Cache *)malloc(sizeof(Cache));

  cache->set_bits = set_bits;
  cache->block_bits = block_bits;
  cache->set_count = exp2(set_bits);
  cache->line_count = line_count;
  cache->block_size = exp2(block_bits);
  cache->sets = make_sets(cache->set_count, line_count, cache->block_size);

  // Create LRU queues for sets:
  if (cache != NULL) {
    lru_init(cache);
  }

  return cache;
}

void delete_block(char *accessed) { free(accessed); }

void delete_lines(Line *lines, int line_count) {
  for (int i = 0; i < line_count; i++) {
    delete_block(lines[i].accessed);
  }
  free(lines);
}

void delete_sets(Set *sets, int set_count) {
  for (int i = 0; i < set_count; i++) {
    delete_lines(sets[i].lines, sets[i].line_count);
  }
  free(sets);
}

void delete_cache(Cache *cache) {
  lru_destroy(cache);
  delete_sets(cache->sets, cache->set_count);
  free(cache);
}

AccessResult cache_access(Cache *cache, TraceLine *trace_line) {
  unsigned int s = get_set(cache, trace_line->address);
  unsigned int t = get_line(cache, trace_line->address);
  unsigned int b = get_byte(cache, trace_line->address);

  // Get the set:
  Set *set = &cache->sets[s];

  // Get the line:
  LRUResult result;
  lru_fetch(set, t, &result);
  Line *line = result.line;

  // If it was a miss we will clear the accessed bits:
  if (result.access != HIT) {
    for (int i = 0; i < cache->block_size; i++) {
      line->accessed[i] = 0;
    }
  }

  // Then set the accessed byte to 1:
  line->accessed[b] = 1;

  return result.access;
}
