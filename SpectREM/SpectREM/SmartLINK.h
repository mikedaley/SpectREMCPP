//
//  SerialCore.h
//  SpectREM
//
//  Created by Michael Daley on 2019-12-02.
//  Copyright © 2019 71Squared Ltd. All rights reserved.
//

#ifndef SerialCore_h
#define SerialCore_h

#import <Foundation/Foundation.h>
#import "NSObject+Bindings.h"

@class ORSSerialPortManager;
@class ORSSerialPort;
@class ORSSerialPacketDescriptor;

#pragma mark - Interface

@interface SmartLINK : NSObject_Bindings

@property (nonatomic, strong) ORSSerialPortManager *serialPortManager;
@property (nonatomic, strong) ORSSerialPort *serialPort;

// Sends data to the current serial port. Also provides an expected response and response length
- (void)sendData:(NSData *)data expectedResponse:(ORSSerialPacketDescriptor *)expectedResponse responseLength:(int)length;

// Sends the supplied snapshot data to the current serial port
- (void)sendSnapshot:(unsigned char *)snapshot;

- (void)sendSmartlinkAction:(uint16_t) action;
@end


#endif /* SerialCore_h */
