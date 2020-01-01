//
//  ConfigurationViewController.h
//  SpectREM
//
//  Created by Mike Daley on 06/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Defaults.h"

@class ORSSerialPortManager;
@class ORSSerialPort;


#pragma mark - Key Path Constants

#pragma mark - Interface

@interface ConfigurationViewController : NSViewController

@property (strong) Defaults *defaults;
@property (nonatomic, readonly) ORSSerialPortManager *serialPortManager;
@property (nonatomic, strong) ORSSerialPort *serialPort;
@property (nonatomic, weak) IBOutlet NSButton *sendSnapshot;

@end
