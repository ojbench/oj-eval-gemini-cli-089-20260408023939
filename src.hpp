// Copyright (c) 2024 ACM Class, SJTU

#ifndef SRC_HPP
#define SRC_HPP

#include <vector>
#include <algorithm>

namespace sjtu {

class BuddyAllocator {
public:
  BuddyAllocator(int ram_size, int min_block_size) {
    this->ram_size = ram_size;
    this->min_block_size = min_block_size;
    this->N = ram_size / min_block_size;
    tree.resize(4 * N + 1);
    build(1, 0, N);
  }

  int malloc(int size) {
    int blocks = size / min_block_size;
    if (blocks > N) return -1;
    int res = malloc_tree(1, 0, N, blocks);
    if (res != -1) {
      return res * min_block_size;
    }
    return -1;
  }

  int malloc_at(int addr, int size) {
    int blocks = size / min_block_size;
    int target_l = addr / min_block_size;
    int target_r = target_l + blocks;
    if (target_r > N) return -1;
    bool success = malloc_at_tree(1, 0, N, target_l, target_r);
    if (success) {
      return addr;
    }
    return -1;
  }

  void free_at(int addr, int size) {
    int blocks = size / min_block_size;
    int target_l = addr / min_block_size;
    int target_r = target_l + blocks;
    free_at_tree(1, 0, N, target_l, target_r);
  }

private:
  struct Node {
    int max_free;
    int lazy; // 0: none, 1: FULL, 2: EMPTY
  };

  int ram_size;
  int min_block_size;
  int N;
  std::vector<Node> tree;

  void build(int u, int l, int r) {
    tree[u].lazy = 0;
    tree[u].max_free = r - l;
    if (r - l == 1) return;
    int mid = l + (r - l) / 2;
    build(2 * u, l, mid);
    build(2 * u + 1, mid, r);
  }

  void apply(int u, int l, int r, int lazy_val) {
    if (lazy_val == 1) {
      tree[u].max_free = 0;
      tree[u].lazy = 1;
    } else if (lazy_val == 2) {
      tree[u].max_free = r - l;
      tree[u].lazy = 2;
    }
  }

  void push_down(int u, int l, int r) {
    if (tree[u].lazy != 0) {
      int mid = l + (r - l) / 2;
      apply(2 * u, l, mid, tree[u].lazy);
      apply(2 * u + 1, mid, r, tree[u].lazy);
      tree[u].lazy = 0;
    }
  }

  void push_up(int u, int l, int r) {
    int mid = l + (r - l) / 2;
    if (tree[2 * u].max_free == mid - l && tree[2 * u + 1].max_free == r - mid) {
      tree[u].max_free = r - l;
    } else {
      tree[u].max_free = std::max(tree[2 * u].max_free, tree[2 * u + 1].max_free);
    }
  }

  int malloc_tree(int u, int l, int r, int size) {
    if (tree[u].max_free < size) return -1;
    if (r - l == size) {
      tree[u].max_free = 0;
      tree[u].lazy = 1;
      return l;
    }
    push_down(u, l, r);
    int mid = l + (r - l) / 2;
    int res = -1;
    if (tree[2 * u].max_free >= size) {
      res = malloc_tree(2 * u, l, mid, size);
    } else {
      res = malloc_tree(2 * u + 1, mid, r, size);
    }
    push_up(u, l, r);
    return res;
  }

  bool malloc_at_tree(int u, int l, int r, int target_l, int target_r) {
    if (l == target_l && r == target_r) {
      if (tree[u].max_free == r - l) {
        tree[u].max_free = 0;
        tree[u].lazy = 1;
        return true;
      } else {
        return false;
      }
    }
    push_down(u, l, r);
    int mid = l + (r - l) / 2;
    bool res = false;
    if (target_r <= mid) {
      res = malloc_at_tree(2 * u, l, mid, target_l, target_r);
    } else if (target_l >= mid) {
      res = malloc_at_tree(2 * u + 1, mid, r, target_l, target_r);
    }
    push_up(u, l, r);
    return res;
  }

  void free_at_tree(int u, int l, int r, int target_l, int target_r) {
    if (l == target_l && r == target_r) {
      tree[u].max_free = r - l;
      tree[u].lazy = 2;
      return;
    }
    push_down(u, l, r);
    int mid = l + (r - l) / 2;
    if (target_r <= mid) {
      free_at_tree(2 * u, l, mid, target_l, target_r);
    } else if (target_l >= mid) {
      free_at_tree(2 * u + 1, mid, r, target_l, target_r);
    }
    push_up(u, l, r);
  }
};

} // namespace sjtu

#endif // SRC_HPP