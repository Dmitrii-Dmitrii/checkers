#include <QApplication>
#include <QTranslator>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "PlayerWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QTranslator translator;
    translator.load("../translations/ru.qm");
    app.installTranslator(&translator);

    QCommandLineParser parser;
    parser.setApplicationDescription("Checkers Game Client");
    parser.addHelpOption();

    QCommandLineOption modeOption(QStringList() << "m" << "mode",
                                 "Player mode (white, black, or both)",
                                 "mode", "both");
    QCommandLineOption hostOption(QStringList() << "s" << "server",
                                 "Server hostname or IP address",
                                 "server", "127.0.0.1");
    QCommandLineOption portOption(QStringList() << "p" << "port",
                                 "Server port",
                                 "port", "8080");

    parser.addOption(modeOption);
    parser.addOption(hostOption);
    parser.addOption(portOption);

    parser.process(app);

    QString mode = parser.value(modeOption);
    QString host = parser.value(hostOption);
    int port = parser.value(portOption).toInt();

    PlayerWindow* whiteWindow = nullptr;
    PlayerWindow* blackWindow = nullptr;

    if (mode == "white" || mode == "both") {
        whiteWindow = new PlayerWindow(PlayerType::White, host, port);
        whiteWindow->show();
    }

    if (mode == "black" || mode == "both") {
        blackWindow = new PlayerWindow(PlayerType::Black, host, port);
        blackWindow->show();

        if (whiteWindow) {
            whiteWindow->move(100, 100);
            blackWindow->move(whiteWindow->x() + whiteWindow->width() + 20, 100);
        }
    }

    return app.exec();
}