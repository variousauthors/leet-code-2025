#include "global.c"
#include "global.h"
#include "os_metrics.c"
#include "os_metrics.h"
#include "repetition_tester.c"
#include "repetition_tester.h"
#include "solution-297-parser.c"
#include "solution-297-parser.h"
#include "tree-node.h"
#include <stdio.h>

unsigned long countNodes(TreeNode *node) {
  if (!node) {
    return 0;
  }

  return 1 + countNodes(node->left) + countNodes(node->right);
}

#define null (-1e6)
#define INPUT_SIZE (3789038)

TreeNode *getBigInputs() {
  int inputSize = INPUT_SIZE;
  int *inputs = malloc(INPUT_SIZE * sizeof(int));

  for (int i = 0; i < INPUT_SIZE; i++) {
    inputs[i] = 99;
  }

  TreeNode *tree1 = calloc(inputSize, sizeof(TreeNode));

  for (int i = 0; i < inputSize; i++) {
    tree1[i].val = inputs[i];

    if ((i * 2 + 2) < inputSize) {
      if (inputs[i * 2 + 1] != null) {
        tree1[i].left = &tree1[i * 2 + 1];
      }

      if (inputs[i * 2 + 2] != null) {
        tree1[i].right = &tree1[i * 2 + 2];
      }
    } else {
      tree1[i].left = NULL;
      tree1[i].right = NULL;
    }
  }

  return &tree1[0];
}

TreeNode *getSmallInputs() {
  int inputSize = 13;
  int *inputs = malloc(13 * sizeof(int));

  inputs[0] = 1;
  inputs[1] = 2;
  inputs[2] = 3;
  inputs[3] = null;
  inputs[4] = null;
  inputs[5] = 4;
  inputs[6] = 5;
  inputs[7] = null;
  inputs[8] = null;
  inputs[9] = null;
  inputs[10] = null;
  inputs[11] = 6;
  inputs[12] = 7;

  TreeNode *tree1 = calloc(inputSize, sizeof(TreeNode));

  for (int i = 0; i < inputSize; i++) {
    tree1[i].val = inputs[i];

    if ((i * 2 + 2) < inputSize) {
      if (inputs[i * 2 + 1] != null) {
        tree1[i].left = &tree1[i * 2 + 1];
      }

      if (inputs[i * 2 + 2] != null) {
        tree1[i].right = &tree1[i * 2 + 2];
      }
    } else {
      tree1[i].left = NULL;
      tree1[i].right = NULL;
    }
  }

  return &tree1[0];
}

TreeNode *root1;

void serializeIterative() { serialize(root1); }
void serializeRecursive() { serializeOld(root1); }

void NO_SETUP() {}
void NO_TEARDOWN() {}

int main() {
  perf = 1;
  perfChannel = stderr;
  verboseChannel = stderr;

  perf = 1;

  if (!perf) {
    root1 = getSmallInputs();
    char *s1 = serializeOld(root1);
    char *s2 = serialize(root1);

    fprintf(stderr, "s1 %s\n", s1);
    fprintf(stderr, "s2 %s\n", s2);

    TreeNode *n1 = deserialize(s1);
    TreeNode *n2 = deserialize(s2);

    char *s3 = serializeOld(root1);
    char *s4 = serialize(root1);

    fprintf(stderr, "s3 %s\n", s3);
    fprintf(stderr, "s4 %s\n", s4);

    return 0;
  } else {
    root1 = getBigInputs();
    unsigned long nodes = countNodes(root1);
    fprintf(verboseChannel, "len: %ld, bytes: %ld\n", nodes,
            nodes * sizeof(TreeNode));

    SubectUnderTestRepetitionTester *serializeRecursiveTest =
        initSubectUnderTestRepetitionTester("serialize - recursive",
                                            serializeRecursive, NO_SETUP,
                                            NO_TEARDOWN, 90936888, 2);

    SubectUnderTestRepetitionTester *serializeIterativeTest =
        initSubectUnderTestRepetitionTester("serialize - iterative",
                                            serializeIterative, NO_SETUP,
                                            NO_TEARDOWN, 90936888, 2);

    subjects[0] = serializeRecursiveTest;
    subjects[1] = serializeIterativeTest;

    runRepetitionTester();

    printResultsRepetitiontester();

    return 0;
  }
}
