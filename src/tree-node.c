#include "tree-node.h"

unsigned long countNodes(TreeNode *node) {
  if (!node) {
    return 0;
  }

  return 1 + countNodes(node->left) + countNodes(node->right);
}
