#pragma once
#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <QString>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 18

typedef enum
/* book-keeping tokens */
{
    ENDFILE, ERROR,
    /* reserved words */
    // 新增WHILE和ENDWHILE，修改IF删除THEN
    IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE, FOR, WHILE, ENDWHILE, TO, DOWNTO, ENDDO, DO ,
    /* multicharacter tokens */
    ID, NUM,
    /* special symbols */
    ASSIGN, EQ, LT, PLUS, MINUS, TIMES, OVER, LPAREN, RPAREN, SEMI, PLUSEQ,
    // 新增求余、乘方运算
    POWER, MOD,
    // 新增自增++和自减--
    PLUSPLUS, SUBSUB,
    /* 扩充比较运算符号 */
    RT, LTEQ, RTEQ, NOTEQ,
    AND, OR, NOT,
    /* 正则表达式符号 */
    REGEX,RGOR,RGAND,RGCLOSE,RGCHOOSE,
    //中括号，用于if语句、while语句和for语句
    LPM,RPM,
} TokenType;

inline FILE* source; /* source code text file */
inline FILE* listing; /* listing output text file */
extern FILE* code; /* code text file for TM simulator */

inline int lineno; /* source line number for listing */

inline QString debugMsg;

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum { StmtK, ExpK } NodeKind;
typedef enum { IfK, RepeatK, AssignK, ReadK, WriteK, ForToK, ForK, WhileK, RegexK} StmtKind;
typedef enum { OpK, ConstK, IdK } ExpKind;

/* ExpType is used for type checking */
typedef enum { Void, Integer, Boolean } ExpType;

#define MAXCHILDREN 4

typedef struct treeNode
{
    struct treeNode* child[MAXCHILDREN];
    struct treeNode* sibling;
    int lineno;
    NodeKind nodekind;
    union { StmtKind stmt; ExpKind exp; } kind;
    union {
        TokenType op;
        int val;
        char* name;
    } attr;
    ExpType type; /* for type checking of exps */
} TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* EchoSource = TRUE causes the source program to
 * be echoed to the listing file with line numbers
 * during parsing
 */
inline int EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
inline int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
inline int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
inline int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
inline int TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
inline int Error;
#endif
