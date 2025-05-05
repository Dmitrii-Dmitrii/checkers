#include <QTest>
#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include "../qt/MainWindow.h"

class QtClientTest : public QObject {
    Q_OBJECT

private:
    QApplication *app;
    MainWindow *window;

    private slots:
        void initTestCase() {
        int argc = 0;
        app = new QApplication(argc, nullptr);
        window = new MainWindow();
        window->show();
    }

    void cleanupTestCase() {
        delete window;
        delete app;
    }

    void testOnSendClickedClearsInput() {
        QLineEdit *moveEdit = window->findChild<QLineEdit *>();
        QPushButton *sendBtn = window->findChild<QPushButton *>();
        QVERIFY(moveEdit);
        QVERIFY(sendBtn);

        moveEdit->setText("test_move");
        QTest::mouseClick(sendBtn, Qt::LeftButton);

        QCOMPARE(moveEdit->text(), QString(""));
    }

    void testOnReceivedAppendsInvalidJson() {
        QTextEdit *log = window->findChild<QTextEdit *>();
        QVERIFY(log);

        log->clear();

        QString msg = "not a json";
        QMetaObject::invokeMethod(window, "onReceived", Qt::DirectConnection,
                                  Q_ARG(QString, msg));

        QString logText = log->toPlainText();
        QVERIFY(logText.contains(msg));
    }
};

QTEST_MAIN(QtClientTest)
#include "tests_QtClientTest.moc"
