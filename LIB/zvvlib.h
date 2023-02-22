#ifndef ZVVLIB_H
#define ZVVLIB_H

#include "zvvenum.h"
#include "zvvarray.h"
#include "zvvserialize.h"

#ifdef CONFIG_FILE
#include CONFIG_FILE
#endif

#include <QDebug>
#include <QString>
#include <QAction>
#include <QDockWidget>
#include <QGridLayout>
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>
#include <QFile>
#include <QIcon>
#include <QException>
#include <QMetaEnum>
#include <QMetaObject>
#include <QException>
#include <QMap>
#include <QFileDialog>

//#ifndef NULL
//#define NULL 0
//#endif

using namespace std;

//class Smo;

namespace zvvlib {

#ifndef ICON_PATH
#define ICON_PATH ":/icons/png/%1.png"
#endif

#ifndef DateTime_format_full
#define DateTime_format_full "yyyy-MM-dd HH:mm:ss.zzz"
#endif

#ifndef TODO
#define TODO(data) {throw data;}
#endif

#define TODO_NotImplemented TODO("not implemented yet!!!")

#ifndef DateTime_format_full
#define DateTime_format_full "yyyy-MM-dd HH:mm:ss.zzz"
#endif

#define SIZE_OF_MEMBER(cls, member) sizeof (static_cast<cls *>(0)->member) /*not tested!!!*/

//typedef enum SEVERITY {info = 0, warning, error } SEVERITY;

/// based on QException!!!
class Except : public QException
{
public:
    enum SEVERITY {info = 0, warning, error };

    Except(const QString &message, SEVERITY severity = SEVERITY::warning);
    ~Except();

    void raise() const { throw *this; }
    Except* clone() const { return new Except(*this); }

    QString eMessage;
    SEVERITY eSeverity;
};

typedef void (*WatchDogCallback)(void);
class WatchDogOnDestroying : public QObject
{
    Q_OBJECT
public:
    WatchDogOnDestroying(WatchDogCallback callback = nullptr, QObject *parent = nullptr);
    ~WatchDogOnDestroying();

    WatchDogCallback callback() const;
    void setCallback(const WatchDogCallback &callback);

private:
    WatchDogCallback m_callback;
};

// lib functions signatures...
QGridLayout *GridLayout(QDockWidget *dockWidget);
QVBoxLayout *VBoxLayout(QDockWidget *dockWidget);
QVBoxLayout *ScrolledVBoxLayout(QDockWidget *dockWidget);

//QVBoxLayout *VBoxScrollLayout(QDockWidget *dockWidget);
QIcon getIcon(const QString &icon);
//QByteArray FileToByteArray(QString FName, const Except& ok);

//QJsonDocument FileToJsonDocument(QString FName);

QString FileSuffix_set(QString filename, const QString &newSuffix_withoutDot);
bool BackupFileWithSuffix(QFile *originalFile, const QString& suffix = QStringLiteral("bak"));
QString BackupFile(const QString& targetFName);
QString BackupFileName(const QString& targetFName);

void Delay_mS(int value_mS);
QList<char *> AppArguments(int argc = 0, char **argv = 0);
int BaseOfNumber(QString);

int getCapability(uint64_t *effective = NULL, uint64_t *permitted = NULL, uint64_t *inheritable = NULL);
int runProcess(const QString& processName, QStringList params = QStringList(), int timeout_mS = 3000, bool isOutputEnable = false);

///TypeId<T>() returns type id known at compile time!!!
/// usage example: QVariant().userType() == TypeId<MyType>()
template<typename V>
static inline int TypeId(void)
{
    return qMetaTypeId<V>();
}

template<typename V>
static inline const char* TypeName(void)
{
    return QMetaType::typeName(TypeId<V>());
}

template<class T>
void saveToFile(QWidget *parent, T *saveable, const QString& suffix = QString())
{
    QFileDialog fd(parent);
    fd.setAcceptMode(QFileDialog::AcceptSave);
    if (!suffix.isEmpty()) fd.setDefaultSuffix(suffix);
    if (fd.exec() == QDialog::Accepted) {
        saveable->save(fd.selectedFiles().first());
    }
}

template<class T>
int registerMetaType()
{
    int ret = qRegisterMetaType<T>();
    qDebug() << __func__ << QMetaType::typeName(ret) << "; with QMetaTypeId:" << ret;
    return ret;
}

} //namespace zvvlib

//deprecated, include it in app code... using namespace zvvlib;

#endif // ZVVLIB_H
