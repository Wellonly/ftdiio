
#include <QDebug>

#include <QFileInfo>
#include <QElapsedTimer>
#include <QTextCodec>
#include <QDateTime>
#include <QScrollArea>
#include <QProcess>

#include <unistd.h>
#ifdef linux
#include <linux/capability.h>
#include <sys/syscall.h>
#endif

//#include <string.h>

#include "zvvlib.h"
#include "LogTree.h"
#include "ZvvConstants.h"
#include "c_cpp_macros.h"

//using namespace zvvlib;
namespace zvvlib {


Except::Except(const QString& message, Except::SEVERITY severity)
:eMessage(message)
,eSeverity(severity)
{
}

Except::~Except()
{
}

WatchDogOnDestroying::WatchDogOnDestroying(WatchDogCallback callback, QObject *parent)
: QObject(parent)
, m_callback(callback)
{
}
WatchDogOnDestroying::~WatchDogOnDestroying()
{
    if (m_callback) {
        m_callback();
    }
}
WatchDogCallback WatchDogOnDestroying::callback() const
{
    return m_callback;
}
void WatchDogOnDestroying::setCallback(const WatchDogCallback &callback)
{
    m_callback = callback;
}

//QByteArray FileToByteArray(QString FName, const Except &ok)
//{
//    QFile file(FName);
//    if (!file.open(QIODevice::ReadOnly)) {
//        ok = new Except(QString("Warnings|Couldn't open file: %1 ; Error: %2").arg(FName).arg(file.errorString())); //Add2logWarning(tr("Warnings|Couldn't open file: %1").arg(FName));
//        return QByteArray();
//    }
//    QByteArray retv = file.readAll();
//    if (!file.errorString().isEmpty()) ok = new Except(file.errorString());
//    return retv;
//}

QIcon getIcon(const QString &icon)
{ //TODO: a class with icons collection!!!
    return QIcon(QString(ICON_PATH).arg(icon)); //":/icons/PNG/%1.png";
}

//zvv: GridLayout needed for a QDockWidget
QGridLayout *GridLayout(QDockWidget *dockWidget)
{
    QGridLayout *gridLayout = new QGridLayout; //Debug it for mem leak & compare with VBoxLayout()
    QWidget *newwidget = new QWidget(dockWidget);
    newwidget->setLayout(gridLayout);
    dockWidget->setWidget(newwidget);
    return gridLayout;
}

QVBoxLayout *VBoxLayout(QDockWidget *dockWidget)
{
    QWidget *newwidget = new QWidget/*(dockWidget)*/;
    QVBoxLayout *vBoxLayout = new QVBoxLayout(newwidget);
    dockWidget->setWidget(newwidget);
    return vBoxLayout;
}

QVBoxLayout *ScrolledVBoxLayout(QDockWidget *dockWidget)
{
    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    QWidget *workPage = new QWidget(scrollArea);
    QVBoxLayout* vboxLayout = new QVBoxLayout;
    workPage->setLayout(vboxLayout);
    scrollArea->setWidget(workPage);
    dockWidget->setWidget(scrollArea);
    return vboxLayout;
}

QString FileSuffix_set(QString filename, const QString& newSuffix_withoutDot)
{
    int pindex = filename.lastIndexOf('.');
    if (pindex == -1) return filename.append('.').append(newSuffix_withoutDot);
    return filename.replace(pindex+1, filename.length()-pindex, newSuffix_withoutDot);
}

bool BackupFileWithSuffix(QFile *originalFile, const QString& suffix)
{
    QString bakfilename = zvvlib::FileSuffix_set(originalFile->fileName(), suffix);
    if (QFile::exists(bakfilename)) QFile::remove(bakfilename);
    return originalFile->copy(bakfilename);
}

///BackupFile() returns Backup file name is success or empty QString() on fail...
QString BackupFile(const QString &targetFName)
{
    if (QFile::exists(targetFName)) {
        QString backupFName = BackupFileName(targetFName);
        if (!backupFName.isEmpty() && QFile::copy(targetFName, backupFName)) return backupFName;
    }
    return QString();
}

///BackupFileName() returns QString() if one tousand tries(1try ~ 1mS) failed...
QString BackupFileName(const QString &targetFName)
{
    int iters = 0;
    while (iters++ < 999) {
        QString ret = QString(targetFName + QDateTime::currentDateTime().toString(BACKUP_FILE_POSTFIX));
        if (!QFile::exists(ret)) return ret;
        Delay_mS(1);
    }
    return QString();
}

void Delay_mS(int value_mS)
{
    QElapsedTimer timer;
    timer.start();
    while (!timer.hasExpired(value_mS)) {}
}

QList<char*> AppArguments(int argc, char **argv)
{
    static QList<char*> app_arguments_list;
    if (argc && app_arguments_list.length() == 0) {
        for (int i=0; i<argc; ++i) {
            app_arguments_list << argv[i];
        }
    }
//    app_arguments_list.append("default.cfg"); //debug only
    return app_arguments_list;
}

//return 10 for decimal, 16 for hex, 0 if undefined
int BaseOfNumber(QString /*number*/)
{ //TODO: see desc for base param in QString::toUShort()!!!
    return 0;
}

#ifdef linux
int getCapability(uint64_t* effective, uint64_t* permitted, uint64_t* inheritable) {
    static struct __user_cap_header_struct cap_header_struct;
    cap_user_header_t cap_user_header = &cap_header_struct;
    static struct __user_cap_data_struct cap_data_struct;
    cap_user_data_t cap_user_data = &cap_data_struct;
    cap_header_struct.version = _LINUX_CAPABILITY_VERSION_3;
    cap_header_struct.pid = getpid();
    if (syscall(SYS_capget, cap_user_header, cap_user_data) < 0) {
        return errno;
    }
    else {
        if (effective) *effective = (uint64_t)cap_user_data->effective;
        if (permitted) *permitted = (uint64_t)cap_user_data->permitted;
        if (inheritable) *inheritable = (uint64_t)cap_user_data->inheritable;
//        qDebug() << "effective:" << cap_user_data->effective << "permitted:" << cap_user_data->permitted << "inheritable" << cap_user_data->inheritable;
    }
    return 0;
}
#endif

int runProcess(const QString& processName, QStringList params/* = QStringList()*/, int timeout_mS/* = 3000*/, bool isOutputEnable/* = false*/)
{
    QProcess proc;
    if (!isOutputEnable) {
        proc.setStandardOutputFile(QProcess::nullDevice());
        proc.setStandardErrorFile(QProcess::nullDevice());
    }
    proc.start(processName, params);
    proc.waitForFinished(timeout_mS);
    return iif(proc.exitStatus() == QProcess::NormalExit, proc.exitCode(), -3);
}

//QVBoxLayout *VBoxScrollLayout(QDockWidget *dockWidget)
//{
//    QWidget *newwidget = new QWidget(dockWidget);
//    QVBoxLayout *vBoxLayout = new QVBoxLayout(newwidget);
//    newwidget->setLayout(vBoxLayout);
//    dockWidget->setWidget(newwidget);
//    return vBoxLayout;
//}

//QJsonDocument FileToJsonDocument(QString FName)
//{

//}

} //namespace zvvlib

