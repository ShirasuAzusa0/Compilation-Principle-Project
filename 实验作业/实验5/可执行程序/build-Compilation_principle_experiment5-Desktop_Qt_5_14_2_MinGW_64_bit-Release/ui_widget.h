/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QLabel *label_2;
    QLabel *label_1;
    QLabel *label_3;
    QPlainTextEdit *plainTextEdit_Status;
    QPlainTextEdit *plainTextEdit_Tiny;
    QLabel *label_4;
    QPushButton *pushButton_Load;
    QPushButton *pushButton_Save;
    QPushButton *pushButton_Generate;
    QLabel *label_5;
    QPlainTextEdit *plainTextEdit_Generate;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(800, 541);
        label_2 = new QLabel(Widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(370, 20, 171, 21));
        QFont font;
        font.setFamily(QString::fromUtf8("Adobe \344\273\277\345\256\213 Std R"));
        font.setPointSize(9);
        font.setBold(false);
        font.setWeight(50);
        label_2->setFont(font);
        label_1 = new QLabel(Widget);
        label_1->setObjectName(QString::fromUtf8("label_1"));
        label_1->setGeometry(QRect(330, 0, 271, 20));
        QFont font1;
        font1.setFamily(QString::fromUtf8("Adobe \344\273\277\345\256\213 Std R"));
        font1.setPointSize(12);
        label_1->setFont(font1);
        label_3 = new QLabel(Widget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(30, 50, 419, 17));
        QFont font2;
        font2.setFamily(QString::fromUtf8("Adobe \344\273\277\345\256\213 Std R"));
        font2.setPointSize(10);
        font2.setBold(false);
        font2.setWeight(50);
        label_3->setFont(font2);
        plainTextEdit_Status = new QPlainTextEdit(Widget);
        plainTextEdit_Status->setObjectName(QString::fromUtf8("plainTextEdit_Status"));
        plainTextEdit_Status->setGeometry(QRect(30, 70, 419, 191));
        plainTextEdit_Tiny = new QPlainTextEdit(Widget);
        plainTextEdit_Tiny->setObjectName(QString::fromUtf8("plainTextEdit_Tiny"));
        plainTextEdit_Tiny->setGeometry(QRect(30, 290, 419, 175));
        label_4 = new QLabel(Widget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(30, 270, 419, 17));
        label_4->setFont(font2);
        pushButton_Load = new QPushButton(Widget);
        pushButton_Load->setObjectName(QString::fromUtf8("pushButton_Load"));
        pushButton_Load->setGeometry(QRect(30, 470, 419, 31));
        QFont font3;
        font3.setFamily(QString::fromUtf8("Adobe \344\273\277\345\256\213 Std R"));
        font3.setBold(false);
        font3.setWeight(50);
        pushButton_Load->setFont(font3);
        pushButton_Save = new QPushButton(Widget);
        pushButton_Save->setObjectName(QString::fromUtf8("pushButton_Save"));
        pushButton_Save->setGeometry(QRect(30, 510, 419, 29));
        pushButton_Save->setFont(font3);
        pushButton_Generate = new QPushButton(Widget);
        pushButton_Generate->setObjectName(QString::fromUtf8("pushButton_Generate"));
        pushButton_Generate->setGeometry(QRect(460, 510, 339, 29));
        pushButton_Generate->setFont(font3);
        label_5 = new QLabel(Widget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(460, 50, 331, 20));
        label_5->setFont(font2);
        plainTextEdit_Generate = new QPlainTextEdit(Widget);
        plainTextEdit_Generate->setObjectName(QString::fromUtf8("plainTextEdit_Generate"));
        plainTextEdit_Generate->setGeometry(QRect(460, 70, 331, 431));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "Widget", nullptr));
        label_2->setText(QCoreApplication::translate("Widget", "\350\256\241\347\247\2212\347\217\255 \350\242\201\347\237\245\346\234\254 20232131006", nullptr));
        label_1->setText(QCoreApplication::translate("Widget", "\345\256\236\351\252\2145  TINY\346\211\251\345\205\205\350\257\255\350\250\200\347\232\204\344\270\255\351\227\264\344\273\243\347\240\201\347\224\237\346\210\220", nullptr));
        label_3->setText(QCoreApplication::translate("Widget", "\347\212\266\346\200\201\344\277\241\346\201\257\346\240\217", nullptr));
        label_4->setText(QCoreApplication::translate("Widget", "TINY\346\272\220\344\273\243\347\240\201", nullptr));
        pushButton_Load->setText(QCoreApplication::translate("Widget", "\350\257\273\345\217\226\346\272\220\347\250\213\345\272\217\346\226\207\344\273\266", nullptr));
        pushButton_Save->setText(QCoreApplication::translate("Widget", "\344\277\235\345\255\230\346\272\220\347\250\213\345\272\217", nullptr));
        pushButton_Generate->setText(QCoreApplication::translate("Widget", "\347\224\237\346\210\220\345\233\233\345\205\203\347\273\204\344\270\255\351\227\264\344\273\243\347\240\201", nullptr));
        label_5->setText(QCoreApplication::translate("Widget", "\345\233\233\345\205\203\347\273\204\344\270\255\351\227\264\344\273\243\347\240\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
