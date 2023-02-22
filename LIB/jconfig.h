#ifndef JCONFIG_H
#define JCONFIG_H

#include <QObject>
#include <QFile>
#include <QString>
#include <QMetaProperty>
#include <QJsonDocument>
#include <QJsonValue>
#include <QVariant>

#ifndef NO_EXCEPTIONS
#include "zvvlib.h"
#endif

#ifndef DEFAULT_CONFIG_FNAME
#define DEFAULT_CONFIG_FNAME "config.json"
#endif

enum JConfigStatus {
    cfgStatusOk = 0,
    cfgStatusError = 1,
};

typedef QMap<int, QPair<QString, QVariant>> IndexedMap;
typedef QMapIterator<int, QPair<QString, QVariant>> IndexedMapIterator;

class JConfig : public QObject
{
    Q_OBJECT
public:
    JConfig(QObject *parent = 0, const char *FName = NULL, bool isCreateIfNotExist = true);
    JConfig(QObject *parent = 0, const QString &fname = QString(DEFAULT_CONFIG_FNAME), bool isCreateIfNotExist = true);

    virtual ~JConfig() {}

#ifndef NO_EXCEPTIONS
    zvvlib::Except *lastException() const;
#else
    QString exceptionString;
#endif

    QString lastErrorString();
    QString fileName();

    bool save(QVariant target);
    bool save(QVariant fromvar, const QString& folder);
    bool readToMap(QVariantMap &tomap, const QString& folder = QString());
    bool readToIndexedMap(IndexedMap &tomap, const QString& folder);

    bool readToJsonObject(QJsonObject &jobject, const QString& folder = QString());
    bool readToJsonDocument(QJsonDocument &jdoc);

    bool readToDynamicProperties(QVariant &tovar, const QString& folder = QString());
    bool readToDynamicProperties(QVariantMap &tomap, const QString& folder = QString());

    bool mapToObjectDynamicProperties(QVariantMap *sourcemap, QObject *tovar);

    bool readToUserProperties(QObject* toobj, const QMetaObject *metaobject, const QString& folder = QString());

    int saveObjectPropertyList(const QObjectList& object_list, const QStringList& property_list);
    int readToObjectPropertyList(const QObjectList& object_list, const QStringList& property_list);

signals:

public slots:

private:
    QFile *file;
#ifndef NO_EXCEPTIONS
    zvvlib::Except *except;
#else
    void* except;
#endif

    bool openFile(QIODevice::OpenMode opmod);
    bool setException(const QString& circumstance, const QString& errorStr);
    bool setException(const QString& circumstance, const QString& errorStr, int offset);
    bool setException(const QString& mess);
    bool clearException();
    bool saveFile(QVariantMap & vmap);
    void file_create();

    bool mapToObjectDynamicProperties_fromFolder(QVariantMap *sourcemap, QVariant *tovar, const QString& folder);
    bool namedMapToObjectDynamicProperties(QVariantMap *sourcemap, QObject *tovar, const QString& name);
    bool mapToObjectUserProperties(QVariantMap *sourcemap, QObject* toobj, const QMetaObject *metaobject, const QString& folder = QString());

    QVariantMap saveObjectToMap(QVariantMap *tomap, QObject *fromvar);
    QVariantMap saveObjectToMapFolder(QVariantMap *tomap, QVariant *fromvar, QStringRef folder);
};

#endif // JCONFIG_H
