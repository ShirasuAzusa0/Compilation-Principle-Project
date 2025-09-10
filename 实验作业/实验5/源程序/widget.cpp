#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>
#include <QTextCodec>
#include <QMessageBox>
#include <QTemporaryFile>
#include <iostream>
#include <map>
#include <vector>
#include <stack>
#include <unordered_map>
#include <queue>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include "widget.h"
#include "ui_widget.h"
#include "globals.h"
#include "parse.h"
#include "scan.h"
#include "util.h"
#pragma execution_character_set("utf-8")

using namespace std;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_Load_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"),
        QDir::homePath(), tr("tiny文件 (*.tny);;文本文件 (*.txt);;所有文件 (*.*)"));
    if (!filePath.isEmpty())
    {
        ifstream inputFile;
        QTextCodec* code = QTextCodec::codecForName("GB2312");
        //QTextCodec* code = QTextCodec::codecForName("UTF-16");

        string selectedFile = code->fromUnicode(filePath.toStdString().c_str()).data();
        inputFile.open(selectedFile.c_str(), ios::in);

        if (!inputFile) {
            QMessageBox::critical(this, "错误信息", "导入错误！无法打开文件，请检查路径和文件是否被占用！");
            cerr << "Error opening file." << endl;
        }

        // 读取文件内容并显示在 plainTextEdit_Tiny 上
        stringstream buffer;
        buffer << inputFile.rdbuf();
        QString fileContents = QString::fromStdString(buffer.str());
        ui->plainTextEdit_Tiny->setPlainText(fileContents);
    }
}

void Widget::on_pushButton_Save_clicked()
{
    // 保存结果到文本文件
    QString saveFilePath = QFileDialog::getSaveFileName(this, tr("保存结果文件"), QDir::homePath(), tr("文本文件 (*.tny)"));
    if (!saveFilePath.isEmpty() && !ui->plainTextEdit_Tiny->toPlainText().isEmpty()) {
        QFile outputFile(saveFilePath);
        if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&outputFile);
            stream << ui->plainTextEdit_Tiny->toPlainText();
            outputFile.close();
            QMessageBox::about(this, "提示", "TINY源代码保存成功！");
        }
    }
    else if (ui->plainTextEdit_Tiny->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, tr("提示"), tr("输入框为空，请重试！"));
    }
}

// 创建临时文件tempfile，用于调用TINY解释器的文件流接口
QTemporaryFile tempFile;

void Widget::on_pushButton_Generate_clicked()
{
    // 重置全局变量
    lineno = 1;
    EchoSource = FALSE;
    TraceScan = FALSE;
    TraceParse = FALSE;
    TraceAnalyze = FALSE;
    TraceCode = FALSE;
    Error = FALSE;
    debugMsg.clear();

    // step 1：将输入的 Tiny 程序写入临时文件
    // 清空文件内容先
    if (source != nullptr){
        fclose(source);
        // 置空文件句柄，避免重复关闭文件
        source = nullptr;
    }

    if (tempFile.isOpen()){
       tempFile.close();
   }
    // 删除之前的临时文件
    if (tempFile.exists()){
        tempFile.remove();
    }

    if (ui->plainTextEdit_Tiny->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, tr("提示"), tr("源码为空，请先输入TINY源码！"));
        exit(0);
    }

    QString sourceCode = ui->plainTextEdit_Tiny->toPlainText();
    if (tempFile.open()){
        // 将TINY源码写入临时文件
        QTextStream out(&tempFile);
        out << sourceCode;
        tempFile.close();

        // 创建文件流
        source = fopen(tempFile.fileName().toStdString().c_str(),"r");
        if (source != nullptr)
        {
            // 核心部分
            listing = stdout;
            Error = false;

            TreeNode* root = parse();
            /*if (Error || !root){
                QMessageBox::critical(this, "错误信息", "程序语法错误，请检查程序斌予以更正");
                //fclose(source);
                //source = nullptr;
                //return;
            }*/

            // 四元组数据结构与状态
            struct Quad { QString op, a1, a2, res; };
            QVector<Quad> quads;
            int tempCnt = 0, lblCnt = 0;
            auto newTemp = [&]() { return QString("t%1").arg(++tempCnt); };
            auto newLabel= [&]() { return QString("L%1").arg(++lblCnt); };
            auto emitQ   = [&](const QString &op,
                               const QString &a1 = QString(),
                               const QString &a2 = QString(),
                               const QString &r  = QString())
            {
                quads.append({op, a1, a2, r});
            };
            auto op2str = [&](TokenType t)->QString {
                switch (t) {
                case PLUS:  return "+";
                case MINUS: return "-";
                case TIMES: return "*";
                case OVER:  return "/";
                case MOD:   return "%";
                case POWER: return "^";
                case EQ:    return "==";
                case LT:    return "<";
                case LTEQ:  return "<=";
                case RT:    return ">";
                case RTEQ:  return ">=";
                case NOTEQ: return "<>";
                default:    return "?";
                }
            };

            // 递归生成表达式
            std::function<QString(TreeNode*)> genExp = [&](TreeNode* t)->QString {
                if (!t) return QString();
                if (t->nodekind == ExpK && t->kind.exp==OpK) {
                    QString left = genExp(t->child[0]);
                    QString right = genExp(t->child[1]);
                    QString tmp = newTemp();
                    emitQ(op2str(t->attr.op), left, right, tmp);
                    return tmp;
                }
                else if (t->nodekind == ExpK && t->kind.exp==ConstK)
                    return QString::number(t->attr.val);
                else if (t->nodekind == ExpK && t->kind.exp==IdK)
                    return t->attr.name;
                return QString();
            };

            // 递归生成语句序列
            std::function<void(TreeNode*)> genStmt = [&](TreeNode* t) {
                // 用 while + 显式 next，确保每次处理一个节点
                // 并且 case IfK/RepeatK 里再也不用担心 sibling 链跑偏
                while (t) {
                    TreeNode* next = t->sibling;  // 先记下下一个节点

                    if (t->nodekind == StmtK) {
                        switch (t->kind.stmt) {

                        case AssignK: {
                            QString v = genExp(t->child[0]);
                            emitQ(":=", v, "", t->attr.name);
                        } break;

                        case ReadK:
                            emitQ("read", t->attr.name);
                            break;

                        case WriteK: {
                            QString v = genExp(t->child[0]);
                            emitQ("write", v);
                        } break;

                        case IfK: {
                            QString cond  = genExp(t->child[0]);   // 条件
                            QString Lelse = newLabel();            // else:
                            QString Lend  = newLabel();            // end:

                            emitQ("ifFalse", cond, "goto", Lelse); // 不满足 → else

                            genStmt(t->child[1]);                  // then 块
                            emitQ("goto", "", "", Lend);           // then 结束 → end

                            emitQ("label", "", "", Lelse);         // else:
                            if (t->child[2])                       // 有无 else 都要生成标签
                                genStmt(t->child[2]);              // else 块

                            emitQ("label", "", "", Lend);          // end:
                        } break;

                        case WhileK: {
                            QString Lbegin = newLabel(), Lbody = newLabel(), Lend = newLabel();
                            emitQ("label", "", "", Lbegin);
                            QString c = genExp(t->child[0]);
                            emitQ("ifFalse", c, "goto", Lend);
                            emitQ("label", "", "", Lbody);
                            genStmt(t->child[1]);
                            emitQ("goto", "", "", Lbegin);
                            emitQ("label", "", "", Lend);
                        } break;

                        case ForK: {
                            // init
                            genStmt(t->child[0]);
                            QString Lchk = newLabel(), Lbody = newLabel(), Lend = newLabel();
                            emitQ("label", "", "", Lchk);
                            {
                                QString c = genExp(t->child[1]);
                                emitQ("ifFalse", c, "goto", Lend);
                            }
                            emitQ("label", "", "", Lbody);
                            genStmt(t->child[3]);  // body
                            genStmt(t->child[2]);  // iter
                            emitQ("goto", "", "", Lchk);
                            emitQ("label", "", "", Lend);
                        } break;

                        case RepeatK: {
                            QString Lbegin = newLabel();
                            emitQ("label", "", "", Lbegin);          // 入口

                            genStmt(t->child[0]);                    // 循环体

                            QString cond = genExp(t->child[1]);      // UNTIL 后布尔表达式
                            if (cond.isEmpty()) {                    // 绝不能为空——否则四元组错误
                                QMessageBox::critical(nullptr,"错误","repeat‑until 条件生成失败");
                            }
                            emitQ("ifFalse", cond, "goto", Lbegin);  // 条件假 → 回头
                        } break;

                        default:
                            // 其它不处理
                            break;
                        }
                    }

                    // 移到原本的 sibling
                    t = next;
                }
            };

            genStmt(root);

            ui->plainTextEdit_Generate->clear();
            for (int i = 0; i < quads.size(); ++i) {
                const auto &q = quads[i];
                ui->plainTextEdit_Generate->appendPlainText(
                    QString("%1:\t(%2, %3, %4, %5)")
                        .arg(i, 2)
                        .arg(q.op)
                        .arg(q.a1)
                        .arg(q.a2)
                        .arg(q.res)
                );
            }

            // 清理
            fclose(source);
            source = nullptr;
        }
    }
    else {
        QMessageBox::critical(this, "错误信息", "创建Tiny源程序临时文件失败");
        return;
    }
    ui->plainTextEdit_Status->setPlainText("生成完毕！");
}
