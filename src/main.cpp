#include <QApplication>

#include "app.hpp"

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    app.setStyle("macOS");

    OUMP::AppWindow app_window;

    return app.exec();
};
