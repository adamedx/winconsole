//****************************************************
//
// main.cxx
//
//****************************************************

#include "ash.hxx"

void __cdecl wmain( int argc, WCHAR** argv )
{
    LONG Status;

    if ( argc < 2 )
    {
        PrintUsage( argv );
        return;
    }

    CMainWindow MainWindow;

    Status = MainWindow.InitializeMain( argv[1] );

    if ( ERROR_SUCCESS == Status )
    {
        MainWindow.ShowMain();
    }

    return;
}


void PrintUsage( WCHAR** argv )
{
    WCHAR UsageBuf[1024];

    wsprintf(
        UsageBuf,
        L"Usage: \n"
        L"\t%s <command line>",
        argv[0]
        );

    (void) MessageBox(
        NULL,
        UsageBuf,
        L"Ash - Incorrect Command Line Parameters",
        MB_OK |
        MB_ICONINFORMATION |
        MB_APPLMODAL
        );
}



