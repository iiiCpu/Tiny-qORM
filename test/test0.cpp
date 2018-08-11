#include "test0.h"

#include "orm.h"

bool Test_QtStructures::operator !=(const Test_QtStructures &o) const {
    //return
    bool ref = false;
    if (m_QChar                 != o.m_QChar                  ) { qDebug() << m_QChar                 << o.m_QChar                 ; ref = true; }
    if (m_QString               != o.m_QString                ) { qDebug() << m_QString               << o.m_QString               ; ref = true; }
    if (m_QStringList           != o.m_QStringList            ) { qDebug() << m_QStringList           << o.m_QStringList           ; ref = true; }
    if (m_QByteArray            != o.m_QByteArray             ) { qDebug() << m_QByteArray            << o.m_QByteArray            ; ref = true; }
    if (m_QBitArray             != o.m_QBitArray              ) { qDebug() << m_QBitArray             << o.m_QBitArray             ; ref = true; }
    if (m_QDate                 != o.m_QDate                  ) { qDebug() << m_QDate                 << o.m_QDate                 ; ref = true; }
    if (m_QTime                 != o.m_QTime                  ) { qDebug() << m_QTime                 << o.m_QTime                 ; ref = true; }
    if (m_QDateTime             != o.m_QDateTime              ) { qDebug() << m_QDateTime             << o.m_QDateTime             ; ref = true; }
    if (m_QUrl                  != o.m_QUrl                   ) { qDebug() << m_QUrl                  << o.m_QUrl                  ; ref = true; }
    if (m_QLocale               != o.m_QLocale                ) { qDebug() << m_QLocale               << o.m_QLocale               ; ref = true; }
    if (m_QRect                 != o.m_QRect                  ) { qDebug() << m_QRect                 << o.m_QRect                 ; ref = true; }
    if (m_QRectF                != o.m_QRectF                 ) { qDebug() << m_QRectF                << o.m_QRectF                ; ref = true; }
    if (m_QSize                 != o.m_QSize                  ) { qDebug() << m_QSize                 << o.m_QSize                 ; ref = true; }
    if (m_QSizeF                != o.m_QSizeF                 ) { qDebug() << m_QSizeF                << o.m_QSizeF                ; ref = true; }
    if (m_QLine                 != o.m_QLine                  ) { qDebug() << m_QLine                 << o.m_QLine                 ; ref = true; }
    if (m_QLineF                != o.m_QLineF                 ) { qDebug() << m_QLineF                << o.m_QLineF                ; ref = true; }
    if (m_QPoint                != o.m_QPoint                 ) { qDebug() << m_QPoint                << o.m_QPoint                ; ref = true; }
    if (m_QPointF               != o.m_QPointF                ) { qDebug() << m_QPointF               << o.m_QPointF               ; ref = true; }
    if (m_QRegExp               != o.m_QRegExp                ) { qDebug() << m_QRegExp               << o.m_QRegExp               ; ref = true; }
    if (m_QEasingCurve          != o.m_QEasingCurve           ) { qDebug() << m_QEasingCurve          << o.m_QEasingCurve          ; ref = true; }
    if (m_QUuid                 != o.m_QUuid                  ) { qDebug() << m_QUuid                 << o.m_QUuid                 ; ref = true; }
    if (m_QModelIndex           != o.m_QModelIndex            ) { qDebug() << m_QModelIndex           << o.m_QModelIndex           ; ref = true; }
    if (m_QRegularExpression    != o.m_QRegularExpression     ) { qDebug() << m_QRegularExpression    << o.m_QRegularExpression    ; ref = true; }
    if (m_QJsonValue            != o.m_QJsonValue             ) { qDebug() << m_QJsonValue            << o.m_QJsonValue            ; ref = true; }
    if (m_QJsonObject           != o.m_QJsonObject            ) { qDebug() << m_QJsonObject           << o.m_QJsonObject           ; ref = true; }
    if (m_QJsonArray            != o.m_QJsonArray             ) { qDebug() << m_QJsonArray            << o.m_QJsonArray            ; ref = true; }
    if (m_QJsonDocument         != o.m_QJsonDocument          ) { qDebug() << m_QJsonDocument         << o.m_QJsonDocument         ; ref = true; }
    if (m_QPersistentModelIndex != o.m_QPersistentModelIndex  ) { qDebug() << m_QPersistentModelIndex << o.m_QPersistentModelIndex ; ref = true; }
    if (m_QByteArrayList        != o.m_QByteArrayList         ) { qDebug() << m_QByteArrayList        << o.m_QByteArrayList        ; ref = true; }
    if (m_QFont                 != o.m_QFont                  ) { qDebug() << m_QFont                 << o.m_QFont                 ; ref = true; }
    if (m_QPixmap               != o.m_QPixmap                ) { qDebug() << m_QPixmap               << o.m_QPixmap               ; ref = true; }
    if (m_QBrush                != o.m_QBrush                 ) { qDebug() << m_QBrush                << o.m_QBrush                ; ref = true; }
    if (m_QColor                != o.m_QColor                 ) { qDebug() << m_QColor                << o.m_QColor                ; ref = true; }
    if (m_QPalette              != o.m_QPalette               ) { qDebug() << m_QPalette              << o.m_QPalette              ; ref = true; }
    //if (m_QIcon                 != o.m_QIcon                  ) { qDebug() << m_QIcon                 << o.m_QIcon                 ; ref = true; } // QIcon have no operator!=
    if (m_QImage                != o.m_QImage                 ) { qDebug() << m_QImage                << o.m_QImage                ; ref = true; }
    if (m_QPolygon              != o.m_QPolygon               ) { qDebug() << m_QPolygon              << o.m_QPolygon              ; ref = true; }
    if (m_QRegion               != o.m_QRegion                ) { qDebug() << m_QRegion               << o.m_QRegion               ; ref = true; }
    if (m_QBitmap               != o.m_QBitmap                ) { qDebug() << m_QBitmap               << o.m_QBitmap               ; ref = true; }
    if (m_QCursor               != o.m_QCursor                ) { qDebug() << m_QCursor               << o.m_QCursor               ; ref = true; }
    if (m_QKeySequence          != o.m_QKeySequence           ) { qDebug() << m_QKeySequence          << o.m_QKeySequence          ; ref = true; }
    if (m_QPen                  != o.m_QPen                   ) { qDebug() << m_QPen                  << o.m_QPen                  ; ref = true; }
    if (m_QTextLength           != o.m_QTextLength            ) { qDebug() << m_QTextLength           << o.m_QTextLength           ; ref = true; }
    if (m_QTextFormat           != o.m_QTextFormat            ) { qDebug() << m_QTextFormat           << o.m_QTextFormat           ; ref = true; }
    if (m_QMatrix               != o.m_QMatrix                ) { qDebug() << m_QMatrix               << o.m_QMatrix               ; ref = true; }
    if (m_QTransform            != o.m_QTransform             ) { qDebug() << m_QTransform            << o.m_QTransform            ; ref = true; }
    if (m_QMatrix4x4            != o.m_QMatrix4x4             ) { qDebug() << m_QMatrix4x4            << o.m_QMatrix4x4            ; ref = true; }
    if (m_QVector2D             != o.m_QVector2D              ) { qDebug() << m_QVector2D             << o.m_QVector2D             ; ref = true; }
    if (m_QVector3D             != o.m_QVector3D              ) { qDebug() << m_QVector3D             << o.m_QVector3D             ; ref = true; }
    if (m_QVector4D             != o.m_QVector4D              ) { qDebug() << m_QVector4D             << o.m_QVector4D             ; ref = true; }
    if (m_QQuaternion           != o.m_QQuaternion            ) { qDebug() << m_QQuaternion           << o.m_QQuaternion           ; ref = true; }
    if (m_QPolygonF             != o.m_QPolygonF              ) { qDebug() << m_QPolygonF             << o.m_QPolygonF             ; ref = true; }
    if (m_QSizePolicy           != o.m_QSizePolicy            ) { qDebug() << m_QSizePolicy           << o.m_QSizePolicy           ; ref = true; }
    return ref;
}

bool Test_QtStructures::operator ==(const Test_QtStructures &o) const
{
    bool ref = true;
    if (m_QChar                 != o.m_QChar                  ) { qDebug() << m_QChar                 << o.m_QChar                 ; ref = false; }
    if (m_QString               != o.m_QString                ) { qDebug() << m_QString               << o.m_QString               ; ref = false; }
    if (m_QStringList           != o.m_QStringList            ) { qDebug() << m_QStringList           << o.m_QStringList           ; ref = false; }
    if (m_QByteArray            != o.m_QByteArray             ) { qDebug() << m_QByteArray            << o.m_QByteArray            ; ref = false; }
    if (m_QBitArray             != o.m_QBitArray              ) { qDebug() << m_QBitArray             << o.m_QBitArray             ; ref = false; }
    if (m_QDate                 != o.m_QDate                  ) { qDebug() << m_QDate                 << o.m_QDate                 ; ref = false; }
    if (m_QTime                 != o.m_QTime                  ) { qDebug() << m_QTime                 << o.m_QTime                 ; ref = false; }
    if (m_QDateTime             != o.m_QDateTime              ) { qDebug() << m_QDateTime             << o.m_QDateTime             ; ref = false; }
    if (m_QUrl                  != o.m_QUrl                   ) { qDebug() << m_QUrl                  << o.m_QUrl                  ; ref = false; }
    if (m_QLocale               != o.m_QLocale                ) { qDebug() << m_QLocale               << o.m_QLocale               ; ref = false; }
    if (m_QRect                 != o.m_QRect                  ) { qDebug() << m_QRect                 << o.m_QRect                 ; ref = false; }
    if (m_QRectF                != o.m_QRectF                 ) { qDebug() << m_QRectF                << o.m_QRectF                ; ref = false; }
    if (m_QSize                 != o.m_QSize                  ) { qDebug() << m_QSize                 << o.m_QSize                 ; ref = false; }
    if (m_QSizeF                != o.m_QSizeF                 ) { qDebug() << m_QSizeF                << o.m_QSizeF                ; ref = false; }
    if (m_QLine                 != o.m_QLine                  ) { qDebug() << m_QLine                 << o.m_QLine                 ; ref = false; }
    if (m_QLineF                != o.m_QLineF                 ) { qDebug() << m_QLineF                << o.m_QLineF                ; ref = false; }
    if (m_QPoint                != o.m_QPoint                 ) { qDebug() << m_QPoint                << o.m_QPoint                ; ref = false; }
    if (m_QPointF               != o.m_QPointF                ) { qDebug() << m_QPointF               << o.m_QPointF               ; ref = false; }
    if (m_QRegExp               != o.m_QRegExp                ) { qDebug() << m_QRegExp               << o.m_QRegExp               ; ref = false; }
    if (m_QEasingCurve          != o.m_QEasingCurve           ) { qDebug() << m_QEasingCurve          << o.m_QEasingCurve          ; ref = false; }
    if (m_QUuid                 != o.m_QUuid                  ) { qDebug() << m_QUuid                 << o.m_QUuid                 ; ref = false; }
    if (m_QModelIndex           != o.m_QModelIndex            ) { qDebug() << m_QModelIndex           << o.m_QModelIndex           ; ref = false; }
    if (m_QRegularExpression    != o.m_QRegularExpression     ) { qDebug() << m_QRegularExpression    << o.m_QRegularExpression    ; ref = false; }
    if (m_QJsonValue            != o.m_QJsonValue             ) { qDebug() << m_QJsonValue            << o.m_QJsonValue            ; ref = false; }
    if (m_QJsonObject           != o.m_QJsonObject            ) { qDebug() << m_QJsonObject           << o.m_QJsonObject           ; ref = false; }
    if (m_QJsonArray            != o.m_QJsonArray             ) { qDebug() << m_QJsonArray            << o.m_QJsonArray            ; ref = false; }
    if (m_QJsonDocument         != o.m_QJsonDocument          ) { qDebug() << m_QJsonDocument         << o.m_QJsonDocument         ; ref = false; }
    if (m_QPersistentModelIndex != o.m_QPersistentModelIndex  ) { qDebug() << m_QPersistentModelIndex << o.m_QPersistentModelIndex ; ref = false; }
    if (m_QByteArrayList        != o.m_QByteArrayList         ) { qDebug() << m_QByteArrayList        << o.m_QByteArrayList        ; ref = false; }
    if (m_QFont                 != o.m_QFont                  ) { qDebug() << m_QFont                 << o.m_QFont                 ; ref = false; }
    if (m_QPixmap               != o.m_QPixmap                ) { qDebug() << m_QPixmap               << o.m_QPixmap               ; ref = false; }
    if (m_QBrush                != o.m_QBrush                 ) { qDebug() << m_QBrush                << o.m_QBrush                ; ref = false; }
    if (m_QColor                != o.m_QColor                 ) { qDebug() << m_QColor                << o.m_QColor                ; ref = false; }
    if (m_QPalette              != o.m_QPalette               ) { qDebug() << m_QPalette              << o.m_QPalette              ; ref = false; }
  //if (m_QIcon                 != o.m_QIcon                  ) { qDebug() << m_QIcon                 << o.m_QIcon                 ; ref = false; } // QIcon have no operator!=
    if (m_QImage                != o.m_QImage                 ) { qDebug() << m_QImage                << o.m_QImage                ; ref = false; }
    if (m_QPolygon              != o.m_QPolygon               ) { qDebug() << m_QPolygon              << o.m_QPolygon              ; ref = false; }
    if (m_QRegion               != o.m_QRegion                ) { qDebug() << m_QRegion               << o.m_QRegion               ; ref = false; }
    if (m_QBitmap               != o.m_QBitmap                ) { qDebug() << m_QBitmap               << o.m_QBitmap               ; ref = false; }
    if (m_QCursor               != o.m_QCursor                ) { qDebug() << m_QCursor               << o.m_QCursor               ; ref = false; }
    if (m_QKeySequence          != o.m_QKeySequence           ) { qDebug() << m_QKeySequence          << o.m_QKeySequence          ; ref = false; }
    if (m_QPen                  != o.m_QPen                   ) { qDebug() << m_QPen                  << o.m_QPen                  ; ref = false; }
    if (m_QTextLength           != o.m_QTextLength            ) { qDebug() << m_QTextLength           << o.m_QTextLength           ; ref = false; }
    if (m_QTextFormat           != o.m_QTextFormat            ) { qDebug() << m_QTextFormat           << o.m_QTextFormat           ; ref = false; }
    if (m_QMatrix               != o.m_QMatrix                ) { qDebug() << m_QMatrix               << o.m_QMatrix               ; ref = false; }
    if (m_QTransform            != o.m_QTransform             ) { qDebug() << m_QTransform            << o.m_QTransform            ; ref = false; }
    if (m_QMatrix4x4            != o.m_QMatrix4x4             ) { qDebug() << m_QMatrix4x4            << o.m_QMatrix4x4            ; ref = false; }
    if (m_QVector2D             != o.m_QVector2D              ) { qDebug() << m_QVector2D             << o.m_QVector2D             ; ref = false; }
    if (m_QVector3D             != o.m_QVector3D              ) { qDebug() << m_QVector3D             << o.m_QVector3D             ; ref = false; }
    if (m_QVector4D             != o.m_QVector4D              ) { qDebug() << m_QVector4D             << o.m_QVector4D             ; ref = false; }
    if (m_QQuaternion           != o.m_QQuaternion            ) { qDebug() << m_QQuaternion           << o.m_QQuaternion           ; ref = false; }
    if (m_QPolygonF             != o.m_QPolygonF              ) { qDebug() << m_QPolygonF             << o.m_QPolygonF             ; ref = false; }
    if (m_QSizePolicy           != o.m_QSizePolicy            ) { qDebug() << m_QSizePolicy           << o.m_QSizePolicy           ; ref = false; }
    return ref;
}

Test_QtStructures &Test_QtStructures::operator =(const Test_QtStructures &o) {
    m_QChar                 = o.m_QChar                  ;
    m_QString               = o.m_QString                ;
    m_QStringList           = o.m_QStringList            ;
    m_QByteArray            = o.m_QByteArray             ;
    m_QBitArray             = o.m_QBitArray              ;
    m_QDate                 = o.m_QDate                  ;
    m_QTime                 = o.m_QTime                  ;
    m_QDateTime             = o.m_QDateTime              ;
    m_QUrl                  = o.m_QUrl                   ;
    m_QLocale               = o.m_QLocale                ;
    m_QRect                 = o.m_QRect                  ;
    m_QRectF                = o.m_QRectF                 ;
    m_QSize                 = o.m_QSize                  ;
    m_QSizeF                = o.m_QSizeF                 ;
    m_QLine                 = o.m_QLine                  ;
    m_QLineF                = o.m_QLineF                 ;
    m_QPoint                = o.m_QPoint                 ;
    m_QPointF               = o.m_QPointF                ;
    m_QRegExp               = o.m_QRegExp                ;
    m_QEasingCurve          = o.m_QEasingCurve           ;
    m_QUuid                 = o.m_QUuid                  ;
    m_QModelIndex           = o.m_QModelIndex            ;
    m_QRegularExpression    = o.m_QRegularExpression     ;
    m_QJsonValue            = o.m_QJsonValue             ;
    m_QJsonObject           = o.m_QJsonObject            ;
    m_QJsonArray            = o.m_QJsonArray             ;
    m_QJsonDocument         = o.m_QJsonDocument          ;
    m_QPersistentModelIndex = o.m_QPersistentModelIndex  ;
    m_QByteArrayList        = o.m_QByteArrayList         ;
    m_QFont                 = o.m_QFont                  ;
    m_QPixmap               = o.m_QPixmap                ;
    m_QBrush                = o.m_QBrush                 ;
    m_QColor                = o.m_QColor                 ;
    m_QPalette              = o.m_QPalette               ;
    m_QIcon                 = o.m_QIcon                  ;
    m_QImage                = o.m_QImage                 ;
    m_QPolygon              = o.m_QPolygon               ;
    m_QRegion               = o.m_QRegion                ;
    m_QBitmap               = o.m_QBitmap                ;
    m_QCursor               = o.m_QCursor                ;
    m_QKeySequence          = o.m_QKeySequence           ;
    m_QPen                  = o.m_QPen                   ;
    m_QTextLength           = o.m_QTextLength            ;
    m_QTextFormat           = o.m_QTextFormat            ;
    m_QMatrix               = o.m_QMatrix                ;
    m_QTransform            = o.m_QTransform             ;
    m_QMatrix4x4            = o.m_QMatrix4x4             ;
    m_QVector2D             = o.m_QVector2D              ;
    m_QVector3D             = o.m_QVector3D              ;
    m_QVector4D             = o.m_QVector4D              ;
    m_QQuaternion           = o.m_QQuaternion            ;
    m_QPolygonF             = o.m_QPolygonF              ;
    m_QSizePolicy           = o.m_QSizePolicy            ;
    return *this;
}

Test_QtStructures::Test_QtStructures()
{

}

Test_QtStructures::Test_QtStructures(const Test_QtStructures &o) {
    m_QChar                 = o.m_QChar                  ;
    m_QString               = o.m_QString                ;
    m_QStringList           = o.m_QStringList            ;
    m_QByteArray            = o.m_QByteArray             ;
    m_QBitArray             = o.m_QBitArray              ;
    m_QDate                 = o.m_QDate                  ;
    m_QTime                 = o.m_QTime                  ;
    m_QDateTime             = o.m_QDateTime              ;
    m_QUrl                  = o.m_QUrl                   ;
    m_QLocale               = o.m_QLocale                ;
    m_QRect                 = o.m_QRect                  ;
    m_QRectF                = o.m_QRectF                 ;
    m_QSize                 = o.m_QSize                  ;
    m_QSizeF                = o.m_QSizeF                 ;
    m_QLine                 = o.m_QLine                  ;
    m_QLineF                = o.m_QLineF                 ;
    m_QPoint                = o.m_QPoint                 ;
    m_QPointF               = o.m_QPointF                ;
    m_QRegExp               = o.m_QRegExp                ;
    m_QEasingCurve          = o.m_QEasingCurve           ;
    m_QUuid                 = o.m_QUuid                  ;
    m_QModelIndex           = o.m_QModelIndex            ;
    m_QRegularExpression    = o.m_QRegularExpression     ;
    m_QJsonValue            = o.m_QJsonValue             ;
    m_QJsonObject           = o.m_QJsonObject            ;
    m_QJsonArray            = o.m_QJsonArray             ;
    m_QJsonDocument         = o.m_QJsonDocument          ;
    m_QPersistentModelIndex = o.m_QPersistentModelIndex  ;
    m_QByteArrayList        = o.m_QByteArrayList         ;
    m_QFont                 = o.m_QFont                  ;
    m_QPixmap               = o.m_QPixmap                ;
    m_QBrush                = o.m_QBrush                 ;
    m_QColor                = o.m_QColor                 ;
    m_QPalette              = o.m_QPalette               ;
    m_QIcon                 = o.m_QIcon                  ;
    m_QImage                = o.m_QImage                 ;
    m_QPolygon              = o.m_QPolygon               ;
    m_QRegion               = o.m_QRegion                ;
    m_QBitmap               = o.m_QBitmap                ;
    m_QCursor               = o.m_QCursor                ;
    m_QKeySequence          = o.m_QKeySequence           ;
    m_QPen                  = o.m_QPen                   ;
    m_QTextLength           = o.m_QTextLength            ;
    m_QTextFormat           = o.m_QTextFormat            ;
    m_QMatrix               = o.m_QMatrix                ;
    m_QTransform            = o.m_QTransform             ;
    m_QMatrix4x4            = o.m_QMatrix4x4             ;
    m_QVector2D             = o.m_QVector2D              ;
    m_QVector3D             = o.m_QVector3D              ;
    m_QVector4D             = o.m_QVector4D              ;
    m_QQuaternion           = o.m_QQuaternion            ;
    m_QPolygonF             = o.m_QPolygonF              ;
    m_QSizePolicy           = o.m_QSizePolicy            ;
}

QDebug& operator<<(QDebug& dbg, const Test_QtStructures &t)
{
    QDebugStateSaver svr(dbg);
    dbg           << "Test_QtStructures:"      << "\n";
    dbg << "    " << t.m_QChar                 << "\n";
    dbg << "    " << t.m_QString               << "\n";
    dbg << "    " << t.m_QStringList           << "\n";
    dbg << "    " << t.m_QByteArray            << "\n";
    dbg << "    " << t.m_QBitArray             << "\n";
    dbg << "    " << t.m_QDate                 << "\n";
    dbg << "    " << t.m_QTime                 << "\n";
    dbg << "    " << t.m_QDateTime             << "\n";
    dbg << "    " << t.m_QUrl                  << "\n";
    dbg << "    " << t.m_QLocale               << "\n";
    dbg << "    " << t.m_QRect                 << "\n";
    dbg << "    " << t.m_QRectF                << "\n";
    dbg << "    " << t.m_QSize                 << "\n";
    dbg << "    " << t.m_QSizeF                << "\n";
    dbg << "    " << t.m_QLine                 << "\n";
    dbg << "    " << t.m_QLineF                << "\n";
    dbg << "    " << t.m_QPoint                << "\n";
    dbg << "    " << t.m_QPointF               << "\n";
    dbg << "    " << t.m_QRegExp               << "\n";
    dbg << "    " << t.m_QEasingCurve          << "\n";
    dbg << "    " << t.m_QUuid                 << "\n";
    dbg << "    " << t.m_QModelIndex           << "\n";
    dbg << "    " << t.m_QRegularExpression    << "\n";
    dbg << "    " << t.m_QJsonValue            << "\n";
    dbg << "    " << t.m_QJsonObject           << "\n";
    dbg << "    " << t.m_QJsonArray            << "\n";
    dbg << "    " << t.m_QJsonDocument         << "\n";
    dbg << "    " << t.m_QPersistentModelIndex << "\n";
    dbg << "    " << t.m_QByteArrayList        << "\n";
    dbg << "    " << t.m_QFont                 << "\n";
    dbg << "    " << t.m_QPixmap               << "\n";
    dbg << "    " << t.m_QBrush                << "\n";
    dbg << "    " << t.m_QColor                << "\n";
    dbg << "    " << t.m_QPalette              << "\n";
    dbg << "    " << t.m_QIcon                 << "\n";
    dbg << "    " << t.m_QImage                << "\n";
    dbg << "    " << t.m_QPolygon              << "\n";
    dbg << "    " << t.m_QRegion               << "\n";
    dbg << "    " << t.m_QBitmap               << "\n";
    dbg << "    " << t.m_QCursor               << "\n";
    dbg << "    " << t.m_QKeySequence          << "\n";
    dbg << "    " << t.m_QPen                  << "\n";
    dbg << "    " << t.m_QTextLength           << "\n";
    dbg << "    " << t.m_QTextFormat           << "\n";
    dbg << "    " << t.m_QMatrix               << "\n";
    dbg << "    " << t.m_QTransform            << "\n";
    dbg << "    " << t.m_QMatrix4x4            << "\n";
    dbg << "    " << t.m_QVector2D             << "\n";
    dbg << "    " << t.m_QVector3D             << "\n";
    dbg << "    " << t.m_QVector4D             << "\n";
    dbg << "    " << t.m_QQuaternion           << "\n";
    dbg << "    " << t.m_QPolygonF             << "\n";
    dbg << "    " << t.m_QSizePolicy           << "\n";
    return dbg;
}




void test_QtStructures()
{
    qDebug() << "Qt structures serialization test:";
    ormRegisterType<Test_QtStructures>();
    ORM_Config::addPrimitiveRawType<QRect>();

    Test_QtStructures s;

    s.m_QChar                 = QChar                     ('y');
    s.m_QString               = QString                   ("Yo, nigga!");
    s.m_QStringList           = QStringList               (s.m_QString.split(' '));
    s.m_QByteArray            = QByteArray                (s.m_QString.toLatin1());
    s.m_QBitArray             = QBitArray                 (25);
    s.m_QDate                 = QDate                     (QDate::currentDate().addDays(-8));
    s.m_QTime                 = QTime                     (QTime::currentTime().addSecs(33333));
    s.m_QDateTime             = QDateTime                 (QDateTime::currentDateTimeUtc());
    s.m_QUrl                  = QUrl                      ("https://github.com");
    s.m_QLocale               = QLocale                   (QLocale::Arabic, QLocale::Egypt);
    s.m_QRect                 = QRect                     (0, 0, 640, 480);
    s.m_QRectF                = QRectF                    (-320.5, -319.5, 319.5, 320.5);
    s.m_QSize                 = QSize                     (400, 300);
    s.m_QSizeF                = QSizeF                    (4096.5, 4095.6);
    s.m_QLine                 = QLine                     (100, 100, 200, 200);
    s.m_QLineF                = QLineF                    (1.2, 3.4, 5.6, 7.8);
    s.m_QPoint                = QPoint                    (0, 9);
    s.m_QPointF               = QPointF                   (3.14, 3.15);
    s.m_QRegExp               = QRegExp                   ("(0x[a-fA-F0-9]+)");
    s.m_QEasingCurve          = QEasingCurve              ();
    s.m_QUuid                 = QUuid                     (QUuid::createUuid());
    s.m_QModelIndex           = QModelIndex               ();
    s.m_QRegularExpression    = QRegularExpression        ("(0x[a-fA-F0-9]+)");
    s.m_QJsonValue            = QJsonValue                (23);
    s.m_QJsonObject           = QJsonObject               ();
    s.m_QJsonArray            = QJsonArray                ();
    s.m_QJsonDocument         = QJsonDocument             ();
    s.m_QPersistentModelIndex = QPersistentModelIndex     ();
    s.m_QByteArrayList        = QByteArrayList            ();
    s.m_QFont                 = QFont                     ("Courier", 3, 24);
    s.m_QPixmap               = QPixmap                   (400, 300);
    s.m_QBrush                = QBrush                    (QColor(Qt::yellow));
    s.m_QColor                = QColor                    (33, 48, 188);
    s.m_QPalette              = QPalette                  (QColor(Qt::black), QColor(Qt::lightGray), QColor(Qt::white), QColor(Qt::darkBlue), QColor(Qt::blue), QColor(Qt::red), QColor(Qt::cyan));
    s.m_QIcon                 = QIcon                     (s.m_QPixmap);
    s.m_QImage                = QImage                    (320, 240, QImage::Format_ARGB32);
    s.m_QPolygon              = QPolygon                  (s.m_QRect);
    s.m_QRegion               = QRegion                   (s.m_QRect);
    s.m_QBitmap               = QBitmap                   (s.m_QPixmap);
    s.m_QCursor               = QCursor                   (Qt::ArrowCursor);
    s.m_QKeySequence          = QKeySequence              (QKeySequence::Print);
    s.m_QPen                  = QPen                      (Qt::green, 3, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin);
    s.m_QTextLength           = QTextLength               ();
    s.m_QTextFormat           = QTextFormat               (QTextFormat::BlockFormat);
    s.m_QMatrix               = QMatrix                   (1, 0, 0, 1, 1, 1);
    s.m_QTransform            = QTransform                (0, 0, 1, 0, 1, 0, 1, 0, 0);
    s.m_QMatrix4x4            = QMatrix4x4                (1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1);
    s.m_QVector2D             = QVector2D                 (6.4f, 6.4f);
    s.m_QVector3D             = QVector3D                 (1.3f, 2.4f, 3.5f);
    s.m_QVector4D             = QVector4D                 (1.4f, 2.5f, 3.6f, 4.7f);
    s.m_QQuaternion           = QQuaternion               (0.5, 7, 7, 7);
    s.m_QPolygonF             = QPolygonF                 (s.m_QPolygon);
    s.m_QSizePolicy           = QSizePolicy               (QSizePolicy::Expanding, QSizePolicy::Expanding);
    qDebug() << s;

    ORM orm;
    orm.drop<Test_QtStructures>();
    orm.create<Test_QtStructures>();
    orm.insert(s);
    QList<Test_QtStructures> mine = orm.select<Test_QtStructures>();
    qDebug() << mine;
    qDebug() << (s == mine.first());
    qDebug() << "End of test.";
}
