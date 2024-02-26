#include <QApplication>
#include "MainWidget.h"
#include <QTabWidget>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);    
    MainWidget w;
    w.resize(1280, 720);
    w.show();        
    return a.exec();
}
