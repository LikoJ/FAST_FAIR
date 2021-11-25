/*
   Copyright (c) 2018, UNIST. All rights reserved. The license is a free
   non-exclusive, non-transferable license to reproduce, use, modify and display
   the source code version of the Software, with or without modifications solely
   for non-commercial research, educational or evaluation purposes. The license
   does not entitle Licensee to technical support, telephone assistance,
   enhancements or updates to the Software. All rights, title to and ownership
   interest in the Software, including all intellectual property rights therein
   shall remain in UNIST.

   Please use at your own risk.
*/

#include <cassert>
#include <climits>
#include <fstream>
#include <future>
#include <iostream>
#include <libpmemobj.h>
#include <math.h>
#include <mutex>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <vector>

#define PAGESIZE (512)

#define CACHE_LINE_SIZE 64

#define IS_FORWARD(c) (c % 2 == 0)

#define KEY_MAX_LENGTH 16

class btree;
class page;
class MyString;

POBJ_LAYOUT_BEGIN(btree);
POBJ_LAYOUT_ROOT(btree, btree);
POBJ_LAYOUT_TOID(btree, page);
POBJ_LAYOUT_END(btree);

// using entry_key_t = int64_t;
using entry_key_t = MyString;

using namespace std;

class MyString {
public:
  MyString() {
    memset(str, 0, KEY_MAX_LENGTH);
  }

  MyString(const char* _str) {
    memcpy(str, _str, KEY_MAX_LENGTH);
  }

  MyString(const MyString& ms) {
    memcpy(str, ms.str, KEY_MAX_LENGTH);
  }

  MyString& operator=(const MyString& ms) {
    memcpy(str, ms.str, KEY_MAX_LENGTH);
    return *this;
  }

  MyString& operator=(const char* _str) {
    memcpy(str, _str, KEY_MAX_LENGTH);
    return *this;
  }

  MyString& operator=(const int64_t num) {
    if (num == LONG_MAX) {
      memset(str, 127, KEY_MAX_LENGTH);
    } else if (num == 0) {
      memset(str, 0, KEY_MAX_LENGTH);
    }
    return *this;
  }

  bool operator==(const MyString& ms) {
    for (int i = 0; i < KEY_MAX_LENGTH; i++) {
      if (str[i] != ms.str[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator<(const MyString& ms) {
    bool equal = true;
    for (int i = 0; i < KEY_MAX_LENGTH; i++) {
      if (str[i] < ms.str[i]) {
        equal = false;
      } else if (str[i] > ms.str[i]) {
        return false;
      }
    }
    return !equal;
  }

  bool operator>(const MyString& ms) {
    bool equal = true;
    for (int i = 0; i < KEY_MAX_LENGTH; i++) {
      if (str[i] > ms.str[i]) {
        equal = false;
      } else if (str[i] < ms.str[i]) {
        return false;
      }
    }
    return !equal;
  }

  bool operator>=(const MyString& ms) {
    for (int i = 0; i < KEY_MAX_LENGTH; i++) {
      if (str[i] < ms.str[i]) {
        return false;
      }
    }
    return true;
  }
private:
  char str[KEY_MAX_LENGTH];
};

class btree {
private:
  int height;
  TOID(page) root;
  PMEMobjpool *pop;

public:
  btree();
  void constructor(PMEMobjpool *);
  void setNewRoot(TOID(page));
  void btree_insert(entry_key_t, char *);
  void btree_insert_internal(char *, entry_key_t, char *, uint32_t);
  void btree_delete(entry_key_t);
  void btree_delete_internal(entry_key_t, char *, uint32_t, entry_key_t *,
                             bool *, page **);
  char *btree_search(entry_key_t);
  void btree_search_range(entry_key_t, entry_key_t, unsigned long *);
  void printAll();
  void randScounter();

  friend class page;
};

class header {
private:
  TOID(page) sibling_ptr; // 16 bytes
  page *leftmost_ptr;     // 8 bytes
  uint32_t level;         // 4 bytes
  uint8_t switch_counter; // 1 bytes
  uint8_t is_deleted;     // 1 bytes
  int16_t last_index;     // 2 bytes

  friend class page;
  friend class btree;

public:
  void constructor() {
    leftmost_ptr = NULL;
    TOID_ASSIGN(sibling_ptr, pmemobj_oid(this));
    sibling_ptr.oid.off = 0;
    switch_counter = 0;
    last_index = -1;
    is_deleted = false;
  }
};

class entry {
private:
  entry_key_t key; // 8 bytes
  char *ptr;       // 8 bytes

public:
  void constructor() {
    key = LONG_MAX;
    ptr = NULL;
  }

  friend class page;
  friend class btree;
};

const int cardinality = (PAGESIZE - sizeof(header)) / sizeof(entry);
const int count_in_line = CACHE_LINE_SIZE / sizeof(entry);

class page {
private:
  header hdr;                 // header in persistent memory, 32 bytes
  entry records[cardinality]; // slots in persistent memory, 16 bytes * n

public:
  friend class btree;

  void constructor(uint32_t level = 0);

  // this is called when tree grows
  void constructor(PMEMobjpool *pop, page *left, entry_key_t key, page *right,
                   uint32_t level = 0);

  inline int count();

  inline bool remove_key(PMEMobjpool *pop, entry_key_t key);

  bool remove(btree *bt, entry_key_t key, bool only_rebalance = false,
              bool with_lock = true);

  inline void insert_key(PMEMobjpool *pop, entry_key_t key, char *ptr,
                         int *num_entries, bool flush = true,
                         bool update_last_index = true);

  // Insert a new key - FAST and FAIR
  page *store(btree *bt, char *left, entry_key_t key, char *right, bool flush,
              page *invalid_sibling = NULL);

  // Search keys with linear search
  void linear_search_range(entry_key_t min, entry_key_t max,
                           unsigned long *buf);

  char *linear_search(entry_key_t key);

  // print a node
  void print();

  void printAll();
};