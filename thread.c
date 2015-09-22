#include "thread.h"
#include "utils.h"
#include "memory.h"

#ifdef _WIN32
#   include <windows.h>
#endif

TThread Thread_Start( int (__stdcall *func)(void*), void * ptr ) {
#ifdef _WIN32
    TThread thread = CreateThread( 0, 0, (LPTHREAD_START_ROUTINE)func, ptr, 0, 0 );
    if( !thread ) {
        Util_RaiseError( "Unable to start thread!" );
    }   
    return thread;
#endif
}

TEvent Event_Create() {
#ifdef _WIN32
    return CreateEvent( 0, 0, 0, 0 );
#endif
}

TCriticalSection * CriticalSection_Create( void ) {
    
#ifdef _WIN32    
    CRITICAL_SECTION * cs = (CRITICAL_SECTION*)malloc( sizeof( CRITICAL_SECTION ) );
    InitializeCriticalSection( cs );
    return (TCriticalSection)cs;
#endif
}

void CriticalSection_Enter( TCriticalSection * cs ) {
    if( !cs ) {
        return;
    }
#ifdef _WIN32
    CRITICAL_SECTION * criticalSection = (CRITICAL_SECTION*)cs;
    EnterCriticalSection( criticalSection );
#endif
}

bool CriticalSection_TryEnter( TCriticalSection * cs ) {
    if( !cs ) {
        return false;
    }
#ifdef _WIN32
    CRITICAL_SECTION * criticalSection = (CRITICAL_SECTION*)cs;
    return TryEnterCriticalSection( criticalSection );
#endif
}

void CriticalSection_Leave( TCriticalSection * cs ) {
    if( !cs ) {
        return;
    }
#ifdef _WIN32
    CRITICAL_SECTION * criticalSection = (CRITICAL_SECTION*)cs;
    LeaveCriticalSection( criticalSection );
#endif
}

void CriticalSection_Delete( TCriticalSection * cs ) {
    if( !cs ) {
        return;
    }
#ifdef _WIN32
    CRITICAL_SECTION * criticalSection = (CRITICAL_SECTION*)cs;
    DeleteCriticalSection( criticalSection );
#endif
}

void Event_Set( TEvent event ) {
#ifdef _WIN32
    SetEvent( event );   
#endif
}

void Event_Destroy( TEvent * event ) {
#ifdef _WIN32
    CloseHandle( event );
#endif
}

void Event_Reset( TEvent * event ) {
#ifdef _WIN32
    ResetEvent( event );
#endif
}

int Event_WaitMultiple( int count, TEvent * event ) {
#ifdef _WIN32
    return WaitForMultipleObjects( count, event, 0, INFINITE );
#endif
}

int Event_WaitSingle( TEvent * event ) {
#ifdef _WIN32
    return WaitForSingleObject( event, INFINITE );
#endif
}