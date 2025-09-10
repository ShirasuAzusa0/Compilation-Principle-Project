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
    // 载入文法规则文件的按钮控件
    void on_pushButton_load_clicked();
    // 保存文法规则文件的按钮控件
    void on_pushButton_save_clicked();
    // 使用说明的按钮控件
    void on_pushButton_help_clicked();
    // 显示first集合和follow集合的按钮控件
    void on_pushButton_firstandfollow_clicked();
    // 判断文法是否为SLR(1)文法的按钮控件
    void on_pushButton_check_clicked();
    // 生成LR(0)DFA图的按钮控件
    void on_pushButton_LR0DFA_clicked();
    // 生成LR(1)DFA图的按钮控件
    void on_pushButton_LR1DFA_clicked();
    // 生成LR(1)分析表的按钮控件
    void on_pushButton_LR1table_clicked();
    // 对语句进行LR(1)分析并生成分析表的按钮控件
    void on_pushButton_sentence_clicked();


private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
