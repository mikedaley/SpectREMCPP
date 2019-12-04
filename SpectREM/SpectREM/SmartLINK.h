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

// SL Actions
static const uint8_t cSMARTLINK_RESET = 0x01;

// SL Target Commands
static const uint8_t cCMD_LOAD_REGS = 0xa0;
static const uint8_t cCMD_LOAD_DATA = 0xaa;
static const uint8_t cCMD_SET_PORTS = 0xbb;
static const uint8_t cCMD_LOAD_PAGE_DATA = 0xcc;
static const uint8_t cCMD_RESTART = 0x80;

static const uint8_t cPKT_CMD_START = 0xfe;
static const uint8_t cPKT_CMD_WAIT = 0xff;

typedef NS_ENUM(uint8_t, SnapshotType)
{
    SnapshotTypeSNA,
    SnapshotTypeZ80
};

#pragma mark - Interface

@interface SmartLink : NSObject_Bindings

@property (nonatomic, strong) ORSSerialPortManager *serialPortManager;
@property (nonatomic, strong) ORSSerialPort *serialPort;

// Sends the supplied snapshot data to the current serial port
- (void)sendSnapshot:(unsigned char *)snapshot ofType:(SnapshotType)snapshotType;
- (void)sendSmartlinkAction:(uint16_t) action;

// Notification functions
- (void)serialPortsWereConnected:(NSNotification *)notification;
- (void)serialPortsWereDisconnected:(NSNotification *)notification;

@end


#endif /* SerialCore_h */
