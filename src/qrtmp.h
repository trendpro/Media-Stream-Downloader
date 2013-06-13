#ifndef QRTMP_H
#define QRTMP_H

#include <QtCore>

struct RTMP_private;

class QRtmp : public QThread
{
    Q_OBJECT
signals:
    void error(const QString &desc);
    void readData(const QByteArray &data);
    void streamStarted();
    void streamStopped();

public:
    enum Proto { Undefined, RTMP, RTMPE, RTMPT, RTMPS, RTMPTE, RTMPTS, RTMFP };
    enum RTMP_LogLevel { RTMP_LOGCRIT=0, RTMP_LOGERROR, RTMP_LOGWARNING, RTMP_LOGINFO, RTMP_LOGDEBUG, RTMP_LOGDEBUG2, RTMP_LOGALL };

    static RTMP_LogLevel logLevel();
    static void setLogLevel(RTMP_LogLevel level);

    QRtmp();
    ~QRtmp();

    void start();
    void start(const QString &rtmpString);
    bool isError() const { return !m_error.isEmpty(); }
    void stop() { m_stop = true; }
    void stopBlocking(int timeoutSec = 10); // 10 sec default
    bool isStreamRunning() const { return m_streamIsRunning; }

    // dont emit readData signal, but save instead all data in a file
    // pass a QString() to revert to default behaviour (readData signal)
    void setDestFilename(const QString &filename) { m_destFile.setFileName(filename); }
    QString destFilename() const { return m_destFile.fileName(); }

    // --swfhash|-w hexstring  SHA256 hash of the decompressed SWF file (32 bytes)
    void setSwfHash(const QByteArray &hash, bool isHex = true);
    const QByteArray & swfHash() const { return m_swfHash; }

    // --swfsize|-x num        Size of the decompressed SWF file, required for SWFVerification
    void setSwfSize(quint32 swfSize);
    quint32 swfSize() const { return m_swfSize; }

    // --swfVfy|-W url         URL to player swf file, compute hash/size automatically
    void setSwfUrl(const QString &url);
    QString swfUrl() const { return QString(m_swfUrl); }

    // --swfAge|-X days        Number of days to use cached SWF hash before refreshing
    void setSwfAge(int age);
    int swfAge() const { return m_swfAge; }

    // --skip|-k num           Skip num keyframes when looking for last keyframe to resume from. Useful if resume fails (default: 0)
    void setSkipKeyFrames(int nSkipKeyFrames);
    int skipKeyFrames() const { return m_nSkipKeyFrames; }

    // --buffer|-b             Buffer time in milliseconds (default: 10 hours)
    void setBufferTime(quint32 time) { m_bufferTime = time; m_bOverrideBufferTime = true; }
    quint32 bufferTime() const { return m_bufferTime; }

    // --live|-v               Save a live stream, no --resume (seeking) of live streams possible
    void setLiveStream(bool live) { m_bLiveStream = live; }
    bool liveStream() const { return m_bLiveStream; }

    // --subscribe|-d string   Stream name to subscribe to (otherwise defaults to playpath if live is specifed)
    void setSubscripePath(const QString &path) { m_subscribepath = path.toLocal8Bit(); }
    QString subscribePath() const { return QString(m_subscribepath); }

    // --host|-n hostname      Overrides the hostname in the rtmp url
    void setHostname(const QString &hostname) { m_hostname = hostname.toLocal8Bit(); }
    QString hostname() const { return QString(m_hostname); }

    // --port|-c port          Overrides the port in the rtmp url
    void setPort(int port) { m_port = port; }
    int port() const { return m_port; }

    // --protocol|-l num       Overrides the protocol in the rtmp url (0 - RTMP, 2 - RTMPE)
    void setProtocol(Proto proto) { m_proto = proto; }
    Proto proto() const { return m_proto; }

    // --playpath|-y path      Overrides the playpath parsed from rtmp url
    void setPlaypath(const QString &playpath) { m_playpath = playpath.toLocal8Bit(); }
    QString playpath() const { return QString(m_playpath); }

    // --playlist|-Y           Set playlist before playing
    void setPlaylist();

    // --rtmp|-r url           URL (e.g. rtmp://host[:port]/path)
    void setRtmpUrl(const QString &url);

    // --flashVer|-f string    Flash version string
    void setFlashVer(const QString &flashVer) { m_flashVer = flashVer.toLocal8Bit(); }
    QString flashVer() const { return QString(m_flashVer); }

    // --tcUrl|-t url          URL to played stream (default: \"rtmp://host[:port]/app\")
    void setTcUrl(const QString &tcUrl) { m_tcUrl = tcUrl.toLocal8Bit(); }
    QString tcUrl() const { return QString(m_tcUrl); }

    // --pageUrl|-p url        Web URL of played programm
    void setPageUrl(const QString &pageUrl) { m_pageUrl = pageUrl.toLocal8Bit(); }
    QString pageUrl() const { return QString(m_pageUrl); }

    // --app|-a app            Name of target app on server
    void setApp(const QString &app) { m_app = app.toLocal8Bit(); }
    QString app() const { return QString(m_app); }

    // --auth|-u string        Authentication string to be appended to the connect string
    void setAuth(const QString &auth) { m_auth = auth.toLocal8Bit(); }
    QString auth() const { return QString(m_auth); }

    // --conn|-C type:data     Arbitrary AMF data to be appended to the connect string
    void setConnData(const QByteArray &data);

    // --timeout|-m num        Timeout connection num seconds (default: 30 sec)
    void setTimeout(long int timeout) { m_timeout = timeout; }
    long int timeout() const { return m_timeout; }

    // --start|-A num          Start at num seconds into stream (not valid when using --live)
    // --stop|-B num           Stop at num seconds into stream
    void setStartOffset(int sec) { dStartOffset = sec * 1000; }
    int startOffset() const { return dStartOffset / 1000; }
    void setStopOffset(int sec) { dStopOffset = sec * 1000; }
    int stopOffset() const { return dStopOffset / 1000; }

    // --token|-T key          Key for SecureToken response
    void setSecureToken(const QByteArray &data);

    // --socks|-S host:port    Use the specified SOCKS proxy
    void setProxy(const QString &proxy) { m_proxy = proxy.toLocal8Bit(); }
    QString proxy() const { return QString(m_proxy); }

    // --jtv|-j JSON           Authentication token for Justin.tv legacy servers
    void setUsherToken(const QByteArray &usherToken) { m_usherToken = usherToken; }
    QString usherToken() const { return QString(m_usherToken); }

    qint64 duration() const;

private:
    // RTMP parameters
    RTMP_private *m_rtmp;
    quint32 m_swfSize;
    int m_swfAge;
    int m_nSkipKeyFrames;
    quint32 m_bufferTime;
    bool m_bOverrideBufferTime;
    bool m_bLiveStream;
    int m_port;
    Proto m_proto;
    long int m_timeout;
    quint32 dStartOffset, dStopOffset, dSeek;
    bool m_bResume;
    QByteArray m_swfHash;
    QByteArray m_swfUrl;
    QByteArray m_subscribepath;
    QByteArray m_hostname;
    QByteArray m_playpath;
    QByteArray m_app;
    QByteArray m_flashVer;
    QByteArray m_tcUrl;
    QByteArray m_pageUrl;
    QByteArray m_auth;
    QByteArray m_connData;
    QByteArray m_secureToken;
    QByteArray m_proxy;
    QByteArray m_usherToken;
    QByteArray m_rtmpString;

    // other parameters
    QString m_error;
    bool m_stop;
    double m_percent, m_duration;
    QFile m_destFile;
    bool m_streamIsRunning;

private:
    static int rtmpProto(Proto p);
    static Proto qrtmpProto(int p);
    void setError(const QString &errorDesc);
    int download();
    void setStreamIsRunning(bool isRunning);

private:
    static bool m_socketsInitialized;

protected:
    void run();
};

#endif // QRTMP_H
