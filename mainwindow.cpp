#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QInputDialog>
#include <QLineEdit>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    createMenus();
    wireGuiElements();

    downloadDialog = new DownloadDialog(this);
    //connect(downloadDialog->startRecordingStreamButton, SIGNAL(clicked()), this, SLOT(startStream()));
    connect(&rtmp, SIGNAL(error(QString)), this, SLOT(rtmpError(QString)));
    connect(&rtmp, SIGNAL(readData(QByteArray)), this, SLOT(rtmpData(QByteArray)));
    connect(&rtmp, SIGNAL(finished()), this, SLOT(rtmpFinished()));

    QRtmp::setLogLevel(QRtmp::RTMP_LOGINFO);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    downloadMenu = menuBar()->addMenu(tr("&Download"));
    optionsMenu = menuBar()->addMenu(tr("&Options"));
    helpMenu = menuBar()->addMenu(tr("&Help"));
}

void MainWindow::createMenuItems()
{
    //code for creating various menu items.
}

void MainWindow::wireGuiElements()
{
    connect(ui->addURLPushButton,SIGNAL(clicked()),this,SLOT(addDownloadURLPushButtonClicked()));
    connect(ui->downloadPushButton,SIGNAL(clicked()),this,SLOT(downloadStreamPushButtonClicked()));
}

void MainWindow::addDownloadURLPushButtonClicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Download File"),
                                              tr("Enter URL of File to be Downloaded"), QLineEdit::Normal,"", &ok);
         if (ok && !text.isEmpty())
         {
             //Validate URL at this point.
             InvokeDownloadThread(text);
         }

   //In future comment code for real////
}

/*
 * Displays dialog for downloading Media Streams
 */
void MainWindow::downloadStreamPushButtonClicked()
{
    //code for initializing stream downloa dialog
    downloadDialog->show();
}

void MainWindow::InvokeDownloadThread(QString url)
{
    fileDownloader = new DownloadWorker(this);
    fileDownloader->setCurrentDownloadURL(url);
    fileDownloader->Execute();
}

void MainWindow::startStream()
{
    if(!rtmp.isRunning())
    {
        downloadDialog->setStartButtonText("Stop Stream");
        destFile.setFileName(downloadDialog->getSaveFileName());
        destFile.open(QIODevice::WriteOnly);

        if(downloadDialog->isRTMPStringChecked())
            rtmp.start(downloadDialog->getRtmpString());
        else
        {
            //Validate this code Later.
            rtmp.setLiveStream(downloadDialog->isLiveStreamChecked());
            //if(!ui->rtmpUrlEdit->text().isEmpty())
                rtmp.setRtmpUrl(downloadDialog->getRtmpUrlString());
            //if(!ui->appEdit->text().isEmpty())
                rtmp.setApp(downloadDialog->getApp());
            //if(!ui->flashUrlEdit->text().isEmpty())
                rtmp.setSwfUrl(downloadDialog->getFlashFileUrl());
            //if(!ui->pageUrlEdit->text().isEmpty())
                rtmp.setPageUrl(downloadDialog->getPageUrl());
            //if(!ui->playpathEdit->text().isEmpty())
                rtmp.setPlaypath(downloadDialog->getPlayPath());

            rtmp.start();
        }
    }
    else
    {
        rtmp.stop();
        destFile.close();
        downloadDialog->setStartButtonText(tr("Start Stream"));
    }
}

void MainWindow::rtmpData(const QByteArray &data)
{
    destFile.write(data);
}

void MainWindow::rtmpError(const QString &errorDesc)
{
    downloadDialog->setStartButtonText(tr("Start Stream"));
    destFile.close();

    downloadDialog->setStatusText("Got QRtmp error: " + errorDesc);
}

void MainWindow::rtmpFinished()
{
    downloadDialog->setStartButtonText(tr("Start Stream"));
    destFile.close();
}

MainWindow::~MainWindow()
{
    if(rtmp.isRunning())
    {
        QEventLoop waiter;
        connect(&rtmp, SIGNAL(finished()), &waiter, SLOT(quit()));
        rtmp.stop();
        waiter.exec();
    }
    delete ui;
}
