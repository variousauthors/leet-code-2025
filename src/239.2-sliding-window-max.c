#include <stdint.h>
#include <stdlib.h>

int *maxSlidingWindow(int *nums, int numsSize, int k, int *returnSize) {
  int *dq = (int *)malloc(sizeof(int) * numsSize);
  int *front = dq;
  int *back = dq;

  uint32_t size = 1 + (numsSize - k);

  *returnSize = size;
  int *result = (int *)malloc(sizeof(int) * size);

  int *outgoing = nums;
  int *incoming = nums + k;

  // initialize the window
  for (uint32_t i = 0; i < k; i++) {
    int el = *outgoing;

    while (front != back && *(back - 1) < el) {
      back--;
    }

    *back++ = el;
    outgoing++;
  }

  int max = *front;
  int *resultWrite = result;
  *resultWrite++ = max;

  outgoing = nums;

  for (uint32_t i = 1; i < size; i++) {
    // int outgoing = nums[i - 1];
    // int incoming = nums[i + k - 1];
    int max = *front;

    if (*outgoing == max) {
      front++;
    }

    // maintain the loop invariant
    while (front != back && *(back - 1) < *incoming) {
      back--;
    }

    *back++ = *incoming;
    *resultWrite++ = *front;
    incoming++;
    outgoing++;
  }

  return result;
}
