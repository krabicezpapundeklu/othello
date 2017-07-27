#import "OthelloView.h"
#include "../othello.h"

@interface OthelloView ()
- (void)onTap:(UIGestureRecognizer*)gr;
- (void)onLongPress:(UIGestureRecognizer*)gr;
- (void)showMenuAtPoint:(CGPoint)p;
- (void)onNewGameMenu:(id)sender;
- (void)drawChar:(char)c atPoint:(CGPoint)p;
- (void)drawString:(NSString*)s atPoint:(CGPoint)p;
- (BOOL)hitTest:(CGPoint)p withHitRow:(int*)row hitCol:(int*)col;
@end

@implementation OthelloView

- (id)initWithBoard:(const othello_t*)b state:(enum State)s
           delegate:(id<OthelloViewDelegate>)d {
    self = [super init];
    if (self) {
        board = b;
        state = s;
        delegate = d;

        [self setBackgroundColor:[UIColor whiteColor]];

        UITapGestureRecognizer *tr = [[UITapGestureRecognizer alloc]
            initWithTarget:self action:@selector(onTap:)];
        tr.numberOfTapsRequired = 1;
        [self addGestureRecognizer:tr];

        UILongPressGestureRecognizer *lr = [[UILongPressGestureRecognizer alloc]
            initWithTarget:self action:@selector(onLongPress:)];
        [self addGestureRecognizer:lr];
    }
    return self;
}

- (BOOL)canBecomeFirstResponder {
    return YES;
}

- (void)onTap:(UIGestureRecognizer*)gr {
    if (gr.state != UIGestureRecognizerStateEnded) {
        return;
    }
    CGPoint p = [gr locationInView:self];

    // Close the menu if it's currently open.
    UIMenuController *menu = [UIMenuController sharedMenuController];
    if ([menu isMenuVisible]) {
        [menu setMenuVisible:NO animated:YES];
        [self setNeedsDisplay];
        return;
    }

    // In the game over state, bring up the menu for any kind of tap.
    if (state == GAME_OVER) {
        [self showMenuAtPoint:p];
        return;
    }

    int row, col;
    if ([self hitTest:p withHitRow:&row hitCol:&col]) {
        [delegate boardWasTappedAtRow:row col:col];
    }
}

- (void)onLongPress:(UIGestureRecognizer*)gr {
    if (gr.state != UIGestureRecognizerStateBegan || state == WHITES_MOVE) {
        return;
    }

    [self showMenuAtPoint:[gr locationInView:self]];
}

- (void)showMenuAtPoint:(CGPoint)p {
    [self becomeFirstResponder];
    UIMenuController *menu = [UIMenuController sharedMenuController];
    UIMenuItem *newGameItem =
            [[UIMenuItem alloc] initWithTitle:@"New Game"
                                       action:@selector(onNewGameMenu:)];
    menu.menuItems = @[ newGameItem ];
    [menu setTargetRect:CGRectMake(p.x, p.y, 2, 2) inView:self];
    [menu setMenuVisible:YES animated:YES];

    [self setNeedsDisplay];
}

- (void)onNewGameMenu:(id)sender {
    assert(state != WHITES_MOVE);
    [delegate newGame];
}

- (void)updateWithState:(enum State)s {
    state = s;
    [self setNeedsDisplay];
}

static const int CELL_GAP = 1;

- (void)drawRect:(CGRect)rect {
    // Compute size and position of the 10x10 grid.
    CGRect bounds = [self bounds];
    float viewSize = MIN(bounds.size.height, bounds.size.width);
    cellSize = (viewSize - 9 * CELL_GAP) / 10;
    size = cellSize * 10 + 9 * CELL_GAP;
    xOffset = bounds.origin.x + bounds.size.width / 2 - size / 2;
    yOffset = bounds.origin.y + bounds.size.height / 2 - size / 2;

    // Draw a background square around the 8x8 cells.
    [[UIColor blackColor] set];
    CGRect backgroundRect =
        CGRectMake(xOffset + cellSize, yOffset + cellSize,
                   8 * cellSize + 9 * CELL_GAP, 8 * cellSize + 9 * CELL_GAP);
    [[UIBezierPath bezierPathWithRect:backgroundRect] fill];

    // Draw labels.
    for (int row = 0; row < 8; row++) {
        float x = xOffset + cellSize / 2;
        float y = yOffset + (row + 1) * (cellSize + CELL_GAP) + cellSize / 2;
        [self drawChar:('1' + row) atPoint:CGPointMake(x, y)];
    }
    for (int col = 0; col < 8; col++) {
        float x = xOffset + (col + 1) * (cellSize + CELL_GAP) + cellSize / 2;
        float y = yOffset + cellSize / 2;
        [self drawChar:('A' + col) atPoint:CGPointMake(x, y)];
    }

    // Draw status text.
    NSString *statusText;
    switch (state) {
    case BLACKS_MOVE:
        statusText = @"Human's move.";
        break;
    case WHITES_MOVE:
        statusText = @"Computer's move...";
        break;
    case GAME_OVER: {
        int blackScore = othello_score(board, PLAYER_BLACK);
        int whiteScore = othello_score(board, PLAYER_WHITE);
        if (blackScore > whiteScore) {
            statusText = [NSString stringWithFormat:@"Human wins %d-%d!",
                                                    blackScore, whiteScore];
        } else if (whiteScore > blackScore) {
            statusText = [NSString stringWithFormat:@"Computer wins %d-%d!",
                                                    whiteScore, blackScore];
        } else {
            statusText = @"Draw!";
        }
        break;
        }
    }
    [self drawString:statusText atPoint:CGPointMake(xOffset + size / 2,
            yOffset + size - cellSize / 2)];

    // Draw the cells.
    UIColor *boardColor =
        [UIColor colorWithRed:0 green:(0x80 / 256.0) blue:0 alpha:1.0];
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            float x = xOffset + (col + 1) * (cellSize + CELL_GAP);
            float y = yOffset + (row + 1) * (cellSize + CELL_GAP);

            // Draw the cell background.
            [boardColor set];
            CGRect cellRect = CGRectMake(x, y, cellSize, cellSize);
            [[UIBezierPath bezierPathWithRect:cellRect] fill];

            // Draw the disk, if any.
            UIBezierPath *diskPath =
                [UIBezierPath bezierPathWithOvalInRect:cellRect];
            switch (othello_cell_state(board, row, col)) {
            case CELL_BLACK:
                [[UIColor blackColor] set];
                [diskPath fill];
                break;
            case CELL_WHITE:
                [[UIColor whiteColor] set];
                [diskPath fill];
                break;
            default:
                break;
            }
        }
    }
}

- (void)drawChar:(char)c atPoint:(CGPoint)p {
    const char s[] = {c, 0};
    [self drawString:[NSString stringWithUTF8String:s] atPoint:p];
}

- (void)drawString:(NSString*)s atPoint:(CGPoint)p {
    UIFont *font = [UIFont preferredFontForTextStyle:UIFontTextStyleBody];
    NSAttributedString *as = [[NSAttributedString alloc]
        initWithString:s attributes:@{NSFontAttributeName: font}];
    CGSize stringSize = [as size];
    [as drawAtPoint:CGPointMake(p.x - stringSize.width / 2,
                                p.y - stringSize.height / 2)];
}

- (BOOL)hitTest:(CGPoint)p withHitRow:(int*)row hitCol:(int*)col {
    *row = (int)(p.y - yOffset) / (cellSize + CELL_GAP) - 1;
    *col = (int)(p.x - xOffset) / (cellSize + CELL_GAP) - 1;
    if (*row >= 0 && *row < 8 && *col >= 0 && *col < 8) {
        return YES;
    }

    return NO;
}

@end
