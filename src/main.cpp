#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    app.setApplicationName("Piano Platform");
    app.setApplicationVersion("1.0.0");
    app.setApplicationDisplayName("🎹 Piano Platform");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
