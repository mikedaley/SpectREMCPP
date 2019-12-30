//
//  MetalView.m
//  SpectREM
//
//  Created by Mike on 20/07/2018.
//  Copyright © 2018 71Squared Ltd. All rights reserved.
//

#import "MetalView.h"
#import "EmulationProtocol.h"
#import "SharedConstants.h"

@implementation MetalView

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self)
    {
        [self registerForDraggedTypes:@[NSPasteboardTypeURL]];
    }
    return self;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

#pragma mark - View Drag/Drop

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    NSPasteboard *pBoard;
    NSDragOperation sourceDragMask;
    sourceDragMask = [sender draggingSourceOperationMask];
    pBoard = [sender draggingPasteboard];
    
    if ([[pBoard types] containsObject:NSPasteboardTypeFileURL])
    {
        if (sourceDragMask * NSDragOperationCopy)
        {
            NSURL *fileURL = [NSURL URLFromPasteboard:pBoard];
            if ([[fileURL.pathExtension uppercaseString] isEqualToString:cZ80_EXTENSION] ||
                [[fileURL.pathExtension uppercaseString] isEqualToString:cSNA_EXTENSION] ||
                [[fileURL.pathExtension uppercaseString] isEqualToString:cTAP_EXTENSION])
            {
                return NSDragOperationCopy;
            }
            else
            {
                return NSDragOperationNone;
            }
        }
    }
    return NSDragOperationNone;
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    NSPasteboard *pBoard = [sender draggingPasteboard];
    if ([[pBoard types] containsObject:NSPasteboardTypeFileURL])
    {
        NSURL *fileURL = [NSURL URLFromPasteboard:pBoard];
        if ([[fileURL.pathExtension uppercaseString] isEqualToString:cZ80_EXTENSION] ||
            [[fileURL.pathExtension uppercaseString] isEqualToString:cSNA_EXTENSION] ||
            [[fileURL.pathExtension uppercaseString] isEqualToString:cTAP_EXTENSION])
        {
            id <EmulationProtocol> emulationViewController = (id <EmulationProtocol>)[self.window contentViewController];
            [emulationViewController loadFileWithURL:fileURL addToRecent:YES];
            return YES;
        }
    }
    return NO;
}

@end
