#include "MessageThread.h"
#include <QDebug>
#include <chrono>

MessageThread::MessageThread(std::shared_ptr<CheckersClient> client, QObject *parent)
    : QThread(parent),
      m_client(client),
      m_running(false),
      m_lastMessageIndex(0)
{
}

MessageThread::~MessageThread()
{
    stop();
    wait();
}

void MessageThread::stop()
{
    m_running = false;
}

void MessageThread::run()
{
    m_running = true;
    
    while (m_running && m_client && m_client->isConnected()) {
        const auto &msgs = m_client->getReceivedMessages();
        
        QMutexLocker locker(&m_mutex);
        for (size_t i = m_lastMessageIndex; i < msgs.size(); ++i) {
            QString message = QString::fromStdString(msgs[i]);
            Q_EMIT messageReceived(message);
        }
        m_lastMessageIndex = msgs.size();
        locker.unlock();

        QThread::msleep(50);
    }
    
    qDebug() << "Message thread stopping";
}