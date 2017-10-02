//
//  EmulationWindowController.m
//  SpectREM
//
//  Created by Mike Daley on 05/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "EmulationWindowController.h"


@interface EmulationWindowController () <NSWindowDelegate>


@end

@implementation EmulationWindowController

- (void)windowDidLoad
{
    [super windowDidLoad];
    self.window.movableByWindowBackground = YES;
    
}

@end
