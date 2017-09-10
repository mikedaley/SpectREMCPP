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
extern NSString *const cMACHINE_INSTANT_TAPE_LOADING;

extern NSString *const cAUDIO_VOLUME;
extern NSString *const cAUDIO_HIGH_PASS_FILTER;
extern NSString *const cAUDIO_LOW_PASS_FILTER;

#pragma mark - Interface

@interface ConfigurationViewController : NSViewController

@end
