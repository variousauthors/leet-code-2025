#include <stdlib.h>

/**
 * Return an array of arrays of size *returnSize.
 * The sizes of the arrays are returned as *returnColumnSizes array.
 * Note: Both returned array and *columnSizes array must be malloced, assume
 * caller calls free().
 */
int **subsets(int *nums, int numsSize, int *returnSize,
              int **returnColumnSizes) {
  // calculate number of ints in the final result
  int powerSetSize = 1 << numsSize;
  int intCount = numsSize * (1 << (numsSize - 1)) + 1;

  // pre-allocate all the necessary storage
  *returnSize = powerSetSize;
  int **result = malloc(sizeof(int *) * powerSetSize);
  int *storage = malloc(sizeof(int) * intCount);
  *returnColumnSizes = malloc(sizeof(int) * powerSetSize);

  (*returnColumnSizes)[0] = 0;

  int *s = &storage[1];

  // for each element in nums
  for (int z = 0; z < numsSize; z++) {
    int i = (1 << z);

    // we're going to copy the existing elements
    // into the new sets, _s will walk those
    int *_s = &storage[1];

    // each time we loop we add i sets
    // each new set is based on one of the previous sets
    for (int j = 0; j < i; j++) {
      // each result is a pointer into our int storage
      *(result + i + j) = s;

      // the size of the set is 1 more than the set it is based on
      (*returnColumnSizes)[i + j] = (*returnColumnSizes)[j] + 1;

      // first we copy in all the elements from
      // the set we had previously generated
      int size = (*returnColumnSizes)[i + j];
      for (int k = 1; k < size; k++) {
        *s++ = *_s++;
      }

      // then we add the new element
      int n = nums[z];
      *s++ = n;
    }
  }

  return result;
}