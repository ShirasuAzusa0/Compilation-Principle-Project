#include "globals.h"
#include "ui_widget.h"
#include "util.h"
#include <string>
#include <QString>
using namespace std;

string tmpToken;

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
void printToken(TokenType token, const char* tokenString)
{
    switch (token)
    {
    case IF:
    //case THEN:    //修改FOR后删除THEN
    case ELSE:
    case END:
    case REPEAT:
    case UNTIL:
    case READ:
    case WRITE:
        fprintf(listing,
            "reserved word: %s\n", tokenString);
        break;
    case ASSIGN: fprintf(listing, ":=\n"); break;
    case REGEX: fprintf(listing, "::=\n"); break;
    case LT: fprintf(listing, "<\n"); break;
    case EQ: fprintf(listing, "=\n"); break;
    case LPAREN: fprintf(listing, "(\n"); break;
    case RPAREN: fprintf(listing, ")\n"); break;
    case LPM: fprintf(listing, "[\n"); break;
    case RPM: fprintf(listing, "]\n"); break;
    case SEMI: fprintf(listing, ";\n"); break;
    case PLUS: fprintf(listing, "+\n"); break;
    case POWER: fprintf(listing, "^\n"); break;
    case MOD: fprintf(listing, "%\n"); break;
    case PLUSEQ: fprintf(listing, "+=\n"); break;
    case PLUSPLUS: fprintf(listing, "++\n"); break;
    case SUBSUB: fprintf(listing, "--\n"); break;
    case MINUS: fprintf(listing, "-\n"); break;
    case TIMES: fprintf(listing, "*\n"); break;
    case OVER: fprintf(listing, "/\n"); break;
    case ENDFILE: fprintf(listing, "EOF\n"); break;
    case RT: fprintf(listing, ">\n"); break;
    case LTEQ: fprintf(listing, "<=\n"); break;
    case RTEQ: fprintf(listing, ">=\n"); break;
    case NOTEQ: fprintf(listing, "<>\n"); break;
    // 正则部分
    case RGOR: fprintf(listing, "|\n"); break;
    case RGAND: fprintf(listing, "&\n"); break;
    case RGCLOSE: fprintf(listing, "#\n"); break;
    case RGCHOOSE: fprintf(listing, "?\n"); break;
    case NUM:
        fprintf(listing,
            "NUM, val= %s\n", tokenString);
        break;
    case ID:
        fprintf(listing,
            "ID, name= %s\n", tokenString);
        break;
    case ERROR:
        fprintf(listing,
            "ERROR: %s\n", tokenString);
        break;
    default: /* should never happen */
        fprintf(listing, "Unknown token: %d\n", token);
    }
}

QString getTokenString(TokenType token, const char* tokenString)
{
    switch (token)
    {
    case IF:
    //case THEN:    //修改FOR后删除THEN
    case ELSE:
    case END:
    case ENDWHILE:  //新增while语句结束标志ENDWHILE
    case REPEAT:
    case UNTIL:
    case READ:
    case WRITE:
        return QString("reserved word: %1").arg(tokenString);
    case ASSIGN: return QString(":=");
    case LT: return "<";
    case EQ: return "=";
    case LPAREN: return "(";
    case RPAREN: return ")";
    case LPM: return "[";
    case RPM: return "]";
    case SEMI: return ";";
    case PLUS: return "+";
    case PLUSEQ: return "+=";
    case PLUSPLUS: return "++";
    case SUBSUB: return "--";
    case POWER: return "^";
    case MOD: return "%";
    case MINUS: return "-";
    case TIMES: return "*";
    case OVER: return "/";
    case ENDFILE: return "EOF";
    case RT: return ">";
    case LTEQ: return "<=";
    case RTEQ: return ">=";
    case NOTEQ: return "<>";
    case AND: return "AND";
    case OR: return "OR";
    case NOT: return "NOT";
    case REGEX: return "::=";
    // 正则部分
    case RGOR: return "|"; break;
    case RGAND: return "&"; break;
    case RGCLOSE: return "#"; break;
    case RGCHOOSE: return "?"; break;
    case NUM:
        return QString("NUM, val= %1").arg(tokenString);
    case ID:
        return QString("ID, name= %1").arg(tokenString);
    case ERROR:
        return QString("ERROR: %1").arg(tokenString);
    default: /* should never happen */
        return QString("Unknown token: %1").arg(token);
    }
}


/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode* newStmtNode(StmtKind kind)
{
    TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else {
        for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = StmtK;
        t->kind.stmt = kind;
        t->lineno = lineno;
    }
    return t;
}

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode* newExpNode(ExpKind kind)
{
    TreeNode* t = (TreeNode*)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else {
        for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = ExpK;
        t->kind.exp = kind;
        t->lineno = lineno;
        t->type = Void;
    }
    return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char* copyString(char* s)
{
    int n;
    char* t;
    if (s == NULL) return NULL;
    n = strlen(s) + 1;
    t = new char[n];
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else strcpy(t, s);
    return t;
}
// Function to add a new item to the tree widget
QTreeWidgetItem* addTreeItem(QTreeWidgetItem* parent, const QString& text)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(parent);
    item->setText(0, text);
    return item;
}

// Function to print the syntax tree using QTreeWidget
QTreeWidgetItem* printTree(TreeNode* tree, QTreeWidgetItem* parentItem)
{
    if (parentItem == nullptr)
    {
        parentItem = new QTreeWidgetItem();
    }
    while (tree != nullptr)
    {
        QTreeWidgetItem* currentItem = nullptr;

        if (tree->nodekind == StmtK)
        {
            switch (tree->kind.stmt)
            {
            case IfK:
                currentItem = addTreeItem(parentItem, "If");
                break;
            case RepeatK:
                currentItem = addTreeItem(parentItem, "Repeat");
                break;
            case AssignK:
                currentItem = addTreeItem(parentItem, "Assign to: " + QString(tree->attr.name));
                break;
            case ReadK:
                currentItem = addTreeItem(parentItem, "Read: " + QString(tree->attr.name));
                break;
            case WriteK:
                currentItem = addTreeItem(parentItem, "Write");
                break;
            case ForToK:
                currentItem = addTreeItem(parentItem, "ForTo");
                break;
            case ForK:
                currentItem = addTreeItem(parentItem, "For");
                break;
            case WhileK:
                currentItem = addTreeItem(parentItem, "While");
                break;
            default:
                currentItem = addTreeItem(parentItem, "Unknown StmtNode kind");
                break;
            }
        }
        else if (tree->nodekind == ExpK)
        {
            switch (tree->kind.exp)
            {
            case OpK:
                currentItem = addTreeItem(parentItem, "Op: " + getTokenString(tree->attr.op, "\0"));
                break;
            case ConstK:
                currentItem = addTreeItem(parentItem, "Const: " + QString::number(tree->attr.val));
                break;
            case IdK:
                currentItem = addTreeItem(parentItem, "Id: " + QString(tree->attr.name));
                break;
            default:
                currentItem = addTreeItem(parentItem, "Unknown ExpNode kind");
                break;
            }
        }
        else
        {
            currentItem = addTreeItem(parentItem, "Unknown node kind");
        }

        for (int i = 0; i < MAXCHILDREN; i++)
        {
            printTree(tree->child[i], currentItem);
        }

        tree = tree->sibling;
    }

    return parentItem;
}
