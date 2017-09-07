//
//  ExportAccessoryViewController.h
//  SpectREM
//
//  Created by Mike Daley on 07/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface ExportAccessoryViewController : NSViewController

@property (weak) IBOutlet NSPopUpButton *exportPopup;
@property (assign) NSInteger exportType;

@end
