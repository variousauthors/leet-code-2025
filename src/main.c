#include "global.c"
#include "global.h"
#include "os_metrics.c"
#include "os_metrics.h"
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
#define INPUT_SIZE (5234567)

int main() {
  int *inputs = malloc(INPUT_SIZE * sizeof(int));

  for (int i = 0; i < INPUT_SIZE; i++) {
    inputs[i] = 99;
  }

  perf = 1;
  perfChannel = stderr;
  verboseChannel = stderr;

  TreeNode *tree1 = calloc(INPUT_SIZE, sizeof(TreeNode));

  for (int i = 0; i < INPUT_SIZE; i++) {
    tree1[i].val = inputs[i];

    if ((i * 2 + 2) < INPUT_SIZE) {
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

  TreeNode *root1 = &tree1[0];
  unsigned long nodes = countNodes(root1);
  fprintf(stderr, "len: %ld, bytes: %ld\n", nodes, nodes * sizeof(TreeNode));

  beginProfiler();

  char *s1 = serialize(root1);
  fprintf(stderr, "s1 len: %ld\n", strlen(s1));

  // root1 = deserialize(s1);

  endAndPrintProfiler();

  // char *s2 = serialize(root1);

  return 0;
}
