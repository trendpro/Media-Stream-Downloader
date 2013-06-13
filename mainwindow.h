#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QFile>
#include "downloadworker.h"
#include "downloaddialog.h"
#include "src/qrtmp.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    DownloadWorker *fileDownloader;
    DownloadDialog *downloadDialog;
    QMenu *fileMenu;
    QMenu *downloadMenu;
    QMenu *optionsMenu;
    QMenu *helpMenu;

    QRtmp rtmp;
    QFile destFile;

    void createMenus();
    void createMenuItems();
    void wireGuiElements();

public slots:
    void InvokeDownloadThread(QString url);
    void addDownloadURLPushButtonClicked();
    void downloadStreamPushButtonClicked();

    void startStream();
    void rtmpData(const QByteArray &data);
    void rtmpError(const QString &errorDesc);
    void rtmpFinished();

signals:
    void startDownloading(QString str);
    void startRecordingStream();

};

#endif // MAINWINDOW_H
