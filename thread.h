#ifndef _THREAD_
#define _THREAD_

#include <stdbool.h>

typedef void * TEvent;
typedef void * TCriticalSection;
typedef void * TThread;

TThread Thread_Start( int (__stdcall *func)(void*), void * ptr );

TEvent Event_Create( void );
void Event_Set( TEvent event );
void Event_Reset( TEvent * event );
int Event_WaitMultiple( int count, TEvent * event );
int Event_WaitSingle( TEvent * event );
void Event_Destroy( TEvent * event );

TCriticalSection * CriticalSection_Create( void );
void CriticalSection_Enter( TCriticalSection * cs );
bool CriticalSection_TryEnter( TCriticalSection * cs );
void CriticalSection_Leave( TCriticalSection * cs );
void CriticalSection_Delete( TCriticalSection * cs );

#endif