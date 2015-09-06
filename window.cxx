//*********************************************
//
// window.cxx
//
//*********************************************

#include "ash.hxx"

CMainWindow* CMainWindow::_pMainWindow = NULL;

CWindow::CWindow() :
    _hWnd(NULL),
    _pfnWindowProc( DefaultWindowProc )
{
    _WindowClass.lpfnWndProc = DefaultWindowProc;
}

CWindow::~CWindow()
{}

LONG CWindow::Initialize(
    WCHAR* wszClass,
    BOOL   bNewClass
    )
{
    LONG    Status;
    HCURSOR hArrowCursor;

    Status = ERROR_SUCCESS;

    _WindowClass.lpszClassName = wszClass;

    if ( bNewClass )
    {
        ATOM    RegistrationAtom;

        hArrowCursor = LoadCursor( NULL, IDC_ARROW );

        if ( ! hArrowCursor )
        {
            return GetLastErrorOnFailure();
        }

        _WindowClass.hInstance = NULL;
        _WindowClass.style = CS_HREDRAW | CS_VREDRAW;
        _WindowClass.cbClsExtra = 0;
        _WindowClass.cbWndExtra = 0;
        _WindowClass.hIcon = NULL;
        _WindowClass.hCursor = hArrowCursor;
        _WindowClass.hbrBackground = CreateSolidBrush(0);
        // (HBRUSH) COLOR_WINDOW;
//        _WindowClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);  
        _WindowClass.lpszMenuName = NULL;

        RegistrationAtom = RegisterClass(&_WindowClass);
        
        if ( ! RegistrationAtom )
        {
            Status = GetLastErrorOnFailure();
        }
    }

    return Status;
}

LONG CWindow::Create( 
    HWND   hParent,
    WCHAR* wszTitle,
    DWORD  dwStyle,
    DWORD  dwExStyle
    )
{
    LONG Status;

    Status = ERROR_SUCCESS;

    _hWnd = CreateWindowEx(
        dwExStyle,
        _WindowClass.lpszClassName,
        wszTitle,
        dwStyle,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        hParent,
        NULL,
        NULL,
        NULL);

    if ( ! _hWnd )
    {
        Status = GetLastErrorOnFailure();

        DebugOut( L"CreateWindow for class %s, title %s failed with error 0x%x", 
                  _WindowClass.lpszClassName, wszTitle, Status );
    }

    return Status;
}

HWND CWindow::GetHandle()
{
    return _hWnd;
}

void CWindow::MessagePump()
{
    MSG msg;

    while(GetMessage( &msg, NULL, 0, 0) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }
}


void CWindow::SyncWindow( CWindow* pOther )
{
    BOOL       bGetRect;
    RECT       Rect;
    
    bGetRect = GetWindowRect(
        _hWnd,
        &Rect);

    if ( bGetRect )
    {
        MoveWindow(
            pOther->GetHandle(),
            Rect.left,
            Rect.top,
            Rect.right - Rect.left + 1,
            Rect.bottom - Rect.top + 1,
            TRUE);  
    }
}

LRESULT CALLBACK CWindow::DefaultWindowProc(
    HWND    hwnd,	
    UINT    uMsg,	
    WPARAM  wParam,	
    LPARAM  lParam)
{
    switch( uMsg )
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc( 
            hwnd,
            uMsg,
            wParam,
            lParam);
    }

    return 0;
}

CControl::CControl( int Id ) :
    _Id ( Id )
{}

CEditControl::CEditControl( int Id ) :
    CControl( Id ),
    _pfnOld(NULL)
{}

CEditControl::~CEditControl()
{
    
}

void CEditControl::SubClass()
{
    _pfnOld = (WNDPROC) SetWindowLong(
        _hWnd,
        GWL_WNDPROC,
        (DWORD) EditProc);
}

void CEditControl::UnsubClass()
{
    if ( _pfnOld )
    {
        (void) SetWindowLong(
            _hWnd,
            GWL_WNDPROC,
            (DWORD) _pfnOld);

        _pfnOld = NULL;
    }
}

void CEditControl::SetColors(
    COLORREF* pTextColor,
    COLORREF* pBackColor)                       
{
    CHARFORMAT2 TextFormat;
    
    memset( &TextFormat, 0, sizeof( TextFormat ) );

    TextFormat.cbSize = sizeof( TextFormat );

    TextFormat.dwMask = 0;

    if ( pTextColor )
    {
        TextFormat.dwMask |= CFM_COLOR;
        TextFormat.crTextColor = *pTextColor;
    }

    if ( pBackColor )
    {
        SendMessage(
            _hWnd,
            EM_SETBKGNDCOLOR,
            0,
            (LPARAM) *pBackColor);
    }

    if ( 0 == TextFormat.dwMask )
    {
        return;
    }  

    SendMessage(
        _hWnd,
        EM_SETCHARFORMAT,
        SCF_ALL,
        (LPARAM) &TextFormat);
}

void CEditControl::SetFont( WCHAR* wszFont )
{
    CHARFORMAT2 TextFormat;

    memset( &TextFormat, 0, sizeof( TextFormat ) );

    TextFormat.cbSize = sizeof( TextFormat );

    TextFormat.dwMask = CFM_FACE;

    lstrcpy( TextFormat.szFaceName, wszFont );

    SendMessage(
        _hWnd,
        EM_SETCHARFORMAT,
        SCF_ALL,
        (LPARAM) &TextFormat);
}

LRESULT CALLBACK CEditControl::EditProc(
    HWND    hwnd,	
    UINT    message,
    WPARAM  wParam,	
    LPARAM  lParam)
{
    LRESULT lHandled;

    lHandled = 1;

    switch ( message )
    {
    case WM_KEYUP:
    
         lHandled = CMainWindow::_pMainWindow->_EditControl.DispatchKeyStrokes(
            hwnd,
            wParam,
            lParam
            );

        break;
    
    case WM_DESTROY:
        
        CMainWindow::_pMainWindow->_EditControl.UnsubClass();

        SendMessage(
            hwnd,
            message,
            wParam,
            lParam);

        lHandled = 0;

        break;


    default:

        break;
    }
     
    if ( ! lHandled )
    {
        return 0;
    }

    switch ( message )
    {
        case EM_SETSEL:
        case EM_REPLACESEL:
            break;
        default:

            (void) SendMessage( 
                CMainWindow::_pMainWindow->GetInputEdit()->GetHandle(),
                message,
                wParam,
                lParam);
            break;
    }

    if ( CMainWindow::_pMainWindow->_EditControl._pfnOld )
    {
        return CMainWindow::_pMainWindow->_EditControl._pfnOld(
            hwnd,
            message,
            wParam,
            lParam);
    }

    return 1;
}

CHAR* CEditControl::ReadTextLine(
    CHAR*    szLineBuf,
    LRESULT* pchBuf
    )
{
    LRESULT CurrentLine;
    LRESULT cchLen;

    CHAR*   szResult;

    szResult = NULL;

    CurrentLine = SendMessage(
        _hWnd,
        EM_LINEFROMCHAR,
        -1,
        0
        );

    if ( -1 == CurrentLine )
    {
        return NULL;
    }

    CurrentLine--;

    *((WORD*) szLineBuf) = (WORD) *pchBuf;

    cchLen = SendMessageA(
        _hWnd,
        EM_GETLINE,
        CurrentLine,
        (LPARAM) szLineBuf
        );

    if ( cchLen )
    {
        szResult = szLineBuf,
        *pchBuf = cchLen;
    }

    if ( szResult )
    {
        szResult[cchLen] = L'\n';

        (*pchBuf)++;
    }
        
    return szResult;
}

LRESULT CEditControl::DispatchKeyStrokes(
    HWND    hwnd,	
    WPARAM  wParam,	
    LPARAM  lParam)
{
    switch ( wParam )
    {
    case VK_RETURN:
    
        (void) CMainWindow::_pMainWindow->SendLine();
        (void) CMainWindow::_pMainWindow->GetInputEdit()->Clear();
        break;

    default:
     
        return 1;
    }
     
    return 0;
}

void CEditControl::MoveToEnd()
{
    LRESULT LastChar;

    LastChar = SendMessage( 
        _hWnd,
        EM_POSFROMCHAR,
        0,
        -1
        );

    LastChar--;

    SendMessage(
        _hWnd,
        EM_SETSEL,
        -1,
        -1
        );
}

void CEditControl::Clear()
{ 
    SendMessageA(
        _hWnd,
        WM_SETTEXT,
        NULL,
        (LPARAM)NULL);
}

LONG CMainWindow::InitializeMain( WCHAR* wszCommandLine )
{
    LONG Status;

    Status = ERROR_SUCCESS;

    _wszCommandLine = wszCommandLine;

    _szLineBuf = new CHAR [ INPUT_LINEBUF_SIZE ];

    if ( ! _szLineBuf )
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    if ( ERROR_SUCCESS == Status )
    {
        Status = StartProcess();
    }

    if ( ERROR_SUCCESS == Status )
    {
        Status = _Console.InitIO();
    }

    if ( ERROR_SUCCESS == Status )
    {
        Status = _BackWindow.Initialize(
            L"BackClass",
            TRUE
            );
    }

    if ( ERROR_SUCCESS == Status )
    {
        Status = Initialize(
            L"_AshClass",
            TRUE
            );
    }

    if ( ERROR_SUCCESS == Status )
    {
        //Status = _EditControl.Initialize( L"edit" );
        Status = _EditControl.Initialize( RICHEDIT_CLASS );
        // Status = _EditControl.Initialize( L"edit" );
    } 

    if ( ERROR_SUCCESS == Status )
    {
        Status = _InputEdit.Initialize( L"edit" );
    } 

    return Status;
}

CMainWindow::CMainWindow() :
    _EditControl(1),
    _InputEdit(2),
    _xSize(0),
    _ySize(0),
    _wszCommandLine( NULL ),
    _Console( this ),
    _szLineBuf( NULL ),
    _hRichEdit( NULL )
{
    _pMainWindow = this;
    _WindowClass.lpfnWndProc = WindowProc;
}

CMainWindow::~CMainWindow()
{
    delete [] _szLineBuf;

    if ( _hRichEdit )
    {
        FreeLibrary( _hRichEdit );
    }
}

void CMainWindow::ShowMain()
{
    LONG Status;

    Status = _BackWindow.Create(
        HWND_DESKTOP,
        NULL,
        WS_VISIBLE,
        WS_EX_LAYERED
        );

    /*if ( ERROR_SUCCESS == Status )
    {
        HBRUSH   BlackBrush;

        BlackBrush = GetSysColorBrush( 0 );
  
        if ( BlackBrush )
        {
            HDC hdc;
            
            hdc = GetDC( _BackWindow.GetHandle() );

            if ( hdc )
            {
                SelectObject(
                    hdc,
                    BlackBrush);

                ReleaseDC( _BackWindow.GetHandle(), hdc );
            }
        }
    }*/

    if ( ERROR_SUCCESS == Status )
    {
        BOOL     bSuccess;
        COLORREF BackColor;

        BackColor = RGB( 0, 0, 255 );

        bSuccess = SetLayeredWindowAttributes(                 
            _BackWindow.GetHandle(),
            BackColor,
            192,
            LWA_ALPHA);

        if ( ! bSuccess )
        {
            Status = GetLastErrorOnFailure();

            DebugOut( L"SetLayeredWindowAttributes failed with 0x%x", Status );
        }
    }

    if ( ERROR_SUCCESS == Status )
    {
        Status = Create(
            HWND_DESKTOP,
            _wszCommandLine,
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            WS_EX_LAYERED
            );
    }

    SyncWindow(&_BackWindow);

    if ( ERROR_SUCCESS == Status )
    {
        BOOL     bSuccess;
        COLORREF BackColor;

        BackColor = RGB( 0, 0, 255 );

        bSuccess = SetLayeredWindowAttributes(                 
            _hWnd,
            BackColor,
            255,
            LWA_COLORKEY);

        if ( ! bSuccess )
        {
            Status = GetLastErrorOnFailure();

            DebugOut( L"SetLayeredWindowAttributes failed with 0x%x", Status );
        }
    }

    if ( ERROR_SUCCESS == Status )
    {
        /*Status = _EditControl.Create(
            _hWnd,
            L"",
            WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL );*/

        Status = _EditControl.Create(
            _hWnd,
            L"",
            WS_VISIBLE | WS_CHILD | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL);
    }

    if ( ERROR_SUCCESS == Status )  
    {
        Status = _InputEdit.Create(
            _hWnd,
            L"",
            WS_CHILD | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL );
    }

    if ( ERROR_SUCCESS == Status )
    {
        COLORREF ForeColor;
        COLORREF BackColor;

        ForeColor = RGB( 255, 255, 0 );
        BackColor = RGB( 0, 0, 255 );

        _EditControl.SubClass();

        PositionEdit();

        _EditControl.SetColors(&ForeColor, &BackColor);

        _EditControl.SetFont( L"Courier New Bold Italic" );

        _Console.Resume();

        _Console.HandleIO();
    }
}

void CMainWindow::PositionEdit()
{
    if ( _pMainWindow->_EditControl.GetHandle() )
    {
        MoveWindow(
            _pMainWindow->_EditControl.GetHandle(),
            0,
            0,
            _xSize,
            _ySize,
            TRUE);  

        SetFocus( _pMainWindow->_EditControl.GetHandle() );
    }
}


LRESULT CALLBACK CBackWindow::WindowProc(
        HWND   hwnd,
        UINT   message,
        WPARAM wParam,
        LPARAM lParam)
{
    return CWindow::DefaultWindowProc( 
        hwnd,
        message,
        wParam,
        lParam);
}

LRESULT CALLBACK CMainWindow::WindowProc(
    HWND    hwnd,	
    UINT    uMsg,	
    WPARAM  wParam,	
    LPARAM  lParam)
{
    switch( uMsg )
    {
    case WM_CREATE:

        _pMainWindow->_hRichEdit = LoadLibrary(L"Riched20.dll");

        if ( ! _pMainWindow->_hRichEdit )
        {
            LONG Status;

            Status = GetLastErrorOnFailure();

            DebugOut( L"LoadLibrary for Riched20.dll failed with error 0x%x", Status );

            return 1;
        }

        break;

    case WM_SIZE:

        _pMainWindow->_xSize = LOWORD( lParam );
        _pMainWindow->_ySize = HIWORD( lParam );    

        //
        // Set edit control size to be that of 
        // entire client area of main window
        //
        _pMainWindow->PositionEdit();

        break;

    case WM_MOVE:
        _pMainWindow->SyncWindow(&_pMainWindow->_BackWindow);
        break;

    case WM_SETFOCUS: 

    {
        HWND hEdit;

        hEdit = _pMainWindow->_EditControl.GetHandle();

        if ( hEdit )
        {
            SetFocus( hEdit );
        }
    }
    
        break;

    default:

        return CWindow::DefaultWindowProc( 
            hwnd,
            uMsg,
            wParam,
            lParam);
    }

    return 0;
}

LONG CMainWindow::StartProcess()
{
    return _Console.StartProcess( _wszCommandLine );
}

LONG CMainWindow::SendLine()
{
    LRESULT cchChars;
    LRESULT cchLine;
    LONG    Status;

    Status = ERROR_SUCCESS;

    cchLine = INPUT_LINEBUF_SIZE;

    /*szLineBuf = _EditControl.ReadTextLine(
        _szLineBuf,
        &cchLine);*/

/*    szLineBuf = _InputEdit.ReadTextLine(
        _szLineBuf,
        &cchLine); */

    cchChars = SendMessageA(
        _InputEdit.GetHandle(),
        WM_GETTEXT,
        cchLine,
        (LPARAM) _szLineBuf);

    if ( cchChars )
    {
        Status = _Console.WriteData(
            _szLineBuf,
            cchChars);
    }
    
    return Status;
}

