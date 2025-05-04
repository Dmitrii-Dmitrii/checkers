#ifndef MYBUTTON_H
#define MYBUTTON_H


#include <QWidget>
#include <QString>

class MyButton : public QWidget {
    Q_OBJECT
signals:
    void clicked();
public:
    explicit MyButton(QString text, QWidget* parent=nullptr);
    QSize sizeHint() const override;
protected:
    void paintEvent(QPaintEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
private:
    QString m_text;
};



#endif //MYBUTTON_H
