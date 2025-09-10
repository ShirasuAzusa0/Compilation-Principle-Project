#include "globals.h"
#include "parse.h"
#include "util.h"
#include <stdio.h>
#include "analyse.h"
#include <QDebug>
#include <scan.h>
#include <QString>
#pragma execution_character_set("utf-8")

/* allocate global variables */
int lineno = 0;
FILE* source;
FILE* listing;
FILE* code;

/* allocate and set tracing flags */
int EchoSource = TRUE;
int TraceScan = FALSE;
int TraceParse = TRUE;

int Error = FALSE;

QString debugMsg = QString::fromStdString("");

QTreeWidgetItem* AnalyzeCode()
{
    reset();
    debugMsg.clear();
    TreeNode* syntaxTree;
    listing = stdout; /* send listing to screen */
    syntaxTree = parse();
    if (!Error)
    {
        qDebug() << "生成成功";
        debugMsg.append("\n").append(QString("生成成功"));
        return printTree(syntaxTree,NULL);
    }
    return NULL;
}
