#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "tree-node.h"

// convert int16_t to string
char toHexDigitB(int16_t n) {
  switch (n) {
  case 0:
    return '0';
  case 1:
    return '1';
  case 2:
    return '2';
  case 3:
    return '3';
  case 4:
    return '4';
  case 5:
    return '5';
  case 6:
    return '6';
  case 7:
    return '7';
  case 8:
    return '8';
  case 9:
    return '9';
  case 10:
    return 'A';
  case 11:
    return 'B';
  case 12:
    return 'C';
  case 13:
    return 'D';
  case 14:
    return 'E';
  case 15:
    return 'F';
  default:
    return 'X';
  }
}

typedef struct TreeNode TreeNode;

typedef struct StringB {
  int len;
  int cap;
  char *end; // null terminator
  char *data;
} StringB;

void initStringB(StringB *str, unsigned long initialCapacity) {
  str->len = 0;
  str->cap = initialCapacity;
  str->data = malloc(str->cap);
  str->data[str->len] = '\0';
  str->end = str->data + str->len;
}

void appendB(StringB *dest, char *source) {
  int len = strlen(source);

  dest->data[dest->len + len] = '\0';
  dest->len += len;
  dest->end += len;
}

void appendCharB(StringB *dest, char c) {
  int len = 1;

  dest->data[dest->len] = c;

  dest->data[dest->len + len] = '\0';
  dest->len += len;
  dest->end += len;
}

void appendNumberB(StringB *buffer, int n) {
  appendCharB(buffer, toHexDigitB((n & 0xF00) >> 8));
  appendCharB(buffer, toHexDigitB((n & 0x0F0) >> 4));
  appendCharB(buffer, toHexDigitB((n & 0x00F) >> 0));
}

int toDigitB(char h) {
  switch (h) {
  case '0':
    return 0;
  case '1':
    return 1;
  case '2':
    return 2;
  case '3':
    return 3;
  case '4':
    return 4;
  case '5':
    return 5;
  case '6':
    return 6;
  case '7':
    return 7;
  case '8':
    return 8;
  case '9':
    return 9;
  case 'A':
    return 10;
  case 'B':
    return 11;
  case 'C':
    return 12;
  case 'D':
    return 13;
  case 'E':
    return 14;
  case 'F':
    return 15;
  default:
    return -1;
  }
}

typedef enum TOKEN_TYPE_B {
  T_NONE_B,
  T_LEFT_PAREN_B,
  T_RIGHT_PAREN_B,
  T_NUMBER_B,
  T_DOT_B,
  T_START_B,
} TOKEN_TYPE_B;

typedef struct TokenizerB {
  char *next;
  char *current;

  // current token
  int16_t value;
  TOKEN_TYPE_B token;
} TokenizerB;

TOKEN_TYPE_B parseTokenB(TokenizerB *tokenizer);

void initTokenizerB(TokenizerB *tokenizer, char *data) {
  tokenizer->next = data;
  tokenizer->current = data;
  tokenizer->value = 0;
  // prime the tokenizer
  parseTokenB(tokenizer);
}

char getNextB(TokenizerB *tokenizer) { return *tokenizer->next++; }

void commitB(TokenizerB *tokenizer) { tokenizer->current = tokenizer->next; }

typedef TreeNode *ArrayElementB;

typedef struct ArrayB {
  int cap;
  int len;
  ArrayElementB *data;
} ArrayB;

#define ARRAY_INITIAL_CAP_B (64 / sizeof(ArrayElementB));
#define ARRAY_GROWTH_FACTOR_B 2;

void initArrayB(ArrayB *array, unsigned long size) {
  array->cap = size;
  array->len = 0;
  array->data = malloc(sizeof(ArrayElementB) * array->cap);
}

void addToArrayB(ArrayB *array, ArrayElementB el) {
  array->data[array->len] = el;
  array->len++;
}

ArrayElementB popB(ArrayB *stack) {
  ArrayElementB result = stack->data[stack->len - 1];

  stack->len--;

  return result;
}

TOKEN_TYPE_B parseTokenB(TokenizerB *tokenizer) {
  char c = getNextB(tokenizer);

  switch (c) {
  case '(': {
    tokenizer->token = T_LEFT_PAREN_B;
    break;
  }
  case ')': {
    tokenizer->token = T_RIGHT_PAREN_B;
    break;
  }
  case '.': {
    tokenizer->token = T_DOT_B;
    break;
  }
  case '0' ... '9':
  case 'A' ... 'F': {
    int16_t raw = toDigitB(c);

    c = getNextB(tokenizer);
    raw <<= 4;
    raw += toDigitB(c);

    c = getNextB(tokenizer);
    raw <<= 4;
    raw += toDigitB(c);

    // sign exted
    if (raw & 0x800) {
      tokenizer->value = raw | 0xF000;
    } else {
      tokenizer->value = raw & 0x0FFF;
    }

    tokenizer->token = T_NUMBER_B;
    break;
  }
  default: {
    tokenizer->token = T_NONE_B;
    break;
  }
  }

  return tokenizer->token;
}

void consumeB(TokenizerB *tok, TOKEN_TYPE_B expected) {
  commitB(tok);
  parseTokenB(tok);
}

TreeNode *nodes;
int freeNodeB = 0;

/** Encodes a tree to a single string. */
char *serializePreAlloc(struct TreeNode *root) {
  StringB buffer;
  initStringB(&buffer, 3789037);

  ArrayB stack;
  initArrayB(&stack, 3789037);

  addToArrayB(&stack, root);
  TreeNode *node;

  // 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C
  // 1, 2, 3, n, n, 4, 5, n, n, n, n, 6, 7
  // 3
  // (1(2

  while (stack.len > 0) {
    node = popB(&stack);

    if (!node) {
      appendCharB(&buffer, '.');
      continue;
    }

    appendNumberB(&buffer, node->val);

    addToArrayB(&stack, node->right);
    addToArrayB(&stack, node->left);
  }

  return realloc(buffer.data, buffer.len + 1);
}

#define MAX_NODES 20000