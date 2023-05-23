#include "lru.h"
#include "cache.h"
#include <stdio.h>
#include <stdlib.h>

void lru_init_queue(Set *set) {
  LRUNode *s = NULL;
  LRUNode **pp = &s; // place to chain in the next node
  for (int i = 0; i < set->line_count; i++) {
    Line *line = &set->lines[i];
    LRUNode *node = (LRUNode *)(malloc(sizeof(LRUNode)));
    node->line = line;
    node->next = NULL;
    (*pp) = node;
    pp = &((*pp)->next);
  }
  set->lru_queue = s;
}

void lru_init(Cache *cache) {
  Set *sets = cache->sets;
  for (int i = 0; i < cache->set_count; i++) {
    lru_init_queue(&sets[i]);
  }
}

void lru_destroy(Cache *cache) {
  Set *sets = cache->sets;
  for (int i = 0; i < cache->set_count; i++) {
    LRUNode *p = sets[i].lru_queue;
    LRUNode *n = p;
    while (p != NULL) {
      p = p->next;
      free(n);
      n = p;
    }
    sets[i].lru_queue = NULL;
  }
}

void lru_fetch(Set *set, unsigned int tag, LRUResult *result) {
  // TODO:
  // Implement the LRU algorithm to determine which line in
  // the cache should be accessed.
  LRUNode *prev_node = NULL;
  LRUNode *current_node = set->lru_queue;

  while (current_node != NULL) {
    Line *line = current_node->line;

    // Case: HIT
    if (line->valid && line->tag == tag) {
      result->access = HIT;
      result->line = line;

      // Move the accessed node to the front of the LRU queue
      if (prev_node != NULL) {
        prev_node->next = current_node->next;
        current_node->next = set->lru_queue;
        set->lru_queue = current_node;
      }
      return;
    }

    prev_node = current_node;
    current_node = current_node->next;
  }

  // Case: MISS
  // Get the least recently used node (last node in the LRU queue)
  LRUNode *lru_node = set->lru_queue;
  prev_node = NULL;
  while (lru_node->next != NULL) {
    prev_node = lru_node;
    lru_node = lru_node->next;
  }

  Line *evicted_line = lru_node->line;

  // Update the result and the line
  result->line = evicted_line;
  result->access = evicted_line->valid ? CONFLICT_MISS : COLD_MISS;
  evicted_line->valid = 1;
  evicted_line->tag = tag;

  // Move the evicted node to the front of the LRU queue
  if (prev_node != NULL) {
    prev_node->next = lru_node->next;
    lru_node->next = set->lru_queue;
    set->lru_queue = lru_node;
  }
}