//
//  DebugViewController.h
//  SpectREM
//
//  Created by Mike on 02/01/2018.
//  Copyright © 2018 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "EmulationViewController.h"
#import "EmulationController.hpp"

@interface DebugViewController : NSViewController <NSTableViewDataSource, NSTabViewDelegate, NSTextFieldDelegate>

@property (assign) BOOL hexFormat;

// Registers
@property (strong) NSString * pc;
@property (strong) NSString * sp;

@property (strong) NSString * af;
@property (strong) NSString * bc;
@property (strong) NSString * de;
@property (strong) NSString * hl;

@property (strong) NSString * a_af;
@property (strong) NSString * a_bc;
@property (strong) NSString * a_de;
@property (strong) NSString * a_hl;

@property (strong) NSString * ix;
@property (strong) NSString * iy;

@property (strong) NSString * i;
@property (strong) NSString * r;

@property (strong) NSString * im;

// Flags
@property (strong) NSString * fs;
@property (strong) NSString * fz;
@property (strong) NSString * f5;
@property (strong) NSString * fh;
@property (strong) NSString * f3;
@property (strong) NSString * fpv;
@property (strong) NSString * fn;
@property (strong) NSString * fc;

// Machine specifics
@property (strong) NSString * currentRom;
@property (strong) NSString * displayPage;
@property (strong) NSString * ramPage;
@property (strong) NSString * iff1;
@property (strong) NSString * tStates;

@property (assign)  EmulationViewController * emulationViewController;
@property (assign)  EmulationController * emulationController;
@property (weak)    IBOutlet NSTableView * disassemblyTableview;
@property (weak)    IBOutlet NSTableView * memoryTableView;
@property (weak)    IBOutlet NSTableView * stackTable;
@property (weak)    IBOutlet NSTableView * breakpointTableView;
@property (strong)  IBOutlet NSVisualEffectView * effectView;
@property (weak)    IBOutlet NSButton * buttonStep;
@property (weak)    IBOutlet NSButton * buttonBreak;

#pragma mark - Methods

- (void)setupDebugger;
- (void)updateMemoryTableSize;
- (void)updateViewDetails;
- (IBAction)pauseMachine:(id)sender;

@end
