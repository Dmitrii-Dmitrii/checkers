#pragma once

#include <QWidget>
#include <QTimer>
#include <QPoint>
#include <vector>

enum class Piece {
    Empty,
    White,
    Black,
    WhiteQueen,
    BlackKing
};

class BoardWidget : public QWidget {
    Q_OBJECT

public:
    explicit BoardWidget(QWidget *parent = nullptr);

    void setBoard(const std::vector<std::vector<Piece> > &board);

    void animateMove(const QPoint &from, const QPoint &to);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initializeBoard();

    void drawCoordinates(QPainter &painter);

    int cellSize() const;

    QRect cellRect(int row, int col) const;

    void drawPiece(QPainter &painter, int row, int col, Piece piece);

    void drawPieceInRect(QPainter &painter, const QRect &rect, Piece piece);

    std::vector<std::vector<Piece> > m_board;
    QPoint m_animationFrom;
    QPoint m_animationTo;
    float m_animationProgress;
    bool m_animating;
    QTimer m_animationTimer;
};
