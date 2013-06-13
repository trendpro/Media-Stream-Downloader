#ifndef DOWNLOADWORKER_H
#define DOWNLOADWORKER_H

#include <QThread>
#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QtNetwork/QNetworkAccessManager>
#include <QList>
#include <QStringList>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QIODevice>
#include <QByteArray>

class DownloadWorker : public QThread
{
    Q_OBJECT

public:
    DownloadWorker(QObject *parent);
    ~DownloadWorker();

    void  Execute();
    void setCurrentDownloadURL(QString url);
    void append(const QStringList &urlList);
    bool saveToDisk(const QString &filename, QIODevice *data);
    QString saveFileName(const QUrl &url);
    void setUpDownloadsLocation();

protected:
    void run();

private:
     bool m_abort;
     QList<QNetworkReply *> currentDownloads;
     QFile output;
     QNetworkAccessManager *networkMgr;
     QString currentDownloadURL;

public slots:
    void downloadFinished(QNetworkReply *reply);
    void startNextDownload(const QUrl &url);
    void mySlot();

signals:
    void mySignal();

};

#endif // DOWNLOADWORKER_H
