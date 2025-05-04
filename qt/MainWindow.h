#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <memory>

#include "BoardWidget.h"
#include "CheckersClient.h"


class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:
    void onSendClicked();

    void onReceived(const QString &msg);

    void resetGame();

private:
    void checkWinner(int whiteCount, int blackCount);

    void displayWinner(const QString &color);

    QLineEdit *m_moveEdit;
    QPushButton *m_sendBtn;
    BoardWidget *m_boardWidget;
    QTextEdit *m_log;
    std::unique_ptr<CheckersClient> m_client;
    size_t m_lastMessageIndex;
    bool m_gameOver;
};
