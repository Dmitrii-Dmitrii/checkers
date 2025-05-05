#ifndef MESSAGETHREAD_H
#define MESSAGETHREAD_H

#include <QThread>
#include <QString>
#include <QMutex>
#include <atomic>
#include <memory>
#include "CheckersClient.h"

class MessageThread : public QThread
{
    Q_OBJECT

public:
    explicit MessageThread(std::shared_ptr<CheckersClient> client, QObject *parent = nullptr);
    ~MessageThread();

    void stop();

    signals:
        void messageReceived(const QString &message);

protected:
    void run() override;

private:
    std::shared_ptr<CheckersClient> m_client;
    std::atomic<bool> m_running;
    QMutex m_mutex;
    size_t m_lastMessageIndex;
};

#endif // MESSAGETHREAD_H