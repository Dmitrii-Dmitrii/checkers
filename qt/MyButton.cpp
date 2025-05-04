#include "MyButton.h"
#include <QPainter>
#include <QMouseEvent>

MyButton::MyButton(QString text, QWidget* parent)
    : QWidget(parent), m_text(std::move(text)) {}

QSize MyButton::sizeHint() const { return {100, 30}; }

void MyButton::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.drawRect(rect().adjusted(0,0,-1,-1));
    p.drawText(rect(), Qt::AlignCenter, m_text);
}

void MyButton::mouseReleaseEvent(QMouseEvent* e) {
    if (rect().contains(e->pos())) emit clicked();
}