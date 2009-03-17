/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
** Copyright (c) 2008 Ilya Petrov <ilya.muromec@gmail.com>
**  based on qmodemcallprovider.cpp
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2. http://www.gnu.org/copyleft/gpl.html
**
****************************************************************************/

#include <ezxmodemcallprovider.h>
#include <qmodemcall.h>
#include <qmodemservice.h>
#include <qatutils.h>
#include <qatresult.h>
#include <qatresultparser.h>
#include <qtopiaphonemodem/private/qmodempppdmanager_p.h>
#include <qtopialog.h>
#include <qtimer.h>
#include <QtopiaChannel>

#include "stdio.h"
/*!
    \class EzxModemCallProvider
    \mainclass
    \brief The EzxModemCallProvider class implements a mechanism for AT-based phone call providers to hook into the telephony system.
    \ingroup telephony::modem

    This class provides a number of methods, such as dialVoiceCommand(),
    releaseCallCommand(), putOnHoldCommand(), etc, that can be used to
    customize the AT commands that are used by QModemCall for specific
    operations.  Modem vendor plug-ins override these methods in
    their own phone call provider.

    Client applications should use QPhoneCall and QPhoneCallManager to make
    and receive phone calls.  The EzxModemCallProvider class is intended for the
    phone server.

    QModemCall instances are created by the EzxModemCallProvider::create()
    function.  If a modem vendor plug-in needs to change some of the
    functionality in this class, they should do the following:

    \list
        \o Inherit from QModemCall and override the functionality that is different.
        \o Inherit from EzxModemCallProvider and override EzxModemCallProvider::create()
           to instantiate the new class that inherits from QModemCall.
    \endlist

    See the documentation for QModemCall for more information.

    \sa QModemCall, QModemDataCall, QPhoneCallProvider, QPhoneCallImpl
*/

class EzxModemCallProviderPrivate
{
public:
    EzxModemCallProviderPrivate()
    {
        incomingModemIdentifier = 0;
        ringBlock = false;
        usedIds = 0;
        vtsType = 0;
        pppdManager = 0;
    }

    QModemService *service;
    QString incomingNumber;
    QString incomingCallType;
    uint incomingModemIdentifier;
    QTimer *detectTimer;
    QTimer *missedTimer;
    QTimer *hangupTimer;
    bool ringBlock;
    uint usedIds;
    int vtsType;
    QModemPPPdManager *pppdManager;
};

// Time to wait between first seeing an incoming call and actually
// reporting it to the higher layers.  This timeout is needed to allow
// all information from RING, +CRING, +CLIP, +CCWA, and similar commands
// to be collected up.
#define INCOMING_DETECT_TIMEOUT     800

// Time to wait after seeing the last ring indication before assuming
// that the call has gone away and was missed.
#define INCOMING_MISSED_TIMEOUT     5500

// Time to wait just after a call to EzxModemCallProvider::hangup()
// before allowing notifications like "NO CARRIER" to work again.
#define HANGUP_TIMEOUT              200

/*!
    Constructs a new AT-based phone call provider for \a service.
*/
EzxModemCallProvider::EzxModemCallProvider( QModemService *service )
    : QPhoneCallProvider( service->service(), service )
{
    d = new EzxModemCallProviderPrivate();
    d->service = service;
    d->detectTimer = new QTimer( this );
    d->detectTimer->setSingleShot( true );
    connect( d->detectTimer, SIGNAL(timeout()), this, SLOT(detectTimeout()) );
    d->missedTimer = new QTimer( this );
    d->missedTimer->setSingleShot( true );
    connect( d->missedTimer, SIGNAL(timeout()), this, SLOT(missedTimeout()) );
    d->hangupTimer = new QTimer( this );
    d->hangupTimer->setSingleShot( true );

    connect( service, SIGNAL(resetModem()), this, SLOT(resetModem()) );

    // Create the pppd manager object for data calls on the internal modem.
    d->pppdManager = new QModemPPPdManager( service->multiplexer(), this );

    // Register for standard notifications that indicate incoming calls.
    atchat()->registerNotificationType( "RING", this, SLOT(ring()) );
    atchat()->registerNotificationType( "+CRING:", this, SLOT(cring(QString)) );
    connect( atchat(), SIGNAL(callNotification(QString)),
             this, SLOT(callNotification(QString)) );
    atchat()->registerNotificationType( "+CLIP:", this, SLOT(clip(QString)), true );
    atchat()->registerNotificationType( "+COLP:", this, SLOT(colp(QString)), true );
    atchat()->registerNotificationType( "+CCWA: \"", this, SLOT(ccwa(QString)) );

    // Advertise the call types that we understand.
    QStringList types;
    types += "Voice";
    types += "Data";
    types += "Fax";
    types += "Video";
    types += "IP";
    setCallTypes( types );
}

/*!
    Destroys this AT-based phone call provider and all QPhoneCallImpl instances
    associated with it.
*/
EzxModemCallProvider::~EzxModemCallProvider()
{
    delete d;
}

/*!
    Sets the ringing state in the phone library, indicating a call
    from \a number of the type \a callType.  Modem vendor plug-ins
    call this if the modem reports incoming calls with something other
    than \c RING or \c +CRING.

    Either \a number or \a callType may be empty indicating that the
    information is not available yet.  The information may become
    available shortly on a different unsolicited result code
    (e.g. \c +CLIP or \c +CCWA).

    If \a modemIdentifier is not zero, it indicates the modem identifier
    for the call.  If \a modemIdentifier is zero, the next available
    modem identifier is used.

    \sa QModemCall::callType(), QModemCall::number(), QModemCall::modemIdentifier()
    \sa hasRepeatingRings()
*/
void EzxModemCallProvider::ringing
    ( const QString& number, const QString& callType, uint modemIdentifier )
{
    // If we already announced an incoming call, then reset the
    // missed call timer but otherwise ignore this.  This prevents
    // us from creating a new incoming call for every new RING.
    if (number.isEmpty() )
      return;

    if ( incomingCall() ) {
        if ( hasRepeatingRings() )
            d->missedTimer->start( INCOMING_MISSED_TIMEOUT );
        return;
    }

    QtopiaChannel::send("QPE/APM", "full()");
    QtopiaChannel::send("QPE/FlipScreen", "hide()");

    // Record the new information that we got.
    if ( !number.isEmpty() )
        d->incomingNumber = number;
    if ( !callType.isEmpty() )
        d->incomingCallType = callType;
    if ( modemIdentifier ) {
        releaseModemIdentifier( d->incomingModemIdentifier );
        useModemIdentifier( modemIdentifier );
        d->incomingModemIdentifier = modemIdentifier;
    } else if ( !d->incomingModemIdentifier ) {
        d->incomingModemIdentifier = nextModemIdentifier();
    }

    // Start the detection timer if it isn't already running.
    if ( !d->detectTimer->isActive() )
        d->detectTimer->start( INCOMING_DETECT_TIMEOUT );

    // Reset the missed call timer if necessary.
    if ( hasRepeatingRings() )
        d->missedTimer->start( INCOMING_MISSED_TIMEOUT );
}

/*!
    Called by the modem vendor plug-in to indicate that it in determined that
    \a call was hung up remotely.  This is usually called in response to a
    proprietary unsolicited result code.

    If \a call is null, then the modem vendor plug-in has determined
    that the active call has hung up but it was unable to be any
    more precise than that.

    \sa QModemCall::hangup()
*/
void EzxModemCallProvider::hangupRemote( QModemCall *call )
{
    beginStateTransaction();
    if ( call ) {

        // A specific call has hung up.
        //bool wasActive = ( call->state() == QPhoneCall::Connected );
        call->setState( QPhoneCall::HangupRemote );

    #if 0
        // If there are no other active calls, then activate the held call.
        if ( wasActive && !hasGroup( QPhoneCall::Connected ) ) {
            changeGroup( QPhoneCall::Hold, QPhoneCall::Connected,
                         QPhoneCallImpl::Hold | QPhoneCallImpl::Tone |
                         QPhoneCallImpl::Transfer );
        }
    #endif

    } else {

        // The active call has hung up.
        changeGroup( QPhoneCall::Connected, QPhoneCall::HangupRemote,
                     QPhoneCallImpl::None );
    #if 0
        changeGroup( QPhoneCall::Hold, QPhoneCall::Connected, // XXX
                     QPhoneCallImpl::Hold | QPhoneCallImpl::Tone |
                     QPhoneCallImpl::Transfer );
    #endif

    }
    endStateTransaction();

    // If "NO CARRIER", "NO DIALTONE", etc occurs in the next small
    // amount of time, then assume that it is a duplicate of this
    // hangup request.  This is needed to work around modems that
    // send an unsolicited response followed by "NO CARRIER", even
    // though the unsolicited response is more than adequate on its own.
    d->hangupTimer->start( HANGUP_TIMEOUT );
}

/*!
    \reimp
*/
QPhoneCallImpl *EzxModemCallProvider::create
        ( const QString& identifier, const QString& callType )
{
    if ( callType == "Voice" )      // No tr
        return new QModemCall( (QModemCallProvider*)this, identifier, callType );
    else
        return new QModemDataCall( (QModemCallProvider*)this, identifier, callType );
}

/*!
    Resolves a call type on a "+CRING" notification into the particular
    type of call that it represents.  The \a type is guaranteed to
    be in lower case on entry to this function.

    \sa resolveCallMode()
*/
QString EzxModemCallProvider::resolveRingType( const QString& type ) const
{
    if ( type == "voice" || type.startsWith( "voice," ) )    // No tr
        return "Voice";         // No tr
    else if ( type == "fax" || type.startsWith( "fax," ) )   // No tr
        return "Fax";           // No tr
    else if ( type.contains( "sync" ) )                      // No tr
        return "Data";          // No tr

    // Don't know what this is, so let ringing() guess at it later.
    return QString();
}

/*!
    Resolves a call \a mode as reported by the \c{AT+CLCC} command.

    \sa resolveRingType()
*/
QString EzxModemCallProvider::resolveCallMode( int mode ) const
{
    if ( mode == 0 || mode == 3 || mode == 4 || mode == 5 ) {
        return "Voice";         // No tr
    } else if ( mode == 1 || mode == 6 || mode == 7 ) {
        return "Data";          // No tr
    } else if ( mode == 2 || mode == 8 ) {
        return "Fax";           // No tr
    } else {
        // Don't know what this is.  Assume voice.
        return "Voice";         // No tr
    }
}

/*!
    Returns true if the modem automatically re-sends \c RING every few
    seconds while a call is incoming, and stops sending \c RING if
    the caller hangs up.  Returns false if the modem does not resend
    \c RING every few seconds and instead uses some other mechanism
    to notify Qtopia that a remote hangup has occurred.  The default
    return value is true.

    \sa ringing()
*/
bool EzxModemCallProvider::hasRepeatingRings() const
{
    return true;
}

/*!
    \enum EzxModemCallProvider::AtdBehavior
    \brief This enum defines the behavior of the \c ATD modem command when dialing voice calls.

    \value AtdOkIsConnect When \c ATD reports \c OK, the call has connected.
    \value AtdOkIsDialing When \c ATD reports \c OK, the call is dialing, but not yet connected.  The \c AT+CLCC command must be used to determine when it transitions from dialing to connected.
    \value AtdOkIsDialingWithStatus When \c ATD reports \c OK, the call is dialing, but not yet connected.  The modem vendor plug-in will call setState() on the call when it transitions to connected.
    \value AtdUnknown When \c ATD reports \c OK, it is unknown whether the call is dialing or connected and \c AT+CLCC must be used to determine the status.  This is the default.
*/

/*!
    Returns the behavior of the \c ATD modem command when dialing
    voice calls.

    The defined behavior in 3GPP TS 27.007 is for the \c ATD command
    to immediately return to command mode when it has a trailing
    semi-colon (i.e. \c{ATDnumber;}), even if the call has not
    yet connected.  Not all modems support this correctly.

    This function can be used to alter how the system uses the \c ATD
    command when dialing voice calls to accommodate non-standard modems.

    \c AtdOkIsConnect indicates that \c ATD blocks until the call
    has connected before reporting \c OK.  \c AtdOkIsDialing and
    \c AtdOkIsDialingWithStatus indicates that the modem obeys
    3GPP TS 27.007 and returns immediately to command mode.

    In the case of \c AtdOkIsDialing, the \c AT+CLCC command is polled
    to determine when the call transitions from dialing to connected.
    The modem vendor plug-in can avoid polling if it has call status
    tracking.  In that case it should return \c AtdOkIsDialingWithStatus
    from this function and call setState() once the transition occurs.

    \c AtdUnknown indicates that it is not known which of these modes is
    supported by the modem.  A separate \c AT+CLCC command is used to
    determine what state the call is actually in once \c ATD reports \c OK.

    The default implementation returns AtdUnknown.

    \sa EzxModemCallProvider::AtdBehavior
*/
EzxModemCallProvider::AtdBehavior EzxModemCallProvider::atdBehavior() const
{
    return AtdOkIsDialingWithStatus;
}

/*!
    Aborts an \c ATD dial command for the call \a modemIdentifier.
    The \a scope parameter is passed from QModemCall::hangup().

    The default implementation calls atchat()->abortDial(),
    followed by either \c{AT+CHLD=1} or \c{AT+CHLD=1n} depending
    upon the value of \a scope.

    \sa QAtChat::abortDial(), QModemCall::hangup()
*/
void EzxModemCallProvider::abortDial
        ( uint modemIdentifier, QPhoneCall::Scope scope )
{
    atchat()->abortDial();
    if ( scope == QPhoneCall::CallOnly ) {
        atchat()->chat( "AT+CHLD=1" + QString::number( modemIdentifier ) );
    } else {
        atchat()->chat( "ATH" );
    }
}

/*!
    Returns true if a particular kind of call is part of the normal hold group.
    On some systems, video calls are separate from the call grouping for
    voice and fax calls.  Returns true only for \c Voice by default.
    The \a type parameter indicates the type of call (\c Voice, \c Video,
    \c Fax, etc).
*/
bool EzxModemCallProvider::partOfHoldGroup( const QString& type ) const
{
    return ( type == "Voice" ); // No tr
}

/*!
    Returns the phone call object associated with the current incoming call.
    Returns null if there is no incoming call.

    \sa dialingCall()
*/
QModemCall *EzxModemCallProvider::incomingCall() const
{
    QList<QPhoneCallImpl *> list = calls();
    QList<QPhoneCallImpl *>::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
        if ( (*it)->state() == QPhoneCall::Incoming )
            return (QModemCall *)*it;
    }
    return 0;
}

/*!
    Returns the phone call object associated with the current dialing
    or alerting call.  Returns null if there is no such call.

    \sa incomingCall()
*/
QModemCall *EzxModemCallProvider::dialingCall() const
{
    QList<QPhoneCallImpl *> list = calls();
    QList<QPhoneCallImpl *>::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
        if ( (*it)->state() == QPhoneCall::Dialing ||
             (*it)->state() == QPhoneCall::Alerting )
            return (QModemCall *)*it;
    }
    return 0;
}

/*!
    Returns the AT command to use to dial the supplementary service
    specified by \a options.  The default implementation returns
    \c{ATDnumber}.

    \sa dialVoiceCommand()
*/
QString EzxModemCallProvider::dialServiceCommand
            ( const QDialOptions& options ) const
{
    return "ATD" + options.number();
}

/*!
    Returns the AT command to use to dial the voice call specified
    by \a options.  The default implementation returns
    \c{ATDnumber[i][g];} where the \c i flag indicates the caller ID
    disposition and the \c g flag indicates the closed user group
    disposition.

    \sa QDialOptions::number(), QDialOptions::callerId()
    \sa QDialOptions::closedUserGroup()

    \sa dialServiceCommand()
*/
QString EzxModemCallProvider::dialVoiceCommand
            ( const QDialOptions& options ) const
{
    QString suffix;
    if ( options.callerId() == QDialOptions::SendCallerId )
        suffix += "i";
    else if ( options.callerId() == QDialOptions::SuppressCallerId )
        suffix += "I";
    if ( options.closedUserGroup() )
        suffix += "g";
    return "ATD" + options.number() + suffix + ";";
}

/*!
    Returns the AT command to use to release the call \a modemIdentifier.
    The default implementation returns \c{AT+CHLD=1modemIdentifier}.

    \sa releaseActiveCallsCommand(), releaseHeldCallsCommand()
*/
QString EzxModemCallProvider::releaseCallCommand( uint modemIdentifier ) const
{
    return "AT+CHLD=1" + QString::number( modemIdentifier );
}

/*!
    Returns the AT command to use to release all active calls.
    The default implementation returns \c{AT+CHLD=1}.

    \sa releaseCallCommand(), releaseHeldCallsCommand()
*/
QString EzxModemCallProvider::releaseActiveCallsCommand() const
{
    return "AT+CHLD=1";
}

/*!
    Returns the AT command to use to release all held calls.
    The default implementation returns \c{AT+CHLD=0}.

    \sa releaseCallCommand(), releaseActiveCallsCommand()
*/
QString EzxModemCallProvider::releaseHeldCallsCommand() const
{
    return "AT+CHLD=0";
}

/*!
    Returns the AT command to use to put the currently active calls on hold.
    The default implementation returns \c{AT+CHLD=2}.

    \sa activateCallCommand(), activateHeldCallsCommand(), QModemCall::hold()
*/
QString EzxModemCallProvider::putOnHoldCommand() const
{
    return "AT+CHLD=2";
}

/*!
    Returns the AT command to use to reject the incoming call and set
    the busy state for the caller.  The default implementation
    returns \c{AT+CHLD=0}.

    \sa acceptCallCommand(), QModemCall::hangup()
*/
QString EzxModemCallProvider::setBusyCommand() const
{
    return "AT+CHLD=6";
}

/*!
    Returns the AT command to use to accept an incoming call.
    If \a otherActiveCalls is true, then there are other active
    calls within the system.  The default implementation
    returns \c{AT+CHLD=2} if \a otherActiveCalls is true, or
    \c{ATA} otherwise.

    \sa setBusyCommand(), QModemCall::accept()
*/
QString EzxModemCallProvider::acceptCallCommand( bool otherActiveCalls ) const
{
    if ( otherActiveCalls )
        return "AT+CHLD=2";
    else
        return "ATA";
}

/*!
    Returns the AT command to use to activate the call \a modemIdentifier.
    If \a otherActiveCalls is true, then there are other active
    calls within this system.  The default implementation returns
    \c{AT+CHLD=2modemIdentifier} if \a otherActiveCalls is true,
    or \c{AT+CHLD=2} if the call being activated is the only one
    in the system.

    \sa activateHeldCallsCommand(), putOnHoldCommand(), QModemCall::activate()
*/
QString EzxModemCallProvider::activateCallCommand
            ( uint modemIdentifier, bool otherActiveCalls ) const
{
    if ( otherActiveCalls )
        return "AT+CHLD=2" + QString::number( modemIdentifier );
    else
        return "AT+CHLD=2";
}

/*!
    Returns the AT command to use to place the currently active calls on
    hold and activate the held calls.  The default implementation
    returns \c{AT+CHLD=2}.

    \sa activateCallCommand(), putOnHoldCommand(), QModemCall::activate()
*/
QString EzxModemCallProvider::activateHeldCallsCommand() const
{
    return "AT+CHLD=2";
}

/*!
    Returns the AT command to use to join the active and held calls
    into a single multi-party conversation.  If \a detachSubscriber
    is true, then detach the local party from the conversation after
    joining the calls.  Returns \c{AT+CHLD=4} if \a detachSubscriber
    is true, or \c{AT+CHLD=3} otherwise.

    \sa QModemCall::join()
*/
QString EzxModemCallProvider::joinCallsCommand( bool detachSubscriber ) const
{
    if ( detachSubscriber )
        return "AT+CHLD=4";
    else
        return "AT+CHLD=3";
}

/*!
    Returns the AT command to use to deflect the incoming call to \a number.
    The default implementation returns \c{AT+CTFR=number}.

    \sa QModemCall::transfer()
*/
QString EzxModemCallProvider::deflectCallCommand( const QString& number ) const
{
    return "AT+CTFR=" + number;
}

/*!
    Returns the phone call associated with modem identifier \a id.
    Returns null if there is no such call.
*/
QModemCall *EzxModemCallProvider::callForIdentifier( uint id ) const
{
    QList<QPhoneCallImpl *> list = calls();
    QList<QPhoneCallImpl *>::ConstIterator it;
    for ( it = list.begin(); it != list.end(); ++it ) {
        if ( ((QModemCall *)(*it))->modemIdentifier() == id )
            return (QModemCall *)*it;
    }
    return 0;
}

/*!
  Returns the AT commands to use to set up a GPRS session. The returned list should not contain
  \c{AT+CGDCONT} and \c{ATD*99***1#} as these commands are automatically inserted.
  The default Implementation returns \c{AT+CGATT=1}.
*/
QStringList EzxModemCallProvider::gprsSetupCommands() const
{
    return QStringList() << "AT+CGATT=1";
}

/*!
    Returns the modem service that this call provider is associated with.
*/
QModemService *EzxModemCallProvider::service() const
{
    return d->service;
}

/*!
    Returns the AT chat handler for this modem call provider.
*/
QAtChat *EzxModemCallProvider::atchat() const
{
    return d->service->primaryAtChat();
}

/*!
    Returns the serial multiplexer for this modem call provider.
*/
QSerialIODeviceMultiplexer *EzxModemCallProvider::multiplexer() const
{
    return d->service->multiplexer();
}

/*!
    Called when the modem resets after PIN entry, to initialize
    the call provider.
*/
void EzxModemCallProvider::resetModem()
{
    // Turn on +CRING notifications because they give us more info than RING.
    atchat()->chat( "AT+CRC=1" );

    // Turn on caller-ID presentation for incoming calls.
    d->service->retryChat( "AT+CLIP=1" );

    // Turn on COLP presentation for outgoing calls.
    d->service->retryChat( "AT+COLP=1" );

    // Turn on unsolicited notifications for incoming calls.
    d->service->retryChat( "AT+CCWA=1" );
}

void EzxModemCallProvider::ring()
{
    ringing( QString(), QString() );
}

void EzxModemCallProvider::cring( const QString& msg )
{
    // Notification of an incoming call, with an explicit type code.
    QString type = msg.mid( 7 ).toLower().trimmed();
    ringing( QString(), resolveRingType( type ) );
}

void EzxModemCallProvider::callNotification( const QString& msg )
{
    QPhoneCall::State state;
    uint posn = 0;

    if ( msg.startsWith ("CONNECT: ") ) {
      posn = 8;
      state = QPhoneCall::Connected;
    }  else if (  msg.startsWith( "BUSY:" ) ) {
      posn = 5;
      state = QPhoneCall::HangupRemote ;
      atchat()->chat( "ATH" );
    } else if  ( msg.startsWith( "NO CARRIER:" )) {
      posn = 11;
      state =  QPhoneCall::HangupRemote;

    }

    if (posn) {
      uint identifier = QAtUtils::parseNumber( msg, posn );
      QModemCall *call = callForIdentifier( identifier );

      if (call) 
        call->setState( state );
      
    }   
}

void EzxModemCallProvider::clip( const QString& msg )
{
    // Caller ID information for an incoming call.
    uint posn = 6;
    QString number = QAtUtils::nextString( msg, posn );
    uint numberType = QAtUtils::parseNumber( msg, posn );
    number = QAtUtils::decodeNumber( number, numberType );

    // Report the number via ringing() with an unknown call type.
    // We will get the real call type from +CRING or AT+CLCC.
    ringing( number, QString() );
}

void EzxModemCallProvider::colp( const QString& msg )
{
    // Caller ID information for an outgoing call.
    uint posn = 6;
    QString number = QAtUtils::nextString( msg, posn );
    uint numberType = QAtUtils::parseNumber( msg, posn );
    number = QAtUtils::decodeNumber( number, numberType );

    // Find the current Dialing or Alerting call and emit a notification on it.
    QModemCall *call = dialingCall();
    if ( call )
        call->emitNotification( QPhoneCall::ConnectedLineId, number );
}

void EzxModemCallProvider::ccwa( const QString& msg )
{
    // A call waiting notification was received: handle like "RING".
    uint posn = 6;
    QString number = QAtUtils::nextString( msg, posn );
    uint numberType = QAtUtils::parseNumber( msg, posn );
    number = QAtUtils::decodeNumber( number, numberType );

    // Report the number via ringing() with an unknown call type.
    // We will get the real call type from +CRING or AT+CLCC.
    ringing( number, QString() );
}

void EzxModemCallProvider::detectTimeout()
{
    // If we still don't have a call type at this point, use AT+CLCC
    // to try to discover the incoming call type.
    if ( d->incomingCallType.isEmpty() ) {
        atchat()->chat( "AT+CLCC", this, SLOT(clccIncoming(bool,QAtResult)) );
    } else {
        announceCall();
    }
}

void EzxModemCallProvider::missedTimeout(QModemCall *call)
{
    d->detectTimer->stop();
    if ( call ) {
        qLog(Modem) << "Reporting missed call.";
        call->setState( QPhoneCall::Missed );
    }
}

void EzxModemCallProvider::missedTimeout()
{
    missedTimeout( incomingCall() );
}

void EzxModemCallProvider::stopRingTimers()
{
    // Stop the ring timers because the call has been accepted or hung up.
    d->detectTimer->stop();
    d->missedTimer->stop();
}

void EzxModemCallProvider::announceCall()
{
    // We now assume that we have enough information to announce
    // the call to the rest of the system.
    qLog(Modem) << "Qtopia detected an incoming call, notifying applications.";

    // Create the new call object and set it to incoming.
    QModemCall *call = (QModemCall *)create( QString(), d->incomingCallType );
    call->setModemIdentifier( d->incomingModemIdentifier );
    call->setNumber( d->incomingNumber );
    call->setActions( QPhoneCallImpl::Accept );
    call->setState( QPhoneCall::Incoming );

    // Reset the incoming values for the next call.
    d->incomingNumber = QString();
    d->incomingCallType = QString();
    d->incomingModemIdentifier = 0;
}

void EzxModemCallProvider::clccIncoming( bool ok, const QAtResult& result )
{
    QString callType;
    if ( ok ) {
        QAtResultParser parser( result );
        while ( parser.next( "+CLCC:" ) ) {
            uint id = parser.readNumeric();
            parser.readNumeric();       // Don't need the direction code.
            uint state = parser.readNumeric();
            uint mode = parser.readNumeric();
            if ( state == 4 || state == 5 ) {
                // Make sure that we use the correct modem identifier.
                releaseModemIdentifier( d->incomingModemIdentifier );
                useModemIdentifier( id );
                d->incomingModemIdentifier = id;

                // Convert the mode into something useful.
                callType = resolveCallMode( mode );
                break;
            }
        }
    } else {
        // The AT+CLCC command failed for some reason.  It may not
        // be responding properly or the modem doesn't support it.
        // Assume that the call is voice.
        callType = "Voice";     // No tr
    }
    if ( callType.isEmpty() ) {
        // We could not determine the call type, so the incoming call
        // has probably disappeared.  Flag it as a missed voice call.
        d->incomingCallType = "Voice";      // No tr
        announceCall();
        d->missedTimer->stop();
        missedTimeout();
    } else {
        d->incomingCallType = callType;
        announceCall();
    }
}

// Mark a modem identifier as currently in use.
void EzxModemCallProvider::useModemIdentifier( uint id )
{
    if ( id > 0 && id <= 32 )
        d->usedIds |= ( ((uint)1) << (id - 1) );
}

// Release a modem identifier that is no longer required.
void EzxModemCallProvider::releaseModemIdentifier( uint id )
{
    if ( id > 0 && id <= 32 )
        d->usedIds &= ~( ((uint)1) << (id - 1) );
}

/*!
    Allocates the next modem identifier in rotation that is not
    currently used by a call.
*/
uint EzxModemCallProvider::nextModemIdentifier()
{
    uint id;
    for ( id = 1; id <= 32; ++id ) {
        if ( ( d->usedIds & ( ((uint)1) << (id - 1) ) ) == 0 ) {
            d->usedIds |= ( ((uint)1) << (id - 1) );
            return id;
        }
    }
    return 0;
}

bool EzxModemCallProvider::otherActiveCalls( QModemCall *notThis )
{
    QList<QPhoneCallImpl *> calls = this->calls();
    QList<QPhoneCallImpl *>::ConstIterator iter;
    QModemCall *callp;
    for ( iter = calls.begin(); iter != calls.end(); ++iter ) {
        callp = (QModemCall *)(*iter);
        if ( callp != notThis && callp->callType() == "Voice" ) { // No tr
            if ( callp->state() == QPhoneCall::Connected ||
                 callp->state() == QPhoneCall::Hold ) {
                return true;
            }
        }
    }
    return false;
}

void EzxModemCallProvider::changeGroup
    ( QPhoneCall::State oldState, QPhoneCall::State newState,
      QPhoneCallImpl::Actions newActions )
{
    QList<QPhoneCallImpl *> calls = this->calls();
    QList<QPhoneCallImpl *>::ConstIterator iter;
    for ( iter = calls.begin(); iter != calls.end(); ++iter ) {
        if ( (*iter)->callType() != "Voice" )   // No tr
            continue;
        if ( (*iter)->state() == oldState ) {
            (*iter)->setActions( newActions );
            (*iter)->setState( newState );
        }
    }
}

bool EzxModemCallProvider::hasGroup( QPhoneCall::State state )
{
    QList<QPhoneCallImpl *> calls = this->calls();
    QList<QPhoneCallImpl *>::ConstIterator iter;
    for ( iter = calls.begin(); iter != calls.end(); ++iter ) {
        if ( (*iter)->callType() != "Voice" )   // No tr
            continue;
        if ( (*iter)->state() == state ) {
            return true;
        }
    }
    return false;
}

void EzxModemCallProvider::activateWaitingOrHeld()
{
    // Note: The GSM 02.30 spec says that commands that "activate the held
    // or waiting call" will choose the waiting call ahead of a held call.

    QModemCall *callp = incomingCall();
    QPhoneCallImpl::Actions actions;
    if ( callp ) {

        // The waiting call has been activated.
        actions = QPhoneCallImpl::Tone | QPhoneCallImpl::Transfer;
        if ( !hasGroup( QPhoneCall::Hold ) )
            actions |= QPhoneCallImpl::Hold;
        else
            actions |= QPhoneCallImpl::Join | QPhoneCallImpl::JoinAndDetach;
        callp->setActions( actions );
        callp->setState( QPhoneCall::Connected );

    } else {

        // The held calls have been activated.
        actions = QPhoneCallImpl::Tone | QPhoneCallImpl::Transfer;
        if ( !hasGroup( QPhoneCall::Connected ) &&
             !hasGroup( (QPhoneCall::State)(-1) ) ) // During swap transition.
            actions |= QPhoneCallImpl::Hold;
        else
            actions |= QPhoneCallImpl::Join | QPhoneCallImpl::JoinAndDetach;
        QList<QPhoneCallImpl *> calls = this->calls();
        QList<QPhoneCallImpl *>::ConstIterator iter;
        for ( iter = calls.begin(); iter != calls.end(); ++iter ) {
            if ( (*iter)->callType() != "Voice" )   // No tr
                continue;
            if ( (*iter)->state() == QPhoneCall::Hold ) {
                (*iter)->setActions( actions );
                (*iter)->setState( QPhoneCall::Connected );
            }
        }

    }
}

QModemPPPdManager *EzxModemCallProvider::pppdManager() const
{
    return d->pppdManager;
}

int EzxModemCallProvider::vtsType() const
{
    return d->vtsType;
}

void EzxModemCallProvider::setVtsType( int type )
{
    d->vtsType = type;
}
