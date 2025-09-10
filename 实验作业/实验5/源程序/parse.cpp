#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"
#include <string>
#include <QDebug>
using namespace std;
#pragma execution_character_set("utf-8")

static TokenType token; /* holds current token */

/* function prototypes for recursive calls */
static TreeNode* stmt_sequence(void);
static TreeNode* statement(void);
static TreeNode* if_stmt(void);
static TreeNode* repeat_stmt(void);
static TreeNode* assign_stmt(void);
static TreeNode* read_stmt(void);
static TreeNode* write_stmt(void);
static TreeNode* exp(void);
static TreeNode* simple_exp(void);
static TreeNode* term(void);
static TreeNode* power(void);
static TreeNode* factor(void);
static TreeNode* for_stmt(void);
static TreeNode* while_stmt(void);
static TreeNode* orexp(void);
static TreeNode* andexp(void);
static TreeNode* notexp(void);
static TreeNode* regex_stmt(void);
static TreeNode* andreg(void);
static TreeNode* topreg(void);
static TreeNode* reg_factor(void);
static TreeNode* plusplus_stmt(void);
static TreeNode* subsub_stmt(void);


static void syntaxError(std::string message)
{
    qDebug() << "row" << lineno << "has error：" << QString::fromStdString(message);
    std::string msg =  (string)"row，has error：" + message;
    debugMsg.append("\n").append(QString::number(lineno)).append(QString::fromStdString(msg));
    /*fprintf(listing, "\n>>> ");
    fprintf(listing, "Syntax error at line %d: %s", lineno, message);*/
    Error = TRUE;
}

static void match(TokenType expected)
{
    if (token == expected) token = getToken();
    else {
        syntaxError("unexpected token -> " + getTokenString(token, tokenString).toStdString());
        printToken(token, tokenString);
        fprintf(listing, "      ");
    }
}

TreeNode* stmt_sequence(void)
{
    TreeNode* t = statement();
    TreeNode* p = t;
    while ((token != ENDFILE) && (token != END) &&
        (token != ELSE) && (token != UNTIL) && (token != ENDDO) && (token != RPM) &&
        (token != ENDWHILE))
    {
        TreeNode* q;
        match(SEMI);
        q = statement();
        if (q != NULL) {
            if (t == NULL) t = p = q;
            else /* now p cannot be NULL either */
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}


//P394 
//lineno: 961
TreeNode* statement(void)
{
    TreeNode* t = NULL;
    switch (token) {
    case IF: t = if_stmt(); break;
    case REPEAT: t = repeat_stmt(); break;
    case ID: t = assign_stmt(); break;
    case READ: t = read_stmt(); break;
    case WRITE: t = write_stmt(); break;
    case FOR: t = for_stmt(); break;
    case WHILE: t = while_stmt(); break;
    case SUBSUB: t = subsub_stmt(); break;
    case PLUSPLUS: t = plusplus_stmt(); break;
    default: syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    } /* end case */
    return t;
}


//P394 
//lineno: 977
// 实验三改写后的if语句文法：if_stmt->if(exp) [[stmt-sequence] else [stmt-sequence]]
// 实验五不修改，恢复原样
TreeNode* if_stmt(void)
{
    TreeNode* t = newStmtNode(IfK);
    match(IF);
    if (t) t->child[0] = exp();
    match(THEN);
    if (t) t->child[1] = stmt_sequence();
    if (token==ELSE){
        match(ELSE);
        if (t) t->child[2] = stmt_sequence();
    }
    match(END);
    return t;
}

//P394 
//lineno:991
TreeNode* repeat_stmt(void)
{
    TreeNode* t = newStmtNode(RepeatK);
    match(REPEAT);
    if (t) t->child[0] = stmt_sequence();
    match(UNTIL);
    if (t) t->child[1] = exp();
    return t;
}

// 新增判断前置自减的文法(subsub)
TreeNode* subsub_stmt(void)
{
    // 创建一个赋值语句节点，类型为 AssignK
        TreeNode* t = newStmtNode(AssignK);
        if ((t) && (token == SUBSUB)) {
            // 保存变量名
            match(SUBSUB);  // 消耗掉前置 -- 符号
            if (t && token == ID) {
                // 记录自增操作的目标变量名
                t->attr.name = copyString(tokenString);
                // 构造表达式部分：等价于 x = x - 1
                TreeNode* p = newExpNode(OpK);
                if (p != NULL) {
                    // 左子节点：标识符节点，表示原始变量 x
                    TreeNode* left = newExpNode(IdK);
                    if (left) {
                        left->attr.name = copyString(t->attr.name);
                    }
                    p->child[0] = left;

                    // 操作符：MINUS
                    p->attr.op = MINUS;

                    // 右子节点：常量节点，表示字面量 1
                    TreeNode* right = newExpNode(ConstK);
                    if (right) {
                        right->attr.val = 1;
                    }
                    p->child[1] = right;

                    // 将整个加法表达式作为赋值语句的右子节点
                    t->child[0] = p;
                }
            }
            // 消耗掉标识符 token
            match(ID);
        }
        return t;
}

// 新增判断前置自增的文法（plusplus）
TreeNode* plusplus_stmt(void)
{
    // 创建一个赋值语句节点，类型为 AssignK
        TreeNode* t = newStmtNode(AssignK);
        if ((t) && (token == PLUSPLUS)) {
            // 保存变量名
            match(PLUSPLUS);  // 消耗掉前置 ++ 符号
            if (t && token == ID) {
                // 记录自增操作的目标变量名
                t->attr.name = copyString(tokenString);
                // 构造表达式部分：等价于 x = x + 1
                TreeNode* p = newExpNode(OpK);
                if (p != NULL) {
                    // 左子节点：标识符节点，表示原始变量 x
                    TreeNode* left = newExpNode(IdK);
                    if (left) {
                        left->attr.name = copyString(t->attr.name);
                    }
                    p->child[0] = left;

                    // 操作符：PLUS
                    p->attr.op = PLUS;

                    // 右子节点：常量节点，表示字面量 1
                    TreeNode* right = newExpNode(ConstK);
                    if (right) {
                        right->attr.val = 1;
                    }
                    p->child[1] = right;

                    // 将整个加法表达式作为赋值语句的右子节点
                    t->child[0] = p;
                }
            }
            // 消耗掉标识符 token
            match(ID);
        }
        return t;
}



TreeNode* assign_stmt(void)
{
    TreeNode* t = newStmtNode(AssignK);
    if ((t) && (token == ID))
        t->attr.name = copyString(tokenString);
    match(ID);
    if (token == ASSIGN)
    {
        match(ASSIGN);
        if (t) t->child[0] = exp();
    }
    else if (token == PLUSEQ)
    {
        match(PLUSEQ);
        //if (t) t->child[0] = exp();
        if (t) {
            TreeNode* p = newExpNode(OpK);
            if (p != NULL) {
                TreeNode* temp = newExpNode(IdK);
                if (temp != NULL)
                    temp->attr.name = copyString(t->attr.name);
                p->child[0] = temp;
                p->attr.op = PLUS;
                p->child[1] = exp();
                t->child[0] = p;
            }
        }
    }
    else if (token == REGEX)
    {
        match(REGEX);
        if (t) t->child[0] = regex_stmt();
    }
    else if (token == PLUSPLUS)
    {
        match(PLUSPLUS);
        if(t) {
            TreeNode* p = newExpNode(OpK);
            if (p != NULL) {
                TreeNode* temp = newExpNode(IdK);
                if (temp != NULL)
                    temp->attr.name = copyString(t->attr.name);
                p->child[0] = temp;
            }
            p->attr.op = PLUS;
            TreeNode* constNode = newExpNode(ConstK);
            if (constNode != NULL) {
                constNode->attr.val = 1;
                p->child[1] = constNode;
            }
            t->child[0] = p;
        }
    }
    else if (token == SUBSUB)
    {
        match(SUBSUB);
        if(t) {
            TreeNode* p = newExpNode(OpK);
            if (p != NULL) {
                TreeNode* temp = newExpNode(IdK);
                if (temp != NULL)
                    temp->attr.name = copyString(t->attr.name);
                p->child[0] = temp;
            }
            p->attr.op = MINUS;
            TreeNode* constNode = newExpNode(ConstK);
            if (constNode != NULL) {
                constNode->attr.val = 1;
                p->child[1] = constNode;
            }
            t->child[0] = p;
        }
    }
    return t;
}

TreeNode* read_stmt(void)
{
    TreeNode* t = newStmtNode(ReadK);
    match(READ);
    if ((t) && (token == ID))
        t->attr.name = copyString(tokenString);
    match(ID);
    return t;
}

TreeNode* write_stmt(void)
{
    TreeNode* t = newStmtNode(WriteK);
    match(WRITE);
    if (t) t->child[0] = exp();
    return t;
}

TreeNode* exp(void)
{
    TreeNode* t = orexp();
    while (token == OR)
    {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = orexp();
        }
    }
    return t;
}

TreeNode* orexp(void)
{
    TreeNode* t = andexp();
    while (token == AND)
    {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = andexp();
        }
    }
    return t;
}

TreeNode* andexp(void)
{
    TreeNode* t = simple_exp();
    if (token == LTEQ || token == RTEQ || token == LT || token == RT || token == NOTEQ || token == EQ) {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
        }
        match(token);
        if (t)
            t->child[1] = simple_exp();
    }
    return t;
}

TreeNode* simple_exp(void)
{
    TreeNode* t = term();
    while ((token == PLUS) || (token == MINUS))
    {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            match(token);
            // 如果下一个 token 是左括号，处理括号内的表达式
            if (token == LPAREN) {
                match(LPAREN);
                p->child[1] = simple_exp();
                match(RPAREN);
            } else {
                p->child[1] = term();
            }
            t = p;
        }
    }
    return t;
}

TreeNode* term(void)
{
    TreeNode* t = NULL;
    if (token == LPAREN) {
        match(LPAREN);
        t = simple_exp();
        match(RPAREN);
    } else {
        t = notexp();
    }
    while ((token == TIMES) || (token == OVER) || (token == MOD))
    {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            if (token == LPAREN) {
                match(LPAREN);
                p->child[1] = simple_exp();
                match(RPAREN);
            } else {
                p->child[1] = notexp();
            }
            t = p;
        }
    }
    return t;
}

TreeNode* notexp(void)
{
    if (token == NOT)
    {
        match(NOT);
        TreeNode* p = newExpNode(OpK);
        p->attr.op = NOT;
        if (token == NOT)
        {
            p->child[0] = notexp();
        }
        else
        {
            p->child[0] = power();
        }
        return p;
    }
    else if (token == LPAREN)
    {
        match(LPAREN);
        TreeNode* t = simple_exp();
        match(RPAREN);
        return t;
    }
    else
    {
        return power();
    }
}

// 新增乘方运算
TreeNode* power(void)
{
    TreeNode* t = factor();
    while ((token == POWER))
    {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = factor();
        }
    }
    return t;
}

TreeNode* factor(void)
{
    TreeNode* t = NULL;
    switch (token) {
    case NUM:
        t = newExpNode(ConstK);
        if ((t) && (token == NUM))
            t->attr.val = atoi(tokenString);
        match(NUM);
        break;
    case ID:
        t = newExpNode(IdK);
        if ((t) && (token == ID))
            t->attr.name = copyString(tokenString);
        match(ID);
        break;
    case LPAREN:
        match(LPAREN);
        t = exp();
        match(RPAREN);
        break;
    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }
    return t;
}

// 修改后的for_stmt
// for_stmt -> for(assign_stmt;exp;assign_stmt) stmt_sequence
TreeNode* for_stmt(void)
{
    TreeNode* t = newStmtNode(ForK);
    match(FOR);
    match(LPAREN);
    // 处理初始化赋值部分
    if (t) t->child[0] = assign_stmt();
    match(SEMI);
    // 处理条件表达式部分
    if (t) t->child[1] = exp();
    match(SEMI);
    // 处理更新赋值部分
    if (t)
    {
        if (token == PLUSPLUS) t->child[2] = plusplus_stmt();
        else if (token == SUBSUB) t->child[2] = subsub_stmt();
        else t->child[2] = assign_stmt();
    }
    match(RPAREN);
    match(LPM);
    if (t) t->child[3] = stmt_sequence();
    match(RPM);
    return t;
}

// 新增while_stmt
// 实验三：while_stmt -> while(exp) stmt_sequence endwhile
// 实验五：while_stmt → while exp do stmt_sequence enddo
TreeNode* while_stmt(void)
{
    // 建立语法树结点：WhileK 表示 “while” 语句
    TreeNode* t = newStmtNode(WhileK);

    // 关键字 while
    match(WHILE);

    // 条件表达式 exp
    // child[0] 存放循环条件
    if (t) t->child[0] = exp();

    // 关键字 do
    match(DO);

    // 循环体 stmt_sequence
    // child[1] 存放循环体
    if (t) t->child[1] = stmt_sequence();

    // 关键字 enddo
    match(ENDDO);

    return t;
}

TreeNode* regex_stmt(void)
{
    TreeNode* t = andreg();
    while ((token == RGOR))
    {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = andreg();
        }
    }
    return t;
}

TreeNode* andreg(void)
{
    TreeNode* t = topreg();
    while ((token == RGAND))
    {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = topreg();
        }
    }
    return t;
}

TreeNode* topreg(void)
{
    TreeNode* t = reg_factor();
    while ((token == RGCLOSE) || (token == RGCHOOSE))
    {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
        }
    }
    return t;
}

TreeNode* reg_factor(void)
{
    TreeNode* t = NULL;
    switch (token) {
    case NUM:
        t = newExpNode(ConstK);
        if ((t) && (token == NUM))
            t->attr.val = atoi(tokenString);
        match(NUM);
        break;
    case ID:
        t = newExpNode(IdK);
        if ((t) && (token == ID))
            t->attr.name = copyString(tokenString);
        match(ID);
        break;
    case LPAREN:
        match(LPAREN);
        t = regex_stmt();
        match(RPAREN);
        break;
    default:
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        token = getToken();
        break;
    }
    return t;
}

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly
 * constructed syntax tree
 */
TreeNode* parse(void)
{
    TreeNode* t;
    token = getToken();
    t = stmt_sequence();
    if (token != ENDFILE)
        syntaxError("Code ends before file\n");
    return t;
}
