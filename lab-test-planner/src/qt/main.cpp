#include "qt/mainwindow.hpp"

#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("Lab Planner"));
    QApplication::setOrganizationName(QStringLiteral("LabPlanner"));

    lab::qtui::MainWindow window;
    window.show();
    return app.exec();
}
