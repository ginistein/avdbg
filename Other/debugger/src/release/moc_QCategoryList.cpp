/****************************************************************************
** Meta object code from reading C++ file 'QCategoryList.h'
**
** Created: Sat Sep 15 17:40:25 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../widgets/QCategoryList.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'QCategoryList.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_QCategoryList[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      20,   15,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_QCategoryList[] = {
    "QCategoryList\0\0item\0"
    "handleMousePress(QTreeWidgetItem*)\0"
};

void QCategoryList::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        QCategoryList *_t = static_cast<QCategoryList *>(_o);
        switch (_id) {
        case 0: _t->handleMousePress((*reinterpret_cast< QTreeWidgetItem*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData QCategoryList::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject QCategoryList::staticMetaObject = {
    { &QTreeWidget::staticMetaObject, qt_meta_stringdata_QCategoryList,
      qt_meta_data_QCategoryList, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &QCategoryList::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *QCategoryList::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *QCategoryList::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_QCategoryList))
        return static_cast<void*>(const_cast< QCategoryList*>(this));
    return QTreeWidget::qt_metacast(_clname);
}

int QCategoryList::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QTreeWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
