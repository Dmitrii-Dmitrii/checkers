#include "PlayerWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>

#include "SimpleJsonParser.h"

PlayerWindow::PlayerWindow(PlayerType type, const QString& serverHost, int serverPort, QWidget *parent)
    : QWidget(parent),
      m_moveEdit(new QLineEdit(this)),
      m_sendBtn(new QPushButton(tr("Make move"), this)),
      m_boardWidget(new BoardWidget(this)),
      m_log(new QTextEdit(this)),
      m_statusLabel(new QLabel(this)),
      m_client(std::make_shared<CheckersClient>(serverHost.toStdString(), serverPort)),
      m_messageThread(nullptr),
      m_gameOver(false),
      m_playerType(type),
      m_myTurn(false) {

    setWindowTitle(m_playerType == PlayerType::White ? tr("White Player") : tr("Black Player"));

    QFont statusFont = m_statusLabel->font();
    statusFont.setBold(true);
    statusFont.setPointSize(12);
    m_statusLabel->setFont(statusFont);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    updateStatus(false);

    auto *topLay = new QHBoxLayout;
    topLay->addWidget(m_moveEdit);
    topLay->addWidget(m_sendBtn);

    auto *mainLay = new QVBoxLayout(this);
    mainLay->addWidget(m_statusLabel);
    mainLay->addLayout(topLay);
    mainLay->addWidget(m_boardWidget, /*stretch=*/1);
    mainLay->addWidget(m_log, /*stretch=*/1);
    setLayout(mainLay);

    m_sendBtn->setEnabled(false);
    m_moveEdit->setEnabled(false);

    if (m_client->connect()) {
        m_messageThread = new MessageThread(m_client, this);
        connect(m_messageThread, &MessageThread::messageReceived, this, &PlayerWindow::onMessageReceived);
        m_messageThread->start();

        m_client->sendRawMessage("{\"type\":\"get_board\"}");
        std::string color = m_playerType == PlayerType::White ? "white" : "black";
        m_client->sendRawMessage("{\"type\":\"player_connect\",\"color\":\"" + color + "\"}");
    } else {
        m_log->append(tr("Unable to connect to the server."));
        m_sendBtn->setEnabled(false);
    }

    connect(m_sendBtn, &QPushButton::clicked, this, &PlayerWindow::onSendClicked);
}

PlayerWindow::~PlayerWindow() {
    if (m_messageThread) {
        m_messageThread->stop();
        m_messageThread->wait();
    }
    m_client->disconnect();
}

void PlayerWindow::onSendClicked() {
    if (m_gameOver) {
        m_log->append(tr("Game over."));
        m_moveEdit->clear();
        return;
    }

    if (!m_myTurn) {
        m_log->append(tr("It's not your turn."));
        m_moveEdit->clear();
        return;
    }

    auto txt = m_moveEdit->text().toStdString();
    m_client->sendRawMessage(txt);
    m_moveEdit->clear();

    m_sendBtn->setEnabled(false);
    m_moveEdit->setEnabled(false);
    updateStatus(false);
}

bool PlayerWindow::isMyTurn(const QString &currentPlayer) {
    if (currentPlayer.toLower() == "white" && m_playerType == PlayerType::White)
        return true;
    if (currentPlayer.toLower() == "black" && m_playerType == PlayerType::Black)
        return true;
    return false;
}

void PlayerWindow::updateStatus(bool myTurn) {
    m_myTurn = myTurn;
    if (m_gameOver) {
        m_statusLabel->setText(tr("Game over"));
        m_statusLabel->setStyleSheet("color: gray;");
    } else if (myTurn) {
        m_statusLabel->setText(tr("Your turn"));
        m_statusLabel->setStyleSheet("color: green;");
    } else {
        m_statusLabel->setText(tr("Opponent's turn"));
        m_statusLabel->setStyleSheet("color: red;");
    }
}

void PlayerWindow::onMessageReceived(const QString &msg) {
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
    } else if (type == "current_move") {
        if (parsed.find("player") != parsed.end()) {
            QString currentPlayer = QString::fromStdString(parsed["player"]);
            bool myTurn = (currentPlayer == "W" && m_playerType == PlayerType::White) ||
                          (currentPlayer == "B" && m_playerType == PlayerType::Black);
            
            updateStatus(myTurn);
            m_sendBtn->setEnabled(myTurn);
            m_moveEdit->setEnabled(myTurn);
            
            if (myTurn) {
                m_log->append(tr("Your turn now."));
            } else {
                m_log->append(tr("Waiting for opponent's move."));
            }
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
    } else if (type == "invalid_move") {
        m_log->append(tr("Invalid move. Try again."));
        if (m_playerType == PlayerType::White || m_playerType == PlayerType::Black) {
            m_sendBtn->setEnabled(true);
            m_moveEdit->setEnabled(true);
            updateStatus(true);
        }
    }
}

void PlayerWindow::checkWinner(int whiteCount, int blackCount) {
    if (m_gameOver) {
        return;
    }

    if (whiteCount == 0) {
        displayWinner("black");
    } else if (blackCount == 0) {
        displayWinner("white");
    }
}

void PlayerWindow::displayWinner(const QString &color) {
    m_gameOver = true;

    QString winnerText;
    if (color.toLower() == "white") {
        winnerText = m_playerType == PlayerType::White ? 
                    tr("You win!") : tr("White is winner! You lose.");
    } else if (color.toLower() == "black") {
        winnerText = m_playerType == PlayerType::Black ? 
                    tr("You win!") : tr("Black is winner! You lose.");
    } else {
        winnerText = tr("Game winner: ") + color;
    }

    m_log->append(winnerText);
    updateStatus(false);

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

void PlayerWindow::resetGame() {
    m_gameOver = false;

    m_sendBtn->setEnabled(false);
    m_moveEdit->setEnabled(false);
    updateStatus(false);

    m_log->clear();
    m_log->append(tr("----------- New Game -----------"));
    m_client->sendRawMessage("{\"type\":\"reset_game\"}");
    m_client->sendRawMessage("{\"type\":\"get_board\"}");
}