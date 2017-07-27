package net.hanshq.othello;

import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;

public class OthelloActivity extends Activity
        implements OthelloView.TouchHandler {
    private static final String BOARD_KEY = "board";
    private static final String STATE_KEY = "state";

    public enum State { BLACKS_MOVE, WHITES_MOVE, GAME_OVER }

    private State mState;
    private OthelloView mView;
    private OthelloBoard mBoard;
    private ComputeWhiteMoveTask mWhiteMoveTask;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_othello);

        mBoard = new OthelloBoard();
        mView = (OthelloView)findViewById(R.id.othello_view);
        mView.setBoard(mBoard);
        mView.setTouchHandler(this);
        newGame();

        if (savedInstanceState != null) {
            byte[] boardBytes = savedInstanceState.getByteArray(BOARD_KEY);
            for (int row = 0; row < 8; row++) {
                for (int col = 0; col < 8; col++) {
                    mBoard.setCellState(row, col, boardBytes[row * 8 + col]);
                }
            }

            mState = State.values()[savedInstanceState.getInt(STATE_KEY)];
            if (mState == State.WHITES_MOVE) {
                computeWhiteMove();
            }

            mView.update(mState);
        }
    }

    @Override
    public void onSaveInstanceState(Bundle savedInstanceState) {
        super.onSaveInstanceState(savedInstanceState);

        byte[] boardBytes = new byte[64];
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                boardBytes[row * 8 + col] = (byte)mBoard.getCellState(row, col);
            }
        }

        savedInstanceState.putByteArray(BOARD_KEY, boardBytes);
        savedInstanceState.putInt(STATE_KEY, mState.ordinal());
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu_othello, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case R.id.menu_item_new_game:
            if (mState != State.WHITES_MOVE) {
                newGame();
            }
            return true;
        default:
            return super.onOptionsItemSelected(item);
        }
    }

    private void newGame() {
        mState = State.BLACKS_MOVE;
        mBoard.reset();
        mView.update(mState);
    }

    @Override
    public void onTouch(int row, int col) {
        if (mState == State.BLACKS_MOVE) {
            makeMove(row, col);
        }
    }

    private void makeMove(int row, int col) {
        assert(mState == State.BLACKS_MOVE || mState == State.WHITES_MOVE);

        if (mState == State.BLACKS_MOVE) {
            if (!mBoard.isValidMove(OthelloBoard.PLAYER_BLACK, row, col)) {
                // Illegal move; ignored.
                return;
            }

            mBoard.makeMove(OthelloBoard.PLAYER_BLACK, row, col);
            mState = State.WHITES_MOVE;
        } else {
            mBoard.makeMove(OthelloBoard.PLAYER_WHITE, row, col);
            mState = State.BLACKS_MOVE;
        }

        if (!mBoard.hasValidMove(OthelloBoard.PLAYER_BLACK) &&
                !mBoard.hasValidMove(OthelloBoard.PLAYER_WHITE)) {
            mState = State.GAME_OVER;
        } else if (mState == State.BLACKS_MOVE &&
                !mBoard.hasValidMove(OthelloBoard.PLAYER_BLACK)) {
            mState = State.WHITES_MOVE;
        } else if (mState == State.WHITES_MOVE &&
                !mBoard.hasValidMove(OthelloBoard.PLAYER_WHITE)) {
            mState = State.BLACKS_MOVE;
        }

        if (mState == State.WHITES_MOVE) {
            computeWhiteMove();
        }

        mView.update(mState);
    }

    private class ComputeWhiteMoveTask extends AsyncTask<Void, Void, int[]> {
        @Override
        protected int[] doInBackground(Void... args) {
            return mBoard.computeMove(OthelloBoard.PLAYER_WHITE);
        }
        @Override
        protected void onPostExecute(int[] result) {
            makeMove(result[0], result[1]);
        }
    }

    private void computeWhiteMove() {
        mWhiteMoveTask = new ComputeWhiteMoveTask();
        mWhiteMoveTask.execute();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mState == State.WHITES_MOVE) {
            mWhiteMoveTask.cancel(false);
        }
        mBoard.destroy();
    }
}
