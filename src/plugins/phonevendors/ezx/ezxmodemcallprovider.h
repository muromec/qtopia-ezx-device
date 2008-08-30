/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
** Copyright (C) 2008 Ilya Petrov <ilya.muromec@gmail.com>
**  based on qmodemcallprovider.h
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2. http://www.gnu.org/copyleft/gpl.html
**
****************************************************************************/

#ifndef EZXMODEMCALLPROVIDER_H
#define EZXMODEMCALLPROVIDER_H

#include <qtopiaglobal.h>
#include <qphonecallprovider.h>

class EzxModemCallProviderPrivate;
class QSerialIODeviceMultiplexer;
class QModemPPPdManager;
class QAtChat;
class QAtResult;
class QModemCall;
class QModemDataCall;
class QModemService;

class QTOPIAPHONEMODEM_EXPORT EzxModemCallProvider : public QPhoneCallProvider
{
    Q_OBJECT
    friend class QModemCall;
    friend class QModemDataCall;
public:
    explicit EzxModemCallProvider( QModemService *service );
    ~EzxModemCallProvider();

    enum AtdBehavior
    {
        AtdOkIsConnect,
        AtdOkIsDialing,
        AtdOkIsDialingWithStatus,
        AtdUnknown
    };

    QModemService *service() const;
    QAtChat *atchat() const;
    QSerialIODeviceMultiplexer *multiplexer() const;
    QModemCall *incomingCall() const;
    QModemCall *dialingCall() const;
    QModemCall *callForIdentifier( uint id ) const;
    uint nextModemIdentifier();

public slots:
    void ringing( const QString& number, const QString& callType,
                  uint modemIdentifier = 0 );
    void hangupRemote( QModemCall *call );

protected:
    QPhoneCallImpl *create
        ( const QString& identifier, const QString& callType );
    virtual QString resolveRingType( const QString& type ) const;
    virtual QString resolveCallMode( int mode ) const;
    virtual bool hasRepeatingRings() const;
    virtual EzxModemCallProvider::AtdBehavior atdBehavior() const;
    virtual void abortDial( uint modemIdentifier, QPhoneCall::Scope scope );
    virtual bool partOfHoldGroup( const QString& callType ) const;

    virtual QString dialServiceCommand( const QDialOptions& options ) const;
    virtual QString dialVoiceCommand( const QDialOptions& options ) const;
    virtual QString releaseCallCommand( uint modemIdentifier ) const;
    virtual QString releaseActiveCallsCommand() const;
    virtual QString releaseHeldCallsCommand() const;
    virtual QString putOnHoldCommand() const;
    virtual QString setBusyCommand() const;
    virtual QString acceptCallCommand( bool otherActiveCalls ) const;
    virtual QString activateCallCommand
                ( uint modemIdentifier, bool otherActiveCalls ) const;
    virtual QString activateHeldCallsCommand() const;
    virtual QString joinCallsCommand( bool detachSubscriber ) const;
    virtual QString deflectCallCommand( const QString& number ) const;
    virtual QStringList gprsSetupCommands() const;

protected slots:
    virtual void resetModem();

private slots:
    void ring();
    void cring( const QString& msg );
    void callNotification( const QString& msg );
    void clip( const QString& msg );
    void colp( const QString& msg );
    void ccwa( const QString& msg );
    void detectTimeout();
    void missedTimeout();
    void clccIncoming( bool, const QAtResult& );

private:
    EzxModemCallProviderPrivate *d;

    void missedTimeout(QModemCall *call);
    void stopRingTimers();
    void announceCall();
    void useModemIdentifier( uint id );
    void releaseModemIdentifier( uint id );
    bool otherActiveCalls( QModemCall *notThis=0 );
    void changeGroup( QPhoneCall::State oldState, QPhoneCall::State newState,
                      QPhoneCallImpl::Actions newActions );
    bool hasGroup( QPhoneCall::State state );
    void activateWaitingOrHeld();
    QModemPPPdManager *pppdManager() const;
    int vtsType() const;
    void setVtsType( int type );
};

#endif // EZXMODEMCALLPROVIDER_H
