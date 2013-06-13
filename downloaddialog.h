#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include <QDialog>
#include <QtGui>
#include <QFile>

class DownloadDialog : public QDialog
{
    Q_OBJECT

public:
    DownloadDialog(QObject *parent);
    ~DownloadDialog();

    void setStartButtonText(QString str);
    void setStatusText(QString str);

    QString getFileDownloadUrl();
    QString getRtmpString();
    QString getRtmpUrlString();
    QString getApp();
    QString getFlashFileUrl();
    QString getPageUrl();
    QString getPlayPath();
    QString getSaveFileName();
    QString getStatusLabel();

    bool isRTMPStringChecked();
    bool isLiveStreamChecked();

private:
    QRadioButton *fileDownloadRadio;
    QLineEdit *filedownloadEdit;
    QPushButton *downloadFileButton;

    QRadioButton *rtmpStringRadio;
    QLineEdit *rtmpStringEdit;

    QRadioButton *separatedParametersRadio;
    QCheckBox *liveStreamCheckbox;

    QLabel *rtmpUrlLabel;
    QLineEdit *rtmpUrlEdit;
    QLabel *appEditLabel;
    QLineEdit *appEdit;
    QLabel *flashFileUrlLabel;
    QLineEdit *flashUrlEdit;
    QLabel *pageUrlLabel;
    QLineEdit *pageUrlEdit;
    QLabel *playPathLabel;
    QLineEdit *playPathEdit;

    QLabel *fileToSaveLabel;
    QLineEdit *fileNameEdit;
    QPushButton *startRecordingStreamButton;
    QLabel *statusLabel;

};

#endif // DOWNLOADDIALOG_H
