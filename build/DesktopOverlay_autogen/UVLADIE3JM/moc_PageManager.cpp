/****************************************************************************
** Meta object code from reading C++ file 'PageManager.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/PageManager.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PageManager.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_PageManager_t {
    uint offsetsAndSizes[40];
    char stringdata0[12];
    char stringdata1[10];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[5];
    char stringdata5[6];
    char stringdata6[12];
    char stringdata7[7];
    char stringdata8[16];
    char stringdata9[18];
    char stringdata10[17];
    char stringdata11[16];
    char stringdata12[10];
    char stringdata13[5];
    char stringdata14[20];
    char stringdata15[7];
    char stringdata16[16];
    char stringdata17[19];
    char stringdata18[18];
    char stringdata19[3];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_PageManager_t::offsetsAndSizes) + ofs), len
Q_CONSTINIT static const qt_meta_stringdata_PageManager_t qt_meta_stringdata_PageManager = {
    {
        QT_MOC_LITERAL(0, 11),  // "PageManager"
        QT_MOC_LITERAL(12, 9),  // "pageAdded"
        QT_MOC_LITERAL(22, 0),  // ""
        QT_MOC_LITERAL(23, 9),  // "PageData*"
        QT_MOC_LITERAL(33, 4),  // "page"
        QT_MOC_LITERAL(38, 5),  // "index"
        QT_MOC_LITERAL(44, 11),  // "pageRemoved"
        QT_MOC_LITERAL(56, 6),  // "pageId"
        QT_MOC_LITERAL(63, 15),  // "pageNameChanged"
        QT_MOC_LITERAL(79, 17),  // "activePageChanged"
        QT_MOC_LITERAL(97, 16),  // "pageOrderChanged"
        QT_MOC_LITERAL(114, 15),  // "zoneAddedToPage"
        QT_MOC_LITERAL(130, 9),  // "ZoneData*"
        QT_MOC_LITERAL(140, 4),  // "zone"
        QT_MOC_LITERAL(145, 19),  // "zoneRemovedFromPage"
        QT_MOC_LITERAL(165, 6),  // "zoneId"
        QT_MOC_LITERAL(172, 15),  // "zoneDataChanged"
        QT_MOC_LITERAL(188, 18),  // "setActivePageIndex"
        QT_MOC_LITERAL(207, 17),  // "setActivePageById"
        QT_MOC_LITERAL(225, 2)   // "id"
    },
    "PageManager",
    "pageAdded",
    "",
    "PageData*",
    "page",
    "index",
    "pageRemoved",
    "pageId",
    "pageNameChanged",
    "activePageChanged",
    "pageOrderChanged",
    "zoneAddedToPage",
    "ZoneData*",
    "zone",
    "zoneRemovedFromPage",
    "zoneId",
    "zoneDataChanged",
    "setActivePageIndex",
    "setActivePageById",
    "id"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_PageManager[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       8,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   74,    2, 0x06,    1 /* Public */,
       6,    2,   79,    2, 0x06,    4 /* Public */,
       8,    1,   84,    2, 0x06,    7 /* Public */,
       9,    2,   87,    2, 0x06,    9 /* Public */,
      10,    0,   92,    2, 0x06,   12 /* Public */,
      11,    2,   93,    2, 0x06,   13 /* Public */,
      14,    2,   98,    2, 0x06,   16 /* Public */,
      16,    1,  103,    2, 0x06,   19 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      17,    1,  106,    2, 0x0a,   21 /* Public */,
      18,    1,  109,    2, 0x0a,   23 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void, QMetaType::QUuid, QMetaType::Int,    7,    5,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Int,    4,    5,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 12,    4,   13,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QUuid,    4,   15,
    QMetaType::Void, 0x80000000 | 12,   13,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::QUuid,   19,

       0        // eod
};

Q_CONSTINIT const QMetaObject PageManager::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_PageManager.offsetsAndSizes,
    qt_meta_data_PageManager,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_PageManager_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<PageManager, std::true_type>,
        // method 'pageAdded'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PageData *, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'pageRemoved'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QUuid, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'pageNameChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PageData *, std::false_type>,
        // method 'activePageChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PageData *, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'pageOrderChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'zoneAddedToPage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PageData *, std::false_type>,
        QtPrivate::TypeAndForceComplete<ZoneData *, std::false_type>,
        // method 'zoneRemovedFromPage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<PageData *, std::false_type>,
        QtPrivate::TypeAndForceComplete<QUuid, std::false_type>,
        // method 'zoneDataChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<ZoneData *, std::false_type>,
        // method 'setActivePageIndex'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'setActivePageById'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUuid &, std::false_type>
    >,
    nullptr
} };

void PageManager::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PageManager *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->pageAdded((*reinterpret_cast< std::add_pointer_t<PageData*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->pageRemoved((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 2: _t->pageNameChanged((*reinterpret_cast< std::add_pointer_t<PageData*>>(_a[1]))); break;
        case 3: _t->activePageChanged((*reinterpret_cast< std::add_pointer_t<PageData*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 4: _t->pageOrderChanged(); break;
        case 5: _t->zoneAddedToPage((*reinterpret_cast< std::add_pointer_t<PageData*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<ZoneData*>>(_a[2]))); break;
        case 6: _t->zoneRemovedFromPage((*reinterpret_cast< std::add_pointer_t<PageData*>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[2]))); break;
        case 7: _t->zoneDataChanged((*reinterpret_cast< std::add_pointer_t<ZoneData*>>(_a[1]))); break;
        case 8: _t->setActivePageIndex((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->setActivePageById((*reinterpret_cast< std::add_pointer_t<QUuid>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PageManager::*)(PageData * , int );
            if (_t _q_method = &PageManager::pageAdded; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PageManager::*)(QUuid , int );
            if (_t _q_method = &PageManager::pageRemoved; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PageManager::*)(PageData * );
            if (_t _q_method = &PageManager::pageNameChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PageManager::*)(PageData * , int );
            if (_t _q_method = &PageManager::activePageChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (PageManager::*)();
            if (_t _q_method = &PageManager::pageOrderChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (PageManager::*)(PageData * , ZoneData * );
            if (_t _q_method = &PageManager::zoneAddedToPage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (PageManager::*)(PageData * , QUuid );
            if (_t _q_method = &PageManager::zoneRemovedFromPage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (PageManager::*)(ZoneData * );
            if (_t _q_method = &PageManager::zoneDataChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
    }
}

const QMetaObject *PageManager::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PageManager::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PageManager.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PageManager::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void PageManager::pageAdded(PageData * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void PageManager::pageRemoved(QUuid _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void PageManager::pageNameChanged(PageData * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void PageManager::activePageChanged(PageData * _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void PageManager::pageOrderChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void PageManager::zoneAddedToPage(PageData * _t1, ZoneData * _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void PageManager::zoneRemovedFromPage(PageData * _t1, QUuid _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void PageManager::zoneDataChanged(ZoneData * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
