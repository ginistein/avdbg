/****************************************************************************
** Meta object code from reading C++ file 'DialogReferences.h'
**
** Created: Sat Sep 15 17:41:28 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../DialogReferences.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DialogReferences.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DialogReferences[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      18,   17,   17,   17, 0x05,

 // slots: signature, parameters, type, tag, flags
      38,   17,   17,   17, 0x0a,
      64,   59,   17,   17, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DialogReferences[] = {
    "DialogReferences\0\0updateProgress(int)\0"
    "on_btnFind_clicked()\0item\0"
    "on_listWidget_itemDoubleClicked(QListWidgetItem*)\0"
};

void DialogReferences::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DialogReferences *_t = static_cast<DialogReferences *>(_o);
        switch (_id) {
        case 0: _t->updateProgress((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->on_btnFind_clicked(); break;
        case 2: _t->on_listWidget_itemDoubleClicked((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DialogReferences::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DialogReferences::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DialogReferences,
      qt_meta_data_DialogReferences, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DialogReferences::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DialogReferences::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DialogReferences::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DialogReferences))
        return static_cast<void*>(const_cast< DialogReferences*>(this));
    return QDialog::qt_metacast(_clname);
}

int DialogReferences::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void DialogReferences::updateProgress(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
