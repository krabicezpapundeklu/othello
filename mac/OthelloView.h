#import <Cocoa/Cocoa.h>

@interface OthelloView : NSView {
    int selCol, selRow;
    int cellSize, xOffset, yOffset, size;
}
@end
