# Tiny-qORM
Tiny-qORM is the most simple ORM engine for Qt you've seen last year. Or maybe in your entire life. 
It allows you to create and drop tables for class, insert and select data, and update\delete it if class have ROWIDs.
 
No cache, no indexes, no 'where' clause. Just load and save data. No schema, no XML. Versioning is not supported too.
That's why it is VERY easy to use. 

Let's take your sample q_gadget structure:
```C++
         struct Ur {
             Q_GADGET
             Q_PROPERTY(int i MEMBER m_i)
         public:
             int m_i;
         };
         Q_DECLARE_METATYPE(Ur)
 ```
There is 1 simple step to make it work:

0) Create ORM object and use it
```C++
         ORM orm;
         orm.create<Ur>();
         QList<Ur> stff = orm.select<Ur>();
         orm.insert(stff);
```
And that's all. You made it! Now you have database with `Ur` gadgets. Take a cookie, you deserve it!
Take note that there are no primary keys, so every time you call 'insert' you actually insert data. Every time.

But what if you want to use it with more advanced structure? Like this:
```C++
struct Mom {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(She is MEMBER m_is)
public:
    enum She {
        Nice,
        Sweet,
        Beautiful,
        Pretty,
        Cozy,
        Fansy,
        Bear
    }; Q_ENUM(She)
public:
    QString m_name;
    She m_is;
    bool operator !=(Mom const& no) { return m_name != no.m_name; }
};
Q_DECLARE_METATYPE(Mom)

struct Car {
    Q_GADGET
    Q_PROPERTY(double gas MEMBER m_gas)
public:
    double m_gas;
};
Q_DECLARE_METATYPE(Car)

struct Dad {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(Car * car MEMBER m_car)
public:
    QString m_name;
    Car * m_car = nullptr; // lost somewhere
    bool operator !=(Dad const& no) { return m_name != no.m_name; }
};
Q_DECLARE_METATYPE(Dad)

struct Brother {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(int last_combo MEMBER m_lastCombo)
    Q_PROPERTY(int total_punches MEMBER m_totalPunches)
public:
    QString m_name;
    int m_lastCombo;
    int m_totalPunches;
    bool operator !=(Brother const& no) { return m_name != no.m_name; }
    bool operator ==(Brother const& no) { return m_name == no.m_name; }
};
Q_DECLARE_METATYPE(Brother)

struct Ur
{
    Q_GADGET
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(Mom mom MEMBER m_mama)
    Q_PROPERTY(Dad dad MEMBER m_papa)
    Q_PROPERTY(QList<Brother> bros MEMBER m_bros)
    Q_PROPERTY(QList<int> drows MEMBER m_drows)
public:
    QString m_name;
    Mom m_mama;
    Dad m_papa;
    QList<Brother> m_bros;
	QList<int> m_drows;
};
Q_DECLARE_METATYPE(Ur)
```
WOW!!! Such a big family of `Ur`! And now you gonna put them into this tiny SQLite3 database? 
You sick freak... I'm in, let's do it!

1) If your structure have other structures as fields, (smart) pointers or container of structures, you have to add ROWID to your structure (or you will recieve error message instead of table). 
You can either add `long long orm_rowid` property or inherit the `ORMValue` structure.
```C++
    struct Dad
    {
        Q_GADGET
        Q_PROPERTY(long long orm_rowid MEMBER m_own_rowid)              // <<<
        Q_PROPERTY(QString name MEMBER m_name)
        Q_PROPERTY(Car * car MEMBER m_car)
    public:
        long long m_own_rowid;                                          // <<<
        QString m_name;
        Car * m_car = nullptr; // lost somewhere
        bool operator !=(Dad const& no) { return m_name != no.m_name; }
    };
    
    // OR
    
        //        VVVVVVVVVVVVVVVVV
        struct Ur : public ORMValue // ...                              // <<<
```
2) Replace `Q_DECLARE_METATYPE` with `ORM_DECLARE_METATYPE`. `ORM_DECLARE_METATYPE` is `Q_DECLARE_METATYPE` inside, but it also generates metadata for pointers and containers. 
```C++
        ORM_DECLARE_METATYPE(Mom)
        ORM_DECLARE_METATYPE(Car)
        ORM_DECLARE_METATYPE(Dad)
        ORM_DECLARE_METATYPE(Brother)
        ORM_DECLARE_METATYPE(Ur)
```
3) Finally replace `qRegisterMetaType` with `ormRegisterType`.
```C++
        ormRegisterType<Ur>("Ur");
        ormRegisterType<Dad>("Dad");
        ormRegisterType<Mom>("Mom");
        ormRegisterType<Brother>("Brother");
        ormRegisterType<Car>("Car");
```
3.1) If your type is used in smart pointer somewhere and you get `QMetaProperty::read: Unable to handle unregistered datatype 'QSharedPointer<T>' for property 't'` error messages, you should use `ORM_DECLARE_METATYPE_EX` instead of `ORM_DECLARE_METATYPE` AND call `orm_pointers::registerTypePointersEx<T>()` after `ormRegisterType<T>("T");`. 

3.2) If your type is used in `QList`/`QVector`, you should call `orm_containers::registerSequentialContainers` after `ormRegisterType<T>("T");`. For `QPair`/`QMap`/`QHash` use relevant functions.

4) Done! Now go back to step 0 and take another coockie. You totally deserve it!

5) Now you might want to improve it a little. Like prevent `Brother::last_combo` property from being saved. Easy. You can hide property from ORM by declaring them `STORABLE false`. ORM does not load data if property is not storable or writable, and also don't save unreadable ones. You can also add any type to ignore list using `ORM_Config`.

OK, that was `Q_GADGET`, but what about `QObject`? Easy. 

1) Register QObject with `ormRegisterQObject`.
```C++
        ormRegisterQObject<QObject>("QObject");
```
2) Pass pointer types instead of the type itself.
```C++
        ORM orm;
        orm.create<QObject*>();
        QList<QObject*> stff = orm.select<QObject*>();
        orm.insert(stff);
```
3) Another coockie!

4) Don't forget: `ORM` **DO NOT** save parent-child relations for your classes so you have to either create them by yourself or delete your classes manually in destructor. Also ORM works **only** with `QObject` pointers due to Qt design limitations. 

That's all for now.

## FAQ:
* Q.: Is it free? What's the license?
* A.: It's as free as love. But speaking in terms of law, let's say, MIT. Is it OK?


* Q.: What types are supported by your ORM?
* A.: 
  * ` + `  Any primitive type. At least I hope so.
  * ` + `  Most of standatd Qt structures listed in QMetaType (as TEXT or BLOB).
  * ` + `  Any `Q_ENUM` (as int), any `Q_GADGET` (as field or pointer), any `Q_OBJECT` (as pointer).
  * ` + `  Smart pointers (`QSharedPointer` and `std::shared_ptr`) to `Q_GADGET` and `Q_OBJECT`. Weak pointers are ignored.
  * ` + `  Any type with `T`->`QString`->`T` conversion. Add it to primitive type list with `ORM_Config::addPrimitiveStringType`
  * ` + `  Any type with `QDataStream& operator<<(QDataStream&, T const&)` and `QDataStream& operator>>(QDataStream&, T&)`. Add it to primitive type list with `ORM_Config::addPrimitiveRawType`
  * ` + `  `QList`/`QVector`/`QPair`/`QMap`/`QHash` of any type listed above.
  * `+/-`  std containers are not supported but easy to add, goto `orm_containers` namespace.
  * ` - `  Static arrays and pointers to arrays will be never supported.
  * ` - `  QObject fields (i.e. `struct Object { QObject object; };`). One does not simple to call copy constructor for QObject as it is forbiden for them.
  * ` - `  Classes without Qt meta are not supported. Obviously.


* Q.: Your ORM can't do `<X>`.
* A.: Well, maybe. So what?


* Q.: Will you add `<X>`?
* A.: Don't think so.


* Q.: Is it as tiny as you claim?
* A.: Yes and no.
   * It is simple and easy to use, so, yes.
   * It is filled with templates and generates tons of unnecessary data, so, no.


* Q.: I used it on a big ass file and ran out of entries in object file. What to do?
* A.: Add this to .pro file and run qmake.
```
        win32-msvc* {
            QMAKE_CXXFLAGS += /bigobj
        }
```
GCC/MinGW/Clang? You're on your own. Good luck!


* Q.: I used it on a big ass file and ran out of heap memory.
* A.: High five! Me too. Now, how about make your file smaller? By, let's see, hmm, dividing them onto smaller ones?


* Q.: Your code sucks and you should burn in hell.
* A.: `Q > /dev/null`
