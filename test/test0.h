#ifndef TEST0_H
#define TEST0_H

#include <QMetaType>
#include "orm_def.h"

class QDebug;

void test_QtStructures();

#include <QChar>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <QBitArray>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QUrl>
#include <QLocale>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <QLine>
#include <QLineF>
#include <QPoint>
#include <QPointF>
#include <QRegExp>
#include <QEasingCurve>
#include <QUuid>
#include <QModelIndex>
#include <QRegularExpression>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QPersistentModelIndex>
#include <QByteArrayList>
#include <QFont>
#include <QPixmap>
#include <QBrush>
#include <QColor>
#include <QPalette>
#include <QIcon>
#include <QImage>
#include <QPolygon>
#include <QRegion>
#include <QBitmap>
#include <QCursor>
#include <QKeySequence>
#include <QPen>
#include <QTextLength>
#include <QTextFormat>
#include <QMatrix>
#include <QTransform>
#include <QMatrix4x4>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QQuaternion>
#include <QPolygonF>
#include <QSizePolicy>

struct Test_QtStructures : public ORMValue
{
    Q_GADGET
    Q_PROPERTY(QChar                     m_QChar                 MEMBER m_QChar                 )
    Q_PROPERTY(QString                   m_QString               MEMBER m_QString               )
    Q_PROPERTY(QStringList               m_QStringList           MEMBER m_QStringList           )
    Q_PROPERTY(QByteArray                m_QByteArray            MEMBER m_QByteArray            )
    Q_PROPERTY(QBitArray                 m_QBitArray             MEMBER m_QBitArray             )
    Q_PROPERTY(QDate                     m_QDate                 MEMBER m_QDate                 )
    Q_PROPERTY(QTime                     m_QTime                 MEMBER m_QTime                 )
    Q_PROPERTY(QDateTime                 m_QDateTime             MEMBER m_QDateTime             )
    Q_PROPERTY(QUrl                      m_QUrl                  MEMBER m_QUrl                  )
    Q_PROPERTY(QLocale                   m_QLocale               MEMBER m_QLocale               )
    Q_PROPERTY(QRect                     m_QRect                 MEMBER m_QRect                 )
    Q_PROPERTY(QRectF                    m_QRectF                MEMBER m_QRectF                )
    Q_PROPERTY(QSize                     m_QSize                 MEMBER m_QSize                 )
    Q_PROPERTY(QSizeF                    m_QSizeF                MEMBER m_QSizeF                )
    Q_PROPERTY(QLine                     m_QLine                 MEMBER m_QLine                 )
    Q_PROPERTY(QLineF                    m_QLineF                MEMBER m_QLineF                )
    Q_PROPERTY(QPoint                    m_QPoint                MEMBER m_QPoint                )
    Q_PROPERTY(QPointF                   m_QPointF               MEMBER m_QPointF               )
    Q_PROPERTY(QRegExp                   m_QRegExp               MEMBER m_QRegExp               )
    Q_PROPERTY(QEasingCurve              m_QEasingCurve          MEMBER m_QEasingCurve          )
    Q_PROPERTY(QUuid                     m_QUuid                 MEMBER m_QUuid                 )
    Q_PROPERTY(QModelIndex               m_QModelIndex           MEMBER m_QModelIndex           )
    Q_PROPERTY(QRegularExpression        m_QRegularExpression    MEMBER m_QRegularExpression    )
    Q_PROPERTY(QJsonValue                m_QJsonValue            MEMBER m_QJsonValue            )
    Q_PROPERTY(QJsonObject               m_QJsonObject           MEMBER m_QJsonObject           )
    Q_PROPERTY(QJsonArray                m_QJsonArray            MEMBER m_QJsonArray            )
    Q_PROPERTY(QJsonDocument             m_QJsonDocument         MEMBER m_QJsonDocument         )
    Q_PROPERTY(QPersistentModelIndex     m_QPersistentModelIndex MEMBER m_QPersistentModelIndex )
    Q_PROPERTY(QByteArrayList            m_QByteArrayList        MEMBER m_QByteArrayList        )
    Q_PROPERTY(QFont                     m_QFont                 MEMBER m_QFont                 )
    Q_PROPERTY(QPixmap                   m_QPixmap               MEMBER m_QPixmap               )
    Q_PROPERTY(QBrush                    m_QBrush                MEMBER m_QBrush                )
    Q_PROPERTY(QColor                    m_QColor                MEMBER m_QColor                )
    Q_PROPERTY(QPalette                  m_QPalette              MEMBER m_QPalette              )
  //Q_PROPERTY(QIcon                     m_QIcon                 MEMBER m_QIcon                 )
    Q_PROPERTY(QImage                    m_QImage                MEMBER m_QImage                )
    Q_PROPERTY(QPolygon                  m_QPolygon              MEMBER m_QPolygon              )
    Q_PROPERTY(QRegion                   m_QRegion               MEMBER m_QRegion               )
    Q_PROPERTY(QBitmap                   m_QBitmap               MEMBER m_QBitmap               )
    Q_PROPERTY(QCursor                   m_QCursor               MEMBER m_QCursor               )
    Q_PROPERTY(QKeySequence              m_QKeySequence          MEMBER m_QKeySequence          )
    Q_PROPERTY(QPen                      m_QPen                  MEMBER m_QPen                  )
    Q_PROPERTY(QTextLength               m_QTextLength           MEMBER m_QTextLength           )
    Q_PROPERTY(QTextFormat               m_QTextFormat           MEMBER m_QTextFormat           )
    Q_PROPERTY(QMatrix                   m_QMatrix               MEMBER m_QMatrix               )
    Q_PROPERTY(QTransform                m_QTransform            MEMBER m_QTransform            )
    Q_PROPERTY(QMatrix4x4                m_QMatrix4x4            MEMBER m_QMatrix4x4            )
    Q_PROPERTY(QVector2D                 m_QVector2D             MEMBER m_QVector2D             )
    Q_PROPERTY(QVector3D                 m_QVector3D             MEMBER m_QVector3D             )
    Q_PROPERTY(QVector4D                 m_QVector4D             MEMBER m_QVector4D             )
    Q_PROPERTY(QQuaternion               m_QQuaternion           MEMBER m_QQuaternion           )
    Q_PROPERTY(QPolygonF                 m_QPolygonF             MEMBER m_QPolygonF             )
    Q_PROPERTY(QSizePolicy               m_QSizePolicy           MEMBER m_QSizePolicy           )
public:
    QChar                     m_QChar                 ;
    QString                   m_QString               ;
    QStringList               m_QStringList           ;
    QByteArray                m_QByteArray            ;
    QBitArray                 m_QBitArray             ;
    QDate                     m_QDate                 ;
    QTime                     m_QTime                 ;
    QDateTime                 m_QDateTime             ;
    QUrl                      m_QUrl                  ;
    QLocale                   m_QLocale               ;
    QRect                     m_QRect                 ;
    QRectF                    m_QRectF                ;
    QSize                     m_QSize                 ;
    QSizeF                    m_QSizeF                ;
    QLine                     m_QLine                 ;
    QLineF                    m_QLineF                ;
    QPoint                    m_QPoint                ;
    QPointF                   m_QPointF               ;
    QRegExp                   m_QRegExp               ;
    QEasingCurve              m_QEasingCurve          ;
    QUuid                     m_QUuid                 ;
    QModelIndex               m_QModelIndex           ;
    QRegularExpression        m_QRegularExpression    ;
    QJsonValue                m_QJsonValue            ;
    QJsonObject               m_QJsonObject           ;
    QJsonArray                m_QJsonArray            ;
    QJsonDocument             m_QJsonDocument         ;
    QPersistentModelIndex     m_QPersistentModelIndex ;
    QByteArrayList            m_QByteArrayList        ;
    QFont                     m_QFont                 ;
    QPixmap                   m_QPixmap               ;
    QBrush                    m_QBrush                ;
    QColor                    m_QColor                ;
    QPalette                  m_QPalette              ;
    QIcon                     m_QIcon                 ;
    QImage                    m_QImage                ;
    QPolygon                  m_QPolygon              ;
    QRegion                   m_QRegion               ;
    QBitmap                   m_QBitmap               ;
    QCursor                   m_QCursor               ;
    QKeySequence              m_QKeySequence          ;
    QPen                      m_QPen                  ;
    QTextLength               m_QTextLength           ;
    QTextFormat               m_QTextFormat           ;
    QMatrix                   m_QMatrix               ;
    QTransform                m_QTransform            ;
    QMatrix4x4                m_QMatrix4x4            ;
    QVector2D                 m_QVector2D             ;
    QVector3D                 m_QVector3D             ;
    QVector4D                 m_QVector4D             ;
    QQuaternion               m_QQuaternion           ;
    QPolygonF                 m_QPolygonF             ;
    QSizePolicy               m_QSizePolicy           ;
public:
    Test_QtStructures();
    Test_QtStructures(Test_QtStructures const& o);

    bool operator !=(Test_QtStructures const& o) const;
    bool operator ==(Test_QtStructures const& o) const;
    Test_QtStructures& operator =(Test_QtStructures const& o);
};
ORM_DECLARE_METATYPE(Test_QtStructures)

QDebug& operator<<(QDebug& dbg, Test_QtStructures const& t);

#endif // TEST0_H
