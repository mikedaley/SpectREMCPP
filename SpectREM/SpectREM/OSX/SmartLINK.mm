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
#import "ZXSpectrum.hpp"

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

// - Constants

const uint16_t              cZ80_V3_HEADER_SIZE = 86;
//const uint16_t              cZ80_V3_ADD_HEADER_SIZE = 54;
const uint8_t               cZ80_V3_PAGE_HEADER_SIZE = 3;

//const uint8_t               cZ80_V2_MACHINE_TYPE_48 = 0;
const uint8_t               cZ80_V3_MACHINE_TYPE_48 = 0;
const uint8_t               cZ80_V2_MACHINE_TYPE_48_IF1 = 1;
//const uint8_t               cZ80_V3_MACHINE_TYPE_48_IF1 = 1;
const uint8_t               cZ80_V2_MACHINE_TYPE_128 = 3;
//const uint8_t               cZ80_V3_MACHINE_TYPE_48_MGT = 3;
//const uint8_t               cZ80_V2_MACHINE_TYPE_128_IF1 = 4;
const uint8_t               cZ80_V3_MACHINE_TYPE_128 = 4;
//const uint8_t               cZ80_V3_MACHINE_TYPE_128_IF1 = 5;
const uint8_t               cz80_V3_MACHINE_TYPE_128_MGT = 6;
const uint8_t               cZ80_V3_MACHINE_TYPE_128_2 = 12;

@interface SmartLink() <ORSSerialPortDelegate, NSUserNotificationCenterDelegate>

@property (strong) ORSSerialPacketDescriptor *sendOkResponse;
@property (strong) ORSSerialPacketDescriptor *verifyResponse;

@end


#pragma mark - Constants


int const cSERIAL_BAUD_RATE = 115200;
int const cSERIAL_TIMEOUT = 5;
int const cSERIAL_BLOCK_SIZE = 9000;

int const cSNAPSHOT_HEADER_LENGTH = 27;
int const cSMARTLINK_SNAPSHOT_HEADER_LENGTH = 29;
int const cSNAPSHOT_DATA_SIZE = 49152;
int const cSNAPSHOT_START_ADDRESS = 16384;

int const cCOMMAND_HEADER_SIZE = 10;


#pragma mark - Static


static uint8_t snapshotBuffer[cSERIAL_BLOCK_SIZE + cCOMMAND_HEADER_SIZE];

// SL Response codes
static const uint8_t cSendOK = 0xaa;


#pragma mark - Implementation


@implementation SmartLink

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        uint8_t responseCode[1] = {cSendOK};
        _sendOkResponse = [[ORSSerialPacketDescriptor alloc] initWithPacketData:[NSData                  dataWithBytes:responseCode length:1]
                                                                       userInfo:NULL];

        NSNotificationCenter *nc = [NSNotificationCenter defaultCenter];
        [nc addObserver:self selector:@selector(serialPortsWereConnected:) name:ORSSerialPortsWereConnectedNotification object:nil];
        [nc addObserver:self selector:@selector(serialPortsWereDisconnected:) name:ORSSerialPortsWereDisconnectedNotification object:nil];
        
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
    self.serialPort = nil;
    
}

- (void)serialPort:(ORSSerialPort *)serialPort didReceiveResponse:(NSData *)responseData
         toRequest:(ORSSerialRequest *)request
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


- (void)sendSnapshot:(unsigned char *)snapshot ofType:(SnapshotType)snapshotType
{
    switch (snapshotType) {
        case SnapshotTypeSNA:
            [self sendSNASnapshot:snapshot];
            break;
            
        case SnapshotTypeZ80:
            [self sendZ80Snapshot:snapshot];
            
        default:
            break;
    }
}

- (void)sendSmartlinkAction:(uint16_t)action
{
    if (!self.serialPort)
    {
        return;
    }

    snapshotBuffer[0] = 0x9f;
    snapshotBuffer[1] = action;
    snapshotBuffer[2] = ((action >> 8) | 0x80);
    snapshotBuffer[3] = [self crc7:snapshotBuffer length:3];
    [self sendData:[NSData dataWithBytes:snapshotBuffer length:4] expectedResponse:_sendOkResponse responseLength:1];
}

- (void)sendZ80Snapshot:(unsigned char *)snapshot
{
    if (!self.serialPort)
    {
        return;
    }
    
    uint8_t snapRegBuffer[cSMARTLINK_SNAPSHOT_HEADER_LENGTH];
    snapRegBuffer[0] = snapshot[10]; // I
    snapRegBuffer[1] = snapshot[19]; // _HL
    snapRegBuffer[2] = snapshot[20];
    snapRegBuffer[3] = snapshot[17]; // _DE
    snapRegBuffer[4] = snapshot[18];
    snapRegBuffer[5] = snapshot[15]; // _BC
    snapRegBuffer[6] = snapshot[16];
    snapRegBuffer[7] = snapshot[22]; // _AF
    snapRegBuffer[8] = snapshot[21];
    snapRegBuffer[9] = snapshot[4]; // HL
    snapRegBuffer[10] = snapshot[5];
    snapRegBuffer[11] = snapshot[13]; // DE
    snapRegBuffer[12] = snapshot[14];
    snapRegBuffer[13] = snapshot[2]; // BC
    snapRegBuffer[14] = snapshot[3];
    snapRegBuffer[15] = snapshot[23]; // IY
    snapRegBuffer[16] = snapshot[24];
    snapRegBuffer[17] = snapshot[25]; //IX
    snapRegBuffer[18] = snapshot[27]; // IFF2
    snapRegBuffer[19] = snapshot[27]; // EI
    snapRegBuffer[20] = snapshot[11]; // R
    snapRegBuffer[21] = snapshot[1]; // AF
    snapRegBuffer[22] = snapshot[0];
    snapRegBuffer[23] = snapshot[8]; // SP
    snapRegBuffer[24] = snapshot[9];
    snapRegBuffer[25] = snapshot[29]; // IM
    snapRegBuffer[26] = snapshot[12]; // Border
    snapRegBuffer[27] = snapshot[32]; // PC
    snapRegBuffer[28] = snapshot[33];

    // Reset Retroleum card
    [self sendSmartlinkAction:cSMARTLINK_RESET];

    // Send register data using the new snapBuffer
    std::cout << "Send Regs" << "\n";
    [self sendBlockWithCommand:cCMD_LOAD_REGS
                      location:0
                        length:cSMARTLINK_SNAPSHOT_HEADER_LENGTH
                          data:snapRegBuffer
              expectedResponse:self.sendOkResponse];

    uint8_t hardwareType = snapshot[34];
    int16_t additionHeaderBlockLength = 0;
    additionHeaderBlockLength = ((uint16_t *)&snapshot[30])[0];
    uint32_t offset = 32 + additionHeaderBlockLength;
    
    int snapshotSize = 0;
    switch (hardwareType)
    {
        case cZ80_V3_MACHINE_TYPE_48:
        case cZ80_V2_MACHINE_TYPE_48_IF1:
                snapshotSize = (48 * 1024) + cZ80_V3_HEADER_SIZE + (cZ80_V3_PAGE_HEADER_SIZE * 3);
            break;
        case cZ80_V2_MACHINE_TYPE_128:
        case cZ80_V3_MACHINE_TYPE_128:
        case cZ80_V3_MACHINE_TYPE_128_2:
        case cz80_V3_MACHINE_TYPE_128_MGT:
                snapshotSize = (128 * 1024) + cZ80_V3_HEADER_SIZE + (cZ80_V3_PAGE_HEADER_SIZE * 8);
            break;
    }
        
    while (offset < snapshotSize)
    {
        uint32_t compressedLength = ((uint16_t *)&snapshot[offset])[0];
        bool isCompressed = true;
        if (compressedLength == 0xffff)
        {
            compressedLength = 0x4000;
            isCompressed = false;
        }

        uint32_t pageId = snapshot[offset + 2];
        
        if (hardwareType == cZ80_V3_MACHINE_TYPE_48)
        {
            // 48k - Uses the Z80 snapshot format page numbers
            switch (pageId) {
            case 4:
                    std::cout << "Send 48k page 4" << "\n";
                    [self bulkSendWithSpectrumMemoryAddress:0x8000 length:compressedLength snapshotOffset:offset + 3 snapshot:snapshot];
                break;
            case 5:
                    std::cout << "Send 48k page 5" << "\n";
                    [self bulkSendWithSpectrumMemoryAddress:0xc000 length:compressedLength snapshotOffset:offset + 3 snapshot:snapshot];
                break;
            case 8:
                    std::cout << "Send 48k page 8" << "\n";
                    [self bulkSendWithSpectrumMemoryAddress:0x4000 length:compressedLength snapshotOffset:offset + 3 snapshot:snapshot];
                break;
            default:
                break;
            }
        }
        else
        {
            uint8_t page = pageId - 3;
            uint8_t bankSwitch[3] = {0xfd, 0x7f, page};
            [self sendBlockWithCommand:cCMD_SET_PORTS
                              location:0
                                length:3
                                  data:bankSwitch
                      expectedResponse:self.sendOkResponse];
            std::cout << "Send 128k page " << pageId - 3 << "\n";
            [self bulkSendWithSpectrumMemoryAddress:0xc000 length:compressedLength snapshotOffset:offset + 3 snapshot:snapshot];
        }

        offset += compressedLength + 3;
    }
    
    uint8_t portValues[3] = {0xfd, 0x7f, static_cast<uint8_t>(snapshot[35])};

    switch (hardwareType)
    {
        case cZ80_V2_MACHINE_TYPE_128:
        case cZ80_V3_MACHINE_TYPE_128:
        case cZ80_V3_MACHINE_TYPE_128_2:
        case cz80_V3_MACHINE_TYPE_128_MGT:
            NSLog(@"Set port 0x7ffd to %i", snapshot[35]);

            [self sendBlockWithCommand:cCMD_SET_PORTS
                              location:0
                                length:3
                                  data:portValues
                      expectedResponse:self.sendOkResponse];

            break;
        default:
            break;
    }

    uint8_t byte12 = snapshot[12];
    byte12 = (byte12 == 255) ? 1 : byte12;
    portValues[0] = 0xfe;
    portValues[1] = 0x0;
    portValues[2] = (byte12 >> 1) & 0x07;
    [self sendBlockWithCommand:cCMD_SET_PORTS
                      location:0
                        length:3
                          data:portValues
              expectedResponse:self.sendOkResponse];

    // Send start game
    [self sendBlockWithCommand:cCMD_RESTART
                      location:0
                        length:0
                          data:snapshot
              expectedResponse:self.sendOkResponse];

}

- (void)bulkSendWithSpectrumMemoryAddress:(uint16_t)spectrumAddress length:(uint16_t)length snapshotOffset:(uint32_t)snapshotIndex snapshot:(unsigned char *)snapshot
{
    // Send memory data
    for (int block = 0; block < (length / cSERIAL_BLOCK_SIZE); block++)
    {
        [self sendBlockWithCommand:cCMD_LOAD_DATA
                          location:spectrumAddress
                            length:cSERIAL_BLOCK_SIZE
                              data:snapshot + snapshotIndex
                  expectedResponse:self.sendOkResponse];

        snapshotIndex += cSERIAL_BLOCK_SIZE;
        spectrumAddress += cSERIAL_BLOCK_SIZE;
    }

    // Deal with any partial block data left over
    if (length % cSERIAL_BLOCK_SIZE)
    {
        [self sendBlockWithCommand:cCMD_LOAD_DATA
                          location:spectrumAddress
                            length:length % cSERIAL_BLOCK_SIZE
                              data:snapshot + snapshotIndex
                  expectedResponse:self.sendOkResponse];
    }
}


- (void)sendSNASnapshot:(unsigned char *)snapshot
{
    if (!self.serialPort)
    {
        return;
    }

    // Reset Retroleum card
    [self sendSmartlinkAction:cSMARTLINK_RESET];

//    uint8_t buffer[21] = {
//        0xfe, 0x0, 0x07,
//        0xfe, 0x0, 0x06,
//        0xfe, 0x0, 0x05,
//        0xfe, 0x0, 0x04,
//        0xfe, 0x0, 0x03,
//        0xfe, 0x0, 0x02,
//        0xfe, 0x0, 0x01
//    };
//
//    [self sendBlockWithCommand:cCMD_SET_PORTS
//                      location:0
//                        length:21
//                          data:buffer
//              expectedResponse:self.sendOkResponse];
//
//    return;
    
    int snapshotIndex = 0;
    unsigned short spectrumAddress = cSNAPSHOT_START_ADDRESS;

    
    // Copy the header from the SNA Snapshot to a new buffer as we will be extending it by two bytes
    uint8_t snapRegBuffer[29];
    memcpy(snapRegBuffer, snapshot, cSMARTLINK_SNAPSHOT_HEADER_LENGTH);
    
    // Get the value at (SP) and decrement SP twice, this is so we can send SP as register data to the spectrum and have a
    // uniform executor (.SNA files store the PC at the current SP)
    uint16_t *pStackPointer =  reinterpret_cast<uint16_t*>(snapRegBuffer + 23);
    uint16_t programCounter = *reinterpret_cast<uint16_t*>(snapshot + cSNAPSHOT_HEADER_LENGTH + *pStackPointer - 0x4000);
    *pStackPointer += 2;
    
    // Add the PC to the end of the SNA header
    snapRegBuffer[27] = programCounter & 0xff;
    snapRegBuffer[28] = programCounter >> 8;
    
    // Send register data using the new snapBuffer
    [self sendBlockWithCommand:cCMD_LOAD_REGS
                      location:snapshotIndex
                        length:cSMARTLINK_SNAPSHOT_HEADER_LENGTH
                          data:snapRegBuffer
              expectedResponse:self.sendOkResponse];

    snapshotIndex += cSNAPSHOT_HEADER_LENGTH;
    
    // Send memory data
    for (int block = 0; block < (cSNAPSHOT_DATA_SIZE / cSERIAL_BLOCK_SIZE); block++)
    {
        [self sendBlockWithCommand:cCMD_LOAD_DATA
                          location:spectrumAddress
                            length:cSERIAL_BLOCK_SIZE
                              data:snapshot + snapshotIndex
                  expectedResponse:self.sendOkResponse];

        snapshotIndex += cSERIAL_BLOCK_SIZE;
        spectrumAddress += cSERIAL_BLOCK_SIZE;
    }

    // Deal with any partial block data left over
    if (cSNAPSHOT_DATA_SIZE % cSERIAL_BLOCK_SIZE)
    {
        [self sendBlockWithCommand:cCMD_LOAD_DATA
                          location:spectrumAddress
                            length:cSNAPSHOT_DATA_SIZE % cSERIAL_BLOCK_SIZE
                              data:snapshot + snapshotIndex
                  expectedResponse:self.sendOkResponse];
    }

    // Send start game
    [self sendBlockWithCommand:cCMD_RESTART
                      location:0
                        length:0
                          data:snapshot
              expectedResponse:self.sendOkResponse];
}

- (void)sendBlockWithCommand:(uint8_t)command
                    location:(uint16_t)location
                      length:(uint16_t)length
                        data:(uint8_t *)data
            expectedResponse:(ORSSerialPacketDescriptor *)expectedResponse
{
    snapshotBuffer[0] = 0x9f;
    snapshotBuffer[1] = length + 6;
    snapshotBuffer[2] = ((length + 6) >> 8);
    snapshotBuffer[3] = [self crc7:snapshotBuffer length:3];
    snapshotBuffer[4] = cPKT_CMD_START;
    snapshotBuffer[5] = command;
    snapshotBuffer[6] = location;
    snapshotBuffer[7] = (location >> 8);
    snapshotBuffer[8] = length;
    snapshotBuffer[9] = (length >> 8);
        
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


#pragma mark - CRC


- (uint8_t)crc7:(const uint8_t *)packet length:(size_t)packet_len
{
    uint8_t crc = 0;
    for (size_t i=0;  i < packet_len; i++)
    {
        crc = crc7_table[(crc << 1) ^ packet[i]];
    }
    return ((crc << 1) | 1);
}


#pragma mark - Getters


- (BOOL)serialPortIsSet
{
    return (self.serialPort) ? YES : NO;
}
@end
