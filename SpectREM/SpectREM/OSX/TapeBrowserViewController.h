//
//  TapeBrowserViewController.h
//  SpectREM
//
//  Created by Michael Daley on 10/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class EmulationViewController;

@interface TapeBrowserViewController : NSViewController <NSTableViewDelegate, NSTableViewDataSource>

@property (strong) EmulationViewController *emulationViewController;

@end
