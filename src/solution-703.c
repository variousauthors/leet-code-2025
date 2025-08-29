
/*
the plan is to have K slots that are easy to insert into
and to discard the ones that are too big or too small
can we do this with a binary tree?

*/

#include <stdio.h>
#include <stdlib.h>
typedef struct TreeNodePreAlloc {
  unsigned int active : 1;
  int val;

  struct TreeNodePreAlloc *left;
  struct TreeNodePreAlloc *right;
} TreeNodePreAlloc;

typedef TreeNodePreAlloc TreeNode;

typedef struct {
  int k;
  int len;

  TreeNode *temp;
  TreeNode *root;
} KthLargest;

TreeNode *findParentForElement(TreeNode *node, int el) {
  fprintf(stderr, "findParentForElement for %d\n", el);
  if (!node || !node->active) {
    fprintf(stderr, "  return empty or inactive node\n");
    return node;
  }

  fprintf(stderr, "  considering %d\n", node->val);

  TreeNode *next = el <= node->val ? node->left : node->right;
  TreeNode *parent = findParentForElement(next, el);

  if (!parent || !parent->active) {
    fprintf(stderr, "  found empty or inactive\n");
    // if findNodeForElement didn't find anything then
    // this is the parent
    return node;
  } else {
    fprintf(stderr, "  found parent %d\n", parent->val);
    return parent;
  }
}

// we need to provide the pre-initialized shell
// and then node needs to be pointer to pointer so that
// we can set it to the shell
// this is so that tree in memory will be [a, b, c, d]
void addBinaryElement(TreeNode **node, int el, TreeNode *shell) {
  if (!(*node)) {
    (*node) = shell;
    (*node)->val = el;
    (*node)->left = 0;
    (*node)->right = 0;
    return;
  }

  if (el <= (*node)->val) {
    return addBinaryElement(&(*node)->left, el, shell);
  } else {
    return addBinaryElement(&(*node)->right, el, shell);
  }
}

void addElement(KthLargest *obj, int el) {
  if (!obj->root->active) {
    // we just add the element as the root
    obj->root->val = el;
    obj->root->active = 1;
    obj->root->left = 0;
    obj->root->right = 0;
  } else {
    obj->temp->val = el;
    obj->temp->active = 1;
    obj->temp->left = 0;
    obj->temp->right = 0;

    TreeNode *node = findParentForElement(obj->root, el);

    if (el <= node->val) {
      node->left = obj->temp;
    } else {
      node->right = obj->temp;
    }

    if (obj->len < obj->k) {
      obj->len++;
    } else {
      // findLeastNode
      // mark it "empty"
      // set scratch to that element
    }
  }
}

void printBinaryTree(TreeNode *node) {
  if (!node || !node->active) {
    return;
  }

  printBinaryTree(node->right);
  fprintf(stderr, "%d\n", node->val);
  printBinaryTree(node->left);
}

KthLargest *kthLargestCreate(int k, int *nums, int numsSize) {
  fprintf(stderr, "1\n");
  KthLargest *result = calloc(sizeof(KthLargest), 1);

  result->k = k;
  result->root = calloc(sizeof(TreeNode), k);
  result->temp = calloc(sizeof(TreeNode), 1);
  result->len = 0;

  fprintf(stderr, "2\n");
  addBinaryElement(&result->root, nums[0], result->root + 0);
  fprintf(stderr, "3\n");
  addBinaryElement(&result->root, nums[1], result->root + 1);
  fprintf(stderr, "4\n");
  // addElement(result, nums[2]);
  printBinaryTree(result->root);
  fprintf(stderr, "5\n");

  exit(1);
  // init by inserting the elements
  for (int i = 0; i < numsSize; i++) {
    addElement(result, nums[i]);
  }

  return result;
}

int kthLargestAdd(KthLargest *obj, int val) { return 0; }

void kthLargestFree(KthLargest *obj) {}

int main() {
  int nums[4] = {4, 5, 8, 2};
  kthLargestCreate(3, nums, 4);
}

/**
 * Your KthLargest struct will be instantiated and called as such:
 * KthLargest* obj = kthLargestCreate(k, nums, numsSize);
 * int param_1 = kthLargestAdd(obj, val);

 * kthLargestFree(obj);
*/