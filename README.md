# Tiny-qORM
The Tiny-qORM is the most simple ORM engine for Qt you've seen last year. Or maybe in your entire life. 
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
And that's all. You made it! Now you have database with `Ur` gadgets. Take the cookie, you deserve it!
Take note that there are no primary key, so every time you insert data you actually insert data. Every time.

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
public:
    QString m_name;
    Mom m_mama;
    Dad m_papa;
    QList<Brother> m_bros;
};
Q_DECLARE_METATYPE(Ur)
```
WOW!!! Such a big family of `Ur`! And now you gonna put them into this tiny SQLite3 database? 
You sick freak... I'm in, let's do it!

1) If your structure have other structures as fields, (smart) pointers or container of structures, you have to add ROWID to your structure.
You can either add `long long orm_rowid` property or inherit the `ORMValue` structure.
Actually, it is better to add ROWIDs into every structure, but 
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
    //        VVVVVVVVVVVVVVVVV
    struct Ur : public ORMValue // ...                              // <<<
```
2) Replace `Q_DECLARE_METATYPE` with `ORM_DECLARE_METATYPE`. `ORM_DECLARE_METATYPE` is actually `Q_DECLARE_METATYPE` inside, but it also generates metadata for pointers and containers.
```C++
ORM_DECLARE_METATYPE(Mom)
ORM_DECLARE_METATYPE(Car)
ORM_DECLARE_METATYPE(Dad)
ORM_DECLARE_METATYPE(Brother)
ORM_DECLARE_METATYPE(Ur)
```
3) Finally replace `qRegisterMetaType` with `registerTypeORM`.
```C++
    registerTypeORM<Ur>("Ur");
    registerTypeORM<Dad>("Dad");
    registerTypeORM<Mom>("Mom");
    registerTypeORM<Brother>("Brother");
    registerTypeORM<Car>("Car");
```
4) Done! Now go back to step 0 and take another coockie. You totally deserve it!

5) Now you might want to improve it a little. Like prevent `Brother::last_combo` property from being saved. Easy. You can hide property from ORM by declaring them `STORABLE false`. ORM does not load data if property is not storable or writable. 

## FAQ:
* Q.: Is it free? What's the license?
* A.: It's as free as love. But speaking in terms of law, let's say, MIT. Is it OK?


* Q.: What types are supported by your ORM?
* A.: 
  *  `+`  Any primitive type. At least I hope so.
  *  `+`  Any registered `Q_ENUM`/`Q_GADGET`/`Q_OBJECT`, `QList`\`QVector` of registered types, (smart)pointers to gadgets.
  *  `+`  Any type with valid `T`->`QVariant`->`T` conversion. Add it to primitive type list with `ORM::addPrimitiveType`
  * `+`/`-` std containers are not supported but easy to add, goto `orm_containers` namespace.
  *  `-`  Static arrays and pointers to arrays will be never supported.
  *  `-`  Associative containers and pairs are not supported. It is possible to add all necessary checkups, but I can't figure out how to turn it into table\value.
  *  `-`  Classes without Qt meta are not supported. Obviously.


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
GCC/MinGW/Clang? Good luck!


* Q.: I used it on a big ass file and ran out of heap memory.
* A.: High five! Me too. Now, how about make your file smaller? By, let's see, hmm, multiplying them?


* Q.: Your code sucks and you should burn in hell.
* A.: `Q > /dev/null`
