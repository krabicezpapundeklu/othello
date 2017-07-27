#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include <process.h>
#include <windows.h>

#include "othello.h"
#include "win_othello_res.h"

static othello_t board;
static enum { BLACKS_MOVE, WHITES_MOVE, GAME_OVER } state;

static struct {
        int x, y;       /* Position of the grid relative to window origin. */
        int size;       /* Size (width and height are equal) of the grid. */
        int cell_size;  /* Size of a grid cell, not including its border. */
        int sel_row;    /* Currently selected row, or -1 if none. */
        int sel_col;    /* Currently selected column. */
} grid;

#define CELL_GAP 1       /* Cell gap in pixels. */
#define MIN_SIZE 300     /* Minimum window size. */
#define INIT_SIZE 450    /* Initial window size. */
#define WM_WHITE_MOVE (WM_USER + 0)

static const char APP_NAME[] = "othello";
static HBRUSH background_brush, board_brush, highlight_brush,
              white_brush, black_brush, valid_brush;

static void err(const char *msg)
{
        MessageBoxA(NULL, msg, APP_NAME, MB_ICONHAND);
        exit(EXIT_FAILURE);
}

/* Compute the grid's size and position in the window. */
static void compute_grid_position(int win_width, int win_height)
{
        /* The grid is a 10x10 grid. The 8x8 centre is the Othello
           board, the top row and left column are used for labels, and the
           bottom row for status text. */

        grid.cell_size = (min(win_width, win_height) - 9 * CELL_GAP) / 10;
        grid.size = grid.cell_size * 10 + 9 * CELL_GAP;
        grid.x = win_width / 2 - grid.size / 2;
        grid.y = win_height / 2 - grid.size / 2;
}

/* Check whether the position is over an Othello cell. */
static bool grid_hit_test(int x, int y, int *row, int *col)
{
        *row = (y - grid.y) / (grid.cell_size + CELL_GAP) - 1;
        *col = (x - grid.x) / (grid.cell_size + CELL_GAP) - 1;

        if (*row >= 0 && *row < 8 && *col >= 0 && *col < 8) {
                return true;
        }

        return false;
}

/* Draw an Othello cell and its contents. */
static void draw_othello_cell(HDC dc, int row, int col)
{
        int x, y;
        bool highlight;
        cell_state_t cs;

        x = grid.x + (col + 1) * (grid.cell_size + CELL_GAP);
        y = grid.y + (row + 1) * (grid.cell_size + CELL_GAP);

        highlight = (row == grid.sel_row && col == grid.sel_col &&
                     state == BLACKS_MOVE);

        /* Draw the cell background. */
        SelectObject(dc, GetStockObject(NULL_PEN));
        SelectObject(dc, highlight ? highlight_brush : board_brush);
        Rectangle(dc, x, y, x + grid.cell_size, y + grid.cell_size);

        if ((cs = othello_cell_state(&board, row, col)) != CELL_EMPTY) {
                /* Draw the disk. */
                SelectObject(dc, cs == CELL_BLACK ? black_brush : white_brush);
                Ellipse(dc, x, y, x + grid.cell_size, y + grid.cell_size);
        } else if (state == BLACKS_MOVE && othello_is_valid_move(&board, PLAYER_BLACK, row, col)) {
                SelectObject(dc, valid_brush);
                Ellipse(dc, x, y, x + grid.cell_size, y + grid.cell_size);
        }
}

/* Draw string s of length len centered at (x,y). */
static void draw_string(HDC dc, const char *s, int len, int x, int y)
{
        SIZE size;

        GetTextExtentPoint32A(dc, s, len, &size);
        TextOut(dc, x - size.cx / 2, y - size.cy / 2, s, len);
}

/* Draw the grid and its contents. */
static void draw_grid(HDC dc)
{
        int row, col, x, y, bs, ws;
        char status[128];

        /* Draw a background square around the 8x8 centre cells. */
        SelectObject(dc, GetStockObject(NULL_PEN));
        SelectObject(dc, black_brush);
        Rectangle(dc, grid.x + grid.cell_size,
                      grid.y + grid.cell_size,
                      grid.x + 9 * grid.cell_size + 9 * CELL_GAP,
                      grid.y + 9 * grid.cell_size + 9 * CELL_GAP);

        /* Draw labels. */
        for (row = 0; row < 8; row++) {
                x = grid.x + grid.cell_size / 2;
                y = grid.y + (row + 1) * (grid.cell_size + CELL_GAP) +
                        grid.cell_size / 2;
                draw_string(dc, &"12345678"[row], 1, x, y);

        }
        for (col = 0; col < 8; col++) {
                x = grid.x + (col + 1) * (grid.cell_size + CELL_GAP) +
                        grid.cell_size / 2;
                y = grid.y + grid.cell_size / 2;
                draw_string(dc, &"ABCDEFGH"[col], 1, x, y);
        }

        /* Draw status text. */
        switch (state) {
        case BLACKS_MOVE:
                sprintf(status, "Human's move.");
                break;
        case WHITES_MOVE:
                sprintf(status, "Computer's move..");
                break;
        case GAME_OVER:
                bs = othello_score(&board, PLAYER_BLACK);
                ws = othello_score(&board, PLAYER_WHITE);
                if (bs > ws) {
                        sprintf(status, "Human wins %d-%d!", bs, ws);
                } else if (ws > bs) {
                        sprintf(status, "Computer wins %d-%d!", ws, bs);
                } else {
                        sprintf(status, "Draw!");
                }
        }
        draw_string(dc, status, strlen(status), grid.x + grid.size / 2,
                    grid.y + grid.size - grid.cell_size / 2);

        /* Draw cells. */
        for (row = 0; row < 8; row++) {
                for (col = 0; col < 8; col++) {
                        draw_othello_cell(dc, row, col);
                }
        }
}

static void compute_white_move(void *window)
{
        int row, col;

        assert(state == WHITES_MOVE);

        othello_compute_move(&board, PLAYER_WHITE, &row, &col);
        SendMessage((HWND)window, WM_WHITE_MOVE, row, col);
}

/* Make a move for the current player and transition the game state. */
static void make_move(HWND window, int row, int col)
{
        assert(state == BLACKS_MOVE || state == WHITES_MOVE);

        if (state == BLACKS_MOVE) {
                if (!othello_is_valid_move(&board, PLAYER_BLACK, row, col)) {
                        /* Illegal move; ignored. */
                        return;
                }

                othello_make_move(&board, PLAYER_BLACK, row, col);
                state = WHITES_MOVE;
        } else {
                othello_make_move(&board, PLAYER_WHITE, row, col);
                state = BLACKS_MOVE;
        }

        if (!othello_has_valid_move(&board, PLAYER_BLACK) &&
            !othello_has_valid_move(&board, PLAYER_WHITE)) {
                state = GAME_OVER;
        } else if (state == WHITES_MOVE &&
                   !othello_has_valid_move(&board, PLAYER_WHITE)) {
                state = BLACKS_MOVE;
        } else if (state == BLACKS_MOVE &&
                   !othello_has_valid_move(&board, PLAYER_BLACK)) {
                state = WHITES_MOVE;
        }

        if (state == WHITES_MOVE) {
                _beginthread(compute_white_move, 0, (void*)window);
        }

        InvalidateRect(window, NULL, TRUE);
}

static void new_game(void)
{
        othello_init(&board);
        state = BLACKS_MOVE;
}

static void on_mouse_click(HWND window)
{
        if (state == GAME_OVER) {
                new_game();
                InvalidateRect(window, NULL, TRUE);
                return;
        }

        if (state == BLACKS_MOVE && grid.sel_row >= 0) {
                make_move(window, grid.sel_row, grid.sel_col);
        }
}

/* Invalidate an Othello cell, causing it to be repainted. */
static void invalidate_othello_cell(HWND window, int row, int col)
{
        RECT r;

        r.left = grid.x + (col + 1) * (grid.cell_size + CELL_GAP);
        r.top = grid.y + (row + 1) * (grid.cell_size + CELL_GAP);
        r.right = r.left + grid.cell_size;
        r.bottom = r.top + grid.cell_size;

        InvalidateRect(window, &r, false);
}

static void select_othello_cell(HWND window, int row, int col)
{
        int old_row = grid.sel_row;
        int old_col = grid.sel_col;

        if (row == old_row && col == old_col) {
                /* This cell is already selected. */
                return;
        }

        grid.sel_row = row;
        grid.sel_col = col;

        if (old_row >= 0) {
                /* Re-draw the previously selected cell. */
                invalidate_othello_cell(window, old_row, old_col);
        }

        if (row >= 0) {
                /* Need to draw the newly selected cell. */
                invalidate_othello_cell(window, row, col);
        }
}


static void on_key_press(HWND window, WPARAM wparam)
{
        int row, col;

        row = grid.sel_row;
        col = grid.sel_col;

        switch (wparam) {
        default:
               return;
        case VK_SPACE:
        case VK_RETURN:
                on_mouse_click(window);
                return;

        case VK_RIGHT: col++; break;
        case VK_LEFT:  col--; break;
        case VK_DOWN:  row++; break;
        case VK_UP:    row--; break;

        case 'A': case 'B': case 'C': case 'D':
        case 'E': case 'F': case 'G': case 'H':
                col = wparam - 'A';
                break;

        case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8':
                row = wparam - '1';
                break;
        }

        select_othello_cell(window, max(0, min(row, 7)), max(0, min(col, 7)));
}

static LRESULT CALLBACK wnd_proc(HWND window, UINT message, WPARAM wparam,
                                 LPARAM lparam)
{
        HDC dc;
        PAINTSTRUCT ps;
        int row, col;

        switch (message) {
        case WM_SIZE:
                /* The window size changed. */
                compute_grid_position(LOWORD(lparam), HIWORD(lparam));
                return 0;

        case WM_GETMINMAXINFO:
                /* Windows is asking about the size constraints. */
                ((MINMAXINFO*)lparam)->ptMinTrackSize.x = MIN_SIZE;
                ((MINMAXINFO*)lparam)->ptMinTrackSize.y = MIN_SIZE;
                return 0;

        case WM_PAINT:
                /* The window needs a re-paint. */
                EnableMenuItem(GetMenu(window), IDM_NEW_GAME,
                               state != WHITES_MOVE ? MF_ENABLED : MF_GRAYED);
                dc = BeginPaint(window, &ps);
                SetBkMode(dc, TRANSPARENT);
                draw_grid(dc);
                EndPaint(window, &ps);
                return 0;

        case WM_MOUSEMOVE:
                /* The mouse moved. */
                if (grid_hit_test(LOWORD(lparam), HIWORD(lparam), &row, &col)) {
                        select_othello_cell(window, row, col);
                } else {
                        select_othello_cell(window, -1, -1);
                }
                return 0;

        case WM_LBUTTONUP:
                /* Left mouse button up. */
                on_mouse_click(window);
                return 0;

        case WM_KEYDOWN:
                /* Keyboard key down. */
                on_key_press(window, wparam);
                return 0;

        case WM_COMMAND:
                if (lparam == 0) {
                        /* Menu item activated. */
                        switch (LOWORD(wparam)) {
                        case IDM_NEW_GAME:
                                assert(state != WHITES_MOVE);
                                new_game();
                                InvalidateRect(window, NULL, TRUE);
                                return 0;

                        case IDM_EXIT:
                                PostQuitMessage(0);
                                return 0;
                        }
                }
                break;

        case WM_WHITE_MOVE:
                /* White computed a move. */
                assert(state == WHITES_MOVE);
                make_move(window, wparam, lparam);
                return 0;

        case WM_DESTROY:
                /* The window is closing. */
                PostQuitMessage(0);
                return 0;
        }

        return DefWindowProc(window, message, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prev_instance,
                   LPSTR cmd_line, int cmd_show)
{
        WNDCLASS window_class;
        HWND window;
        HACCEL accelerators;
        MSG message;

        grid.sel_row = -1;
        new_game();

        background_brush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        board_brush      = CreateSolidBrush(RGB(0x00, 0x80, 0x00));
        highlight_brush  = CreateSolidBrush(RGB(0x00, 0xAA, 0x00));
        white_brush      = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
        black_brush      = CreateSolidBrush(RGB(0, 0, 0));
        valid_brush      = CreateSolidBrush(RGB(0, 0x60, 0));

        window_class.style         = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc   = &wnd_proc;
        window_class.cbClsExtra    = 0;
        window_class.cbWndExtra    = 0;
        window_class.hInstance     = instance;
        window_class.hIcon         = LoadIcon(instance,
                                              MAKEINTRESOURCE(IDI_ICON));
        window_class.hCursor       = LoadCursor(NULL, IDC_ARROW);
        window_class.hbrBackground = background_brush;
        window_class.lpszMenuName  = MAKEINTRESOURCE(IDM_MENU);
        window_class.lpszClassName = APP_NAME;

        if (!RegisterClassA(&window_class)) {
                err("RegisterClassA failed!");
        }

        window = CreateWindowA(APP_NAME, "Othello", WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT,
                               INIT_SIZE, INIT_SIZE,
                               NULL, NULL, instance, NULL);

        ShowWindow(window, cmd_show);
        UpdateWindow(window);

        accelerators = LoadAccelerators(instance, MAKEINTRESOURCE(IDA_ACC));

        while (GetMessage(&message, NULL, 0, 0)) {
                if (!TranslateAccelerator(window, accelerators, &message)) {
                        TranslateMessage(&message);
                        DispatchMessage(&message);
                }
        }

        DeleteObject(background_brush);
        DeleteObject(board_brush);
        DeleteObject(highlight_brush);
        DeleteObject(white_brush);
        DeleteObject(black_brush);
        DeleteObject(valid_brush);

        return message.wParam;
}
