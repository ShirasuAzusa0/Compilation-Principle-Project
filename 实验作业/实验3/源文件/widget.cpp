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
#include "analyse.h"
#include "globals.h"
#pragma execution_character_set("utf-8")

using namespace std;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    // 获取 QTreeWidget 树型控件的指针
    QTreeWidget *treeWidget = ui->treeWidget;
    // 删除默认顶级项目
    QTreeWidgetItem *defaultTopLevelItem = treeWidget->topLevelItem(0);
    if (defaultTopLevelItem != nullptr)
        delete defaultTopLevelItem;
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_1_clicked()
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

        // 读取文件内容并显示在 plainTextEdit
        stringstream buffer;
        buffer << inputFile.rdbuf();
        QString fileContents = QString::fromStdString(buffer.str());
        ui->plainTextEdit->setPlainText(fileContents);
    }
}

void Widget::on_pushButton_2_clicked()
{
    // 保存结果到文本文件
    QString saveFilePath = QFileDialog::getSaveFileName(this, tr("保存结果文件"), QDir::homePath(), tr("文本文件 (*.tny)"));
    if (!saveFilePath.isEmpty() && !ui->plainTextEdit->toPlainText().isEmpty()) {
        QFile outputFile(saveFilePath);
        if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&outputFile);
            stream << ui->plainTextEdit->toPlainText();
            outputFile.close();
            QMessageBox::about(this, "提示", "TINY源代码保存成功！");
        }
    }
    else if (ui->plainTextEdit->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, tr("提示"), tr("输入框为空，请重试！"));
    }
}

// 创建临时文件tempfile，用于调用TINY解释器的文件流接口
QTemporaryFile tempFile;

void Widget::on_pushButton_3_clicked()
{
    lineno = 0;
    // 清空文件内容
    if (source != nullptr) {
        fclose(source);
        // 置空文件句柄，以避免重复关闭
        source = nullptr;
    }
    // 删除之前的临时文件
    if (tempFile.exists()) {
        tempFile.remove();
    }
    if (ui->plainTextEdit->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, tr("提示"), tr("代码框为空，请重试！"));
        exit(0);
    }
    QString sourceCode = ui->plainTextEdit->toPlainText();
    if (tempFile.open())
    {
        // 将文本框的东西写入临时文件中
        QTextStream out(&tempFile);
        out << sourceCode;
        tempFile.close();

        // 创建文件流
        source = fopen(tempFile.fileName().toStdString().c_str(), "r");
        if (source != nullptr)
        {
            // 核心分析函数
            QTreeWidgetItem* res = AnalyzeCode();
            if (Error)
            {
                QMessageBox::critical(this, "错误信息", "程序语法错误！请检查程序");
                ui->plainTextEdit_2->setPlainText(debugMsg);
                // 清空所有顶级项目
                ui->treeWidget->clear();
                Error = FALSE;
                return;
            }
            if (res != NULL)
            {
                // 清空所有顶级项目
                ui->treeWidget->clear();

                // 添加新的顶级项目
                ui->treeWidget->addTopLevelItem(res);

                // 默认展开第一层
                if (ui->treeWidget->topLevelItemCount() > 0) {
                    QTreeWidgetItem* firstTopLevelItem = ui->treeWidget->topLevelItem(0);
                    firstTopLevelItem->setExpanded(true);
                }

                ui->plainTextEdit_2->setPlainText(debugMsg);
            }
        }
        else
        {
            QMessageBox::critical(this, "错误信息", "导入错误！无法打开文件，请检查路径和文件是否被占用！");
            cerr << "Error opening temporary file." << endl;
        }

    }
    else
    {
        QMessageBox::critical(this, "错误信息", "导入错误！无法创建临时文件！");
        cerr << "Error creating temporary file." << endl;
    }
}

