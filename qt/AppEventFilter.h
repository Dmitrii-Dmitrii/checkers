#ifndef APPEVENTFILTER_H
#define APPEVENTFILTER_H

#include <QObject>

class AppEventFilter : public QObject {
public:
    AppEventFilter(QObject* app);
    bool eventFilter(QObject* o, QEvent* e) override;
private:
    QObject* m_app;
};


#endif //APPEVENTFILTER_H
