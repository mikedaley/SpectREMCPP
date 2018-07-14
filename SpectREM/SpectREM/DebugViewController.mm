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
#include "Debug.hpp"

#pragma mark - Debugger Command Tokens/Operators & Actions

static NSString *const cTOKEN_BREAKPOINT = @"BP";
static NSString *const cTOKEN_PAUSE = @"P";
static NSString *const cTOKEN_RESUME = @"R";
static NSString *const cTOKEN_STEP = @"S";
static NSString *const cTOKEN_STEP_FRAME = @"SF";
static NSString *const cTOKEN_SEARCH_MEMORY = @"SM";
static NSString *const cTOKEN_WRITE_MEMORY = @"WM";
static NSString *const cTOKEN_DISASSEMBLE = @"D";
static NSString *const cTOKEN_SET_REGISTER = @"SR";
static NSString *const cTOKEN_FILL_MEMORY = @"FM";

static NSString *const cOPERATOR_GREATER_THAN = @">";
static NSString *const cOPERATOR_LESS_THAN = @"<";
static NSString *const cOPERATOR_EQUAL_TO = @"=";

static NSString *const cTOKEN_BREAKPOINT_ACTION_EXECUTE = @"X";
static NSString *const cTOKEN_BREAKPOINT_ACTION_READ = @"R";
static NSString *const cTOKEN_BREAKPOINT_ACTION_WRITE = @"W";

#pragma mark - Private Interface

@interface DebugViewController ()

@property (assign) unsigned short disassembleAddress;
@property (strong) NSMutableArray *disassemblyArray;
@property (strong) NSMutableArray *stackArray;
@property (assign) unsigned int byteWidth;
@property (assign) int memoryTableSearchAddress;
@property (strong) NSDictionary *z80ByteRegisters;
@property (strong) NSDictionary *z80WordRegisters;
@property (assign) NSInteger colWidth1;
@property (assign) NSInteger colWidth2;
@property (strong) NSTimer *updateTimer;
@end

@implementation DebugViewController

- (instancetype)initWithCoder:(NSCoder *)coder
{
    if ([super initWithCoder:coder])
    {
        NSLog(@"DEBUG_VIEW_CONTROLLER INIT");
        self.byteWidth = 12;
        self.disassembleAddress = 0;
        self.memoryTableSearchAddress = -1;
        self.hexFormat =  YES;

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
    
    self.disassemblyTableview.enclosingScrollView.wantsLayer = YES;
    self.disassemblyTableview.enclosingScrollView.layer.cornerRadius = 5;
    self.stackTable.enclosingScrollView.wantsLayer = YES;
    self.stackTable.enclosingScrollView.layer.cornerRadius = 6;
    self.breakpointTableView.enclosingScrollView.wantsLayer = YES;
    self.breakpointTableView.enclosingScrollView.layer.cornerRadius = 6;
    self.memoryTableView.enclosingScrollView.wantsLayer = YES;
    self.memoryTableView.enclosingScrollView.layer.cornerRadius = 6;
    
    self.effectView.material = NSVisualEffectMaterialUltraDark;
}

- (void)viewWillDisappear
{
    [[NSNotificationCenter defaultCenter] removeObserver:NSViewFrameDidChangeNotification];
    [[NSNotificationCenter defaultCenter] removeObserver:cCPU_PAUSED_NOTIFICATION];
    [[NSNotificationCenter defaultCenter] removeObserver:cCPU_RESUMED_NOTIFICATION];
    [self.updateTimer invalidate];
}

- (void)viewWillAppear
{
    [[NSNotificationCenter defaultCenter] addObserverForName:NSViewFrameDidChangeNotification object:self.memoryTableView.enclosingScrollView queue:NULL usingBlock:^(NSNotification * _Nonnull note) {
        [self updateMemoryTableSize];
    }];

    [self.emulationViewController pauseMachine];

    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];
    debugger->disassemble(0, 65535, self.hexFormat);
    
    [self updateDisassemblyTable];
    [self updateCPUDetails];
    [self updateStackTable];
    [self updateMemoryTable];
    [self.emulationViewController updateDisplay];
}

#pragma mark - Table View Methods

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];
    
    if (debugger)
    {
        if ([tableView.identifier isEqualToString:@"DisassembleTableView"])
        {
            return debugger->numberOfMnemonics();
        }
        
        else if ([tableView.identifier isEqualToString:@"MemoryTableView"])
        {
            return (65535 / self.byteWidth) + 1;
        }
        
        else if ([tableView.identifier isEqualToString:@"StackTable"])
        {
            return debugger->numberOfStackEntries();
        }

        else if ([tableView.identifier isEqualToString:@"BreakpointTableView"])
        {
            return debugger->numberOfBreakpoints();
        }
    }
    return 0;
}

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];
 
    if (debugger)
    {
        if ([tableView.identifier isEqualToString:@"DisassembleTableView"])
        {
            return [self disassemblyTable:tableView viewForTableColumn:tableColumn row:row];
        }
        
        if ([tableView.identifier isEqualToString:@"MemoryTableView"])
        {
            return [self memoryTable:tableView viewForTableColumn:tableColumn row:row];
        }
        
        if ([tableView.identifier isEqualToString:@"StackTable"])
        {
            return [self stackTable:tableView viewForTableColumn:tableColumn row:row];
        }
        
        if ([tableView.identifier isEqualToString:@"BreakpointTableView"])
        {
            return [self breakpointTable:tableView viewForTableColumn:tableColumn row:row];
        }
    }
    
    return nil;
}

#pragma mark - Table View Rows/Columns

- (NSView *)disassemblyTable:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

    NSTableCellView *view;
    view = [tableView makeViewWithIdentifier:tableColumn.identifier owner:self];
    if (view)
    {
        Debug::DisassembledOpcode dop = debugger->disassembly(row);
        
        if ([tableColumn.identifier isEqualToString:@"AddressColID"])
        {
            if (self.hexFormat)
            {
                view.textField.stringValue = [NSString stringWithFormat:@"$%04X", stoi(dop.address)];
            }
            else
            {
                view.textField.stringValue = [NSString stringWithUTF8String:dop.address.c_str()];
            }
        }
        else if ([tableColumn.identifier isEqualToString:@"BytesColID"])
        {
            if (self.hexFormat)
            {
                NSArray *values = [[NSString stringWithUTF8String:dop.bytes.c_str()] componentsSeparatedByString:@" "];
                NSString *bytes = @"";
                bytes = [NSString stringWithFormat:@"%02X", (uint8_t)[(NSString *)values[0] intValue]];
                for (int i = 1; i < values.count - 1; i++)
                {
                    bytes = [NSString stringWithFormat:@"%@ %02X", bytes, (uint8_t)[(NSString *)values[i] intValue]];
                }
                view.textField.stringValue = bytes;
            }
            else
            {
                view.textField.stringValue = [NSString stringWithUTF8String:dop.bytes.c_str()];
            }
        }
        else if ([tableColumn.identifier isEqualToString:@"DisassemblyColID"])
        {
            view.textField.stringValue = [NSString stringWithUTF8String:dop.mnemonic.c_str()];
        }
    }
    
    return view;
}

- (NSView *)breakpointTable:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

    NSTableCellView *view;
    view = [tableView makeViewWithIdentifier:tableColumn.identifier owner:self];
    if (view)
    {
        Debug::Breakpoint bp = debugger->breakpoint(row);
        if ([tableColumn.identifier isEqualToString:@"BreakpointAddressColID"])
        {
            if (self.hexFormat)
            {
                view.textField.stringValue = [NSString stringWithFormat:@"$%04X", bp.address];
            }
            else
            {
                view.textField.stringValue = [NSString stringWithFormat:@"%05i", bp.address];
            }
        }
        else if ([tableColumn.identifier isEqualToString:@"BreakpointConditionColID"])
        {
            NSString *condition = @"";
            uint8_t type = bp.type;
            if (type & ZXSpectrum::eDebugReadOp)
            {
                condition = [condition stringByAppendingString:@"READ "];
            }
            if (type & ZXSpectrum::eDebugWriteOp)
            {
                condition = [condition stringByAppendingString:@"WRITE "];
            }
            if (type & ZXSpectrum::eDebugExecuteOp)
            {
                condition = [condition stringByAppendingString:@"EXEC "];
            }
            view.textField.stringValue = condition;
        }
    }
    
    return view;
}

- (NSView *)memoryTable:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

    NSTableCellView *view;
    view = [tableView makeViewWithIdentifier:tableColumn.identifier owner:self];
    if (view)
    {
        if ([tableColumn.identifier isEqualToString:@"MemoryAddressColID"])
        {
            unsigned short address = row * self.byteWidth;
            
            if (self.hexFormat)
            {
                view.textField.stringValue = [NSString stringWithFormat:@"$%04X", address];
            }
            else
            {
                view.textField.stringValue = [NSString stringWithFormat:@"%05i", address];
            }
        }
        else if ([tableColumn.identifier isEqualToString:@"MemoryBytesColID"])
        {
            NSMutableAttributedString *content = [NSMutableAttributedString new];
            for (unsigned int i = 0; i < self.byteWidth; i++)
            {
                unsigned int address = ((int)row * self.byteWidth) + i;
                
                NSMutableAttributedString *attrString;
                if (self.hexFormat)
                {
                    attrString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithFormat:@"%02X ", (unsigned short)debugger->machine->z80Core.Z80CoreDebugMemRead(address, NULL)]];
                }
                else
                {
                    attrString = [[NSMutableAttributedString alloc] initWithString:[NSString stringWithFormat:@"%03i ", (unsigned short)debugger->machine->z80Core.Z80CoreDebugMemRead(address, NULL)]];
                }
                
                int highlightLength = 2;
                if (!self.hexFormat)
                {
                    highlightLength = 3;
                }

                if (self.memoryTableSearchAddress == address)
                {
                    [attrString addAttribute:NSBackgroundColorAttributeName value:[NSColor colorWithRed:0 green:0.5 blue:0 alpha:1.0] range:NSMakeRange(0, highlightLength)];
                }
                else if (address > 0xffff)
                {
                    [content appendAttributedString:[[NSAttributedString alloc] initWithString:@"   "]];
                }
                else
                {
                    NSColor *color = [NSColor clearColor];
                    switch (debugger->breakpointAtAddress(address)) {
                        case ZXSpectrum::eDebugReadOp:
                        case ZXSpectrum::eDebugWriteOp:
                            color = [NSColor colorWithRed:0.75 green:0 blue:0 alpha:1.0];
                            break;
                        case ZXSpectrum::eDebugExecuteOp:
                            color = [NSColor colorWithRed:0.75 green:0 blue:0.75 alpha:1.0];
                    }
                    [attrString addAttribute:NSBackgroundColorAttributeName value:color range:NSMakeRange(0, highlightLength)];
                }
                [content appendAttributedString:attrString];
            }
            
            view.textField.attributedStringValue = content;
        }
        else if ([tableColumn.identifier isEqualToString:@"MemoryASCIIColID"])
        {
            NSMutableString *content = [NSMutableString new];
            for (int i = 0; i < self.byteWidth; i++)
            {
                unsigned char c = debugger->machine->z80Core.Z80CoreDebugMemRead((unsigned short)((row * self.byteWidth) + i), NULL);
                if ((c >= 0 && c < 32) || c > 126)
                {
                    [content appendString:@"."];
                }
                else
                {
                    NSString *character = [NSString stringWithFormat:@"%c", c];
                    [content appendString:character];
                }
            }
            view.textField.stringValue = content;
        }
    }
    
    return view;
}

- (NSView *)stackTable:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

    NSTableCellView *view;
    view = [tableView makeViewWithIdentifier:tableColumn.identifier owner:self];
    if (view)
    {
        if (self.hexFormat)
        {
            view.textField.stringValue = [NSString stringWithFormat:@"$%04X", debugger->stackAddress(row)];
        }
        else
        {
            view.textField.stringValue = [NSString stringWithFormat:@"%05i", debugger->stackAddress(row)];
        }
    }
    
    return view;
}

- (void)tableView:(NSTableView *)tableView didAddRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

    if ([tableView.identifier isEqualToString:@"DisassembleTableView"])
    {
        NSColor *rowColor = [NSColor clearColor];
        
        Debug::DisassembledOpcode dop = debugger->disassembly(row);

        for (int i = 0; i < debugger->numberOfBreakpoints(); i++)
        {
            if (debugger->breakpoint(i).address == stoi(dop.address))
            {
                rowColor =  [NSColor colorWithRed:0.6 green:0 blue:0 alpha:1.0];
                break;
            }
        }

        if (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_PC) == stoi(dop.address))
        {
            rowColor = [NSColor colorWithRed:0 green:0.6 blue:0 alpha:1.0];
        }

        rowView.backgroundColor = rowColor;
    }
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
    
}

- (IBAction)disassembleTableDoubleClick:(id)sender {
    
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

//    uint address;
//    if (self.hexFormat)
//    {
//        sscanf(debugger->disassembly(self.disassemblyTableview.clickedRow).address.c_str(), "%x", &address);
//    }
//    else
//    {
//        sscanf(debugger->disassembly(self.disassemblyTableview.clickedRow).address.c_str(), "%i", &address);
//    }

    uint address = stoi(debugger->disassembly(self.disassemblyTableview.clickedRow).address);
    
    bool breakpointRemoved = false;
    for (int i = 0; i < debugger->numberOfBreakpoints(); i++)
    {
        if (debugger->breakpoint(i).address == address)
        {
            debugger->removeBreakpoint(address, ZXSpectrum::eDebugExecuteOp);
            breakpointRemoved = true;
            break;
        }
    }

    if (!breakpointRemoved)
    {
        debugger->addBreakpoint(address, ZXSpectrum::eDebugExecuteOp);
    }

    [self reloadDisassemblyData];
    [self.breakpointTableView reloadData];
    [self.memoryTableView reloadData];
}

- (void)updateMemoryTableSize
{
    NSInteger col1Width = self.memoryTableView.enclosingScrollView.frame.size.width * 0.614;
    NSInteger col2Width = self.memoryTableView.enclosingScrollView.frame.size.width * 0.260;
    [self.memoryTableView.tableColumns[1] setWidth:col1Width];
    [self.memoryTableView.tableColumns[2] setWidth:col2Width];
    self.byteWidth = fabs(self.memoryTableView.tableColumns[1].width / 21.66);

    [self.memoryTableView reloadDataForRowIndexes:
     [NSIndexSet indexSetWithIndexesInRange:
      [self.memoryTableView rowsInRect:self.memoryTableView.visibleRect]
      ] columnIndexes:[self.memoryTableView columnIndexesInRect:self.memoryTableView.visibleRect]
     ];
}

#pragma mark - Debug Controls

- (IBAction)step:(id)sender {
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];
    debugger->machine->step();
    [self updateViewDetails];
}

- (IBAction)startMachine:(id)sender
{
    [self.emulationViewController startMachine];
}

- (IBAction)pauseMachine:(id)sender
{
    [self.emulationViewController pauseMachine];
    [self updateViewDetails];
    [self updateDisassemblyTable];
}

#pragma mark - Parse Commands

- (void)controlTextDidEndEditing:(NSNotification *)obj
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

    // Explode the entered command on space
    NSArray *commandList = [[(NSTextField *)obj.object stringValue] componentsSeparatedByString:@" "];
    NSString *command = [(NSString *)commandList[0] uppercaseString];
    
    // Check to see if the command is recognised
    if ([command isEqualToString:cTOKEN_DISASSEMBLE])
    {
        [self tokenDisassemble:commandList machine:debugger->machine];
    }
    else if ([command isEqualToString:cTOKEN_RESUME])
    {
        [self tokenResume:commandList];
    }
    else if ([command isEqualToString:cTOKEN_PAUSE])
    {
        [self tokenPause:commandList];
    }
    else if ([command isEqualToString:cTOKEN_STEP])
    {
        [self tokenStep:commandList];
    }
    else if ([command isEqualToString:cTOKEN_SEARCH_MEMORY])
    {
        [self tokenMemorySearch:commandList];
    }
    else if ([command isEqualToString:cTOKEN_WRITE_MEMORY])
    {
        [self tokenMemoryWrite:commandList machine:debugger->machine];
    }
    else if ([command isEqualToString:cTOKEN_SET_REGISTER])
    {
        [self tokenSetRegister:commandList machine:debugger->machine];
    }
    else if ([command isEqualToString:cTOKEN_STEP_FRAME])
    {
        [self tokenStepFrame:commandList machine:debugger->machine];
    }
    else if ([command isEqualToString:cTOKEN_BREAKPOINT])
    {
        [self tokenBreakpoint:commandList machine:debugger->machine];
    }
    else if ([command isEqualToString:cTOKEN_FILL_MEMORY])
    {
        [self tokenFillMemory:commandList machine:debugger->machine];
    }
}

#pragma mark - Token Methods

- (void)tokenPause:(NSArray *)commandList
{
    [self.emulationViewController pauseMachine];
    [self updateViewDetails];
}

- (void)tokenResume:(NSArray *)commandList
{
    [self.emulationViewController startMachine];
}

- (void)tokenStep:(NSArray *)commandList
{
    [self.emulationViewController pauseMachine];
    [self step:nil];
    [self updateViewDetails];
}

- (void)tokenStepFrame:(NSArray *)commandList machine:(ZXSpectrum *)machine
{
    [self.emulationViewController pauseMachine];
    machine->generateFrame();
    [self updateViewDetails];
}

- (void)tokenMemorySearch:(NSArray *)commandList
{
    if ( commandList.count > 1 )
    {
        NSScanner *scanner = [NSScanner scannerWithString:commandList[1]];
        unsigned int address;
        if ( [scanner scanHexInt:&address]  && address <= 0xffff )
        {
            self.memoryTableSearchAddress = address;
            NSUInteger row = (address / self.byteWidth);
            [self.memoryTableView scrollRowToVisible:row];
            [self updateMemoryTable];
        }
        else
        {
            [self displayTokenError:@"Address must be in the range 0x0000 to 0xFFFF"];
        }
    }
    else
    {
        [self displayTokenError:@"No valid address was provided!"];
    }
}

- (void)tokenMemoryWrite:(NSArray *)commandList machine:(ZXSpectrum *)machine
{
    if ( commandList.count > 2 )
    {
        NSScanner *scanner = [NSScanner scannerWithString:commandList[1]];
        unsigned int address;
        if ( [scanner scanHexInt:&address] && address <= 0xffff )
        {
            scanner = [NSScanner scannerWithString:commandList[2]];
            unsigned int value;
            if ( [scanner scanHexInt:&value] && value <= 0xff )
            {
                self.memoryTableSearchAddress = address;
                machine->z80Core.Z80CoreDebugMemWrite(address, value, NULL);
                NSUInteger row = (address / self.byteWidth);
                [self.memoryTableView scrollRowToVisible:row];
                [self updateMemoryTable];
            }
            else
            {
                [self displayTokenError:@"Value must be in the range 0x00 to 0xFF"];
            }
        }
        else
        {
            [self displayTokenError:@"Address must be in the range 0x0000 to 0xFFFF"];
        }
    }
    else
    {
        [self displayTokenError:@"An address and value must be supplied!"];
    }
}

- (void)tokenDisassemble:(NSArray *)commandList machine:(ZXSpectrum *)machine
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];
    
    if (commandList.count >= 1)
    {
        NSScanner *scanner = [NSScanner scannerWithString:commandList[1]];
        unsigned int address;
        if ( [scanner scanHexInt:&address] )
        {
            self.disassembleAddress = address;
            debugger->disassemble(self.disassembleAddress, 65535 - self.disassembleAddress, self.hexFormat);
        }
        else if ( [[commandList[1] uppercaseString]isEqualToString:@"PC"] )
        {
            self.disassembleAddress = machine->z80Core.GetRegister(CZ80Core::eREG_PC);
            debugger->disassemble(self.disassembleAddress, 65535 - self.disassembleAddress, self.hexFormat);
        }
        else if ( [[commandList[1] uppercaseString]isEqualToString:@"SP"] )
        {
            if (self.stackArray.count > 0)
            {
                self.disassembleAddress = [self.stackArray[0] unsignedShortValue];
                debugger->disassemble(self.disassembleAddress, 65535 - self.disassembleAddress, self.hexFormat);
            }
        }
        [self reloadDisassemblyData];
    }
    else
    {
        [self displayTokenError:@"An address to disassemble from must be provided!"];
    }
}

- (void)tokenBreakpoint:(NSArray *)commandList machine:(ZXSpectrum *)machine
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];
    
    if ( [[commandList[1] uppercaseString] isEqualToString:cTOKEN_BREAKPOINT_ACTION_EXECUTE] )
    {
        NSScanner *scanner = [NSScanner scannerWithString:commandList[2]];
        unsigned int value;
        if ( [scanner scanHexInt:&value] )
        {
            machine->breakpoints[ value ] = machine->breakpoints[ value ] | ZXSpectrum::eDebugExecuteOp;
            debugger->addBreakpoint(value, ZXSpectrum::eDebugExecuteOp);
            [self updateViewDetails];
        }
    }
    else if ( [[commandList[1] uppercaseString] isEqualToString:cTOKEN_BREAKPOINT_ACTION_READ] )
    {
        NSScanner *scanner = [NSScanner scannerWithString:commandList[2]];
        unsigned int value;
        if ( [scanner scanHexInt:&value] )
        {
            machine->breakpoints[ value ] = machine->breakpoints[ value ] | ZXSpectrum::eDebugReadOp;
            debugger->addBreakpoint(value, ZXSpectrum::eDebugReadOp);
            [self updateViewDetails];
        }
    }
    else if ( [[commandList[1] uppercaseString] isEqualToString:cTOKEN_BREAKPOINT_ACTION_WRITE] )
    {
        NSScanner *scanner = [NSScanner scannerWithString:commandList[2]];
        unsigned int value;
        if ( [scanner scanHexInt:&value] )
        {
            machine->breakpoints[ value ] = machine->breakpoints[ value ] | ZXSpectrum::eDebugWriteOp;
            debugger->addBreakpoint(value, ZXSpectrum::eDebugWriteOp);
            [self updateViewDetails];
        }
    }
}

- (void)tokenSetRegister:(NSArray *)commandList machine:(ZXSpectrum *)machine
{
    NSString *reg = [commandList[1] uppercaseString];
    for ( NSString *key in self.z80ByteRegisters )
    {
        if ([key isEqualToString:reg])
        {
            NSScanner *scanner = [NSScanner scannerWithString:commandList[2]];
            unsigned int value;
            if ( [scanner scanHexInt:&value] )
            {
                machine->z80Core.SetRegister((CZ80Core::eZ80BYTEREGISTERS)[self.z80ByteRegisters[key] integerValue], value);
            }
            [self updateViewDetails];
            return;
        }
    }
    
    for ( NSString *key in self.z80WordRegisters )
    {
        if ([key isEqualToString:reg])
        {
            NSScanner *scanner = [NSScanner scannerWithString:commandList[2]];
            unsigned int value;
            if ( [scanner scanHexInt:&value] )
            {
                machine->z80Core.SetRegister((CZ80Core::eZ80WORDREGISTERS)[self.z80WordRegisters[key] integerValue], value);
            }
            [self updateViewDetails];
            return;
        }
    }
}

- (void)tokenFillMemory:(NSArray *)commandList machine:(ZXSpectrum *)machine
{
    if (commandList.count == 4)
    {
        unsigned int fromAddress;
        unsigned int toAddress;
        unsigned int value;
        NSScanner *scanner = [NSScanner scannerWithString:commandList[1]];
        [scanner scanHexInt:&fromAddress];
        scanner = [NSScanner scannerWithString:commandList[2]];
        [scanner scanHexInt:&toAddress];
        scanner = [NSScanner scannerWithString:commandList[3]];
        [scanner scanHexInt:&value];

        for(int i = fromAddress; i < toAddress; i++)
        {
            machine->coreDebugWrite(i, value, NULL);
        }
        [self updateViewDetails];
    }
    else
    {
        [self displayTokenError:@"From address, to address and value must be specific"];
    }
}

- (void)displayTokenError:(NSString*)errorString
{
    NSAlert *alert = [NSAlert new];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"Syntax Error"];
    [alert setInformativeText:errorString];
    [alert runModal];
}

#pragma mark - View Updates

- (void)breakpointHitAddress:(unsigned short)address operation:(uint8_t)operation
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

    self.disassembleAddress = address;
    debugger->disassemble(self.disassembleAddress, 65535 - self.disassembleAddress, self.hexFormat);
    [self updateDisassemblyTable];
    [self reloadDisassemblyData];
    [self updateViewDetails];
}

- (void)updateViewDetails
{
    [self updateCPUDetails];
    [self updateMemoryTable];
    [self updateStackTable];
    [self updateDisassemblyTable];
    [self updateBreakpointTable];
    [[NSNotificationCenter defaultCenter] postNotificationName:cDISPLAY_UPDATE_NOTIFICATION object:NULL];
}

- (void)updateDisassemblyTable
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

    dispatch_async(dispatch_get_main_queue(), ^{

        NSRect visibleRect = self.disassemblyTableview.visibleRect;
        NSRange range = [self.disassemblyTableview rowsInRect:visibleRect];
        
        if (range.length != 0)
        {
            for (NSUInteger i = range.location; i < range.location + range.length - 1; i++)
            {
                Debug::DisassembledOpcode dop = debugger->disassembly(i);

                if (stoi(dop.address) == debugger->machine->z80Core.GetRegister(CZ80Core::eREG_PC))
                {
                    debugger->disassemble(self.disassembleAddress, 65535 - self.disassembleAddress, self.hexFormat);
                    [self reloadDisassemblyData];
                    [self.disassemblyTableview deselectAll:NULL];
                    [self.disassemblyTableview scrollRowToVisible:i];
                    return;
                }
            }
        }
        self.disassembleAddress = debugger->machine->z80Core.GetRegister(CZ80Core::eREG_PC);
        debugger->disassemble(self.disassembleAddress, 65535 - self.disassembleAddress, self.hexFormat);
        [self reloadDisassemblyData];
        [self.disassemblyTableview scrollRowToVisible:0];
    });

}

- (void)reloadDisassemblyData
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.disassemblyTableview reloadData];
    });
}

- (void)updateCPUDetails
{
    dispatch_async(dispatch_get_main_queue(), ^{
 
        Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

        if (self.hexFormat)
        {
            self.pc = [NSString stringWithFormat:@"$%04X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_PC)];
            self.sp = [NSString stringWithFormat:@"$%04X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_SP)];
            
            self.af = [NSString stringWithFormat:@"$%04X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_AF)];
            self.bc = [NSString stringWithFormat:@"$%04X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_BC)];
            self.de = [NSString stringWithFormat:@"$%04X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_DE)];
            self.hl = [NSString stringWithFormat:@"$%04X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_HL)];
            
            self.a_af = [NSString stringWithFormat:@"$%04X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_ALT_AF)];
            self.a_bc = [NSString stringWithFormat:@"$%04X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_ALT_BC)];
            self.a_de = [NSString stringWithFormat:@"$%04X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_ALT_DE)];
            self.a_hl = [NSString stringWithFormat:@"$%04X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_ALT_HL)];
            
            self.i = [NSString stringWithFormat:@"$%02X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_I)];
            self.r = [NSString stringWithFormat:@"$%02X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_R)];
            
            self.ix = [NSString stringWithFormat:@"$%04X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_IX)];
            self.iy = [NSString stringWithFormat:@"$%04X", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_IY)];
        }
        else
        {
            self.pc = [NSString stringWithFormat:@"%04i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_PC)];
            self.sp = [NSString stringWithFormat:@"%04i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_SP)];
            
            self.af = [NSString stringWithFormat:@"%04i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_AF)];
            self.bc = [NSString stringWithFormat:@"%04i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_BC)];
            self.de = [NSString stringWithFormat:@"%04i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_DE)];
            self.hl = [NSString stringWithFormat:@"%04i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_HL)];
            
            self.a_af = [NSString stringWithFormat:@"%04i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_ALT_AF)];
            self.a_bc = [NSString stringWithFormat:@"%04i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_ALT_BC)];
            self.a_de = [NSString stringWithFormat:@"%04i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_ALT_DE)];
            self.a_hl = [NSString stringWithFormat:@"%04i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_ALT_HL)];
            
            self.i = [NSString stringWithFormat:@"%02i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_I)];
            self.r = [NSString stringWithFormat:@"%02i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_R)];
            
            self.ix = [NSString stringWithFormat:@"%04i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_IX)];
            self.iy = [NSString stringWithFormat:@"%04i", debugger->machine->z80Core.GetRegister(CZ80Core::eREG_IY)];
        }
        
        self.currentRom = [NSString stringWithFormat:@"%02i", debugger->machine->emuROMPage];
        self.displayPage = [NSString stringWithFormat:@"%02i", debugger->machine->emuDisplayPage];
        self.ramPage = [NSString stringWithFormat:@"%02i", debugger->machine->emuRAMPage];
        self.iff1 = [NSString stringWithFormat:@"%02i", debugger->machine->z80Core.GetIFF1()];
        self.im = [NSString stringWithFormat:@"%02i", debugger->machine->z80Core.GetIMMode()];
        
        self.tStates = [NSString stringWithFormat:@"%04i", debugger->machine->z80Core.GetTStates()];
        
        self.fs = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_S) ? @"1" : @"0";
        self.fz = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_Z) ? @"1" : @"0";
        self.f5 = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_5) ? @"1" : @"0";
        self.fh = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_H) ? @"1" : @"0";
        self.f3 = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_3) ? @"1" : @"0";
        self.fpv = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_P) ? @"1" : @"0";
        self.fn = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_N) ? @"1" : @"0";
        self.fc = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_C) ? @"1" : @"0";
    });
    

}

- (void)updateStackTable
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];
    debugger->stackTableUpdate();
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.stackTable reloadData];
    });
}

- (void)updateMemoryTable
{
    dispatch_async(dispatch_get_main_queue(), ^{
        NSRect visibleRect = self.memoryTableView.visibleRect;
        NSRange visibleRows = [self.memoryTableView rowsInRect:visibleRect];
        if (visibleRows.length == 0)
        {
            [self.memoryTableView reloadData];
            return;
        }
        NSIndexSet *visibleCols = [self.memoryTableView columnIndexesInRect:visibleRect];
        [self.memoryTableView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndexesInRange:visibleRows] columnIndexes:visibleCols];
    });
}

- (void)updateBreakpointTable
{
    dispatch_async(dispatch_get_main_queue(), ^{
        NSRect visibleRect = self.breakpointTableView.visibleRect;
        NSRange visibleRows = [self.breakpointTableView rowsInRect:visibleRect];
        NSIndexSet *visibleCols = [self.breakpointTableView columnIndexesInRect:visibleRect];
        [self.breakpointTableView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndexesInRange:visibleRows] columnIndexes:visibleCols];
    });
}

@end

