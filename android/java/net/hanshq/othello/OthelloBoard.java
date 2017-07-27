package net.hanshq.othello;

public class OthelloBoard {
    static { System.loadLibrary("othello"); }

    private long mNativeBoard;
    private native void nativeInit();
    private native void nativeDestroy();

    // These must match the native implementation's values.
    static final int CELL_BLACK = 0;
    static final int CELL_WHITE = 1;
    static final int CELL_EMPTY = 2;
    static final int PLAYER_BLACK = 0;
    static final int PLAYER_WHITE = 1;

    public OthelloBoard() { nativeInit(); }
    public void destroy() { nativeDestroy(); }

    public native void reset();
    public native int getCellState(int row, int col);
    public native void setCellState(int row, int col, int state);
    public native int getScore(int player);
    public native boolean hasValidMove(int player);
    public native boolean isValidMove(int player, int row, int col);
    public native void makeMove(int player, int row, int col);
    public native int[] computeMove(int player);
}
