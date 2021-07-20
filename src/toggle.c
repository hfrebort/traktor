#define  UNICODE
#define _UNICODE
#define STRICT
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used
#include <windows.h>
#include <mmsystem.h>
 
static volatile LONG _Shared = 0L;
static volatile WCHAR _Usage[] = L" --- ";
 
//-------------------------------------------------------------------------------------------------
static VOID Message( WCHAR *p, ... ) {
//-------------------------------------------------------------------------------------------------
 
    WCHAR msg[ 32 ];

    va_list vaList;
    va_start( vaList, p );
 
    wvsprintf( msg, p, vaList );
    OutputDebugString( msg );
 
    va_end( vaList );
}
 
 
//=================================================================================================
DWORD WINAPI CameraThreadProc( PVOID unused ) {
//=================================================================================================
 
    LONG WorkNr, CurrNr, NextNr, PrevNr;
 
    (VOID)unused;
 
    _Usage[ 1 ] = L'R'; Message( L"%s CAMERA first 1", _Usage );
    _Usage[ 1 ] = L'-'; Message( L"%s CAMERA  done", _Usage );
 
    _Shared = WorkNr = 1;
    CurrNr = 2;
 
    for (;;) {
 
        if ( _Usage[ CurrNr ] != L'-' ) {
            OutputDebugString( L"ERROR" );
            break;
        } /* endif */

        _Usage[ CurrNr ] = L'R'; Message( L"%s CAMERA  read %u", _Usage, CurrNr );
        Sleep( 2 );
        _Usage[ CurrNr ] = L'-'; Message( L"%s CAMERA  done", _Usage );
 
        PrevNr = _InterlockedExchange( &_Shared, CurrNr );
        NextNr = ( PrevNr == 0 ) ? 6 - ( WorkNr + CurrNr ) : PrevNr;
        WorkNr = CurrNr;
        CurrNr = NextNr;
 
    } /* endfor */
 
    return 0;
}
 
//=================================================================================================
VOID rawmain( VOID ) {
//=================================================================================================
 
    LONG WorkNr;
    HANDLE hCamera;
    TIMECAPS tc;

    timeGetDevCaps( &tc, sizeof( tc ));
    timeBeginPeriod( tc.wPeriodMin ); // make things more accurate
 
    hCamera = CreateThread( NULL, 0, CameraThreadProc, NULL, 0, NULL );
 
    for (;;) {
 
        while (( WorkNr = _InterlockedExchange( &_Shared, 0L )) == 0L ) {
           Sleep( 1 );
        } /* endwhile */
 
        if ( _Usage[ WorkNr ] != L'-' ) {
            OutputDebugString( L"ERROR" );
            break;
        } /* endif */

        _Usage[ WorkNr ] = L'W'; Message( L"%s WORKING with %u", _Usage, WorkNr );
        Sleep( 7 );
        _Usage[ WorkNr ] = L'-'; Message( L"%s WORKING done", _Usage );
 
    } /* endfor */
 
    ExitProcess( 0 );
}
 
// -=EOF=-
