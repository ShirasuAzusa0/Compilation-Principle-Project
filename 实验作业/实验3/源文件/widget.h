#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    // 读取源程序的按钮控件
    void on_pushButton_1_clicked();
    // 保存源程序的按钮控件
    void on_pushButton_2_clicked();
    // 生成语法树的按钮控件
    void on_pushButton_3_clicked();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
