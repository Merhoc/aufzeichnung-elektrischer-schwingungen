#include <QtGui/QApplication>
#include "analyzer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    analyzer w;
    w.show();
    
    return a.exec();
}
