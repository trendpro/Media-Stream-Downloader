#include "downloaddialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

DownloadDialog::DownloadDialog(QObject *parent)
{
    fileDownloadRadio = new QRadioButton(tr("File Url"),this);
    filedownloadEdit = new QLineEdit;
    downloadFileButton = new QPushButton(tr("Download"));
    QHBoxLayout *fileDownloadLayout = new QHBoxLayout;
    fileDownloadLayout->addWidget(fileDownloadRadio);
    fileDownloadLayout->addWidget(filedownloadEdit);
    fileDownloadLayout->addWidget(downloadFileButton);
    fileDownloadLayout->addSpacing(2);


    rtmpStringRadio = new QRadioButton(tr("RTMP String"),this);
    rtmpStringEdit = new QLineEdit;
    QHBoxLayout *rtmpStringLayout = new QHBoxLayout;
    rtmpStringLayout->addWidget(rtmpStringRadio);
    rtmpStringLayout->addWidget(rtmpStringEdit);

    separatedParametersRadio = new QRadioButton(tr("Or Separated Parameters"),this);
    liveStreamCheckbox = new QCheckBox(tr("Live Stream"));

    rtmpUrlLabel = new QLabel(tr("Rtmp Url"));
    rtmpUrlEdit = new QLineEdit;
    appEditLabel = new QLabel(tr("App"));
    appEdit = new QLineEdit;
    flashFileUrlLabel = new QLabel(tr("Flash File Url"));
    flashUrlEdit = new QLineEdit;
    pageUrlLabel = new QLabel(tr("Page Url"));
    pageUrlEdit = new QLineEdit;
    playPathLabel = new QLabel(tr("Play Path"));
    playPathEdit = new QLineEdit;
    QFormLayout *separatedFormLayout = new QFormLayout;
    separatedFormLayout->addRow(rtmpUrlLabel,rtmpUrlEdit);
    separatedFormLayout->addRow(appEditLabel,appEdit);
    separatedFormLayout->addRow(flashFileUrlLabel,flashUrlEdit);
    separatedFormLayout->addRow(pageUrlLabel,pageUrlEdit);
    separatedFormLayout->addRow(playPathLabel,playPathEdit);

    fileToSaveLabel = new QLabel(tr("File to Save"));
    fileNameEdit = new QLineEdit("sample.flv");
    startRecordingStreamButton = new QPushButton(tr("Start Recording"));
    QHBoxLayout *saveFileLayout = new QHBoxLayout;
    saveFileLayout->addWidget(fileToSaveLabel);
    saveFileLayout->addWidget(fileNameEdit);
    saveFileLayout->addWidget(startRecordingStreamButton);

    statusLabel = new QLabel;

    QVBoxLayout *mainDialogLayout = new QVBoxLayout;
    mainDialogLayout->addLayout(fileDownloadLayout);
    mainDialogLayout->addLayout(rtmpStringLayout);
    mainDialogLayout->addWidget(separatedParametersRadio);
    mainDialogLayout->addWidget(liveStreamCheckbox);
    mainDialogLayout->addLayout(separatedFormLayout);
    mainDialogLayout->addLayout(saveFileLayout);
    mainDialogLayout->addWidget(statusLabel);

    setLayout(mainDialogLayout);
    setWindowTitle(tr("Stream Download Dialog"));
    setFixedSize(600,400);

    connect(this->startRecordingStreamButton, SIGNAL(clicked()), parent, SLOT(startStream()));
}

DownloadDialog::~DownloadDialog()
{
    //destruction code here
}

void DownloadDialog::setStartButtonText(QString str)
{
    startRecordingStreamButton->setText(str);
}

void DownloadDialog::setStatusText(QString str)
{
    statusLabel->setText(str);
}

QString DownloadDialog::getFileDownloadUrl()
{
    return filedownloadEdit->text();
}

QString DownloadDialog::getRtmpString()
{
    return rtmpStringEdit->text();
}

QString DownloadDialog::getRtmpUrlString()
{
    return rtmpUrlEdit->text();
}

QString DownloadDialog::getApp()
{
    return appEdit->text();
}

QString DownloadDialog::getFlashFileUrl()
{
    return flashUrlEdit->text();
}

QString DownloadDialog::getPageUrl()
{
    return pageUrlEdit->text();
}

QString DownloadDialog::getPlayPath()
{
    return playPathEdit->text();
}

QString DownloadDialog::getSaveFileName()
{
    return fileNameEdit->text();
}

QString DownloadDialog::getStatusLabel()
{
    return statusLabel->text();
}

bool DownloadDialog::isRTMPStringChecked()
{
    return rtmpStringRadio->isChecked();
}

bool DownloadDialog::isLiveStreamChecked()
{
    return liveStreamCheckbox->isChecked();
}
