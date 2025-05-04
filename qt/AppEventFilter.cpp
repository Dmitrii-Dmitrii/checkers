#include "AppEventFilter.h"
#include <QEvent>

AppEventFilter::AppEventFilter(QObject* app)
    : QObject(app), m_app(app) {}

bool AppEventFilter::eventFilter(QObject* o, QEvent* e) {
    return QObject::eventFilter(o, e);
}