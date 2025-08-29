#ifndef SOLUTION_297_PARSER
#define SOLUTION_297_PARSER

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *serializeRecursive(struct TreeNode *root);
char *serializeIterative(struct TreeNode *root);
struct TreeNode *deserialize(char *data);

#endif