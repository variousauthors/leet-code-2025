
#include <stdio.h>
#include <stdlib.h>
typedef struct DQ {
  int capacity;
  int count;

  int *left;
  int *endLeft;

  int *right;
  int *endRight;

  int *data;
} DQ;

// allocates storage for size elements
void initDQ(DQ *dq, int size) {
  dq->data = calloc(sizeof(int), size + 2);

  dq->left = dq->data + 1;
  dq->endLeft = dq->data;

  dq->right = dq->data + 1;
  dq->endRight = dq->data + size + 1;

  dq->count = 0;
  dq->capacity = size;
}

int isFull(DQ *dq) { return dq->count == dq->capacity; }

int isEmpty(DQ *dq) { return dq->count == 0; }

void pushRightDQ(DQ *dq, int el) {
  *dq->right = el;
  dq->right++;

  if (dq->right == dq->endRight) {
    dq->right = dq->endLeft + 1;
  }

  dq->count++;
}

void pushLeftDQ(DQ *dq, int el) {
  dq->left--;

  if (dq->left == dq->endLeft) {
    dq->left = dq->endRight - 1;
  }

  *dq->left = el;

  dq->count++;
}

int popRightDQ(DQ *dq) {
  dq->right--;

  if (dq->right == dq->endLeft) {
    dq->right = dq->endRight - 1;
  }

  int el = *dq->right;
  dq->count--;

  return el;
}

int popLeftDQ(DQ *dq) {
  int el = *dq->left;

  dq->left++;

  if (dq->left == dq->endRight) {
    dq->left = dq->endLeft + 1;
  }

  dq->count--;

  return el;
}

int peekLeftDQ(DQ *dq) { return *dq->left; }

int peekRightDQ(DQ *dq) {
  return *(dq->right - 1 == dq->endLeft ? dq->endRight - 1 : dq->right - 1);
}

void debugDQ(DQ *dq) {
  fprintf(stderr, "left: %ld, right: %ld\n", dq->left - dq->data - 1,
          dq->right - dq->data - 1);

  for (int i = 0; i < dq->capacity; i++) {
    fprintf(stderr, "%d, ", *(dq->data + 1 + i));
  }

  fprintf(stderr, "\n");
}

int *maxSlidingWindow(int *nums, int numsSize, int k, int *returnSize) {
  DQ dq;
  initDQ(&dq, k);
  *returnSize = 1 + (numsSize - k);
  int *result = malloc(sizeof(int) * (*returnSize));

  // initialize the window
  for (int i = 0; i < k; i++) {
    int el = nums[i];
    while (!isEmpty(&dq) && peekRightDQ(&dq) < el) {
      popRightDQ(&dq);
    }

    pushRightDQ(&dq, el);
  }

  int max = peekLeftDQ(&dq);
  result[0] = max;

  for (int i = 1; i < (*returnSize); i++) {
    int outgoing = nums[i - 1];
    int incoming = nums[i + k - 1];
    int max = peekLeftDQ(&dq);

    if (outgoing == max) {
      popLeftDQ(&dq);
    }

    // maintain the loop invariant
    while (!isEmpty(&dq) && peekRightDQ(&dq) < incoming) {
      popRightDQ(&dq);
    }

    pushRightDQ(&dq, incoming);

    result[i] = peekLeftDQ(&dq);
  }

  return result;
}