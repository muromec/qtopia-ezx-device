#include "qphonebooktapi.h"

/*
 *
 *
 *
 * BOOK
 *
 * TODO: now this not works at all
 */
QPhoneBookTapi::QPhoneBookTapi( const QString& service, QObject *parent )
    : QPhoneBook( service, parent, QCommInterface::Server )
{
    fixedDialingEnabled = false;
}

QPhoneBookTapi::~QPhoneBookTapi()
{
}

void QPhoneBookTapi::getEntries( const QString& store )
{
    QList<QPhoneBookEntry> list;

#ifdef TAPI_SM
    int max = 40;
    int ret;
   
    if ( store == "SM" ) {
      // 
      printf("SM\n");
      PHONEBOOK_ENTRY  book[max]; 
      memset (book, 0, sizeof(book));
      ret = TAPI_PHONEBOOK_GetEntryList( 1, max, book ); // FIXME: end
      printf("ret: %d\n",ret);
      for ( int i = 0; i < max; i++ ) {
        QPhoneBookEntry *e = new QPhoneBookEntry();
        e->setIndex(book[i].index);
        e->setNumber((char*)book[i].number);
        e->setText(  (char*)book[i].x);

        if (book[i].number) {
          list += *e;
          printf("SM %d: %d, %s, %s, %d\n",
            i,
            book[i].index, 
            book[i].number, 
            book[i].x,
            book[i].type
          );
        }


      }

    }

#endif
    emit entries( store, list );
}

void QPhoneBookTapi::add( const QPhoneBookEntry& entry, const QString& store, bool flush )
{
    if ( store != "SM" ) {
        if ( flush )
            getEntries( store );
        return;
    }

    int index;
    for ( index = 0; index < ents.size(); ++index ) {
        if ( ents[index].number().isEmpty() )
            break;
    }

    QPhoneBookEntry newEntry( entry );
    newEntry.setIndex( (uint)index );

    if ( index < ents.size() ) {
        ents[index] = newEntry;
    } else {
        ents += newEntry;
    }

    if ( flush )
        getEntries( store );
}

void QPhoneBookTapi::remove( uint index, const QString& store, bool flush )
{
    if ( store != "SM" ) {
        if ( flush )
            getEntries( store );
        return;
    }

    if ( ((int)index) < ents.size() ) {
        ents[(int)index].setNumber( "" );
    }

    if ( flush )
        getEntries( store );
}

void QPhoneBookTapi::update( const QPhoneBookEntry& entry, const QString& store, bool flush )
{
    if ( store != "SM" ) {
        if ( flush )
            getEntries( store );
        return;
    }

    int index = (int)entry.index();
    if ( index < ents.size() ) {
        ents[index] = entry;
    } else {
        add( entry, store, flush );
        return;
    }

    if ( flush )
        getEntries( store );
}

void QPhoneBookTapi::flush( const QString& store )
{
    getEntries( store );
}

void QPhoneBookTapi::setPassword( const QString&, const QString& )
{
    // Nothing to do here.
}

void QPhoneBookTapi::clearPassword( const QString& )
{
    // Nothing to do here.
}

void QPhoneBookTapi::requestLimits( const QString& store )
{
    QPhoneBookLimits l;
    l.setNumberLength( 20 );
    l.setTextLength( 18 );
    l.setFirstIndex( 1 );
    l.setLastIndex( 150 );
    emit limits( store, l );
}

void QPhoneBookTapi::requestFixedDialingState()
{
    emit fixedDialingState( fixedDialingEnabled );
}

void QPhoneBookTapi::setFixedDialingState( bool enabled, const QString& )
{
    fixedDialingEnabled = enabled;
    emit setFixedDialingStateResult( QTelephony::OK );
}


