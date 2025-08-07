#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *serialize(struct TreeNode *root);
struct TreeNode *deserialize(char *data);
