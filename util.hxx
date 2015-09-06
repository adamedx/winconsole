//*********************************************
//
// util.hxx
//
//*********************************************

#if !defined (_UTIL_HXX_)
#define _UTIL_HXX_

#define CLOSEHANDLE_IFVALID(x) {if (x) CloseHandle(x);x = NULL;}


void DebugOut( WCHAR* wszFormat, ... );

inline LONG GetLastErrorOnFailure()
{
    LONG Status;

    Status = GetLastError();

    if ( ERROR_SUCCESS == Status )
    {
        Status = ERROR_GEN_FAILURE;
    }

    return Status;
}

#endif // _UTIL_HXX_
