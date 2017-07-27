package net.hanshq.othello;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

public class OthelloView extends View {
    private String mStatus;
    private OthelloBoard mBoard;
    private TouchHandler mTouchHandler;

    private final int CELL_GAP = 2;
    private int cellSize, xOffset, yOffset, size;

    public interface TouchHandler {
        public void onTouch(int row, int col);
    }

    public OthelloView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public void setBoard(OthelloBoard board) {
        mBoard = board;
    }

    public void setTouchHandler(TouchHandler handler) {
        mTouchHandler = handler;
    }

    public void update(OthelloActivity.State state) {
        switch (state) {
        case BLACKS_MOVE:
            mStatus = "Human's move.";
            break;
        case WHITES_MOVE:
            mStatus = "Computer's move...";
            break;
        case GAME_OVER:
            int bs = mBoard.getScore(OthelloBoard.PLAYER_BLACK);
            int ws = mBoard.getScore(OthelloBoard.PLAYER_WHITE);

            if (bs > ws) {
                mStatus = "Human wins " + bs + "-" + ws + "!";
            } else if (ws > bs) {
                mStatus = "Computer wins " + ws + "-" + bs + "!";
            } else {
                mStatus = "Draw!";
            }
        }

        invalidate();
    }

    @Override
    public boolean onTouchEvent(MotionEvent e) {
        if (e.getAction() != MotionEvent.ACTION_DOWN) {
            return false;
        }

        int row = (int)(e.getY() - yOffset) / (cellSize + CELL_GAP) - 1;
        int col = (int)(e.getX() - xOffset) / (cellSize + CELL_GAP) - 1;

        if (row >= 0 && row < 8 && col >= 0 && col < 8) {
            mTouchHandler.onTouch((int)row, (int)col);
        }

        return true;
    }

    @Override
    protected void onDraw(Canvas canvas) {
        // Compute size and position of the 10x10 grid.
        int viewSize = Math.min(canvas.getWidth(), canvas.getHeight());
        cellSize = (viewSize - 9 * CELL_GAP) / 10;
        size = cellSize * 10 + 9 * CELL_GAP;
        xOffset = canvas.getWidth() / 2 - size / 2;
        yOffset = canvas.getHeight() / 2 - size / 2;

        // Draw labels.
        for (int row = 0; row < 8; row++) {
            int x = xOffset + cellSize / 2;
            int y = yOffset + (row + 1) * (cellSize + CELL_GAP) + cellSize / 2;
            drawChar((char)('1' + row), x, y, canvas);
        }
        for (int col = 0; col < 8; col++) {
            int x = xOffset + (col + 1) * (cellSize + CELL_GAP) + cellSize / 2;
            int y = yOffset + cellSize / 2;
            drawChar((char)('A' + col), x, y, canvas);
        }

        // Draw status text.
        drawString(mStatus, xOffset + size / 2, yOffset + size - cellSize / 2,
                canvas);

        // Draw the cells.
        Paint cellPaint = new Paint();
        cellPaint.setColor(Color.rgb(0, 0x80, 0));
        Paint whiteDiskPaint = new Paint();
        whiteDiskPaint.setColor(Color.WHITE);
        Paint blackDiskPaint = new Paint();
        blackDiskPaint.setColor(Color.BLACK);

        Rect cellRect = new Rect(0, 0, cellSize, cellSize);
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                int x = xOffset + (col + 1) * (cellSize + CELL_GAP);
                int y = yOffset + (row + 1) * (cellSize + CELL_GAP);

                // Draw the cell background.
                cellRect.offsetTo(x, y);
                canvas.drawRect(cellRect, cellPaint);

                // Draw the disk, if any.
                int r = cellSize / 2;
                switch (mBoard.getCellState(row, col)) {
                case OthelloBoard.CELL_BLACK:
                    canvas.drawCircle(x + r, y + r, r, blackDiskPaint);
                    break;
                case OthelloBoard.CELL_WHITE:
                    canvas.drawCircle(x + r, y + r, r, whiteDiskPaint);
                    break;
                }
            }
        }
    }

    private void drawChar(char c, int x, int y, Canvas canvas) {
        drawString(String.valueOf(c), x, y, canvas);
    }

    private void drawString(String s, int x, int y, Canvas c) {
        Paint textPaint = new Paint();
        textPaint.setColor(Color.WHITE);
        textPaint.setTextSize(cellSize / 2);
        textPaint.setTextAlign(Paint.Align.CENTER);
        Rect bounds = new Rect();
        textPaint.getTextBounds(s, 0, s.length(), bounds);
        c.drawText(s, 0, s.length(), x, y + bounds.height() / 2, textPaint);
    }
}
