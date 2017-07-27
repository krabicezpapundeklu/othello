#import "OthelloView.h"
#import "AppDelegate.h"
#include "../othello.h"

@interface OthelloView ()
- (void)drawChar:(char)c atPoint:(NSPoint)p;
- (void)drawString:(NSString*)s atPoint:(NSPoint)p;
- (BOOL)hitTest:(NSPoint)p withHitRow:(int*)row hitCol:(int*)col;

@property (weak) IBOutlet AppDelegate *delegate;
@end

static const int CELL_GAP = 1;

@implementation OthelloView

- (void)awakeFromNib {
    [super awakeFromNib];

    selRow = -1;
    selCol = -1;

    [self.window setAcceptsMouseMovedEvents:YES];
    [self setNeedsDisplay:YES];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)isFlipped {
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];

    // Compute size and position of the 10x10 grid.
    NSRect bounds = [self bounds];
    float viewSize = MIN(bounds.size.height, bounds.size.width);
    cellSize = (viewSize - 9 * CELL_GAP) / 10;
    size = cellSize * 10 + 9 * CELL_GAP;
    xOffset = bounds.origin.x + bounds.size.width / 2 - size / 2;
    yOffset = bounds.origin.y + bounds.size.height / 2 - size / 2;

    // Draw a background square around the 8x8 centre cells.
    [[NSColor blackColor] set];
    NSRect backgroundRect =
        NSMakeRect(xOffset + cellSize, yOffset + cellSize,
                   8 * cellSize + 9 * CELL_GAP, 8 * cellSize + 9 * CELL_GAP);
    [NSBezierPath fillRect:backgroundRect];

    // Draw labels.
    for (int row = 0; row < 8; row++) {
        float x = xOffset + cellSize / 2;
        float y = yOffset + (row + 1) * (cellSize + CELL_GAP) + cellSize / 2;
        [self drawChar:('1' + row) atPoint:NSMakePoint(x, y)];
    }
    for (int col = 0; col < 8; col++) {
        float x = xOffset + (col + 1) * (cellSize + CELL_GAP) + cellSize / 2;
        float y = yOffset + cellSize / 2;
        [self drawChar:('A' + col) atPoint:NSMakePoint(x, y)];
    }

    const othello_t *board = [self.delegate board];

    // Draw status text.
    NSString *statusText;
    switch ([self.delegate state]) {
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
    [self drawString:statusText atPoint:NSMakePoint(xOffset + size / 2,
            yOffset + size - cellSize / 2)];

    // Draw the cells.
    NSColor *boardColor =
        [NSColor colorWithCalibratedRed:0 green:(0x80 / 256.0) blue:0 alpha:1];
    NSColor *highlightColor =
        [NSColor colorWithCalibratedRed:0 green:(0xAA / 256.0) blue:0 alpha:1];

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            float x = xOffset + (col + 1) * (cellSize + CELL_GAP);
            float y = yOffset + (row + 1) * (cellSize + CELL_GAP);
            bool highlight = (row == selRow && col == selCol &&
                              [self.delegate state] == BLACKS_MOVE);

            // Draw the cell background.
            [(highlight ? highlightColor : boardColor) set];
            NSRect cellRect = NSMakeRect(x, y, cellSize, cellSize);
            [NSBezierPath fillRect:cellRect];

            // Draw the disk, if any.
            NSBezierPath *diskPath =
                [NSBezierPath bezierPathWithOvalInRect:cellRect];
            switch (othello_cell_state(board, row, col)) {
            case CELL_BLACK:
                [[NSColor blackColor] set];
                [diskPath fill];
                break;
            case CELL_WHITE:
                [[NSColor whiteColor] set];
                [diskPath fill];
                break;
            default:
                break;
            }
        }
    }
}

- (void)drawChar:(char)c atPoint:(NSPoint)p {
    const char s[] = {c, 0};
    [self drawString:[NSString stringWithUTF8String:s] atPoint:p];
}

- (void)drawString:(NSString*)s atPoint:(NSPoint)p {
    NSFont *font = [NSFont boldSystemFontOfSize:[NSFont systemFontSize]];
    NSAttributedString *as = [[NSAttributedString alloc]
        initWithString:s attributes:@{ NSFontAttributeName: font }];
    NSSize stringSize = [as size];
    [as drawAtPoint:NSMakePoint(p.x - stringSize.width / 2,
                                p.y - stringSize.height / 2)];
}

- (void)mouseDown:(NSEvent*)event {
    int row, col;
    if ([self hitTest:[event locationInWindow] withHitRow:&row hitCol:&col]) {
        [self.delegate boardWasClickedAtRow:row Column:col];
    }
}

- (void)mouseMoved:(NSEvent*)event {
    int row, col;
    if (![self hitTest:[event locationInWindow] withHitRow:&row hitCol:&col]) {
        row = -1;
        col = -1;
    }
    if (row != selRow || col != selCol) {
        selRow = row;
        selCol = col;
        [self setNeedsDisplay:YES];
    }
}

- (BOOL)hitTest:(NSPoint)p withHitRow:(int*)row hitCol:(int*)col {
    p = [self convertPoint:p fromView:nil];
    *row = (int)(p.y - yOffset) / (cellSize + CELL_GAP) - 1;
    *col = (int)(p.x - xOffset) / (cellSize + CELL_GAP) - 1;
    if (*row >= 0 && *row < 8 && *col >= 0 && *col < 8) {
        return YES;
    }

    return NO;
}

- (void)keyDown:(NSEvent*)event {
    int row = selRow;
    int col = selCol;
    unichar c = [[event charactersIgnoringModifiers] characterAtIndex:0];

    switch (c) {
    default:
        return;

    case ' ':
    case NSCarriageReturnCharacter:
        if (selRow >= 0) {
            [self.delegate boardWasClickedAtRow:selRow Column:selCol];
        }
        return;

    case NSRightArrowFunctionKey: col++; break;
    case NSLeftArrowFunctionKey:  col--; break;
    case NSDownArrowFunctionKey:  row++; break;
    case NSUpArrowFunctionKey:    row--; break;

    case 'a': case 'b': case 'c': case 'd':
    case 'e': case 'f': case 'g': case 'h':
        col = c - 'a';
        break;

    case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8':
        row = c - '1';
        break;
    }

    selRow = MAX(0, MIN(row, 7));
    selCol = MAX(0, MIN(col, 7));
    [self setNeedsDisplay:YES];
}

@end
