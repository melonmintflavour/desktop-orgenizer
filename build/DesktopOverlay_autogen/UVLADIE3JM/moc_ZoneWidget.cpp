/****************************************************************************
** Meta object code from reading C++ file 'ZoneWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/ZoneWidget.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ZoneWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_ZoneWidget_t {
    uint offsetsAndSizes[28];
    char stringdata0[11];
    char stringdata1[20];
    char stringdata2[1];
    char stringdata3[13];
    char stringdata4[5];
    char stringdata5[4];
    char stringdata6[7];
    char stringdata7[5];
    char stringdata8[6];
    char stringdata9[8];
    char stringdata10[9];
    char stringdata11[11];
    char stringdata12[12];
    char stringdata13[5];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_ZoneWidget_t::offsetsAndSizes) + ofs), len
Q_CONSTINIT static const qt_meta_stringdata_ZoneWidget_t qt_meta_stringdata_ZoneWidget = {
    {
        QT_MOC_LITERAL(0, 10),  // "ZoneWidget"
        QT_MOC_LITERAL(11, 19),  // "removeZoneRequested"
        QT_MOC_LITERAL(31, 0),  // ""
        QT_MOC_LITERAL(32, 12),  // "ResizeRegion"
        QT_MOC_LITERAL(45, 4),  // "None"
        QT_MOC_LITERAL(50, 3),  // "Top"
        QT_MOC_LITERAL(54, 6),  // "Bottom"
        QT_MOC_LITERAL(61, 4),  // "Left"
        QT_MOC_LITERAL(66, 5),  // "Right"
        QT_MOC_LITERAL(72, 7),  // "TopLeft"
        QT_MOC_LITERAL(80, 8),  // "TopRight"
        QT_MOC_LITERAL(89, 10),  // "BottomLeft"
        QT_MOC_LITERAL(100, 11),  // "BottomRight"
        QT_MOC_LITERAL(112, 4)   // "Move"
    },
    "ZoneWidget",
    "removeZoneRequested",
    "",
    "ResizeRegion",
    "None",
    "Top",
    "Bottom",
    "Left",
    "Right",
    "TopLeft",
    "TopRight",
    "BottomLeft",
    "BottomRight",
    "Move"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_ZoneWidget[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       1,   21, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   20,    2, 0x08,    1 /* Private */,

 // slots: parameters
    QMetaType::Void,

 // enums: name, alias, flags, count, data
       3,    3, 0x2,   10,   26,

 // enum data: key, value
       4, uint(ZoneWidget::ResizeRegion::None),
       5, uint(ZoneWidget::ResizeRegion::Top),
       6, uint(ZoneWidget::ResizeRegion::Bottom),
       7, uint(ZoneWidget::ResizeRegion::Left),
       8, uint(ZoneWidget::ResizeRegion::Right),
       9, uint(ZoneWidget::ResizeRegion::TopLeft),
      10, uint(ZoneWidget::ResizeRegion::TopRight),
      11, uint(ZoneWidget::ResizeRegion::BottomLeft),
      12, uint(ZoneWidget::ResizeRegion::BottomRight),
      13, uint(ZoneWidget::ResizeRegion::Move),

       0        // eod
};

Q_CONSTINIT const QMetaObject ZoneWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_ZoneWidget.offsetsAndSizes,
    qt_meta_data_ZoneWidget,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_ZoneWidget_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ZoneWidget, std::true_type>,
        // method 'removeZoneRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void ZoneWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ZoneWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->removeZoneRequested(); break;
        default: ;
        }
    }
    (void)_a;
}

const QMetaObject *ZoneWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ZoneWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ZoneWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int ZoneWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
