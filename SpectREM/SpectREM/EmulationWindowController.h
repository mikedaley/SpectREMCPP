//
//  EmulationWindowController.h
//  SpectREM
//
//  Created by Mike Daley on 05/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface EmulationWindowController : NSWindowController

@property (nonatomic, assign) NSView *controlsView;

- (void)updateTrafficlightsWithMouseLocation:(NSPoint)point;

@end
