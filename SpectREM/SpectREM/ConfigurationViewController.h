//
//  ConfigurationViewController.h
//  SpectREM
//
//  Created by Mike Daley on 06/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#pragma mark - Key Path Constants

extern NSString *const cSELECTED_MACHINE;
extern NSString *const cDISPLAY_BORDER_WIDTH;
extern NSString *const cDISPLAY_FILTER_VALUE;

#pragma mark - Interface

@interface ConfigurationViewController : NSViewController

// Machine
@property (assign)  NSInteger   selectedMachine;

// Display
@property (assign)  float       displayBorderWidth;
@property (assign)  float       displayFilterValue;

@end
