/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/MainWindow.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_MainWindow_t {
    uint offsetsAndSizes[52];
    char stringdata0[11];
    char stringdata1[11];
    char stringdata2[1];
    char stringdata3[20];
    char stringdata4[10];
    char stringdata5[5];
    char stringdata6[6];
    char stringdata7[21];
    char stringdata8[25];
    char stringdata9[7];
    char stringdata10[17];
    char stringdata11[24];
    char stringdata12[24];
    char stringdata13[22];
    char stringdata14[10];
    char stringdata15[9];
    char stringdata16[26];
    char stringdata17[7];
    char stringdata18[22];
    char stringdata19[5];
    char stringdata20[22];
    char stringdata21[15];
    char stringdata22[10];
    char stringdata23[8];
    char stringdata24[21];
    char stringdata25[23];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_MainWindow_t::offsetsAndSizes) + ofs), len
Q_CONSTINIT static const qt_meta_stringdata_MainWindow_t qt_meta_stringdata_MainWindow = {
    {
        QT_MOC_LITERAL(0, 10),  // "MainWindow"
        QT_MOC_LITERAL(11, 10),  // "addNewPage"
        QT_MOC_LITERAL(22, 0),  // ""
        QT_MOC_LITERAL(23, 19),  // "onActivePageChanged"
        QT_MOC_LITERAL(43, 9),  // "PageData*"
        QT_MOC_LITERAL(53, 4),  // "page"
        QT_MOC_LITERAL(58, 5),  // "index"
        QT_MOC_LITERAL(64, 20),  // "onPageAddedToManager"
        QT_MOC_LITERAL(85, 24),  // "onPageRemovedFromManager"
        QT_MOC_LITERAL(110, 6),  // "pageId"
        QT_MOC_LITERAL(117, 16),  // "tabIndexToRemove"
        QT_MOC_LITERAL(134, 23),  // "handleTabCloseRequested"
        QT_MOC_LITERAL(158, 23),  // "handleCurrentTabChanged"
        QT_MOC_LITERAL(182, 21),  // "handleZoneAddedToPage"
        QT_MOC_LITERAL(204, 9),  // "ZoneData*"
        QT_MOC_LITERAL(214, 8),  // "zoneData"
        QT_MOC_LITERAL(223, 25),  // "handleZoneRemovedFromPage"
        QT_MOC_LITERAL(249, 6),  // "zoneId"
        QT_MOC_LITERAL(256, 21),  // "handleZoneDataChanged"
        QT_MOC_LITERAL(278, 4),  // "zone"
        QT_MOC_LITERAL(283, 21),  // "handlePageNameChanged"
        QT_MOC_LITERAL(305, 14),  // "handleTabMoved"
        QT_MOC_LITERAL(320, 9),  // "fromIndex"
        QT_MOC_LITERAL(330, 7),  // "toIndex"
        QT_MOC_LITERAL(338, 20),  // "addZoneToCurrentPage"
        QT_MOC_LITERAL(359, 22)   // "handleTabDoubleClicked"
    },
    "MainWindow",
    "addNewPage",
    "",
    "onActivePageChanged",
    "PageData*",
    "page",
    "index",
    "onPageAddedToManager",
    "onPageRemovedFromManager",
    "pageId",
    "tabIndexToRemove",
    "handleTabCloseRequested",
    "handleCurrentTabChanged",
    "handleZoneAddedToPage",
    "ZoneData*",
    "zoneData",
    "handleZoneRemovedFromPage",
    "zoneId",
    "handleZoneDataChanged",
    "zone",
    "handlePageNameChanged",
    "handleTabMoved",
    "fromIndex",
    "toIndex",
    "addZoneToCurrentPage",
    "handleTabDoubleClicked"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_MainWindow[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   92,    2, 0x08,    1 /* Private */,
       3,    2,   93,    2, 0x08,    2 /* Private */,
       7,    2,   98,    2, 0x08,    5 /* Private */,
       8,    2,  103,    2, 0x08,    8 /* Private */,
      11,    1,  108,    2, 0x08,   11 /* Private */,
      12,    1,  111,    2, 0x08,   13 /* Private */,
      13,    2,  114,    2, 0x08,   15 /* Private */,
      16,    2,  119,    2, 0x08,   18 /* Private */,
      18,    1,  124,    2, 0x08,   21 /* Private */,
      20,    1,  127,    2, 0x08,   23 /* Private */,
      21,    2,  130,    2, 0x08,   25 /* Private */,
      24,    0,  135,    2, 0x08,   28 /* Private */,
      25,    1,  136,    2, 0x08,   29 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 4, QMetaType::Int,    5,    6,
    QMetaType::Void, 0x80000000 | 4, QMetaType::Int,    5,    6,
    QMetaType::Void, QMetaType::QUuid, QMetaType::Int,    9,   10,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, 0x80000000 | 4, 0x80000000 | 14,    5,   15,
    QMetaType::Void, 0x80000000 | 4, QMetaType::QUuid,    5,   17,
    QMetaType::Void, 0x80000000 | 14,   19,
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   22,   23,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    6,

       0        // eod
};

Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MainWindow.offsetsAndSizes,
    qt_meta_data_MainWindow,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_MainWindow_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>,
        // method 'addNewPage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onActivePageChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PageData *, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onPageAddedToManager'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PageData *, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onPageRemovedFromManager'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QUuid, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'handleTabCloseRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'handleCurrentTabChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'handleZoneAddedToPage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PageData *, std::false_type>,
        QtPrivate::TypeAndForceComplete<ZoneData *, std::false_type>,
        // method 'handleZoneRemovedFromPage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PageData *, std::false_type>,
        QtPrivate::TypeAndForceComplete<QUuid, std::false_type>,
        // method 'handleZoneDataChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<ZoneData *, std::false_type>,
        // method 'handlePageNameChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PageData *, std::false_type>,
        // method 'handleTabMoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'addZoneToCurrentPage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'handleTabDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->addNewPage(); break;
        case 1: _t->onActivePageChanged((*reinterpret_cast< std::add_pointer_t<PageData*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 2: _t->onPageAddedToManager((*reinterpret_cast< std::add_pointer_t<PageData*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->onPageRemovedFromManager((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: _t->handleTabCloseRequested((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->handleCurrentTabChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->handleZoneAddedToPage((*reinterpret_cast< std::add_pointer_t<PageData*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<ZoneData*>>(_a[2]))); break;
        case 7: _t->handleZoneRemovedFromPage((*reinterpret_cast< std::add_pointer_t<PageData*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[2]))); break;
        case 8: _t->handleZoneDataChanged((*reinterpret_cast< std::add_pointer_t<ZoneData*>>(_a[1]))); break;
        case 9: _t->handlePageNameChanged((*reinterpret_cast< std::add_pointer_t<PageData*>>(_a[1]))); break;
        case 10: _t->handleTabMoved((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 11: _t->addZoneToCurrentPage(); break;
        case 12: _t->handleTabDoubleClicked((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
