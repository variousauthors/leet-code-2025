#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int indexToPrime(int i) {
  switch (i) {
  case 0:
    return 2;
  case 1:
    return 3;
  case 2:
    return 5;
  case 3:
    return 7;
  case 4:
    return 11;
  case 5:
    return 13;
  case 6:
    return 17;
  case 7:
    return 19;
  case 8:
    return 23;
  case 9:
    return 29;
  case 10:
    return 31;
  case 11:
    return 37;
  case 12:
    return 41;
  case 13:
    return 43;
  case 14:
    return 47;
  case 15:
    return 53;
  case 16:
    return 59;
  case 17:
    return 61;
  case 18:
    return 67;
  case 19:
    return 71;
  case 20:
    return 73;
  case 21:
    return 79;
  case 22:
    return 83;
  case 23:
    return 89;
  case 24:
    return 97;
  case 25:
    return 101;
  case 26:
    return 103;
  case 27:
    return 107;
  case 28:
    return 109;
  case 29:
    return 113;
  default:
    return -1; // not a valid index
  }
}

int primeToIndex(int n) {
  switch (n) {
  case 2:
    return 0;
  case 3:
    return 1;
  case 5:
    return 2;
  case 7:
    return 3;
  case 11:
    return 4;
  case 13:
    return 5;
  case 17:
    return 6;
  case 19:
    return 7;
  case 23:
    return 8;
  case 29:
    return 9;
  case 31:
    return 10;
  case 37:
    return 11;
  case 41:
    return 12;
  case 43:
    return 13;
  case 47:
    return 14;
  case 53:
    return 15;
  case 59:
    return 16;
  case 61:
    return 17;
  case 67:
    return 18;
  case 71:
    return 19;
  case 73:
    return 20;
  case 79:
    return 21;
  case 83:
    return 22;
  case 89:
    return 23;
  case 97:
    return 24;
  case 101:
    return 25;
  case 103:
    return 26;
  case 107:
    return 27;
  case 109:
    return 28;
  case 113:
    return 29;
  default:
    return -1; // not in first 30 primes
  }
}

int primesCount = 30;
static const int primes[] = {2,  3,  5,  7,  11, 13,  17,  19,  23,  29,
                             31, 37, 41, 43, 47, 53,  59,  61,  67,  71,
                             73, 79, 83, 89, 97, 101, 103, 107, 109, 113};

// intermediate representation
typedef struct IR {
  int children;
  uint16_t value;
  unsigned __int128 index;
  uint16_t returnColumnSize;
  int *path;
} IR;

// OK new plan: we allocate a big array of ints
// and we store the path of a depth-first traversal
// the idea being that if the path is
// 2, 2, 3, 2
// ^
// we can store a pointer into the path storage and a length
// we don't have to copy the values into the objects as we
// make new ones, just copy the pointer to the root of the path
// and increment the length
// how do we do depth first again?

// 7, 6, 3, 3
// {0}, {2}, {2, 2}, {}
// 2, 2, 2

int **combinationSum(int *candidates, int candidatesSize, int target,
                     int *returnSize, int **returnColumnSizes) {
  // a stack of indexes into candidates
  int *stack = malloc((4096u * 128) / sizeof(int));
  int *top = stack;

  // a stack of IRs
  IR *stackIR = malloc((4096u * 256) / sizeof(IR));
  IR *topIR = stackIR;

  // a place to store the good combinations
  IR *result = malloc((4096u * 256) / sizeof(IR));
  IR *next = result;

  int count = 0; // number of results

  // what paths will point to
  int *store = malloc(4096u * 256 * sizeof(int));
  int *nextStore = store;

  for (int i = candidatesSize - 1; i >= 0; i--) {
    *top++ = i;
  }

  *topIR++ = (IR){INT_MAX, 0, 1, 0, nextStore};

  while (top != stack) {
    int currentIndex = *(--top);
    // printf("considering %d\n", candidates[currentIndex]);
    IR *previous = topIR - 1;

    // if we have no children left to explore, pop it
    if (--(previous->children) < 1) {
      topIR--;
    }

    IR work = (IR){
        0,
        previous->value + candidates[currentIndex],
        previous->index * indexToPrime(currentIndex),
        previous->returnColumnSize + 1,
        nextStore,
    };

    // copy the previous elements
    memcpy(nextStore, previous->path, previous->returnColumnSize * sizeof(int));
    nextStore += previous->returnColumnSize;

    // for (int i = 0; i < previous->returnColumnSize; i++) {
    //     *nextStore++ = *(previous->path + i);
    // }

    // copy the new element
    *nextStore++ = candidates[currentIndex];

    // printf("new work - val: %d, len: %d, path: ", work.value,
    // work.returnColumnSize, work.index); for (int j = 0; j <
    // work.returnColumnSize; j++) {
    //     printf("%d, ", *(work.path + j));
    // }
    // printf("\n");

    if (work.value == target) {
      int found = 0;

      // check against existing results
      for (int i = 0; i < count; i++) {
        // indexes are a product of primes
        if (result[i].index == work.index) {
          found = 1;
          break;
        }
      }

      if (!found) {
        // printf("commiting - val: %d, len: %d, index: %d, path: ", work.value,
        // work.returnColumnSize, work.index); for (int j = 0; j <
        // work.returnColumnSize; j++) {
        //     printf("%d, ", *(work.path + j));
        // }
        // printf("\n");
        // add to result set
        *next++ = work; // (IR){work.children, work.value, work.index,
                        // work.returnColumnSize, work.path};
        count++;
      }
    } else {
      int children = 0;

      // then push more candidates
      for (int i = candidatesSize - 1; i >= 0; i--) {
        if (candidates[i] + work.value <= target) {
          *top++ = i;
          children++;
        }
      }

      // if we were able to push any work then we know
      // we can use this value in the next iteration
      if (children > 0) {
        work.children = children;
        *topIR++ = work;
      }
    }
  }

  // debug
  // for (int i = 0; i < count; i++) {
  //     printf("result %d - val: %d, len: %d, index: %d, path: ", i,
  //     result[i].value, result[i].returnColumnSize, result[i].index); for (int
  //     j = 0; j < result[i].returnColumnSize; j++) {
  //         printf("%d, ", *(result[i].path + j));
  //     }
  //     printf("\n");
  // }

  int sum = 0;
  for (int i = 0; i < count; i++) {
    sum += result[i].returnColumnSize;
  }

  *returnColumnSizes = calloc(count, sizeof(int));
  int *retColSizes = *returnColumnSizes;
  *returnSize = count;
  int **resultArray = malloc(count * sizeof(int *));
  int **pResultArray = resultArray;

  // iterate over results
  // copying the pointers
  for (int i = 0; i < count; i++) {
    *pResultArray++ = result[i].path;
    *retColSizes++ = result[i].returnColumnSize;
  }

  /*
  int *resultStorage = malloc(sum * sizeof(int));

  // convert solutions into the result shape
  for (int i = 0; i < count; i++) {
      *pResultArray++ = resultStorage;

      for (int j = 0; j < primesCount; j++) {
          while (result[i].index % primes[j] == 0) {
              (*retColSizes)++;
              *resultStorage++ = candidates[primeToIndex(primes[j])];
              result[i].index /= primes[j];
          }
      }
      retColSizes++;
  }
  */

  return resultArray;
}