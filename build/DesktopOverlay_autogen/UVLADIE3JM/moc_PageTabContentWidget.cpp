/****************************************************************************
** Meta object code from reading C++ file 'PageTabContentWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/PageTabContentWidget.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PageTabContentWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_PageTabContentWidget_t {
    uint offsetsAndSizes[20];
    char stringdata0[21];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[5];
    char stringdata5[10];
    char stringdata6[9];
    char stringdata7[18];
    char stringdata8[7];
    char stringdata9[22];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_PageTabContentWidget_t::offsetsAndSizes) + ofs), len
Q_CONSTINIT static const qt_meta_stringdata_PageTabContentWidget_t qt_meta_stringdata_PageTabContentWidget = {
    {
        QT_MOC_LITERAL(0, 20),  // "PageTabContentWidget"
        QT_MOC_LITERAL(21, 15),  // "handleZoneAdded"
        QT_MOC_LITERAL(37, 0),  // ""
        QT_MOC_LITERAL(38, 9),  // "PageData*"
        QT_MOC_LITERAL(48, 4),  // "page"
        QT_MOC_LITERAL(53, 9),  // "ZoneData*"
        QT_MOC_LITERAL(63, 8),  // "zoneData"
        QT_MOC_LITERAL(72, 17),  // "handleZoneRemoved"
        QT_MOC_LITERAL(90, 6),  // "zoneId"
        QT_MOC_LITERAL(97, 21)   // "handleZoneDataChanged"
    },
    "PageTabContentWidget",
    "handleZoneAdded",
    "",
    "PageData*",
    "page",
    "ZoneData*",
    "zoneData",
    "handleZoneRemoved",
    "zoneId",
    "handleZoneDataChanged"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_PageTabContentWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   32,    2, 0x0a,    1 /* Public */,
       7,    2,   37,    2, 0x0a,    4 /* Public */,
       9,    1,   42,    2, 0x0a,    7 /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5,    4,    6,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QUuid,    4,    8,
    QMetaType::Void, 0x80000000 | 5,    6,

       0        // eod
};

Q_CONSTINIT const QMetaObject PageTabContentWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_PageTabContentWidget.offsetsAndSizes,
    qt_meta_data_PageTabContentWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_PageTabContentWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<PageTabContentWidget, std::true_type>,
        // method 'handleZoneAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PageData *, std::false_type>,
        QtPrivate::TypeAndForceComplete<ZoneData *, std::false_type>,
        // method 'handleZoneRemoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PageData *, std::false_type>,
        QtPrivate::TypeAndForceComplete<QUuid, std::false_type>,
        // method 'handleZoneDataChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<ZoneData *, std::false_type>
    >,
    nullptr
} };

void PageTabContentWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PageTabContentWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->handleZoneAdded((*reinterpret_cast< std::add_pointer_t<PageData*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<ZoneData*>>(_a[2]))); break;
        case 1: _t->handleZoneRemoved((*reinterpret_cast< std::add_pointer_t<PageData*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[2]))); break;
        case 2: _t->handleZoneDataChanged((*reinterpret_cast< std::add_pointer_t<ZoneData*>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *PageTabContentWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PageTabContentWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PageTabContentWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int PageTabContentWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
