#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "tree-node.h"

// convert int16_t to string
char toHexDigit(int16_t n) {
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

typedef struct String {
  int len;
  int cap;
  char *end; // null terminator
  char *data;
} String;

#define INITIAL_STRING_CAP 80
#define STRING_GROWTH_FACTOR 2

void initString(String *str) {
  str->len = 0;
  str->cap = INITIAL_STRING_CAP;
  str->data = malloc(str->cap);
  str->data[str->len] = '\0';
  str->end = str->data + str->len;
}

void append(String *dest, char *source) {
  int len = strlen(source);

  if (dest->len + len + 1 > dest->cap) {
    // grow string
    dest->cap = ((dest->cap - 1 + len) << (STRING_GROWTH_FACTOR - 1)) + 1;
    dest->data = realloc(dest->data, sizeof(char) * dest->cap);
  }

  for (int i = 0; i < len; i++) {
    dest->data[dest->len + i] = source[i];
  }

  dest->data[dest->len + len] = '\0';
  dest->len += len;
  dest->end += len;
}

void appendChar(String *dest, char c) {
  int len = 1;

  if (dest->len + len + 1 > dest->cap) {
    // grow string
    dest->cap = ((dest->cap - 1 + len) << (STRING_GROWTH_FACTOR - 1)) + 1;
    dest->data = realloc(dest->data, sizeof(char) * dest->cap);
  }

  dest->data[dest->len] = c;

  dest->data[dest->len + len] = '\0';
  dest->len += len;
  dest->end += len;
}

void appendNumber(String *buffer, int n) {
  appendChar(buffer, toHexDigit((n & 0xF00) >> 8));
  appendChar(buffer, toHexDigit((n & 0x0F0) >> 4));
  appendChar(buffer, toHexDigit((n & 0x00F) >> 0));
}

void toString(String *buffer, TreeNode *node) {
  if (!node) {
    append(buffer, ".");
    return;
  }

  append(buffer, "(");
  appendNumber(buffer, node->val);

  toString(buffer, node->left);
  toString(buffer, node->right);

  append(buffer, ")");
}

int toDigit(char h) {
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

int16_t readCount(char *data) {
  return (toDigit(data[0]) << 12) + (toDigit(data[1]) << 8) +
         (toDigit(data[2]) << 4) + toDigit(data[3]);
}

void emitChar(char c) { fprintf(stderr, "%c", c); }

void emitNumber(int n) { fprintf(stderr, "%d", n); }

typedef enum TOKEN_TYPE {
  T_NONE,
  T_LEFT_PAREN,
  T_RIGHT_PAREN,
  T_NUMBER,
  T_DOT,
} TOKEN_TYPE;

typedef struct Tokenizer {
  char *next;
  char *current;

  // current token
  int16_t value;
  TOKEN_TYPE token;
} Tokenizer;

TOKEN_TYPE parseToken(Tokenizer *tokenizer);

void initTokenizer(Tokenizer *tokenizer, char *data) {
  tokenizer->next = data;
  tokenizer->current = data;
  tokenizer->value = 0;
  // prime the tokenizer
  parseToken(tokenizer);
}

char getNext(Tokenizer *tokenizer) { return *tokenizer->next++; }

void commit(Tokenizer *tokenizer) { tokenizer->current = tokenizer->next; }

void rollback(Tokenizer *tokenizer) { tokenizer->next = tokenizer->current; }

typedef TreeNode *ArrayElement;

typedef struct Array {
  int cap;
  int len;
  ArrayElement *data;
} Array;

#define ARRAY_INITIAL_CAP 1;
#define ARRAY_GROWTH_FACTOR 2;

void initArray(Array *array) {
  array->cap = ARRAY_INITIAL_CAP;
  array->len = 0;
  array->data = malloc(sizeof(ArrayElement) * array->cap);
}

void addToArray(Array *array, ArrayElement el) {
  if (array->len + 1 > array->cap) {
    // grow array
    array->cap <<= ARRAY_GROWTH_FACTOR;
    array->data = realloc(array, sizeof(ArrayElement) * array->cap);
  }

  array->data[array->len] = el;
  array->len++;
}

TOKEN_TYPE parseToken(Tokenizer *tokenizer) {
  char c = getNext(tokenizer);

  switch (c) {
  case '(': {
    tokenizer->token = T_LEFT_PAREN;
    break;
  }
  case ')': {
    tokenizer->token = T_RIGHT_PAREN;
    break;
  }
  case '.': {
    tokenizer->token = T_DOT;
    break;
  }
  case '0' ... '9':
  case 'A' ... 'F': {
    int16_t raw = toDigit(c);

    while (isalnum(c = getNext(tokenizer))) {
      raw <<= 4;
      raw += toDigit(c);
    }

    // the last char we got was not digit
    tokenizer->next--;

    // sign exted
    if (raw & 0x800) {
      tokenizer->value = raw | 0xF000;
    } else {
      tokenizer->value = raw & 0x0FFF;
    }

    tokenizer->token = T_NUMBER;
    break;
  }
  default: {
    tokenizer->token = T_NONE;
    break;
  }
  }

  return tokenizer->token;
}

void consume(Tokenizer *tok, TOKEN_TYPE expected) {
  commit(tok);
  parseToken(tok);
}

TreeNode *nodes;
int freeNode = 0;

TreeNode *treeNode(Tokenizer *tok) {
  if (tok->token == T_DOT) {
    consume(tok, T_DOT);
    return NULL;
  }

  consume(tok, T_LEFT_PAREN);

  int index = freeNode++;

  consume(tok, T_NUMBER);
  nodes[index].val = tok->value;
  nodes[index].left = treeNode(tok);
  nodes[index].right = treeNode(tok);

  consume(tok, T_RIGHT_PAREN);
  return &nodes[index];
}

/** Encodes a tree to a single string. */
char *serialize(struct TreeNode *root) {
  TimeBandwidth(127457256);
  String buffer;
  initString(&buffer);

  toString(&buffer, root);

  return realloc(buffer.data, buffer.len + 1);
}

#define MAX_NODES 20000

/** Decodes your encoded data to tree. */
struct TreeNode *deserialize(char *data) {
  TimeBandwidth(31864315);
  nodes = calloc(MAX_NODES, sizeof(TreeNode));

  TreeNode *result;
  Tokenizer tokenizer;
  initTokenizer(&tokenizer, data);

  return treeNode(&tokenizer);
}