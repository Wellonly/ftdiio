#ifndef ZVVDEBUG_H
#define ZVVDEBUG_H

//#ifdef __cplusplus
//extern "C" {
//#endif

#include <stdio.h>
#include <stdlib.h>

//#ifdef __cplusplus
//}
//#endif

#include <qapplication.h>
#include <QDebug>
#include <QObject>
#include <QEvent>
//class QMetaCallEvent;

#ifdef USE_LOG_TREE
#include "LogTree.h"
#endif

#ifdef QT_DEBUG
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#define debug_msg(mess) qDebug() << mess
#define debug_func(mess) qDebug() << __func__ << ":" << mess
#ifdef USE_LOG_TREE
#define debug_tree_info(mess) Log::AddDebugInfo(mess)
#else
#define debug_tree_info(mess)
#endif
#else
#define LOG(...) do {} while (0)
#define debug_msg(mess)
#define debug_func(mess)
#define debug_tree_info(mess) /*Log::AddDebugInfo(mess)*/
#endif

#define delay_thread(ticks) {int cou = ticks; while (--cou) {}} /* if ticks == 0 then max delay!!! */


//#include <qobject_p.h>
#include <QMetaObject>
#include <QMetaMethod>
#include <QMetaEnum>

namespace zvvdebug {

// MetaCallWatcher<>() example:
//int main(int argc, char ** argv) {
//  MetaCallWatcher<QApplication> app(argc, argv);
//  ...
//}

QT_BEGIN_NAMESPACE

//#if QT_VERSION < 0x050800
//template <class BaseClass> class MetaCallWatcher : public BaseClass {
//  MetaCallWatcher(int& argc, char** argv) : BaseClass(argc, argv) {}
//  bool notify(QObject * receiver, QEvent * event) {
//    if (event->type() == QEvent::MetaCall) {
//      QMetaCallEvent * mev = static_cast<QMetaCallEvent*>(event);
//      QMetaMethod slot = receiver->metaObject()->method(mev->id());
//      qDebug() << "Metacall:" << receiver << slot.methodSignature();
//    }
//    return BaseClass::notify(receiver, event);
//  }
//};
//#endif

QT_END_NAMESPACE

/// To debug events: override in the widget...
//bool WidgetToDebug::event(QEvent *event)
//{
//    qDebug() << event;
//    return QWidget::event(event);
//}

} //namespace zvvdebug {
#endif // ZVVDEBUG_H
