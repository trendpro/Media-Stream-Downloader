#include "downloadworker.h"

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QtNetwork/QNetworkAccessManager>
#include <QList>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QIODevice>
#include <QDir>
#include <QMessageBox>

DownloadWorker::DownloadWorker(QObject *parent)
    : QThread(parent)
{
    m_abort = false;
    networkMgr = new QNetworkAccessManager(this);
    connect(networkMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));

    connect(this, SIGNAL(mySignal()), this, SLOT(mySlot()));
 }

DownloadWorker::~DownloadWorker()
{
   m_abort = true;
   wait();
}

void DownloadWorker::Execute()
{
    m_abort = false;
    start();
}

void DownloadWorker::run()
{
    emit mySignal();
}

/*
 * This slot runs in the Worker Thread.
 */
void DownloadWorker::mySlot()
{
    //QStringList m_urlList;
    //m_urlList.append("http://www.dogcenteronline.com/scientific_cal/Smart_Scientific_Calculator.exe");
    //append(m_urlList);

    startNextDownload(QUrl::fromEncoded(currentDownloadURL.toLocal8Bit()));

    exec();
}

/*
 * Checks whether the Downloads Directory exists, if not it creates them.
 */
void DownloadWorker::setUpDownloadsLocation()
{
    QDir dir = QDir::home();
    if (!dir.cd("qDownloads")) {
        dir.mkdir("qDownloads");
    }
}

void DownloadWorker::setCurrentDownloadURL(QString url)
{
    //Make it THread Safe.
    currentDownloadURL = url;
}

void DownloadWorker::append(const QStringList &urlList)
{
    foreach (QString url, urlList)
    {
         startNextDownload(QUrl::fromEncoded(url.toLocal8Bit()));
    }
 }

void DownloadWorker::startNextDownload(const QUrl &url)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = networkMgr->get(request);
    currentDownloads.append(reply);
}

bool DownloadWorker::saveToDisk(const QString &filename, QIODevice *data)
{
    setUpDownloadsLocation();

    QDir dir = QDir::home();
    dir.cd("qDownloads");//Poor Programming....
    QString homeDir = dir.path();

    QFile file(homeDir+"/"+filename);
    if (!file.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "Could not open %s for writing: %s\n",
                qPrintable(filename),
                qPrintable(file.errorString()));
        return false;
    }

    file.write(data->readAll());
    file.close();

    return true;
}

void DownloadWorker::downloadFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();
    if (reply->error()) {
        fprintf(stderr, "Download of %s failed: %s\n",
                url.toEncoded().constData(),
                qPrintable(reply->errorString()));

            //Notify user when download fails.
            QMessageBox msgBox;
            QString fName = url.toEncoded().constData();
            msgBox.setText("Download of "+fName+" Failed!");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
    } else {
        QString filename = saveFileName(url);
        if (saveToDisk(filename, reply))
            printf("Download of %s succeeded (saved to %s)\n",
                   url.toEncoded().constData(), qPrintable(filename));

            //Notify user when download is complete
            QMessageBox msgBox;
            msgBox.setText("Download of "+filename+" complete!");
            msgBox.setIcon(QMessageBox::Information);
            msgBox.exec();
    }

     reply->deleteLater();
     currentDownloads.removeAll(reply);
     if (currentDownloads.isEmpty())
     {

         this->exit();
     }

}

QString DownloadWorker::saveFileName(const QUrl &url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    if (basename.isEmpty())
        basename = "download";

    if (QFile::exists(basename)) {
        // already exists, don't overwrite
        int i = 0;
        basename += '.';
        while (QFile::exists(basename + QString::number(i)))
            ++i;

        basename += QString::number(i);
    }

    return basename;
}
