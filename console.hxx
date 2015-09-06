//****************************************************
// 
// console.hxx
//
//****************************************************

#if !defined(_CONSOLE_HXX_)
#define _CONSOLE_HXX_

#define READ_BUFFER_SIZE 4096

class CConsole;

struct OutputContext
{
    OutputContext( 
        CConsole* pConsole
        ) :
        _pConsole( pConsole ),
        _szData ( NULL ),
        _hRead( NULL )
    {
        memset( &_AsyncIO, 0, sizeof(_AsyncIO) );
        _AsyncIO.hEvent = this;
    }
    
    ~OutputContext()
    {
        delete [] _szData;
    }

    LONG ReadData();

    static void CALLBACK ShowOutput(
        DWORD        dwErrorCode,                // completion code
        DWORD        dwNumberOfBytesTransfered,  // number of bytes transferred
        LPOVERLAPPED lpOverlapped         // I/O information buffer
        );

    CConsole*  _pConsole;
    OVERLAPPED _AsyncIO;
    HANDLE     _hRead;
    BYTE*      _szData;
};

class CMainWindow;

class CConsole
{

public:

    CConsole( CMainWindow* pMainWindow );

    ~CConsole();

    LONG StartProcess( WCHAR* wszCommandLine );
    
    LONG InitIO();

    HANDLE GetProcessHandle();

    LONG WriteData( 
        CHAR*  wszData,
        DWORD  cchData
        );

    LONG ReadData( DWORD dwStream );

    void HandleIO();
    
    void Resume();

    void AddText( BYTE* rgText );
    
private:

    static void CALLBACK WriteFinished(
        DWORD        dwErrorCode,                // completion code
        DWORD        dwNumberOfBytesTransfered,  // number of bytes transferred
        LPOVERLAPPED lpOverlapped         // I/O information buffer
        );

    CMainWindow* _pMainWindow;

    HANDLE _hProcess;
    
    HANDLE _hReadIn;
    HANDLE _hWriteIn;
    
    HANDLE _hWriteOut;  
    
    HANDLE _hWriteError;

    DWORD  _cchStdout;
    DWORD  _cchStderr;

    OutputContext _StdoutContext;
    OutputContext _StderrContext;

    OVERLAPPED    _AsyncIO;

    HANDLE _hThread;
};


#endif // _CONSOLE_HXX_























