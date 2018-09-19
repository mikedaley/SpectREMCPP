//
//  InfoPanelViewController.h
//  SpectREM
//
//  Created by Mike on 10/09/2018.
//  Copyright © 2018 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface InfoPanelViewController : NSViewController

@property (weak) IBOutlet NSTextField *infoText;
@property (assign) BOOL panelHidden;

- (void)displayMessage:(NSString *)message duration:(NSInteger)duration;
- (void)hideMessage;

@end
