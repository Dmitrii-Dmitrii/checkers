#include "MainWindow.h"
#include <QVBoxLayout>
#include <QMetaObject>
#include <QMessageBox>
#include <chrono>
#include <thread>

#include "SimpleJsonParser.h"

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      m_moveEdit(new QLineEdit(this)),
      m_sendBtn(new QPushButton(tr("Make move"), this)),
      m_boardWidget(new BoardWidget(this)),
      m_log(new QTextEdit(this)),
      m_client(std::make_unique<CheckersClient>("127.0.0.1", 8080)),
      m_lastMessageIndex(0),
      m_gameOver(false) {
    auto *topLay = new QHBoxLayout;
    topLay->addWidget(m_moveEdit);
    topLay->addWidget(m_sendBtn);

    auto *mainLay = new QVBoxLayout(this);
    mainLay->addLayout(topLay);
    mainLay->addWidget(m_boardWidget, /*stretch=*/1);
    mainLay->addWidget(m_log, /*stretch=*/1);
    setLayout(mainLay);

    if (m_client->connect()) {
        m_client->sendRawMessage("{\"type\":\"get_board\"}");
    } else {
        m_log->append(tr("Unable to connect to the server."));
        m_sendBtn->setEnabled(false);
    }

    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onSendClicked);

    std::thread([this]() {
        while (m_client->isConnected()) {
            const auto &msgs = m_client->getReceivedMessages();
            for (size_t i = m_lastMessageIndex; i < msgs.size(); ++i) {
                QString qs = QString::fromStdString(msgs[i]);
                QMetaObject::invokeMethod(this, [this, qs]() {
                    onReceived(qs);
                });
            }
            m_lastMessageIndex = msgs.size();

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }).detach();
}

MainWindow::~MainWindow() {
    m_client->disconnect();
}

void MainWindow::onSendClicked() {
    if (m_gameOver) {
        m_log->append(tr("Game over."));
        m_moveEdit->clear();
        return;
    }

    auto txt = m_moveEdit->text().toStdString();
    m_client->sendRawMessage(txt);
    m_moveEdit->clear();
}

void MainWindow::onReceived(const QString &msg) {
    m_log->append(msg);
    qDebug() << "Received message from server:" << msg;

    std::map<std::string, std::string> parsed;
    if (!SimpleJsonParser::parse(msg.toStdString(), parsed)) {
        qDebug() << "Failed to parse JSON message";
        return;
    }

    qDebug() << "Message type:" << QString::fromStdString(parsed["type"]);

    auto type = parsed["type"];
    if (type == "board" || type == "board_state") {
        if (parsed.find("state") != parsed.end()) {
            const auto &st = parsed["state"];
            qDebug() << "Board state string length:" << st.length();
            qDebug() << "Board state:" << QString::fromStdString(st);

            std::vector<std::vector<Piece> > bd(8, std::vector<Piece>(8, Piece::Empty));

            if (st.length() >= 64) {
                int whiteCount = 0;
                int blackCount = 0;

                for (int i = 0; i < 64; ++i) {
                    int row = i / 8;
                    int col = i % 8;
                    char ch = st[i];

                    qDebug() << "Cell" << row << "," << col << "=" << ch;

                    if (ch == 'B') {
                        bd[row][col] = Piece::Black;
                        blackCount++;
                    } else if (ch == 'W') {
                        bd[row][col] = Piece::White;
                        whiteCount++;
                    } else if (ch == 'K') {
                        bd[row][col] = Piece::BlackKing;
                        blackCount++;
                    } else if (ch == 'Q') {
                        bd[row][col] = Piece::WhiteQueen;
                        whiteCount++;
                    } else {
                        bd[row][col] = Piece::Empty;
                    }
                }

                m_boardWidget->setBoard(bd);
                qDebug() << "Board updated with new state";

                checkWinner(whiteCount, blackCount);
            } else {
                qDebug() << "Board state string is too short:" << st.length();
            }
        } else {
            qDebug() << "No 'state' field found in board message";
        }
    } else if (type == "move") {
        auto parsePt = [](const std::string &s) -> QPoint {
            int x = -1, y = -1;
            sscanf(s.c_str(), "%d,%d", &x, &y);
            qDebug() << "Parsed point:" << x << "," << y;
            return QPoint(x, y);
        };

        if (parsed.find("from") != parsed.end() && parsed.find("to") != parsed.end()) {
            QPoint f = parsePt(parsed["from"]);
            QPoint t = parsePt(parsed["to"]);

            qDebug() << "Animating move from" << f << "to" << t;
            m_boardWidget->animateMove(f, t);
        } else {
            qDebug() << "Missing 'from' or 'to' in move message";
        }
    } else if (type == "winner") {
        if (parsed.find("color") != parsed.end()) {
            QString winner = QString::fromStdString(parsed["color"]);
            displayWinner(winner);
        }
    }
}

void MainWindow::checkWinner(int whiteCount, int blackCount) {
    if (m_gameOver) {
        return;
    }

    if (whiteCount == 0) {
        displayWinner("black");
    } else if (blackCount == 0) {
        displayWinner("white");
    }
}

void MainWindow::displayWinner(const QString &color) {
    m_gameOver = true;

    QString winnerText;
    if (color.toLower() == "white") {
        winnerText = tr("White is winner!");
    } else if (color.toLower() == "black") {
        winnerText = tr("Black is winner!");
    } else {
        winnerText = tr("Game winner: ") + color;
    }

    m_log->append(winnerText);

    QMessageBox msgBox(QMessageBox::Information, tr("Game over"), winnerText, QMessageBox::Ok, this);
    msgBox.setWindowModality(Qt::WindowModal);
    msgBox.addButton(tr("New game"), QMessageBox::AcceptRole);

    int result = msgBox.exec();
    if (result == QMessageBox::AcceptRole) {
        resetGame();
    } else {
        m_sendBtn->setEnabled(false);
        m_moveEdit->setEnabled(false);
    }
}

void MainWindow::resetGame() {
    m_gameOver = false;

    m_sendBtn->setEnabled(true);
    m_moveEdit->setEnabled(true);

    m_log->append(tr("----------- New Game -----------"));
    m_client->sendRawMessage("{\"type\":\"reset_game\"}");

    m_client->sendRawMessage("{\"type\":\"get_board\"}");
}
