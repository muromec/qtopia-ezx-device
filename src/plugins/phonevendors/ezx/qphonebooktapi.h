#ifndef QPHONEBOOKTAPI_H
#define QPHONEBOOKTAPI_H

#include <QPhoneBook>

class QPhoneBookTapi : public QPhoneBook
{
    Q_OBJECT
public:
    QPhoneBookTapi( const QString& service, QObject *parent );
    ~QPhoneBookTapi();

public slots:
    void getEntries( const QString& store );
    void add( const QPhoneBookEntry& entry, const QString& store, bool flush );
    void remove( uint index, const QString& store, bool flush );
    void update( const QPhoneBookEntry& entry, const QString& store, bool flush );
    void flush( const QString& store );
    void setPassword( const QString& store, const QString& passwd );
    void clearPassword( const QString& store );
    void requestLimits( const QString& store );
    void requestFixedDialingState();
    void setFixedDialingState( bool enabled, const QString& pin2 );

private:
    QList<QPhoneBookEntry> ents;
    bool fixedDialingEnabled;
};

#endif
