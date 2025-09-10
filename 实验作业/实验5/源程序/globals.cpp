#include "globals.h"

// 真正的定义（去掉 extern），并给出初始值
FILE* source   = nullptr;
FILE* listing  = nullptr;
FILE* code     = nullptr;

int lineno     = 1;

QString debugMsg;
