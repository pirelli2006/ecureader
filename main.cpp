#include "loggerwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoggerWindow w;
    w.show();
    return a.exec();
}
