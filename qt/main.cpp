#include <QApplication>
#include <QTranslator>
#include "AppEventFilter.h"
#include "MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QTranslator translator;
    translator.load(".../translations/ru.qm");
    app.installTranslator(&translator);
    MainWindow w;
    AppEventFilter filter(&app);
    app.installEventFilter(&filter);
    w.show();
    return app.exec();
}
