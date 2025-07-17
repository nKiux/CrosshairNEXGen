// --- Preprocessor Definitions ---
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0501
#define WINVER 0x0501
#define _WIN32_IE 0x0600

// --- Headers ---
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <vector>
#include <string>

// --- Libraries ---
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")

// --- Global Control IDs ---
enum {
    IDC_COMBO_STYLE = 101,
    IDC_BUTTON_COLOR,
    IDC_TRACKBAR_LENGTH,
    IDC_TRACKBAR_THICKNESS,
    IDC_TRACKBAR_GAP,
    IDC_TRACKBAR_DOTSIZE,
    IDC_TRACKBAR_OPACITY,
    IDC_CHECKBOX_OUTLINE
};

// --- Window Class Names ---
const WCHAR g_szCrosshairClassName[] = L"AdvancedCrosshairWindow";
const WCHAR g_szUIClassName[] = L"AdvancedCrosshairUI";

// --- Settings Structure ---
// By consolidating settings, we simplify state management and passing.
struct CrosshairSettings {
    COLORREF color;
    int style;
    int length;
    int thickness;
    int gap;
    int dotSize;
    BYTE opacity;
    bool drawOutline;
};

// --- Global Variables ---
CrosshairSettings g_settings = {
    RGB(0, 255, 0), // color
    0,              // style
    12,             // length
    2,              // thickness
    4,              // gap
    2,              // dotSize
    255,            // opacity
    true            // drawOutline
};

HWND g_hCrosshairWnd = NULL;
HFONT g_hFont = NULL;

const std::vector<std::wstring> g_styleNames = {
    L"Cross", L"Hollow Cross", L"Dot", L"T-Shape", L"Circle",
    L"Triangle", L"V-Shape", L"Dotted Cross", L"Inverted V",
    L"X-Cross", L"Dotted Circle"
};

// --- Function Declarations ---
void UpdateCrosshair();
void DrawCrosshair(HDC hdc, int width, int height);
LRESULT CALLBACK CrosshairWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK UIWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// --- Core Functions ---

void UpdateCrosshair() {
    if (g_hCrosshairWnd) {
        SetLayeredWindowAttributes(g_hCrosshairWnd, RGB(0, 0, 0), g_settings.opacity, LWA_COLORKEY | LWA_ALPHA);
        InvalidateRect(g_hCrosshairWnd, NULL, TRUE);
    }
}

void DrawCrosshair(HDC hdc, int width, int height) {
    int center_x = width / 2;
    int center_y = height / 2;

    auto draw_shape = [&](HPEN hPen) {
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        // Use a hollow brush for shapes that shouldn't be filled, and a solid one for those that should.
        // GetStockObject does not need to be deleted.
        HBRUSH hBrush = (g_settings.style == 2 || g_settings.style == 7 || g_settings.style == 10) 
            ? CreateSolidBrush(g_settings.color) 
            : (HBRUSH)GetStockObject(HOLLOW_BRUSH);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

        switch (g_settings.style) {
            case 0: // Cross
                MoveToEx(hdc, center_x - g_settings.length, center_y, NULL); LineTo(hdc, center_x + g_settings.length, center_y);
                MoveToEx(hdc, center_x, center_y - g_settings.length, NULL); LineTo(hdc, center_x, center_y + g_settings.length);
                break;
            case 1: // Hollow Cross
                MoveToEx(hdc, center_x - g_settings.length, center_y, NULL); LineTo(hdc, center_x - g_settings.gap, center_y);
                MoveToEx(hdc, center_x + g_settings.length, center_y, NULL); LineTo(hdc, center_x + g_settings.gap, center_y);
                MoveToEx(hdc, center_x, center_y - g_settings.length, NULL); LineTo(hdc, center_x, center_y - g_settings.gap);
                MoveToEx(hdc, center_x, center_y + g_settings.length, NULL); LineTo(hdc, center_x, center_y + g_settings.gap);
                break;
            case 2: // Dot
                Ellipse(hdc, center_x - g_settings.dotSize, center_y - g_settings.dotSize, center_x + g_settings.dotSize, center_y + g_settings.dotSize);
                break;
            case 3: // T-Shape
                MoveToEx(hdc, center_x - g_settings.length, center_y, NULL); LineTo(hdc, center_x + g_settings.length, center_y);
                MoveToEx(hdc, center_x, center_y - g_settings.length, NULL); LineTo(hdc, center_x, center_y);
                break;
            case 4: // Circle
                Ellipse(hdc, center_x - g_settings.length, center_y - g_settings.length, center_x + g_settings.length, center_y + g_settings.length);
                break;
            case 5: // Triangle
                {
                    POINT points[3] = {{center_x, center_y - g_settings.length}, {center_x - g_settings.length, center_y + g_settings.length / 2}, {center_x + g_settings.length, center_y + g_settings.length / 2}};
                    Polygon(hdc, points, 3);
                }
                break;
            case 6: // V-Shape
                MoveToEx(hdc, center_x - g_settings.length, center_y - g_settings.length, NULL); LineTo(hdc, center_x, center_y);
                LineTo(hdc, center_x + g_settings.length, center_y - g_settings.length);
                break;
            case 7: // Dotted Cross
                MoveToEx(hdc, center_x - g_settings.length, center_y, NULL); LineTo(hdc, center_x - g_settings.gap, center_y);
                MoveToEx(hdc, center_x + g_settings.length, center_y, NULL); LineTo(hdc, center_x + g_settings.gap, center_y);
                MoveToEx(hdc, center_x, center_y - g_settings.length, NULL); LineTo(hdc, center_x, center_y - g_settings.gap);
                MoveToEx(hdc, center_x, center_y + g_settings.length, NULL); LineTo(hdc, center_x, center_y + g_settings.gap);
                Ellipse(hdc, center_x - g_settings.dotSize, center_y - g_settings.dotSize, center_x + g_settings.dotSize, center_y + g_settings.dotSize);
                break;
            case 8: // Inverted V
                MoveToEx(hdc, center_x - g_settings.length, center_y + g_settings.length, NULL); LineTo(hdc, center_x, center_y);
                LineTo(hdc, center_x + g_settings.length, center_y + g_settings.length);
                break;
            case 9: // X-Cross
                MoveToEx(hdc, center_x - g_settings.length, center_y - g_settings.length, NULL); LineTo(hdc, center_x + g_settings.length, center_y + g_settings.length);
                MoveToEx(hdc, center_x + g_settings.length, center_y - g_settings.length, NULL); LineTo(hdc, center_x - g_settings.length, center_y + g_settings.length);
                break;
            // --- MODIFIED START ---
            case 10: // Dotted Circle
                // For this style, use 'gap' to control the circle's radius to ensure clear separation.
                // 'length' is unused for this specific style to provide intuitive control over the gap.
                
                // Draw the circle using 'gap' as the radius. Temporarily use a HOLLOW_BRUSH.
                SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
                Ellipse(hdc, center_x - g_settings.gap, center_y - g_settings.gap, center_x + g_settings.gap, center_y + g_settings.gap);
                
                // Restore the solid brush (which was set at the start of the lambda) and draw the dot.
                SelectObject(hdc, hBrush);
                Ellipse(hdc, center_x - g_settings.dotSize, center_y - g_settings.dotSize, center_x + g_settings.dotSize, center_y + g_settings.dotSize);
                break;
            // --- MODIFIED END ---
        }
        SelectObject(hdc, hOldPen);
        SelectObject(hdc, hOldBrush);
        if (hBrush != GetStockObject(HOLLOW_BRUSH)) {
            DeleteObject(hBrush); // Only delete brushes we created.
        }
    };

    // Draw outline first, if enabled
    if (g_settings.drawOutline) {
        HPEN hOutlinePen = CreatePen(PS_SOLID, g_settings.thickness + 2, RGB(0, 0, 0));
        draw_shape(hOutlinePen);
        DeleteObject(hOutlinePen);
    }
    
    // Draw the main crosshair
    HPEN hMainPen = CreatePen(PS_SOLID, g_settings.thickness, g_settings.color);
    draw_shape(hMainPen);
    DeleteObject(hMainPen);
}

// --- Window Procedures ---

LRESULT CALLBACK CrosshairWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
            UpdateCrosshair();
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(hwnd, &rect);
            DrawCrosshair(hdc, rect.right, rect.bottom);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Helper function to create UI controls, reducing code duplication.
HWND CreateControl(HWND hParent, const WCHAR* className, const WCHAR* text, DWORD style,
                   int x, int y, int w, int h, HMENU id) {
    HWND hCtrl = CreateWindowW(className, text, WS_CHILD | WS_VISIBLE | style,
                               x, y, w, h, hParent, id, NULL, NULL);
    SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, TRUE);
    return hCtrl;
}


LRESULT CALLBACK UIWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Layout constants for easier UI management
    const int PADDING = 10;
    const int GROUP_WIDTH = 280;
    const int CONTROL_X = PADDING + 10;
    const int LABEL_W = 80;
    const int TRACK_W = 170;
    const int ROW_H = 35;

    switch (msg) {
        case WM_CREATE: {
            g_hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            
            int yPos = PADDING;

            // Style Group
            CreateControl(hwnd, L"BUTTON", L"Style", BS_GROUPBOX, PADDING, yPos, GROUP_WIDTH, 60, NULL);
            HWND hCombo = CreateControl(hwnd, L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VSCROLL, CONTROL_X, yPos + 20, GROUP_WIDTH - 20, 150, (HMENU)IDC_COMBO_STYLE);
            for (const auto& name : g_styleNames) {
                SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)name.c_str());
            }
            SendMessageW(hCombo, CB_SETCURSEL, (WPARAM)g_settings.style, 0);
            yPos += 70;

            // Appearance Group
            CreateControl(hwnd, L"BUTTON", L"Appearance", BS_GROUPBOX, PADDING, yPos, GROUP_WIDTH, 230, NULL);
            yPos += 20;
            CreateControl(hwnd, L"BUTTON", L"Change Color", 0, CONTROL_X, yPos, 120, 30, (HMENU)IDC_BUTTON_COLOR);
            HWND hCheckOutline = CreateControl(hwnd, L"BUTTON", L"Enable Outline", BS_AUTOCHECKBOX, 150, yPos + 5, 120, 20, (HMENU)IDC_CHECKBOX_OUTLINE);
            SendMessageW(hCheckOutline, BM_SETCHECK, g_settings.drawOutline ? BST_CHECKED : BST_UNCHECKED, 0);
            yPos += 40;

            // Sliders
            CreateControl(hwnd, L"STATIC", L"Length:", 0, CONTROL_X, yPos + 5, LABEL_W, 20, NULL);
            HWND hTrackLength = CreateControl(hwnd, TRACKBAR_CLASSW, L"", TBS_AUTOTICKS, CONTROL_X + LABEL_W, yPos, TRACK_W, 30, (HMENU)IDC_TRACKBAR_LENGTH);
            SendMessageW(hTrackLength, TBM_SETRANGE, TRUE, MAKELONG(1, 50));
            SendMessageW(hTrackLength, TBM_SETPOS, TRUE, g_settings.length);
            yPos += ROW_H;

            CreateControl(hwnd, L"STATIC", L"Thickness:", 0, CONTROL_X, yPos + 5, LABEL_W, 20, NULL);
            HWND hTrackThick = CreateControl(hwnd, TRACKBAR_CLASSW, L"", TBS_AUTOTICKS, CONTROL_X + LABEL_W, yPos, TRACK_W, 30, (HMENU)IDC_TRACKBAR_THICKNESS);
            SendMessageW(hTrackThick, TBM_SETRANGE, TRUE, MAKELONG(1, 20));
            SendMessageW(hTrackThick, TBM_SETPOS, TRUE, g_settings.thickness);
            yPos += ROW_H;
            
            CreateControl(hwnd, L"STATIC", L"Gap:", 0, CONTROL_X, yPos + 5, LABEL_W, 20, NULL);
            HWND hTrackGap = CreateControl(hwnd, TRACKBAR_CLASSW, L"", TBS_AUTOTICKS, CONTROL_X + LABEL_W, yPos, TRACK_W, 30, (HMENU)IDC_TRACKBAR_GAP);
            SendMessageW(hTrackGap, TBM_SETRANGE, TRUE, MAKELONG(0, 30));
            SendMessageW(hTrackGap, TBM_SETPOS, TRUE, g_settings.gap);
            yPos += ROW_H;

            CreateControl(hwnd, L"STATIC", L"Dot Size:", 0, CONTROL_X, yPos + 5, LABEL_W, 20, NULL);
            HWND hTrackDot = CreateControl(hwnd, TRACKBAR_CLASSW, L"", TBS_AUTOTICKS, CONTROL_X + LABEL_W, yPos, TRACK_W, 30, (HMENU)IDC_TRACKBAR_DOTSIZE);
            SendMessageW(hTrackDot, TBM_SETRANGE, TRUE, MAKELONG(1, 20));
            SendMessageW(hTrackDot, TBM_SETPOS, TRUE, g_settings.dotSize);
            yPos += 45;

            // Advanced Group
            CreateControl(hwnd, L"BUTTON", L"Advanced", BS_GROUPBOX, PADDING, yPos, GROUP_WIDTH, 70, NULL);
            yPos += 20;
            CreateControl(hwnd, L"STATIC", L"Opacity:", 0, CONTROL_X, yPos + 5, LABEL_W, 20, NULL);
            HWND hTrackOpacity = CreateControl(hwnd, TRACKBAR_CLASSW, L"", TBS_AUTOTICKS, CONTROL_X + LABEL_W, yPos, TRACK_W, 30, (HMENU)IDC_TRACKBAR_OPACITY);
            SendMessageW(hTrackOpacity, TBM_SETRANGE, TRUE, MAKELONG(30, 255));
            SendMessageW(hTrackOpacity, TBM_SETPOS, TRUE, g_settings.opacity);

            break;
        }
        case WM_COMMAND: {
            WORD id = LOWORD(wParam);
            WORD code = HIWORD(wParam);
            if (code == CBN_SELCHANGE && id == IDC_COMBO_STYLE) {
                g_settings.style = SendMessageW((HWND)lParam, CB_GETCURSEL, 0, 0);
            } else if (id == IDC_BUTTON_COLOR) {
                CHOOSECOLORW cc = { sizeof(cc) };
                static COLORREF customColors[16];
                cc.hwndOwner = hwnd;
                cc.rgbResult = g_settings.color;
                cc.lpCustColors = customColors;
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;
                if (ChooseColorW(&cc)) {
                    g_settings.color = cc.rgbResult;
                }
            } else if (id == IDC_CHECKBOX_OUTLINE) {
                g_settings.drawOutline = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
            }
            UpdateCrosshair();
            break;
        }
        case WM_HSCROLL: {
            HWND hTrack = (HWND)lParam;
            int pos = SendMessageW(hTrack, TBM_GETPOS, 0, 0);
            int id = GetDlgCtrlID(hTrack);
            switch(id) {
                case IDC_TRACKBAR_LENGTH:   g_settings.length = pos; break;
                case IDC_TRACKBAR_THICKNESS:g_settings.thickness = pos; break;
                case IDC_TRACKBAR_GAP:      g_settings.gap = pos; break;
                case IDC_TRACKBAR_DOTSIZE:  g_settings.dotSize = pos; break;
                case IDC_TRACKBAR_OPACITY:  g_settings.opacity = pos; break;
            }
            UpdateCrosshair();
            break;
        }
        case WM_DESTROY: {
            if (g_hFont) DeleteObject(g_hFont);
            DestroyWindow(g_hCrosshairWnd);
            PostQuitMessage(0);
            break;
        }
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// --- Program Entry Point ---
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow) {
    InitCommonControls();

    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.lpfnWndProc = UIWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = g_szUIClassName;
    RegisterClassExW(&wc);

    wc.lpfnWndProc = CrosshairWndProc;
    wc.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wc.lpszClassName = g_szCrosshairClassName;
    RegisterClassExW(&wc);

    HWND hUIWnd = CreateWindowExW(WS_EX_TOPMOST, g_szUIClassName, L"Advanced Crosshair Settings",
                                 WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_MINIMIZEBOX,
                                 CW_USEDEFAULT, CW_USEDEFAULT, 315, 430,
                                 NULL, NULL, hInstance, NULL);
    if (!hUIWnd) return 1;

    int screen_x = GetSystemMetrics(SM_CXSCREEN);
    int screen_y = GetSystemMetrics(SM_CYSCREEN);
    int crosshair_size = 200; // Increased size to prevent clipping on large crosshairs
    g_hCrosshairWnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED, g_szCrosshairClassName, NULL, WS_POPUP,
                                     (screen_x - crosshair_size) / 2, (screen_y - crosshair_size) / 2,
                                     crosshair_size, crosshair_size, NULL, NULL, hInstance, NULL);
    if (!g_hCrosshairWnd) return 1;

    ShowWindow(hUIWnd, nCmdShow);
    ShowWindow(g_hCrosshairWnd, SW_SHOW);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}