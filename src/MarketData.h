#ifndef MARKETDATA_H
#define MARKETDATA_H

#include <QString>
#include <QDateTime>
#include <QMetaType>
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>

struct Tick {
    QString symbol;
    double price;
    qint64 timestamp; // Milliseconds since epoch
};
Q_DECLARE_METATYPE(Tick);

struct Candle {
    qint64 timestamp;
    double open;
    double high;
    double low;
    double close;
    double volume;
};
Q_DECLARE_METATYPE(Candle);

// Thread-safe queue for processing incoming ticks
template <typename T>
class ThreadSafeQueue {
public:
    void push(const T& value) {
        QMutexLocker locker(&mutex);
        queue.enqueue(value);
        condition.wakeOne();
    }

    bool tryPop(T& value) {
        QMutexLocker locker(&mutex);
        if (queue.isEmpty()) {
            return false;
        }
        value = queue.dequeue();
        return true;
    }
    
    // Blocking pop with timeout
    bool waitAndPop(T& value, unsigned long time = ULONG_MAX) {
        QMutexLocker locker(&mutex);
        if (queue.isEmpty()) {
            if (!condition.wait(&mutex, time)) {
                return false;
            }
        }
        if (queue.isEmpty()) return false; // Spurious wake
        value = queue.dequeue();
        return true;
    }

private:
    QQueue<T> queue;
    QMutex mutex;
    QWaitCondition condition;
};

#endif // MARKETDATA_H
