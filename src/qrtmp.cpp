#include "qrtmp.h"
#include "librtmp/rtmp.h"
#include <stdlib.h>
#include <cstring> // for memset
#include "librtmp/log.h"
#ifdef WIN32
#include <winsock2.h>
#endif

//static const AVal av_onMetaData = AVC("onMetaData");
//static const AVal av_duration = AVC("duration");
static const AVal av_conn = AVC("conn");
static const AVal av_token = AVC("token");
static const AVal av_playlist = AVC("playlist");
static const AVal av_true = AVC("true");

#define RD_SUCCESS		0
#define RD_FAILED		1
#define RD_INCOMPLETE		2
#define RD_NO_CONNECT		3

struct RTMP_private : RTMP
{
#define CLEAR(x) memset(&x, 0, sizeof(x))
    RTMP_private()
    {
        CLEAR(*this);
    }
#undef CLEAR
};

inline AVal aval(const QByteArray &data)
{
    AVal ret;
    ret.av_len = data.size();
    ret.av_val = const_cast<char*>(data.data());
    return ret;
}

bool QRtmp::m_socketsInitialized = false;

QRtmp::QRtmp() :
    m_rtmp(new RTMP_private),
    m_swfSize(0),
    m_nSkipKeyFrames(0),
    m_bufferTime(10 * 60 * 60 * 1000), /* 10 hours default */
    m_bOverrideBufferTime(false),
    m_bLiveStream(true),
    m_port(-1),
    m_proto(Undefined),
    m_timeout(30),
    dStartOffset(0), dStopOffset(0), dSeek(0),
    m_bResume(false),
    m_stop(false),
    m_percent(0),
    m_duration(0),
    m_streamIsRunning(false)
{
    if(!m_socketsInitialized)
    {
#ifdef WIN32
      WORD version;
      WSADATA wsaData;

      version = MAKEWORD(1, 1);
      m_socketsInitialized = (WSAStartup(version, &wsaData) == 0);
#else
      m_socketsInitialized = true;
#endif
    }

    RTMP_Init(m_rtmp);
}

QRtmp::~QRtmp()
{
    delete m_rtmp;
}

void QRtmp::setSwfHash(const QByteArray &hash, bool isHex)
{
    QByteArray tmp = isHex ? QByteArray::fromHex(hash) : hash;

    if(tmp.size() != RTMP_SWF_HASHLEN)
        RTMP_Log(RTMP_LOGWARNING, "Couldn't parse swf hash hex string, not hexstring or not %d bytes, ignoring!", RTMP_SWF_HASHLEN);
    else
        m_swfHash = tmp;
}

void QRtmp::setSwfSize(uint32_t swfSize)
{
    if(swfSize <= 0)
        RTMP_Log(RTMP_LOGERROR, "SWF Size must be at least 1, ignoring\n");
    else
        m_swfSize = swfSize;
}

void QRtmp::setSwfUrl(const QString &url)
{
    m_swfUrl = url.toLocal8Bit();
}

void QRtmp::setSwfAge(int age)
{
    if(age < 0)
        RTMP_Log(RTMP_LOGERROR, "SWF Age must be non-negative, ignoring\n");
    else
        m_swfAge = age;
}

void QRtmp::setSkipKeyFrames(int nSkipKeyFrames)
{
    if(nSkipKeyFrames < 0)
    {
        RTMP_Log(RTMP_LOGERROR, "Number of keyframes skipped must be greater or equal zero, using zero!");
        m_nSkipKeyFrames = 0;
    }
    else
        m_nSkipKeyFrames = nSkipKeyFrames;
}

int QRtmp::rtmpProto(Proto p)
{
    switch(p)
    {
    case RTMP:
        return RTMP_PROTOCOL_RTMP;
    case RTMPE:
        return RTMP_PROTOCOL_RTMPE;
    case RTMPT:
        return RTMP_PROTOCOL_RTMPT;
    case RTMPS:
        return RTMP_PROTOCOL_RTMPS;
    case RTMPTE:
        return RTMP_PROTOCOL_RTMPTE;
    case RTMPTS:
        return RTMP_PROTOCOL_RTMPTS;
    case RTMFP:
        return RTMP_PROTOCOL_RTMFP;
    default:
        return RTMP_PROTOCOL_UNDEFINED;
    }
}

void QRtmp::setPlaylist()
{
    RTMP_SetOpt(m_rtmp, &av_playlist, (AVal *)&av_true);
}

void QRtmp::setRtmpUrl(const QString &url)
{
    AVal parsedHost, parsedApp, parsedPlaypath;
    unsigned int parsedPort = 0;
    int parsedProtocol = RTMP_PROTOCOL_UNDEFINED;

    if(!RTMP_ParseURL(url.toLocal8Bit().data(), &parsedProtocol, &parsedHost, &parsedPort, &parsedPlaypath, &parsedApp))
        RTMP_Log(RTMP_LOGWARNING, "Couldn't parse the specified url!");
    else
    {
        // use constructor instead of QByteArray::fromRawData to perform a deep copy
        if(m_hostname.isEmpty())
            m_hostname = QByteArray(parsedHost.av_val, parsedHost.av_len);
        if(m_port == -1)
            m_port = parsedPort;
        if(m_playpath.isEmpty())
            m_playpath = QByteArray(parsedPlaypath.av_val, parsedPlaypath.av_len);
        if(m_app.isEmpty())
            m_app = QByteArray(parsedApp.av_val, parsedApp.av_len);
        if(m_proto == Undefined)
            m_proto = qrtmpProto(parsedProtocol);
    }
}

QRtmp::Proto QRtmp::qrtmpProto(int p)
{
    switch(p)
    {
    case RTMP_PROTOCOL_RTMP:
        return RTMP;
    case RTMP_PROTOCOL_RTMPE:
        return RTMPE;
    case RTMP_PROTOCOL_RTMPT:
        return RTMPT;
    case RTMP_PROTOCOL_RTMPS:
        return RTMPS;
    case RTMP_PROTOCOL_RTMPTE:
        return RTMPTE;
    case RTMP_PROTOCOL_RTMPTS:
        return RTMPTS;
    case RTMP_PROTOCOL_RTMFP:
        return RTMFP;
    default:
        return Undefined;
    }
}

void QRtmp::setConnData(const QByteArray &data)
{
    m_connData = data;
    AVal connData = aval(m_connData);
    if(!RTMP_SetOpt(m_rtmp, &av_conn, &connData))
        RTMP_Log(RTMP_LOGERROR, "Invalid AMF parameter");
}

void QRtmp::setSecureToken(const QByteArray &data)
{
    m_secureToken = data;
    AVal secureToken = aval(m_secureToken);
    RTMP_SetOpt(m_rtmp, &av_token, &secureToken);
}

void QRtmp::setError(const QString &errorDesc)
{
    m_error = errorDesc;
    RTMP_Log(RTMP_LOGERROR, errorDesc.toLocal8Bit().data());
    emit error(m_error);
}

void QRtmp::start(const QString &rtmpString)
{
    m_error.clear();
    m_stop = false;

    if(dStartOffset > 0 && m_bLiveStream)
    {
        RTMP_Log(RTMP_LOGWARNING, "Can't seek in a live stream, ignoring --start option");
        dStartOffset = 0;
    }

    m_rtmpString = rtmpString.toLocal8Bit();
    RTMP_SetupURL(m_rtmp, m_rtmpString.data());

    /* Try to keep the stream moving if it pauses on us */
    if(!m_bLiveStream && !(rtmpProto(m_proto) & RTMP_FEATURE_HTTP))
        m_rtmp->Link.lFlags |= RTMP_LF_BUFX;

    QThread::start();
}

void QRtmp::start()
{
    m_error.clear();
    m_stop = false;

    if(m_hostname.isEmpty())
    {
        setError("You must specify a hostname (--host) or url (-r \"rtmp://host[:port]/playpath\") containing a hostname");
        return;
    }
    if(m_playpath.isEmpty())
    {
        setError("You must specify a playpath (--playpath) or url (-r \"rtmp://host[:port]/playpath\") containing a playpath");
        return;
    }
    if(m_proto == Undefined)
    {
        RTMP_Log(RTMP_LOGWARNING, "You haven't specified a protocol (--protocol) or rtmp url (-r), using default protocol RTMP");
        m_proto = RTMP;
    }
    if(m_port == -1)
    {
        RTMP_Log(RTMP_LOGWARNING, "You haven't specified a port (--port) or rtmp url (-r), using default port");
        m_port = 0;
    }
    int protocol = rtmpProto(m_proto);
    if(m_port == 0)
    {
        if(protocol & RTMP_FEATURE_SSL)
            m_port = 443;
        else if(protocol & RTMP_FEATURE_HTTP)
            m_port = 80;
        else
            m_port = 1935;
    }

    // calculate hash
    {
        unsigned char hash[RTMP_SWF_HASHLEN];
        if(RTMP_HashSWF(m_swfUrl.data(), &m_swfSize, hash, m_swfAge) == 0)
            m_swfHash = QByteArray::fromRawData(reinterpret_cast<const char*>(hash), sizeof(hash));
    }

    if(m_swfHash.isEmpty() && m_swfSize > 0)
    {
        RTMP_Log(RTMP_LOGWARNING, "Ignoring SWF size, supply also the hash with --swfhash");
        m_swfSize = 0;
    }

    if(!m_swfHash.isEmpty() && m_swfSize == 0)
    {
        RTMP_Log(RTMP_LOGWARNING, "Ignoring SWF hash, supply also the swf size  with --swfsize");
        m_swfHash.clear();
    }

    if(m_tcUrl.isEmpty())
        m_tcUrl = QString("%1://%2:%3/%4")
                .arg(RTMPProtocolStringsLower[protocol])
                .arg(QString(m_hostname))
                .arg(m_port)
                .arg(QString(m_app))
                .toLocal8Bit();

    if(dStartOffset > 0 && m_bLiveStream)
    {
        RTMP_Log(RTMP_LOGWARNING, "Can't seek in a live stream, ignoring --start option");
        dStartOffset = 0;
    }

    // usherToken is unavailable for librtmp 2.3
    AVal hostname = aval(m_hostname),
            proxy = aval(m_proxy),
            playpath = aval(m_playpath),
            tcUrl = aval(m_tcUrl),
            swfUrl = aval(m_swfUrl),
            pageUrl = aval(m_pageUrl),
            app = aval(m_app),
            auth = aval(m_auth),
            swfHash = aval(m_swfHash),
            flashVer = aval(m_flashVer),
            subscribePath = aval(m_subscribepath);
    RTMP_SetupStream(m_rtmp, protocol, &hostname, m_port, &proxy, &playpath,
                     &tcUrl, &swfUrl, &pageUrl, &app, &auth,
                     &swfHash, m_swfSize, &flashVer, &subscribePath, /*&aval(m_usherToken),*/
                     dSeek, dStopOffset, m_bLiveStream, m_timeout);

    /* Try to keep the stream moving if it pauses on us */
    if(!m_bLiveStream && !(protocol & RTMP_FEATURE_HTTP))
        m_rtmp->Link.lFlags |= RTMP_LF_BUFX;

    QThread::start();
}

void QRtmp::run()
{
    bool first = true;
    int retries = 0;

    if(!m_destFile.fileName().isEmpty())
        if(!m_destFile.open(QIODevice::WriteOnly))
        {
            setError(QString("Can't open %1 for writing").arg(m_destFile.fileName()));
            return;
        }

    while(!m_stop)
    {
        RTMP_Log(RTMP_LOGDEBUG, "Setting buffer time to: %dms", m_bufferTime);
        RTMP_SetBufferMS(m_rtmp, m_bufferTime);

        if(first)
        {
            first = false;
            RTMP_LogPrintf("Connecting ...\n");

            if(!RTMP_Connect(m_rtmp, NULL))
            {
                setError("RTMP_Connect failed");
                break;
            }

            RTMP_Log(RTMP_LOGINFO, "Connected...");

            // User defined seek offset
            if(dStartOffset > 0)
            {
                // Don't need the start offset if resuming an existing file
                if(m_bResume)
                {
                    RTMP_Log(RTMP_LOGWARNING, "Can't seek a resumed stream, ignoring --start option");
                    dStartOffset = 0;
                }
                else
                    dSeek = dStartOffset;
            }

            // Calculate the length of the stream to still play
            if(dStopOffset > 0)
            {
                // Quit if start seek is past required stop offset
                if(dStopOffset <= dSeek)
                {
                    RTMP_LogPrintf("Already Completed\n");
                    break;
                }
            }

            if(!RTMP_ConnectStream(m_rtmp, dSeek))
            {
                setError("RTMP_ConnectStream failed");
                break;
            }
        }
        else
        {
            if(retries)
            {
                RTMP_Log(RTMP_LOGERROR, "Failed to resume the stream\n\n");
                if(!RTMP_IsTimedout(m_rtmp))
                    setError("RTMP_IsTimedout failed");
                else
                    setError("RTMP_IsTimedout RD_INCOMPLETE");
                break;
            }

            RTMP_Log(RTMP_LOGINFO, "Connection timed out, trying to resume.\n\n");

            /* Did we already try pausing, and it still didn't work? */
            if(m_rtmp->m_pausing == 3)
            {
                /* Only one try at reconnecting... */
                retries = 1;
                dSeek = m_rtmp->m_pauseStamp;
                if(dStopOffset > 0)
                {
                    if(dStopOffset <= dSeek)
                    {
                        RTMP_LogPrintf("Already Completed\n");
                        break;
                    }
                }
                if(!RTMP_ReconnectStream(m_rtmp, dSeek))
                {
                    RTMP_Log(RTMP_LOGERROR, "Failed to resume the stream\n\n");
                    if(!RTMP_IsTimedout(m_rtmp))
                        setError("RTMP_IsTimedout failed");
                    else
                        setError("RTMP_IsTimedout RD_INCOMPLETE");
                    break;
                }
            }
            else if(!RTMP_ToggleStream(m_rtmp))
            {
                RTMP_Log(RTMP_LOGERROR, "Failed to resume the stream\n\n");
                if(!RTMP_IsTimedout(m_rtmp))
                    setError("RTMP_IsTimedout failed");
                else
                    setError("RTMP_IsTimedout RD_INCOMPLETE");
                break;
            }
            m_bResume = true;
        }
        int nStatus = download();
        if(nStatus != RD_INCOMPLETE || !RTMP_IsTimedout(m_rtmp) || m_bLiveStream)
            break;
    }
    RTMP_Log(RTMP_LOGDEBUG, "Closing connection.\n");
    RTMP_Close(m_rtmp);
    if(m_destFile.isOpen())
        m_destFile.close();
    setStreamIsRunning(false);
}

int QRtmp::download()
{
    qint32 now, lastUpdate;
    int bufferSize = 64 * 1024;
    char *buffer = (char *) malloc(bufferSize);
    int nRead = 0;
    off_t size = 0;

    m_rtmp->m_read.timestamp = dSeek;
    m_percent = 0.0;

    if(m_rtmp->m_read.timestamp)
        RTMP_Log(RTMP_LOGDEBUG, "Continuing at TS: %d ms\n", m_rtmp->m_read.timestamp);

    if(m_bLiveStream)
        RTMP_LogPrintf("Starting Live Stream\n");
    else
    {
        // print initial status
        // Workaround to exit with 0 if the file is fully (> 99.9%) downloaded
        if(m_duration > 0)
        {
            if((double) m_rtmp->m_read.timestamp >= (double) m_duration * 999.0)
            {
                RTMP_LogPrintf("Already Completed at: %.3f sec Duration=%.3f sec\n",
                               (double) m_rtmp->m_read.timestamp / 1000.0,
                               (double) m_duration / 1000.0);
                return RD_SUCCESS;
            }
            else
            {
                m_percent = ((double) m_rtmp->m_read.timestamp) / (m_duration * 1000.0) * 100.0;
                m_percent = ((double) (int) (m_percent * 10.0)) / 10.0;
                RTMP_LogPrintf("%s download at: %.3f kB / %.3f sec (%.1f%%)\n",
                               m_bResume ? "Resuming" : "Starting",
                               (double) size / 1024.0, (double) m_rtmp->m_read.timestamp / 1000.0,
                               m_percent);
            }
        }
        else
        {
            RTMP_LogPrintf("%s download at: %.3f kB\n",
                           m_bResume ? "Resuming" : "Starting",
                           (double) size / 1024.0);
        }
    }

    if(dStopOffset > 0)
        RTMP_LogPrintf("For duration: %.3f sec\n", (double) (dStopOffset - dSeek) / 1000.0);

    m_rtmp->m_read.nResumeTS = dSeek;

    now = RTMP_GetTime();
    lastUpdate = now - 1000;
    do
    {
        nRead = RTMP_Read(m_rtmp, buffer, bufferSize);
        //RTMP_LogPrintf("nRead: %d\n", nRead);
        if(nRead > 0)
        {
            if(m_destFile.isOpen())
            {
                if(m_destFile.write(buffer, nRead) != nRead)
                {
                    setError(QString("Can't write to %1 - %2").arg(m_destFile.fileName()).arg(m_destFile.errorString()));
                    m_stop = true;
                }
            }
            else
                emit readData(QByteArray(buffer, nRead));
            m_destFile.flush();
            size += nRead;
            setStreamIsRunning(true);

            //RTMP_LogPrintf("write %dbytes (%.1f kB)\n", nRead, nRead/1024.0);
            if(m_duration <= 0)	// if duration unknown try to get it from the stream (onMetaData)
                m_duration = RTMP_GetDuration(m_rtmp);

            if(m_duration > 0)
            {
                // make sure we claim to have enough buffer time!
                if(!m_bOverrideBufferTime && m_bufferTime < (m_duration * 1000.0))
                {
                    m_bufferTime = (quint32) (m_duration * 1000.0) + 5000;	// extra 5sec to make sure we've got enough

                    RTMP_Log(RTMP_LOGDEBUG, "Detected that buffer time is less than duration, resetting to: %dms", m_bufferTime);
                    RTMP_SetBufferMS(m_rtmp, m_bufferTime);
                    RTMP_UpdateBufferMS(m_rtmp);
                }
                m_percent = ((double) m_rtmp->m_read.timestamp) / (m_duration * 1000.0) * 100.0;
                m_percent = ((double) (int) (m_percent * 10.0)) / 10.0;
                now = RTMP_GetTime();
                if(abs(now - lastUpdate) > 200)
                {
                    RTMP_LogStatus("\r%.3f kB / %.2f sec (%.1f%%)",
                                   (double) size / 1024.0,
                                   (double) (m_rtmp->m_read.timestamp) / 1000.0, m_percent);
                    lastUpdate = now;
                }
            }
            else
            {
                now = RTMP_GetTime();
                if(abs(now - lastUpdate) > 200)
                {
                    RTMP_LogStatus("\r%.3f kB / %.2f sec", (double) size / 1024.0,
                                   (double) (m_rtmp->m_read.timestamp) / 1000.0);
                    lastUpdate = now;
                }
            }
        }
    }while (!m_stop && nRead > -1 && RTMP_IsConnected(m_rtmp) && !RTMP_IsTimedout(m_rtmp));
    free(buffer);
    if (nRead < 0)
        nRead = m_rtmp->m_read.status;

    /* Final status update */
    if(m_duration > 0)
    {
        m_percent = ((double) m_rtmp->m_read.timestamp) / (m_duration * 1000.0) * 100.0;
        m_percent = ((double) (int) (m_percent * 10.0)) / 10.0;
        RTMP_LogStatus("\r%.3f kB / %.2f sec (%.1f%%)",
                       (double) size / 1024.0,
                       (double) (m_rtmp->m_read.timestamp) / 1000.0, m_percent);
    }
    else
    {
        RTMP_LogStatus("\r%.3f kB / %.2f sec", (double) size / 1024.0,
                       (double) (m_rtmp->m_read.timestamp) / 1000.0);
    }

    RTMP_Log(RTMP_LOGDEBUG, "RTMP_Read returned: %d", nRead);

    if(m_bResume && nRead == -2)
    {
        RTMP_LogPrintf("Couldn't resume FLV file, try --skip %d\n\n",
                       m_nSkipKeyFrames + 1);
        return RD_FAILED;
    }

    if(nRead == -3)
        return RD_SUCCESS;

    if((m_duration > 0 && m_percent < 99.9) || m_stop || nRead < 0
            || RTMP_IsTimedout(m_rtmp))
    {
        return RD_INCOMPLETE;
    }

    return RD_SUCCESS;
}

qint64 QRtmp::duration() const
{
    if(!isRunning())
        return -1;
    else
        return m_rtmp->m_read.timestamp;
}

QRtmp::RTMP_LogLevel QRtmp::logLevel()
{
    return static_cast<RTMP_LogLevel>(static_cast<int>(RTMP_LogGetLevel()));
}

void QRtmp::setLogLevel(RTMP_LogLevel level)
{
    RTMP_LogSetLevel((::RTMP_LogLevel)(static_cast<int>(level)));
}

void QRtmp::stopBlocking(int timeoutSec)
{
    if(isRunning())
    {
        QEventLoop waiter;
        connect(this, SIGNAL(finished()), &waiter, SLOT(quit()));
        QTimer::singleShot(timeoutSec * 1000, &waiter, SLOT(quit()));
        m_stop = true;
        waiter.exec();

        if(isRunning()) // timeout exceeded
            RTMP_Log(RTMP_LOGWARNING, "Thread has not finished while timeout has exceeded");
    }
}

void QRtmp::setStreamIsRunning(bool isRunning)
{
    if(isRunning ^ m_streamIsRunning)
    {
        m_streamIsRunning = isRunning;
        if(isRunning)
            emit streamStarted();
        else
            emit streamStopped();
    }
}
