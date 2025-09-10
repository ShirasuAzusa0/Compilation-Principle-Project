#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    // 启用自动缩放
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    // 支持高清图标
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication a(argc, argv);
    Widget w;
    w.show();
    return a.exec();
}
