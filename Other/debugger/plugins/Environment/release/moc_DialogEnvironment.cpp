/****************************************************************************
** Meta object code from reading C++ file 'DialogEnvironment.h'
**
** Created: Sat Sep 15 17:41:17 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../DialogEnvironment.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DialogEnvironment.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DialogEnvironment[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      25,   19,   18,   18, 0x0a,
      64,   18,   18,   18, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DialogEnvironment[] = {
    "DialogEnvironment\0\0index\0"
    "on_listView_doubleClicked(QModelIndex)\0"
    "on_btnRefresh_clicked()\0"
};

void DialogEnvironment::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DialogEnvironment *_t = static_cast<DialogEnvironment *>(_o);
        switch (_id) {
        case 0: _t->on_listView_doubleClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        case 1: _t->on_btnRefresh_clicked(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DialogEnvironment::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DialogEnvironment::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DialogEnvironment,
      qt_meta_data_DialogEnvironment, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DialogEnvironment::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DialogEnvironment::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DialogEnvironment::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DialogEnvironment))
        return static_cast<void*>(const_cast< DialogEnvironment*>(this));
    return QDialog::qt_metacast(_clname);
}

int DialogEnvironment::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE