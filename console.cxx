//****************************************************
// 
// console.hxx
//
//****************************************************

#include "ash.hxx"


LONG OutputContext::ReadData()
{
    DWORD  cbSize;
    BOOL   bStatus;
    LONG   Status;

    Status = ERROR_SUCCESS;

    cbSize = READ_BUFFER_SIZE - 1;

    bStatus = ReadFileEx(
        _hRead,
        _szData,
        cbSize,
        &_AsyncIO,
        (LPOVERLAPPED_COMPLETION_ROUTINE) ShowOutput
        );

    if ( ! bStatus )
    {
        Status = GetLastErrorOnFailure();
    }

    if ( ERROR_SUCCESS != Status )
    {
        DebugOut( L"ReadData failed with 0x%x", Status );
    }

    return Status;
}

void CALLBACK OutputContext::ShowOutput(
    DWORD        dwErrorCode,                // completion code
    DWORD        dwNumberOfBytesTransferred,  // number of bytes transferred
    LPOVERLAPPED lpOverlapped         // I/O information buffer
    )
{
    if ( 0 != dwErrorCode )
    {
        return;
    }

    if ( 0 == dwNumberOfBytesTransferred )
    {
        return;
    }

    OutputContext* pContext;
    CConsole*      pConsole;
    
    pContext = (OutputContext*) (lpOverlapped->hEvent);

    pConsole = pContext->_pConsole;

    pContext->_szData[ dwNumberOfBytesTransferred ] = '\0';

    pConsole->AddText( pContext->_szData );

    pContext->ReadData();
}


CConsole::CConsole( CMainWindow* pMainWindow ) :
    _hProcess(NULL),
    _hWriteOut(NULL),
    _hWriteError(NULL),
    _hWriteIn(NULL),
    _StdoutContext( this ),
    _StderrContext( this ),
    _cchStdout( 0 ),
    _cchStderr( 0 ),
    _pMainWindow( pMainWindow ),
    _hThread( NULL )
{
    memset( &_AsyncIO, 0, sizeof(_AsyncIO) );
    _AsyncIO.hEvent = (HANDLE) this;
}

CConsole::~CConsole()
{
    CLOSEHANDLE_IFVALID( _hProcess );
    CLOSEHANDLE_IFVALID( _hWriteOut );
    CLOSEHANDLE_IFVALID( _hWriteIn );
    CLOSEHANDLE_IFVALID( _StdoutContext._hRead );
    CLOSEHANDLE_IFVALID( _StderrContext._hRead );
    CLOSEHANDLE_IFVALID( _hWriteError ); 
    CLOSEHANDLE_IFVALID( _hThread );
}

LONG CConsole::StartProcess( WCHAR* wszCommandLine )
{
    STARTUPINFO         sinf;
    PROCESS_INFORMATION pinf;
    SECURITY_ATTRIBUTES saAttr;
    LONG                Status;
    HANDLE              hOriginalWriteIn;
    BOOL                bStatus;

    Status = ERROR_SUCCESS;

    hOriginalWriteIn = NULL;

    memset(&sinf,0,sizeof(sinf));
    sinf.cb = sizeof(sinf);

    //
    // Set the bInheritHandle flag so pipe handles are inherited. 
    //
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle = TRUE; 
    saAttr.lpSecurityDescriptor = NULL; 

    //
    // create stdin pipe
    //
    hOriginalWriteIn = CreateNamedPipe(
        L"\\\\.\\pipe\\rshellstdin",
        PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,    
        PIPE_WAIT | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
        1,
        4096,
        4096,
        0,
        NULL);
  
    if( INVALID_HANDLE_VALUE == hOriginalWriteIn )
    {
        Status = GetLastErrorOnFailure();
        goto StartProcess_CleanupAndExit;
    }

    _hReadIn = CreateFile(
            L"\\\\.\\pipe\\rshellstdin",
            GENERIC_READ,
            0,
            &saAttr,
            OPEN_EXISTING,
            0,
            NULL);

    if ( INVALID_HANDLE_VALUE == _hWriteOut )
    {
        Status = GetLastErrorOnFailure();
        goto StartProcess_CleanupAndExit;
    }

    //
    // create stdout pipe
    //
    _StdoutContext._hRead = CreateNamedPipe(
        L"\\\\.\\pipe\\rshellstdout",
        PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,    
        PIPE_WAIT | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
        1,
        4096,
        4096,
        0,
        NULL);
  
    if( INVALID_HANDLE_VALUE == _StdoutContext._hRead )
    {
        Status = GetLastErrorOnFailure();
        goto StartProcess_CleanupAndExit;
    }

    _hWriteOut = CreateFile(
        L"\\\\.\\pipe\\rshellstdout",
        GENERIC_WRITE,
        0,
        &saAttr,
        OPEN_EXISTING,
        0,
        NULL);

    if ( INVALID_HANDLE_VALUE == _hReadIn )
    {
        Status = GetLastErrorOnFailure();
        goto StartProcess_CleanupAndExit;
    }

    //
    // create stderr pipe
    // 
    _StderrContext._hRead = CreateNamedPipe(
        L"\\\\.\\pipe\\rshellstderr",
        PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,    
        PIPE_WAIT | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
        1,
        4096,
        4096,
        0,
        NULL);
  
    if( INVALID_HANDLE_VALUE == _StderrContext._hRead )
    {
        Status = GetLastErrorOnFailure();
        goto StartProcess_CleanupAndExit;
    }

    _hWriteError = CreateFile(
        L"\\\\.\\pipe\\rshellstderr",
        GENERIC_WRITE,
        0,
        &saAttr,
        OPEN_EXISTING,
        0,
        NULL);

    if ( INVALID_HANDLE_VALUE == _hWriteError )
    {
        Status = GetLastErrorOnFailure();
        goto StartProcess_CleanupAndExit;
    }


    //
    // duplicate the write end of stdin
    //
    bStatus = DuplicateHandle(
        GetCurrentProcess(),
        hOriginalWriteIn,
        GetCurrentProcess(),
        &_hWriteIn,
        0, 
        FALSE,
        DUPLICATE_SAME_ACCESS
        );

    if ( ! bStatus )
    {
        Status = GetLastErrorOnFailure();
        goto StartProcess_CleanupAndExit;
    }

    CLOSEHANDLE_IFVALID(hOriginalWriteIn);

    sinf.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;

    sinf.hStdInput = _hReadIn;
    sinf.hStdOutput = _hWriteOut;
    sinf.hStdError = _hWriteError;
    
    sinf.wShowWindow = SW_HIDE;
    
    bStatus = CreateProcess(
        NULL,
        wszCommandLine,
        NULL,
        NULL,
        TRUE,
        CREATE_SUSPENDED,
        NULL,
        NULL,
        &sinf,
        &pinf
        );

    if ( ! bStatus )
    {
        Status = GetLastErrorOnFailure();
        goto StartProcess_CleanupAndExit;
    }

    _hProcess = pinf.hProcess;

    _hThread = pinf.hThread;

StartProcess_CleanupAndExit:

    return Status;
}


LONG CConsole::InitIO()
{
    LONG Status;

    Status = ERROR_SUCCESS;

    _StdoutContext._szData = new BYTE [ READ_BUFFER_SIZE ];
    _StderrContext._szData = new BYTE [ READ_BUFFER_SIZE ];

    if ( ! _StdoutContext._szData || ! _StderrContext._szData )
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    return Status;
}


void CConsole::HandleIO()
{
    MSG    msg;
    DWORD  cObjects;

    HANDLE rgIOEvents[] = {
        _hProcess
    };

    cObjects = sizeof( rgIOEvents ) / sizeof ( *rgIOEvents );

    _StdoutContext.ReadData();
    _StderrContext.ReadData();

    for ( ;; )
    {
        DWORD dwReason;
        
        dwReason = MsgWaitForMultipleObjectsEx(
            cObjects,
            rgIOEvents,
            INFINITE,
            QS_ALLINPUT,
            MWMO_ALERTABLE
            );

        if ( dwReason == ( WAIT_OBJECT_0 + cObjects ) )
        {
            if ( GetMessage( &msg, NULL, 0, 0) )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
            else
            {
                break;
            }
        }
        else if ( dwReason == WAIT_IO_COMPLETION )
        {
        }
        else 
        {
            DebugOut(L"Object %d was signaled\n", dwReason);
            break;
        }
    }
}

void CConsole::Resume()
{
    ResumeThread( _hThread );
}
    
LONG CConsole::WriteData( 
    CHAR*  szData,
    DWORD  cchData
    )
{
    BOOL  bStatus;
    LONG  Status;

    Status = ERROR_SUCCESS;

    bStatus = WriteFileEx(
        _hWriteIn,
        szData,
        cchData,
        &_AsyncIO,
        WriteFinished);

    if ( ! bStatus )
    {
        Status = GetLastErrorOnFailure();
    }

    return Status;
}

void
CConsole::AddText( BYTE* rgText )
{
    SendMessageA( 
        _pMainWindow->GetEdit()->GetHandle(),
        EM_REPLACESEL,
        FALSE,
        (LPARAM) rgText
        );

    _pMainWindow->GetEdit()->MoveToEnd();
}

void 
CALLBACK CConsole::WriteFinished(
    DWORD        dwErrorCode,                // completion code
    DWORD        dwNumberOfBytesTransfered,  // number of bytes transferred
    LPOVERLAPPED lpOverlapped         // I/O information buffer
    )
{
    return;
}
