//*********************************************
//
// window.hxx
//
//*********************************************

#if !defined(_WINDOW_HXX_)
#define _WINDOW_HXX_

#define INPUT_LINEBUF_SIZE 2048

class CWindow
{
public:
    
    CWindow();

    ~CWindow();

    LONG Initialize(
        WCHAR* wszClass,
        BOOL   bNewClass = FALSE
        );

    LONG Create( 
        HWND   hParent,
        WCHAR* wszTitle,
        DWORD  dwStyle,
        DWORD  dwExStyle = 0
        );

    HWND GetHandle();

    virtual void MessagePump();

    void SyncWindow( CWindow* pOther );

    static LRESULT CALLBACK DefaultWindowProc(
        HWND   hWnd,
        UINT   message,
        WPARAM wParam,
        LPARAM lParam);

protected:

    HWND       _hWnd;
    HWND       _hParent;

    WNDCLASS   _WindowClass;

    WNDPROC _pfnWindowProc;
};


class CControl : public CWindow
{
public:
    CControl( int Id );
    
private:
    
    int _Id;
};

class CEditControl: public CControl
{
public:

    CEditControl( int Id );
    ~CEditControl();

    void SubClass();
    void UnsubClass();

    CHAR* ReadTextLine(
        CHAR*    szDefault,
        LRESULT* pchDefault
        );

    void MoveToEnd();

    void Clear();

    void SetColors(
        COLORREF* pTextColor,
        COLORREF* pBackColor);

    void SetFont( WCHAR* wszFont );

private:

    static LRESULT CALLBACK EditProc(
        HWND    hwnd,	
        UINT    message,
        WPARAM  wParam,	
        LPARAM  lParam);

    LRESULT DispatchKeyStrokes(
        HWND    hwnd,	
        WPARAM  wParam,	
        LPARAM  lParam);


    WNDPROC     _pfnOld;
};

class CBackWindow : public CWindow
{

public:

    CBackWindow() 
    {
        _WindowClass.lpfnWndProc = WindowProc;
    }
        
    static LRESULT CALLBACK WindowProc(
        HWND   hWnd,
        UINT   message,
        WPARAM wParam,
        LPARAM lParam);

};


class CMainWindow : public CWindow
{

public:

    friend class CEditControl;

    CMainWindow();
    ~CMainWindow();

    LONG InitializeMain( WCHAR* wszCommandLine );
    
    void ShowMain();

    static LRESULT CALLBACK WindowProc(
        HWND   hWnd,
        UINT   message,
        WPARAM wParam,
        LPARAM lParam);

    LRESULT DispatchKeyStrokes(
        HWND    hwnd,	
        WPARAM  wParam,	
        LPARAM  lParam);

    CEditControl* GetEdit()
    {
        return &_EditControl;
    }

    CEditControl* GetInputEdit()
    {
        return &_InputEdit;
    }

private:

    void PositionEdit();

    LONG StartProcess();

    LONG SendLine();
    
    static CMainWindow* _pMainWindow;
    
    WCHAR*              _wszCommandLine;

    CBackWindow         _BackWindow;

    CEditControl        _EditControl;
    CEditControl        _InputEdit;

    CConsole            _Console;

    int                 _xSize;
    int                 _ySize; 

    CHAR*               _szLineBuf;

    HINSTANCE           _hRichEdit;
};


#endif // _WINDOW_HXX_

















