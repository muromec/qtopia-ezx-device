/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PHONESERVERTAPIMODEM_H
#define PHONESERVERTAPIMODEM_H

#include <QModemService>
#include <qphonecallprovider.h>
#include <qphonebook.h>
#include <qnetworkregistration.h>
#include <qsiminfo.h>
#include <qphonerffunctionality.h>
#include <qpinmanager.h>
#include <qvibrateaccessory.h>

//#include "tapi.h"

class QPhoneCallTapi : public QPhoneCallImpl
{
    Q_OBJECT
public:
    QPhoneCallTapi( QPhoneCallProvider *provider, const QString& identifier,
                     const QString& callType );
    ~QPhoneCallTapi();

    void dial( const QDialOptions& options );
    void hangup( QPhoneCall::Scope scope );
    void accept();
    void hold();
    void activate( QPhoneCall::Scope scope );
    void join( bool detachSubscriber );
    void tone( const QString& tones );
    void transfer( const QString& number );

private slots:
    void dialTimeout();
    void tapi_fd();
};

class  EZXVibrateAccessory : public QVibrateAccessoryProvider
{
    Q_OBJECT
public:
     EZXVibrateAccessory( QModemService *service );
    ~ EZXVibrateAccessory();

public slots:
    void setVibrateNow( const bool value );
    void setVibrateOnRing( const bool value );
};


class QPhoneCallProviderTapi : public QPhoneCallProvider
{
    Q_OBJECT
public:
    QPhoneCallProviderTapi( const QString& service, QObject *parent );
    ~QPhoneCallProviderTapi();

protected:
    QPhoneCallImpl *create
        ( const QString& identifier, const QString& callType );
};


class QPhoneLineTapi;


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

class QNetworkRegistrationTapi : public QNetworkRegistrationServer
{
    Q_OBJECT
public:
    QNetworkRegistrationTapi( const QString& service, QObject *parent );
    ~QNetworkRegistrationTapi();

public slots:
    void setCurrentOperator( QTelephony::OperatorMode mode,
                             const QString& id, const QString& technology );
    void requestAvailableOperators();

private slots:
    void searching();
    void home();
    void initDone();
};

class QSimInfoTapi : public QSimInfo
{
    Q_OBJECT
public:
    QSimInfoTapi( const QString& service, QObject *parent );
    ~QSimInfoTapi();
};

class QPhoneRfFunctionalityTapi : public QPhoneRfFunctionality
{
    Q_OBJECT
public:
    QPhoneRfFunctionalityTapi( const QString& service, QObject *parent );
    ~QPhoneRfFunctionalityTapi();

public slots:
    void forceLevelRequest();
    void setLevel( QPhoneRfFunctionality::Level level );
};

class QPinManagerTapi : public QPinManager
{
    Q_OBJECT
public:
    QPinManagerTapi( const QString& service, QObject *parent );
    ~QPinManagerTapi();

public slots:
    void querySimPinStatus();
    void enterPin( const QString& type, const QString& pin );
    void enterPuk( const QString& type, const QString& puk,
                   const QString& newPin );
    void cancelPin( const QString& type );
    void changePin( const QString& type, const QString& oldPin,
                    const QString& newPin );
};

class QTelephonyServiceTapi : public QModemService
{
    Q_OBJECT
public:
    QTelephonyServiceTapi( const QString& service,  QSerialIODeviceMultiplexer *mux, QObject *parent = 0 );
    ~QTelephonyServiceTapi();

    void initialize();
public slots:
    void tapi_fd(int n);
    void SignalStrengthUpdate();
};

#endif // PHONESERVERTAPIMODEM_H
