//*********************************************
//
// util.cxx
//
//*********************************************

#include "ash.hxx"

#if DBG
void DebugOut( WCHAR* wszFormat, ... )
{
    WCHAR   wszResultBuf[1024]; 
    va_list VAList;

    va_start(VAList, wszFormat );

    wvsprintf(wszResultBuf, wszFormat, VAList );

    va_end( VAList );

    OutputDebugString( wszResultBuf );
    OutputDebugString( L"\n" );
}
#else // DBG

void DebugOut( WCHAR* wszFormat, ... ) {}

#endif // DBG

