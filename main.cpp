#include <iostream>
#include "widget.h"
#include <filesystem>
#include <QApplication>
namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;

    w.show();
    return a.exec();
}
