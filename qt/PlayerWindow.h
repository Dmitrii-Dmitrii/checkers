#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <memory>

#include "BoardWidget.h"
#include "CheckersClient.h"

enum class PlayerType {
    White,
    Black
};

class PlayerWindow : public QWidget {
    Q_OBJECT

public:
    explicit PlayerWindow(PlayerType type, const QString& serverHost = "127.0.0.1", int serverPort = 8080, QWidget *parent = nullptr);
    ~PlayerWindow();

    private slots:
        void onSendClicked();
    void onReceived(const QString &msg);
    void resetGame();
    void updateStatus(bool myTurn);

private:
    void checkWinner(int whiteCount, int blackCount);
    void displayWinner(const QString &color);
    bool isMyTurn(const QString &currentPlayer);

    QLineEdit *m_moveEdit;
    QPushButton *m_sendBtn;
    BoardWidget *m_boardWidget;
    QTextEdit *m_log;
    QLabel *m_statusLabel;
    std::unique_ptr<CheckersClient> m_client;
    size_t m_lastMessageIndex;
    bool m_gameOver;
    PlayerType m_playerType;
    bool m_myTurn;
};

#endif // PLAYERWINDOW_H