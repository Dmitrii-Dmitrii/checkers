#include "BoardWidget.h"
#include <QPainter>
#include <QDebug>

#define LABEL_MARGIN 20

BoardWidget::BoardWidget(QWidget* parent)
    : QWidget(parent)
    , m_board(8, std::vector<Piece>(8, Piece::Empty))
    , m_animationFrom(-1, -1)
    , m_animationTo(-1, -1)
    , m_animationProgress(0.0f)
    , m_animating(false)
{
    m_animationTimer.setInterval(16);
    connect(&m_animationTimer, &QTimer::timeout, [this]() {
        m_animationProgress += 0.05f;
        if (m_animationProgress >= 1.0f) {
            m_animationProgress = 0.0f;
            m_animating = false;
            m_animationTimer.stop();
        }
        update();
    });

    setMinimumSize(400 + LABEL_MARGIN * 2, 400 + LABEL_MARGIN * 2);

    initializeBoard();
}

void BoardWidget::initializeBoard() {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            m_board[row][col] = Piece::Empty;
        }
    }

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 8; ++col) {
            if ((row + col) % 2 == 1) {
                m_board[row][col] = Piece::Black;
            }
        }
    }

    for (int row = 5; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if ((row + col) % 2 == 1) {
                m_board[row][col] = Piece::White;
            }
        }
    }

    update();
}

void BoardWidget::setBoard(const std::vector<std::vector<Piece>>& board) {
    m_board = board;
    update();
}

void BoardWidget::animateMove(const QPoint& from, const QPoint& to) {
    m_animationFrom = from;
    m_animationTo = to;
    m_animationProgress = 0.0f;
    m_animating = true;
    m_animationTimer.start();
}

void BoardWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawCoordinates(painter);

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            QRect rect = cellRect(row, col);

            if ((row + col) % 2 == 0) {
                painter.fillRect(rect, QColor(255, 206, 158));
            } else {
                painter.fillRect(rect, QColor(209, 139, 71));
            }
        }
    }

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (!(m_animating && row == m_animationFrom.y() && col == m_animationFrom.x())) {
                drawPiece(painter, row, col, m_board[row][col]);
            }
        }
    }

    if (m_animating) {
        float x = m_animationFrom.x() + (m_animationTo.x() - m_animationFrom.x()) * m_animationProgress;
        float y = m_animationFrom.y() + (m_animationTo.y() - m_animationFrom.y()) * m_animationProgress;

        int size = cellSize();
        int centerX = LABEL_MARGIN + size * (x + 0.5);
        int centerY = LABEL_MARGIN + size * (y + 0.5);

        Piece piece = m_board[m_animationTo.y()][m_animationTo.x()];
        QRect rect(centerX - size/2, centerY - size/2, size, size);

        drawPieceInRect(painter, rect, piece);
    }
}

void BoardWidget::drawCoordinates(QPainter& painter) {
    int size = cellSize();
    painter.setPen(Qt::black);
    QFont font = painter.font();
    font.setBold(true);
    painter.setFont(font);

    for (int col = 0; col < 8; ++col) {
        QRect rect(LABEL_MARGIN + col * size, 0, size, LABEL_MARGIN);
        painter.drawText(rect, Qt::AlignCenter, QString::number(col));

        rect.moveTop(LABEL_MARGIN + 8 * size);
        painter.drawText(rect, Qt::AlignCenter, QString::number(col));
    }

    for (int row = 0; row < 8; ++row) {
        QRect rect(0, LABEL_MARGIN + row * size, LABEL_MARGIN, size);
        painter.drawText(rect, Qt::AlignCenter, QString::number(row));

        rect.moveLeft(LABEL_MARGIN + 8 * size);
        painter.drawText(rect, Qt::AlignCenter, QString::number(row));
    }
}

int BoardWidget::cellSize() const {
    return qMin(width() - LABEL_MARGIN * 2, height() - LABEL_MARGIN * 2) / 8;
}

QRect BoardWidget::cellRect(int row, int col) const {
    int size = cellSize();
    return QRect(LABEL_MARGIN + col * size, LABEL_MARGIN + row * size, size, size);
}

void BoardWidget::drawPiece(QPainter& painter, int row, int col, Piece piece) {
    if (piece == Piece::Empty) {
        return;
    }

    QRect rect = cellRect(row, col);

    rect.adjust(rect.width()/4, rect.height()/4, -rect.width()/4, -rect.height()/4);

    drawPieceInRect(painter, rect, piece);
}

void BoardWidget::drawPieceInRect(QPainter& painter, const QRect& rect, Piece piece) {
    if (piece == Piece::Empty) {
        return;
    }

    if (piece == Piece::White || piece == Piece::WhiteQueen) {
        painter.setBrush(Qt::white);
        painter.setPen(Qt::black);
        painter.drawEllipse(rect);

        if (piece == Piece::WhiteQueen) {
            QFont font = painter.font();
            font.setBold(true);
            font.setPointSize(rect.height() / 2);
            painter.setFont(font);
            painter.drawText(rect, Qt::AlignCenter, "Q");
        }
    } else {
        painter.setBrush(Qt::black);
        painter.setPen(Qt::white);
        painter.drawEllipse(rect);

        if (piece == Piece::BlackKing) {
            QFont font = painter.font();
            font.setBold(true);
            font.setPointSize(rect.height() / 2);
            painter.setFont(font);
            painter.drawText(rect, Qt::AlignCenter, "K");
        }
    }
}