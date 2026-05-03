#include <QApplication>
#include <QCoreApplication>
#include "Widgets/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    QApplication::setStyle("Fusion");
    QCoreApplication::setOrganizationName("PabloPicose");
    QCoreApplication::setApplicationName("ComputerVisionBlueprint");
    a.setWindowIcon(QIcon(":/images/logo.png"));

    MainWindow w;
    w.show();

    return a.exec();
}
