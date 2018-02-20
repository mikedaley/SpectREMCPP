//
//  DebugViewController.m
//  SpectREM
//
//  Created by Mike on 02/01/2018.
//  Copyright © 2018 71Squared Ltd. All rights reserved.
//

#import "DebugViewController.h"
#include "ZXSpectrum.hpp"
#include "Z80Core.h"

@interface DebugViewController ()

@property (assign) unsigned short disassembleAddress;
@property (strong) NSMutableArray *disassemblyArray;
@property (strong) NSMutableArray *stackArray;
@property (assign) unsigned int byteWidth;
@property (assign) int memoryTableSearchAddress;
@property (strong) NSDictionary *z80ByteRegisters;
@property (strong) NSDictionary *z80WordRegisters;

@end

@implementation DebugViewController

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if ([super initWithCoder:coder])
    {
        NSLog(@"DEBUG_VIEW_CONTROLLER INIT");
        self.byteWidth = 28;
        self.disassembleAddress = 0;
        self.memoryTableSearchAddress = -1;
        
        self.z80ByteRegisters = @{@"A" : @(CZ80Core::eREG_A),
                                  @"F" : @(CZ80Core::eREG_F),
                                  @"B" : @(CZ80Core::eREG_B),
                                  @"C" : @(CZ80Core::eREG_C),
                                  @"D" : @(CZ80Core::eREG_D),
                                  @"E" : @(CZ80Core::eREG_E),
                                  @"H" : @(CZ80Core::eREG_H),
                                  @"L" : @(CZ80Core::eREG_L),
                                  @"A'" : @(CZ80Core::eREG_ALT_A),
                                  @"F'" : @(CZ80Core::eREG_ALT_F),
                                  @"B'" : @(CZ80Core::eREG_ALT_B),
                                  @"C'" : @(CZ80Core::eREG_ALT_C),
                                  @"D'" : @(CZ80Core::eREG_ALT_D),
                                  @"E'" : @(CZ80Core::eREG_ALT_E),
                                  @"H'" : @(CZ80Core::eREG_ALT_H),
                                  @"L'" : @(CZ80Core::eREG_ALT_L),
                                  @"I" : @(CZ80Core::eREG_I),
                                  @"R" : @(CZ80Core::eREG_R)
                                  };
        
        self.z80WordRegisters = @{
                                  @"AF" : @(CZ80Core::eREG_AF),
                                  @"BC" : @(CZ80Core::eREG_BC),
                                  @"DE" : @(CZ80Core::eREG_DE),
                                  @"HL" : @(CZ80Core::eREG_HL),
                                  @"AF'" : @(CZ80Core::eREG_ALT_AF),
                                  @"BC'" : @(CZ80Core::eREG_ALT_BC),
                                  @"DE'" : @(CZ80Core::eREG_ALT_DE),
                                  @"HL'" : @(CZ80Core::eREG_ALT_HL),
                                  @"PC" : @(CZ80Core::eREG_PC),
                                  @"SP" : @(CZ80Core::eREG_SP),
                                  };
        
        return self;
    }
    
    return nil;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    self.decimalFormat = NO;
    
    
}

- (void)viewWillDisappear
{

}

- (void)viewWillAppear
{
    [[NSNotificationCenter defaultCenter] addObserverForName:NSViewFrameDidChangeNotification object:self.memoryTableView queue:NULL usingBlock:^(NSNotification * _Nonnull note) {
        [self updateMemoryTableSize];
    }];
    
}

#pragma mark - Debug Controls

@end

#pragma mark - Disassembled Instruction Implementation

@implementation DisassembledOpcode



@end

