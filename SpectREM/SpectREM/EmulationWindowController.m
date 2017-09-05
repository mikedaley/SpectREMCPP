//
//  EmulationWindowController.m
//  SpectREM
//
//  Created by Mike Daley on 05/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "EmulationWindowController.h"
#import <CoreImage/CoreImage.h>

static CGFloat const MIN_DISTANCE = 75;
static CGFloat const MAX_DISTNACE = 200;

@interface EmulationWindowController () <NSWindowDelegate>

@property (nonatomic, strong) NSTrackingArea *trackingArea;

@end

@implementation EmulationWindowController

- (void)windowDidLoad
{
    [super windowDidLoad];
    
}

@end
