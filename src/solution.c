/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     struct TreeNode *left;
 *     struct TreeNode *right;
 * };
 */
/**
 * Note: The returned array must be malloced, assume caller calls free().
 */

#include "tree-node.h"
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct TreeNode TreeNode;
typedef TreeNode *ArrayElement;
TreeNode NULL_TREE_NODE = {INT_MAX, 0, 0}; // huge so that we never consider it
TreeNode MIN_NODE = {-INT_MAX, 0, 0};
ArrayElement NULL_ARRAY_ELEMENT = &NULL_TREE_NODE;

typedef struct Array {
  int cap;
  int len;
  ArrayElement *data;
} Array;

typedef struct ArrayInt {
  int cap;
  int len;
  int *data;
} ArrayInt;

#define INITIAL_CAP 1
#define GROWTH_FACTOR 1

void initArray(Array *array) {
  array->len = 0;
  array->cap = INITIAL_CAP;
  array->data = malloc(sizeof(ArrayElement) * array->cap);
}

void initArrayInt(ArrayInt *array) {
  array->len = 0;
  array->cap = INITIAL_CAP;
  array->data = malloc(sizeof(int) * array->cap);
}

void addToArray(Array *array, ArrayElement el) {
  if (array->len + 1 > array->cap) {
    array->cap <<= GROWTH_FACTOR; // double the cap
    array->data = realloc(array->data, sizeof(ArrayElement) * array->cap);
  }

  array->data[array->len++] = el;
}

void addToArrayInt(ArrayInt *array, int el) {
  if (array->len + 1 > array->cap) {
    array->cap <<= GROWTH_FACTOR; // double the cap
    array->data = realloc(array->data, sizeof(int) * array->cap);
  }

  array->data[array->len++] = el;
}

void push(Array *stack, ArrayElement el) { addToArray(stack, el); }

ArrayElement pop(Array *stack) {
  if (stack->len == 0) {
    return NULL;
  }

  return stack->data[--stack->len];
}

ArrayElement peek(Array *stack) {
  if (stack->len == 0) {
    return NULL_ARRAY_ELEMENT;
  }

  return stack->data[stack->len - 1];
}

bool isEmpty(Array *array) { return array->len == 0; }

int *getAllElements(TreeNode *root1, TreeNode *root2, int *returnSize) {
  Array stack1;
  initArray(&stack1);

  Array stack2;
  initArray(&stack2);

  ArrayInt result;
  initArrayInt(&result);

  // prime the stack
  push(&stack1, root1);
  push(&stack2, root2);

  TreeNode *current1;
  TreeNode *current2;
  TreeNode *next = &MIN_NODE;

  while (!isEmpty(&stack1) || !isEmpty(&stack2)) {
    current1 = pop(&stack1);

    while (current1) {
      push(&stack1, current1);

      if (current1->left && current1->left->val > next->val) {
        current1 = current1->left;
      } else {
        current1 = NULL;
      }
    }

    current2 = pop(&stack2);

    while (current2) {
      push(&stack2, current2);

      // we want smaller unexplored nodes
      if (current2->left && current2->left->val > next->val) {
        current2 = current2->left;
      } else {
        current2 = NULL;
      }
    }

    if (isEmpty(&stack1) && isEmpty(&stack2)) {
      break;
    }

    if (isEmpty(&stack1) || (peek(&stack1)->val > peek(&stack2)->val)) {
      next = pop(&stack2);
      addToArrayInt(&result, next->val);

      if (next->right) {
        push(&stack2, next->right);
      }
    } else if (isEmpty(&stack2) || (peek(&stack1)->val <= peek(&stack2)->val)) {
      next = pop(&stack1);
      addToArrayInt(&result, next->val);

      if (next->right) {
        push(&stack1, next->right);
      }
    }
  }

  *returnSize = result.len;

  return realloc(result.data, result.len * sizeof(int));
}