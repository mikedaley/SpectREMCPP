//
//  SerialCore.m
//  SpectREM
//
//  Created by Michael Daley on 2019-12-02.
//  Copyright © 2019 71Squared Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import "SmartLINK.h"

#import "ORSSerial/ORSSerial.h"

#pragma mark - Private Interface

constexpr uint8_t crc7_table[256] =
{
    0x00, 0x09, 0x12, 0x1b, 0x24, 0x2d, 0x36, 0x3f,
    0x48, 0x41, 0x5a, 0x53, 0x6c, 0x65, 0x7e, 0x77,
    0x19, 0x10, 0x0b, 0x02, 0x3d, 0x34, 0x2f, 0x26,
    0x51, 0x58, 0x43, 0x4a, 0x75, 0x7c, 0x67, 0x6e,
    0x32, 0x3b, 0x20, 0x29, 0x16, 0x1f, 0x04, 0x0d,
    0x7a, 0x73, 0x68, 0x61, 0x5e, 0x57, 0x4c, 0x45,
    0x2b, 0x22, 0x39, 0x30, 0x0f, 0x06, 0x1d, 0x14,
    0x63, 0x6a, 0x71, 0x78, 0x47, 0x4e, 0x55, 0x5c,
    0x64, 0x6d, 0x76, 0x7f, 0x40, 0x49, 0x52, 0x5b,
    0x2c, 0x25, 0x3e, 0x37, 0x08, 0x01, 0x1a, 0x13,
    0x7d, 0x74, 0x6f, 0x66, 0x59, 0x50, 0x4b, 0x42,
    0x35, 0x3c, 0x27, 0x2e, 0x11, 0x18, 0x03, 0x0a,
    0x56, 0x5f, 0x44, 0x4d, 0x72, 0x7b, 0x60, 0x69,
    0x1e, 0x17, 0x0c, 0x05, 0x3a, 0x33, 0x28, 0x21,
    0x4f, 0x46, 0x5d, 0x54, 0x6b, 0x62, 0x79, 0x70,
    0x07, 0x0e, 0x15, 0x1c, 0x23, 0x2a, 0x31, 0x38,
    0x41, 0x48, 0x53, 0x5a, 0x65, 0x6c, 0x77, 0x7e,
    0x09, 0x00, 0x1b, 0x12, 0x2d, 0x24, 0x3f, 0x36,
    0x58, 0x51, 0x4a, 0x43, 0x7c, 0x75, 0x6e, 0x67,
    0x10, 0x19, 0x02, 0x0b, 0x34, 0x3d, 0x26, 0x2f,
    0x73, 0x7a, 0x61, 0x68, 0x57, 0x5e, 0x45, 0x4c,
    0x3b, 0x32, 0x29, 0x20, 0x1f, 0x16, 0x0d, 0x04,
    0x6a, 0x63, 0x78, 0x71, 0x4e, 0x47, 0x5c, 0x55,
    0x22, 0x2b, 0x30, 0x39, 0x06, 0x0f, 0x14, 0x1d,
    0x25, 0x2c, 0x37, 0x3e, 0x01, 0x08, 0x13, 0x1a,
    0x6d, 0x64, 0x7f, 0x76, 0x49, 0x40, 0x5b, 0x52,
    0x3c, 0x35, 0x2e, 0x27, 0x18, 0x11, 0x0a, 0x03,
    0x74, 0x7d, 0x66, 0x6f, 0x50, 0x59, 0x42, 0x4b,
    0x17, 0x1e, 0x05, 0x0c, 0x33, 0x3a, 0x21, 0x28,
    0x5f, 0x56, 0x4d, 0x44, 0x7b, 0x72, 0x69, 0x60,
    0x0e, 0x07, 0x1c, 0x15, 0x2a, 0x23, 0x38, 0x31,
    0x46, 0x4f, 0x54, 0x5d, 0x62, 0x6b, 0x70, 0x79
};

@interface SmartLINK() <ORSSerialPortDelegate, NSUserNotificationCenterDelegate>

@property (strong) ORSSerialPacketDescriptor *sendOkResponse;
@property (strong) ORSSerialPacketDescriptor *verifyResponse;

@end


#pragma mark - Constants


int const cSERIAL_BAUD_RATE = 115200;
int const cSERIAL_TIMEOUT = 3;
int const cSERIAL_BLOCK_SIZE = 8000;

//int const cSNAPSHOT_HEADER_LENGTH = 27;
//int const cSNAPSHOT_DATA_SIZE = 49152;
//int const cSNAPSHOT_START_ADDRESS = 16384;

int const cCOMMAND_HEADER_SIZE = 5;


#pragma mark - Static


static char snapshotBuffer[cSERIAL_BLOCK_SIZE + cCOMMAND_HEADER_SIZE];


#pragma mark - Implementation


@implementation SmartLINK

- (instancetype)init
{
    self = [super init];
    if (self)
    {
//        char responseCode[1] = {eSEND_OK};
//        _sendOkResponse = [[ORSSerialPacketDescriptor alloc] initWithPacketData:[NSData dataWithBytes:responseCode
//                                                                                                          length:1]
//                                                                       userInfo:NULL];
//        responseCode[0] = eVERIFY_RESPONSE;
//        _verifyResponse = [[ORSSerialPacketDescriptor alloc] initWithPacketData:[NSData dataWithBytes:responseCode
//                                                                                               length:1]
//                                                                       userInfo:NULL];


//        NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
//        [nc addObserver:self selector:@selector(serialPortsWereConnected:) name:ORSSerialPortsWereConnectedNotification object:nil];
//        [nc addObserver:self selector:@selector(serialPortsWereDisconnected:) name:ORSSerialPortsWereDisconnectedNotification object:nil];
        
        #if (MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_7)
            [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:self];
        #endif

    }
    return self;
}

#pragma mark - Actions


- (void)sendData:(NSData *)data expectedResponse:(ORSSerialPacketDescriptor *)expectedResponse responseLength:(int)length
{
    if (self.serialPort)
    {
        ORSSerialRequest *request = [ORSSerialRequest requestWithDataToSend:data
                                                                   userInfo:NULL
                                                            timeoutInterval:cSERIAL_TIMEOUT
                                                         responseDescriptor:expectedResponse];
        [self.serialPort sendRequest:request];
    }
}


#pragma mark - ORSSerialPortDelegate


- (void)serialPort:(ORSSerialPort *)serialPort didEncounterError:(NSError *)error
{
    NSLog(@"Serial port %@ encountered an error: %@", self.serialPort, error);
}

- (void)serialPort:(ORSSerialPort *)serialPort didReceiveResponse:(NSData *)responseData toRequest:(ORSSerialRequest *)request
{
    NSLog(@"Serial port didReceiveResponse: %@", responseData.description);
}

- (void)serialPort:(ORSSerialPort *)serialPort didReceiveData:(NSData *)data
{
    NSLog(@"Serial port didReceiveData: %@", data.description);
}

- (void)serialPort:(ORSSerialPort *)serialPort requestDidTimeout:(ORSSerialRequest *)request
{
    [self.serialPort cancelAllQueuedRequests];
    NSLog(@"Serial port command timed out!");
}

- (void)serialPortWasOpened:(ORSSerialPort *)serialPort
{
    NSLog(@"Serial port %@ opened", self.serialPort);
}

- (void)serialPortWasClosed:(ORSSerialPort *)serialPort
{
    NSLog(@"Serial port %@ closed", self.serialPort);
}

- (void)serialPortWasRemovedFromSystem:(ORSSerialPort *)serialPort
{
    NSLog(@"Serial port %@ removed from system", self.serialPort);
    self.serialPort = nil;
}


#pragma mark - SmartLINK


- (void)sendSnapshot:(unsigned char *)snapshot
{
//    int snapshotIndex = 0;
//    unsigned short spectrumAddress = cSNAPSHOT_START_ADDRESS;
//
//    // Reset Retroleum card
//    [self sendBlockWithCommand:eRETROLEUM_RESET
//                      location:0
//                        length:0
//                          data:snapshot
//              expectedResponse:self.sendOkResponse];
//
//
//    // Send register data
//    [self sendBlockWithCommand:eSEND_SNAPSHOT_REGISTERS
//                      location:snapshotIndex
//                        length:cSNAPSHOT_HEADER_LENGTH
//                          data:snapshot
//              expectedResponse:self.sendOkResponse];
//
//    snapshotIndex += cSNAPSHOT_HEADER_LENGTH;
//
//    // Send memory data
//    for (int block = 0; block < (cSNAPSHOT_DATA_SIZE / cSERIAL_BLOCK_SIZE); block++)
//    {
//        [self sendBlockWithCommand:eSEND_SNAPSHOT_DATA
//                          location:spectrumAddress
//                            length:cSERIAL_BLOCK_SIZE
//                              data:snapshot + snapshotIndex
//                  expectedResponse:self.self.sendOkResponse];
//
//        snapshotIndex += cSERIAL_BLOCK_SIZE;
//        spectrumAddress += cSERIAL_BLOCK_SIZE;
//    }
//
//    // Deal with any partial block data left over
//    if (cSNAPSHOT_DATA_SIZE % cSERIAL_BLOCK_SIZE)
//    {
//        [self sendBlockWithCommand:eSEND_SNAPSHOT_DATA
//                          location:spectrumAddress
//                            length:cSNAPSHOT_DATA_SIZE % cSERIAL_BLOCK_SIZE
//                              data:snapshot + snapshotIndex
//                  expectedResponse:self.sendOkResponse];
//    }
//
//    // Send start game
//    [self sendBlockWithCommand:eRUN_SNAPSHOT
//                      location:0
//                        length:0
//                          data:snapshot
//              expectedResponse:self.sendOkResponse];
    
}

- (uint8_t)crc7:(const char *)packet length:(size_t)packet_len
{
    uint8_t crc = 0;
    for (size_t i=0;  i < packet_len; i++)
    {
        crc = crc7_table[(crc << 1) ^ packet[i]];
    }
    return ((crc << 1) | 1);
}


- (void)sendSmartlinkAction:(uint16_t) action
{
    snapshotBuffer[0] = 'S';
    snapshotBuffer[1] = 'L';
    snapshotBuffer[2] = action;
    snapshotBuffer[3] = ((action >> 8) | 0x80);
    snapshotBuffer[4] = [self crc7:snapshotBuffer length:5];
    [self sendData:[NSData dataWithBytes:snapshotBuffer length:5] expectedResponse:nil responseLength:1];
}

- (void)sendBlockWithCommand:(uint8_t)command location:(uint16_t)location length:(uint16_t)length data:(unsigned char *)data expectedResponse:(ORSSerialPacketDescriptor *)expectedResponse
{
    snapshotBuffer[0] = command;
    snapshotBuffer[1] = location & 255;
    snapshotBuffer[2] = location >> 8;
    snapshotBuffer[3] = length & 255;
    snapshotBuffer[4] = length >> 8;
    
    memcpy(snapshotBuffer + cCOMMAND_HEADER_SIZE, data, length);
    
    [self sendData:[NSData dataWithBytes:snapshotBuffer length:length + cCOMMAND_HEADER_SIZE]
  expectedResponse:expectedResponse
    responseLength:1];
}


#pragma mark - Properties


- (ORSSerialPortManager *)serialPortManager
{
    return [ORSSerialPortManager sharedSerialPortManager];
}

- (void)setSerialPort:(ORSSerialPort *)serialPort
{
    if (serialPort != _serialPort)
    {
        [_serialPort close];
        _serialPort.baudRate = @(cSERIAL_BAUD_RATE);
        _serialPort = serialPort;
        _serialPort.delegate = self;
        _serialPort.RTS = YES;
        _serialPort.DTR = YES;
        [_serialPort open];

        // Make sure that the change is propogated back to any bindings which may exist for this property
        [self propagateValue:_serialPort forBinding:@"serialPort"];
    }
}


#pragma mark - NSUserNotificationCenterDelegate


#if (MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_7)

- (void)userNotificationCenter:(NSUserNotificationCenter *)center didDeliverNotification:(NSUserNotification *)notification
{
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, 3.0 * NSEC_PER_SEC);
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        [center removeDeliveredNotification:notification];
    });
}

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification
{
    return YES;
}

#endif


#pragma mark - Notifications


- (void)serialPortsWereConnected:(NSNotification *)notification
{
    NSArray *connectedPorts = [notification userInfo][ORSConnectedSerialPortsKey];
    NSLog(@"Ports were connected: %@", connectedPorts);
    [self postUserNotificationForConnectedPorts:connectedPorts];
}

- (void)serialPortsWereDisconnected:(NSNotification *)notification
{
    NSArray *disconnectedPorts = [notification userInfo][ORSDisconnectedSerialPortsKey];
    NSLog(@"Ports were disconnected: %@", disconnectedPorts);
    [self postUserNotificationForDisconnectedPorts:disconnectedPorts];
    
}

- (void)postUserNotificationForConnectedPorts:(NSArray *)connectedPorts
{
    #if (MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_7)
        if (!NSClassFromString(@"NSUserNotificationCenter")) return;
        
        NSUserNotificationCenter *unc = [NSUserNotificationCenter defaultUserNotificationCenter];
        for (ORSSerialPort *port in connectedPorts)
        {
            NSUserNotification *userNote = [[NSUserNotification alloc] init];
            userNote.title = NSLocalizedString(@"Serial Port Connected", @"Serial Port Connected");
            NSString *informativeTextFormat = NSLocalizedString(@"Serial Port %@ was connected to your Mac.", @"Serial port connected user notification informative text");
            userNote.informativeText = [NSString stringWithFormat:informativeTextFormat, port.name];
            userNote.soundName = nil;
            [unc deliverNotification:userNote];
        }
    #endif
}

- (void)postUserNotificationForDisconnectedPorts:(NSArray *)disconnectedPorts
{
    #if (MAC_OS_X_VERSION_MAX_ALLOWED > MAC_OS_X_VERSION_10_7)
        if (!NSClassFromString(@"NSUserNotificationCenter")) return;
        
        NSUserNotificationCenter *unc = [NSUserNotificationCenter defaultUserNotificationCenter];
        for (ORSSerialPort *port in disconnectedPorts)
        {
            NSUserNotification *userNote = [[NSUserNotification alloc] init];
            userNote.title = NSLocalizedString(@"Serial Port Disconnected", @"Serial Port Disconnected");
            NSString *informativeTextFormat = NSLocalizedString(@"Serial Port %@ was disconnected from your Mac.", @"Serial port disconnected user notification informative text");
            userNote.informativeText = [NSString stringWithFormat:informativeTextFormat, port.name];
            userNote.soundName = nil;
            [unc deliverNotification:userNote];
        }
    #endif
}

@end
