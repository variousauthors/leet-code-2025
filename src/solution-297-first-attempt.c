/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     struct TreeNode *left;
 *     struct TreeNode *right;
 * };
 */

typedef struct Buffer {
  int cap;
  int len;
  char *data;
} Buffer;

#define INITIAL_CAP 3
#define GROWTH_FACTOR 1
#define META_DATA 4

void initBuffer(Buffer *buffer) {
  // we reserve several bytes at the start for the length
  // we know the max length is 10000 so we need 4 hex digits FFFF
  buffer->cap = INITIAL_CAP;
  buffer->len = 0;
  buffer->data = malloc(sizeof(char) * buffer->cap + 1 + META_DATA);
  buffer->data[0] = 'F';
  buffer->data[1] = 'F';
  buffer->data[2] = 'F';
  buffer->data[3] = 'F';
  buffer->data[META_DATA + buffer->len] = '\0';
}

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

// values are in -1000 to 1000 so we always write either FFF or NUL
void writeToBuffer(Buffer *buffer, int16_t n) {
  if (buffer->len + 3 > buffer->cap) {
    buffer->cap <<= GROWTH_FACTOR;
    buffer->data =
        realloc(buffer->data, sizeof(char) * buffer->cap + 1 + META_DATA);
  }

  buffer->data[META_DATA + buffer->len++] = toHexDigit((n & 0xF00) >> 8);
  buffer->data[META_DATA + buffer->len++] = toHexDigit((n & 0x0F0) >> 4);
  buffer->data[META_DATA + buffer->len++] = toHexDigit((n & 0x00F));
  buffer->data[META_DATA + buffer->len] = '\0';
}

void writeNULLToBuffer(Buffer *buffer) {
  if (buffer->len + 3 > buffer->cap) {
    buffer->cap <<= GROWTH_FACTOR;
    buffer->data =
        realloc(buffer->data, sizeof(char) * buffer->cap + 1 + META_DATA);
  }

  buffer->data[META_DATA + buffer->len++] = 'N';
  buffer->data[META_DATA + buffer->len++] = 'N';
  buffer->data[META_DATA + buffer->len++] = 'N';
  buffer->data[META_DATA + buffer->len] = '\0';
}

void overwriteMetadataToBuffer(Buffer *buffer, int16_t n) {
  buffer->data[0] = toHexDigit((n & 0xF000) >> 12);
  buffer->data[1] = toHexDigit((n & 0x0F00) >> 8);
  buffer->data[2] = toHexDigit((n & 0x00F0) >> 4);
  buffer->data[3] = toHexDigit((n & 0x000F));
}

typedef struct TreeNode TreeNode;

int countNodes(TreeNode *root) {
  if (!root) {
    return 0;
  }

  return 1 + countNodes(root->left) + countNodes(root->right);
}

typedef TreeNode *Element;

typedef struct RingBuffer {
  Element *end;
  Element *read;
  Element *write;
  Element *data;
} RingBuffer;

void initRingBuffer(RingBuffer *buffer, int count) {
  buffer->data = malloc(sizeof(Element) * (count + 1));
  buffer->read = buffer->data;
  buffer->write = buffer->data;
  buffer->end = buffer->data + count;
}

bool isFull(RingBuffer *buffer) {
  return (buffer->write + 1) == (buffer->read) ||
         buffer->write == (buffer->end) && (buffer->read == buffer->data);
}

bool isEmpty(RingBuffer *buffer) { return buffer->read == buffer->write; }

void advancePointer(RingBuffer *buffer, Element **ptr) {
  if ((*ptr) == buffer->end) {
    (*ptr) = buffer->data;
  } else {
    (*ptr)++;
  }
}

Element readFromRingBuffer(RingBuffer *buffer) {
  if (isEmpty(buffer)) {
    return NULL;
  }

  Element result = *buffer->read;
  advancePointer(buffer, &buffer->read);

  return result;
}

int writeToRingBuffer(RingBuffer *buffer, Element node) {
  if (isFull(buffer)) {
    return 0;
  }

  *buffer->write = node;
  advancePointer(buffer, &buffer->write);

  return 1;
}

/** Encodes a tree to a single string. */
char *serializeIterative(struct TreeNode *root) {
  RingBuffer ringBuffer;
  initRingBuffer(&ringBuffer, 80000);

  int len = 0;

  writeToRingBuffer(&ringBuffer, root);

  Buffer buffer;
  initBuffer(&buffer);
  TreeNode *node, *left, *right;
  int layerMax = 1;
  int nonNullNodes;
  int count = 0;

  while (!isEmpty(&ringBuffer)) {
    nonNullNodes = 0;

    // do a whole layer
    for (int i = 0; i < layerMax; i++) {
      node = readFromRingBuffer(&ringBuffer);

      if (node) {
        nonNullNodes++;
        count++;
        writeToBuffer(&buffer, node->val);
        writeToRingBuffer(&ringBuffer, node->left);
        writeToRingBuffer(&ringBuffer, node->right);
      } else {
        writeNULLToBuffer(&buffer);
        writeToRingBuffer(&ringBuffer, NULL);
        writeToRingBuffer(&ringBuffer, NULL);
      }
    }

    if (nonNullNodes == 0) {
      break;
    }

    layerMax <<= 1;
  }

  overwriteMetadataToBuffer(&buffer, count);
  // get rid of the last layer, it is all NULL
  buffer.len -= layerMax;
  buffer.data = realloc(buffer.data, sizeof(char) * buffer.len + 1 + META_DATA);
  buffer.data[META_DATA + buffer.len] = '\0';

  return buffer.data;
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

#define NULL_VALUE (-1001)
#define OFFSET META_DATA

int16_t readValue(char *data, int len, int i) {
  if ((OFFSET + i * 3) > (len - 1) || data[OFFSET + i * 3] == 'N') {
    return NULL_VALUE; // a number out of the range
  }

  uint16_t raw = (toDigit(data[OFFSET + i * 3]) << 8) +
                 (toDigit(data[OFFSET + i * 3 + 1]) << 4) +
                 toDigit(data[OFFSET + i * 3 + 2]);

  // sign exted
  if (raw & 0x800) {
    return raw | 0xF000;
  } else {
    return raw & 0x0FFF;
  }
}

/** Decodes your encoded data to tree. */
struct TreeNode *deserialize(char *data) {
  int16_t count = readCount(data);
  int len = strlen(data);

  if (count == 0) {
    return 0;
  }

  TreeNode *nodes = calloc(count, sizeof(TreeNode));

  int reserve = 1;
  int nodeIndex = 0;
  int i = 0;
  int left;
  int right;

  while (count > 0) {
    int16_t value = readValue(data, len, i);

    //          v
    // i: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C
    // v: 1, 2, 3, n, n, 4, 5, n, n, n, n, 6, 7
    // n: 1, 2, 3, c, c,
    //          ^
    // r:                ^

    if (value != NULL_VALUE) {
      count--;
      nodes[nodeIndex].val = value;

      // reach forward and try to connect
      left = readValue(data, len, i * 2 + 1);
      right = readValue(data, len, i * 2 + 2);

      // reserve the nodes
      if (left != NULL_VALUE) {
        nodes[nodeIndex].left = &nodes[reserve++];
      }

      if (right != NULL_VALUE) {
        nodes[nodeIndex].right = &nodes[reserve++];
      }

      nodeIndex++;
    }

    i++;
  }

  count = readCount(data);

  return nodes;
}

// Your functions will be called as such:
// char* data = serialize(root);
// deserialize(data);