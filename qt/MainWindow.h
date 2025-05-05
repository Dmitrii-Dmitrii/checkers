#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <memory>

#include "BoardWidget.h"
#include "CheckersClient.h"
#include "MessageThread.h"

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    private slots:
        void onSendClicked();
    void onMessageReceived(const QString &msg);
    void resetGame();

private:
    void checkWinner(int whiteCount, int blackCount);
    void displayWinner(const QString &color);

    QLineEdit *m_moveEdit;
    QPushButton *m_sendBtn;
    BoardWidget *m_boardWidget;
    QTextEdit *m_log;
    std::shared_ptr<CheckersClient> m_client;
    MessageThread *m_messageThread;
    bool m_gameOver;
};

#endif // MAINWINDOW_H