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
static NSString *const cTOKEN_BREAK = @"B";
static NSString *const cTOKEN_CONTINUE = @"C";
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

static NSString *const cTOKEN_BREAKPOINT_ADD_ACTION_EXECUTE = @"+X";
static NSString *const cTOKEN_BREAKPOINT_ADD_ACTION_READ = @"+R";
static NSString *const cTOKEN_BREAKPOINT_ADD_ACTION_WRITE = @"+W";

static NSString *const cTOKEN_BREAKPOINT_REMOVE_ACTION_EXECUTE = @"-X";
static NSString *const cTOKEN_BREAKPOINT_REMOVE_ACTION_READ = @"-R";
static NSString *const cTOKEN_BREAKPOINT_REMOVE_ACTION_WRITE = @"-W";

#pragma mark - Table View & Column Storyboard Identifiers

static NSString *const cDISASSEMBLY_TABLE_VIEW = @"DisassemblyTableView";
static NSString *const cSTACK_TABLE_VIEW = @"StackTableView";
static NSString *const cBREAKPOINTS_TABLE_VIEW = @"BreakpointTableView";
static NSString *const cMEMORY_TABLE_VIEW = @"MemoryTableView";

static NSString *const cBREAKPOINT_ADDRESS_COL = @"BreakpointAddressColID";
static NSString *const cBREAKPOINT_CONDITION_COL = @"BreakpointConditionColID";

static NSString *const cSTACK_ADDRESS_COL = @"StackAddressColID";
static NSString *const cSTACK_VALUE_COL = @"StackValueColID";

static NSString *const cDISASSEMBLY_ADDRESS_COL = @"DisassemblyAddressColID";
static NSString *const cDISASSEMBLY_BYTES_COL = @"DisassemblyBytesColID";
static NSString *const cDISASSEMBLY_MNEMONIC_COL = @"DisassemblyMnemonicColID";

static NSString *const cMEMORY_ADDRESS_COL = @"MemoryAddressColID";
static NSString *const cMEMORY_BYTES_COL = @"MemoryBytesColID";
static NSString *const cMEMORY_ASCII_COL = @"MemoryASCIIColID";

#pragma mark - Colors

static NSColor *const cHIGHLIGHT_COLOR = [NSColor colorWithRed:0 green:0.6 blue:0 alpha:1.0];
static NSColor *const cEXEC_BREAKPOINT_COLOR = [NSColor colorWithRed:0.6 green:0.0 blue:0 alpha:1.0];
static NSColor *const cRDWR_BREAKPOINT_COLOR = [NSColor colorWithRed:0.6 green:0.0 blue:0.6 alpha:1.0];

#pragma mark - Private Interface

@interface DebugViewController ()

@property (assign) unsigned short disassembleAddress;
@property (strong) NSMutableArray *disassemblyArray;
@property (strong) NSMutableArray *stackArray;
@property (assign) int byteWidth;
@property (assign) int memoryTableSearchAddress;
@property (strong) NSDictionary *z80ByteRegisters;
@property (strong) NSDictionary *z80WordRegisters;
@property (assign) NSInteger colWidth1;
@property (assign) NSInteger colWidth2;
@property (strong) NSTimer *updateTimer;
@property (weak) IBOutlet NSTextField *commandTextField;

@end

@implementation DebugViewController

@synthesize hexFormat = _hexFormat;

- (instancetype)initWithCoder:(NSCoder *)coder{ 
    if (self = [super initWithCoder:coder])
    {
        NSLog(@"DEBUG_VIEW_CONTROLLER INIT");
        self.byteWidth = 12;
        self.disassembleAddress = 0;
        self.memoryTableSearchAddress = -1;
        self.hexFormat = YES;
        return self;
    }
    
    return nil;
}

- (void)viewDidLoad {
    [super viewDidLoad];
//    self.effectView.material = NSVisualEffectMaterialUltraDark;
//    self.disassemblyTableview.enclosingScrollView.wantsLayer = YES;
//    self.disassemblyTableview.enclosingScrollView.layer.cornerRadius = 6;
//    self.stackTable.enclosingScrollView.wantsLayer = YES;
//    self.stackTable.enclosingScrollView.layer.cornerRadius = 6;
//    self.breakpointTableView.enclosingScrollView.wantsLayer = YES;
//    self.breakpointTableView.enclosingScrollView.layer.cornerRadius = 6;
//    self.memoryTableView.enclosingScrollView.wantsLayer = YES;
//    self.memoryTableView.enclosingScrollView.layer.cornerRadius = 6;
}

- (void)viewWillDisappear
{
    [[NSNotificationCenter defaultCenter] removeObserver:NSViewFrameDidChangeNotification];
    [self.updateTimer invalidate];
}

- (void)viewWillAppear
{
    [[NSNotificationCenter defaultCenter] addObserverForName:NSViewFrameDidChangeNotification
                                                      object:self.memoryTableView.enclosingScrollView
                                                       queue:NULL
                                                  usingBlock:^(NSNotification * _Nonnull note) {
        [self updateMemoryTableSize];
    }];

    [self.emulationViewController pauseMachine];

    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];
    int pc = debugger->machine->z80Core.GetRegister(CZ80Core::eREG_PC);
    debugger->disassemble(pc , 0xffff - pc, self.hexFormat);
    
    [self.commandTextField becomeFirstResponder];
    
    [self updateDisassemblyTable];
    [self updateCPUDetails];
    [self updateStackTable];
    [self updateMemoryTable];
    [self.emulationViewController updateDisplay];
    [self updateMemoryTableSize];
    [self updateButtonStates];
}

#pragma mark - Table View Methods

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];
    
    if (debugger)
    {
        if ([tableView.identifier isEqualToString:cDISASSEMBLY_TABLE_VIEW])
        {
            return debugger->numberOfMnemonics();
        }
        
        else if ([tableView.identifier isEqualToString:cMEMORY_TABLE_VIEW])
        {
            return (0xffff / self.byteWidth) + 1;
        }
        
        else if ([tableView.identifier isEqualToString:cSTACK_TABLE_VIEW])
        {
            return debugger->numberOfStackEntries();
        }

        else if ([tableView.identifier isEqualToString:cBREAKPOINTS_TABLE_VIEW])
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
        if ([tableView.identifier isEqualToString:cDISASSEMBLY_TABLE_VIEW])
        {
            return [self disassemblyTable:tableView viewForTableColumn:tableColumn row:row];
        }
        
        if ([tableView.identifier isEqualToString:cMEMORY_TABLE_VIEW])
        {
            return [self memoryTable:tableView viewForTableColumn:tableColumn row:row];
        }
        
        if ([tableView.identifier isEqualToString:cSTACK_TABLE_VIEW])
        {
            return [self stackTable:tableView viewForTableColumn:tableColumn row:row];
        }
        
        if ([tableView.identifier isEqualToString:cBREAKPOINTS_TABLE_VIEW])
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
        Debug::DisassembledOpcode dop = debugger->disassembly(static_cast<unsigned int>(row));
        
        if ([tableColumn.identifier isEqualToString:cDISASSEMBLY_ADDRESS_COL])
        {
            if (self.hexFormat)
            {
                view.textField.stringValue = [NSString stringWithFormat:@"$%04X", dop.address];
            }
            else
            {
                view.textField.stringValue = [NSString stringWithFormat:@"%05i", dop.address];
            }
        }
        else if ([tableColumn.identifier isEqualToString:cDISASSEMBLY_BYTES_COL])
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
        else if ([tableColumn.identifier isEqualToString:cDISASSEMBLY_MNEMONIC_COL])
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
        Debug::Breakpoint bp = debugger->breakpoint(static_cast<unsigned int>(row));
        if (bp.type == 0xff) { return nil; }
        if ([tableColumn.identifier isEqualToString:cBREAKPOINT_ADDRESS_COL])
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
        else if ([tableColumn.identifier isEqualToString:cBREAKPOINT_CONDITION_COL])
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
        if ([tableColumn.identifier isEqualToString:cMEMORY_ADDRESS_COL])
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
        else if ([tableColumn.identifier isEqualToString:cMEMORY_BYTES_COL])
        {
            NSMutableAttributedString *content = [NSMutableAttributedString new];
            for (unsigned int i = 0; i < self.byteWidth; i++)
            {
                unsigned int address = ((int)row * self.byteWidth) + i;
                
                NSMutableAttributedString *attrString;
                if (address <= 0xffff)
                {
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
                        [attrString addAttribute:NSBackgroundColorAttributeName value:cHIGHLIGHT_COLOR range:NSMakeRange(0, highlightLength)];
                    }
                    else
                    {
                        NSColor *color = [NSColor clearColor];
                        switch (debugger->breakpointAtAddress(address)) {
                            case ZXSpectrum::eDebugReadOp:
                            case ZXSpectrum::eDebugWriteOp:
                                color = cRDWR_BREAKPOINT_COLOR;
                                break;
                            case ZXSpectrum::eDebugExecuteOp:
                                color = cEXEC_BREAKPOINT_COLOR;
                        }
                        [attrString addAttribute:NSBackgroundColorAttributeName value:color range:NSMakeRange(0, highlightLength)];
                    }
                    [content appendAttributedString:attrString];
                }
            }
            
            view.textField.attributedStringValue = content;
        }
        else if ([tableColumn.identifier isEqualToString:cMEMORY_ASCII_COL])
        {
            NSMutableAttributedString *attrString = [NSMutableAttributedString new];
            NSColor *color;
            for (int i = 0; i < self.byteWidth; i++)
            {
                color = [NSColor clearColor];
                
                int address = (unsigned short)((row * self.byteWidth)) + i;
                if (address <= 0xffff)
                {
                    if (self.memoryTableSearchAddress == address)
                    {
                        color = cHIGHLIGHT_COLOR;
                    }
                
                    unsigned char c = debugger->machine->z80Core.Z80CoreDebugMemRead(address, NULL);
                    if ((c >= 0 && c < 32) || c > 126)
                    {
                        [attrString appendAttributedString:[[NSAttributedString alloc] initWithString:@"・ "]];
                        [attrString addAttribute:NSBackgroundColorAttributeName value:color range:NSMakeRange(i * 2, 1)];
                    }
                    else
                    {
                        [attrString appendAttributedString:[[NSAttributedString alloc] initWithString:[NSString stringWithFormat:@"%c ", c]]];
                        [attrString addAttribute:NSBackgroundColorAttributeName value:color range:NSMakeRange(i * 2, 1)];
                    }
                }
                else
                {
                    [attrString appendAttributedString:[[NSAttributedString alloc] initWithString:@" "]];
                }
            }
                     
            view.textField.attributedStringValue = attrString;
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
        if ([tableColumn.identifier isEqualToString:cSTACK_ADDRESS_COL])
        {
            if (self.hexFormat)
            {
                view.textField.stringValue = [NSString stringWithFormat:@"$%04X", debugger->stackAddress(static_cast<unsigned int>(row)).address];
            }
            else
            {
                view.textField.stringValue = [NSString stringWithFormat:@"%05i", debugger->stackAddress(static_cast<unsigned int>(row)).address];
            }
        }
        else if ([tableColumn.identifier isEqualToString:cSTACK_VALUE_COL])
        {
            if (self.hexFormat)
            {
                view.textField.stringValue = [NSString stringWithFormat:@"$%04X", debugger->stackAddress(static_cast<unsigned int>(row)).value];
            }
            else
            {
                view.textField.stringValue = [NSString stringWithFormat:@"%05i", debugger->stackAddress(static_cast<unsigned int>(row)).value];
            }
        }
    }
    
    return view;
}

- (void)tableView:(NSTableView *)tableView didAddRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

    if ([tableView.identifier isEqualToString:cDISASSEMBLY_TABLE_VIEW])
    {
        NSColor *rowColor = [NSColor clearColor];
        
        Debug::DisassembledOpcode dop = debugger->disassembly(static_cast<unsigned int>(row));

        for (int i = 0; i < debugger->numberOfBreakpoints(); i++)
        {
            if (debugger->breakpoint(i).address == dop.address)
            {
                if (debugger->breakpoint(i).type & ZXSpectrum::eDebugExecuteOp)
                {
                    rowColor = cEXEC_BREAKPOINT_COLOR;
                }
                else if (debugger->breakpoint(i).type & ZXSpectrum::eDebugReadOp || debugger->breakpoint(i).type & ZXSpectrum::eDebugWriteOp)
                {
                    rowColor = cRDWR_BREAKPOINT_COLOR;
                }
                break;
            }
        }

        if (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_PC) == dop.address)
        {
            rowColor = cHIGHLIGHT_COLOR;
        }

        rowView.backgroundColor = rowColor;
    }
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
    
}

- (IBAction)disassembleTableDoubleClick:(id)sender {
    
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

    uint16_t address = debugger->disassembly(static_cast<unsigned int>(self.disassemblyTableview.clickedRow)).address;
    
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
    float byteColWidthPercentage = 0.53;
    if (!self.hexFormat)
    {
        byteColWidthPercentage = 0.50;
    }
    NSInteger width = self.memoryTableView.enclosingScrollView.frame.size.width - (self.memoryTableView.tableColumns[0].width + 9);
    NSInteger col1Width = width * byteColWidthPercentage;
    NSInteger col2Width = width - col1Width;
    [self.memoryTableView.tableColumns[1] setWidth:col1Width];
    [self.memoryTableView.tableColumns[2] setWidth:col2Width];
    if (self.hexFormat)
    {
        self.byteWidth = (int)col1Width / 21.66;
    }
    else
    {
        self.byteWidth = (int)col1Width / 31;
    }
    
    [self.memoryTableView reloadData];
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
    [self updateButtonStates];
}

- (IBAction)pauseMachine:(id)sender
{
    [self.emulationViewController pauseMachine];
    [self updateViewDetails];
    [self updateDisassemblyTable];
    [self updateButtonStates];
}

- (void)updateButtonStates
{
    self.buttonStep.enabled = NO;
    if (self.emulationViewController.isEmulatorPaused)
    {
        self.buttonStep.enabled = YES;
    }
    
    self.buttonBreak.enabled = NO;
    if (!self.emulationViewController.isEmulatorPaused)
    {
        self.buttonBreak.enabled = YES;
    }
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
    else if ([command isEqualToString:cTOKEN_CONTINUE])
    {
        [self tokenResume:commandList];
    }
    else if ([command isEqualToString:cTOKEN_BREAK])
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
    machine->emuPaused = false;
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
            debugger->disassemble(self.disassembleAddress, 0xffff - self.disassembleAddress, self.hexFormat);
        }
        else if ( [[commandList[1] uppercaseString]isEqualToString:@"PC"] )
        {
            self.disassembleAddress = machine->z80Core.GetRegister(CZ80Core::eREG_PC);
            debugger->disassemble(self.disassembleAddress, 0xffff - self.disassembleAddress, self.hexFormat);
        }
        else if ( [[commandList[1] uppercaseString]isEqualToString:@"SP"] )
        {
            if (self.stackArray.count > 0)
            {
                self.disassembleAddress = [self.stackArray[0] unsignedShortValue];
                debugger->disassemble(self.disassembleAddress, 0xffff - self.disassembleAddress, self.hexFormat);
            }
        }
        [self reloadDisassemblyData];
    }
    else
    {
        [self displayTokenError:@"A address must be provided for where disassembly will start!"];
    }
}

- (void)tokenBreakpoint:(NSArray *)commandList machine:(ZXSpectrum *)machine
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];
    
    NSScanner *scanner = [NSScanner scannerWithString:commandList[2]];
    unsigned int value;
    if ( ![scanner scanHexInt:&value] || value > 0xffff)
    {
        [self displayTokenError:@"The address for the breakpoint is invalid!"];
        return;
    }
    
    if ( [[commandList[1] uppercaseString] isEqualToString:cTOKEN_BREAKPOINT_ADD_ACTION_EXECUTE] )
    {
        debugger->addBreakpoint(value, ZXSpectrum::eDebugExecuteOp);
    }
    else if ( [[commandList[1] uppercaseString] isEqualToString:cTOKEN_BREAKPOINT_ADD_ACTION_READ] )
    {
        debugger->addBreakpoint(value, ZXSpectrum::eDebugReadOp);
    }
    else if ( [[commandList[1] uppercaseString] isEqualToString:cTOKEN_BREAKPOINT_ADD_ACTION_WRITE] )
    {
        debugger->addBreakpoint(value, ZXSpectrum::eDebugWriteOp);
    }
    else if ( [[commandList[1] uppercaseString] isEqualToString:cTOKEN_BREAKPOINT_REMOVE_ACTION_READ] )
    {
        debugger->removeBreakpoint(value, ZXSpectrum::eDebugReadOp);
    }
    else if ( [[commandList[1] uppercaseString] isEqualToString:cTOKEN_BREAKPOINT_REMOVE_ACTION_WRITE] )
    {
        debugger->removeBreakpoint(value, ZXSpectrum::eDebugWriteOp);
    }
    else if ( [[commandList[1] uppercaseString] isEqualToString:cTOKEN_BREAKPOINT_REMOVE_ACTION_EXECUTE] )
    {
        debugger->removeBreakpoint(value, ZXSpectrum::eDebugExecuteOp);
    }
    [self updateViewDetails];
}

- (void)tokenSetRegister:(NSArray *)commandList machine:(ZXSpectrum *)machine
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

    NSString *reg = [commandList[1] uppercaseString];
    NSScanner *scanner = [NSScanner scannerWithString:commandList[2]];
    unsigned int value;
    if (![scanner scanHexInt:&value])
    {
        [self displayTokenError:@"Invalid register value provided!"];
        return;
    }
    
    if (!debugger->setRegister([reg cStringUsingEncoding:NSUTF8StringEncoding], value))
    {
        [self displayTokenError:@"Failed to set register value!"];
    }
    
    [self updateViewDetails];
}

- (void)tokenFillMemory:(NSArray *)commandList machine:(ZXSpectrum *)machine
{
    Debug *debugger = (Debug *)[self.emulationViewController getDebugger];

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

        debugger->fillMemory(fromAddress, toAddress, value);
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

- (void)updateViewDetails
{
    [self updateCPUDetails];
    [self updateMemoryTable];
    [self updateStackTable];
    [self updateDisassemblyTable];
    [self updateBreakpointTable];
    [self.emulationViewController updateDisplay];
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
                Debug::DisassembledOpcode dop = debugger->disassembly(static_cast<unsigned int>(i));

                if (dop.address == debugger->machine->z80Core.GetRegister(CZ80Core::eREG_PC))
                {
                    debugger->disassemble(self.disassembleAddress, 0xffff - self.disassembleAddress, self.hexFormat);
                    [self reloadDisassemblyData];
                    [self.disassemblyTableview deselectAll:NULL];
                    [self.disassemblyTableview scrollRowToVisible:i];
                    return;
                }
            }
        }
        self.disassembleAddress = debugger->machine->z80Core.GetRegister(CZ80Core::eREG_PC);
        debugger->disassemble(self.disassembleAddress, 0xffff - self.disassembleAddress, self.hexFormat);
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
        
        self.fs = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_S) ? @"1" : @"-";
        self.fz = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_Z) ? @"1" : @"-";
        self.f5 = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_5) ? @"1" : @"-";
        self.fh = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_H) ? @"1" : @"-";
        self.f3 = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_3) ? @"1" : @"-";
        self.fpv = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_P) ? @"1" : @"-";
        self.fn = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_N) ? @"1" : @"-";
        self.fc = (debugger->machine->z80Core.GetRegister(CZ80Core::eREG_F) & debugger->machine->z80Core.FLAG_C) ? @"1" : @"-";
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
        }
        else
        {
            NSIndexSet *visibleCols = [self.memoryTableView columnIndexesInRect:visibleRect];
            [self.memoryTableView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndexesInRange:visibleRows] columnIndexes:visibleCols];
        }
    });
}

- (void)updateBreakpointTable
{
    dispatch_async(dispatch_get_main_queue(), ^{
        NSRect visibleRect = self.breakpointTableView.visibleRect;
        NSRange visibleRows = [self.breakpointTableView rowsInRect:visibleRect];
        if (visibleRows.length == 0)
        {
            [self.breakpointTableView reloadData];
        }
        else
        {
            NSIndexSet *visibleCols = [self.breakpointTableView columnIndexesInRect:visibleRect];
            [self.breakpointTableView reloadDataForRowIndexes:[NSIndexSet indexSetWithIndexesInRange:visibleRows] columnIndexes:visibleCols];
        }
    });
}

#pragma mark - Accessors

- (void)setHexFormat:(BOOL)hexFormat1
{
    _hexFormat = hexFormat1;
    [self updateMemoryTableSize];
}

- (BOOL)hexFormat
{
    return _hexFormat;
}

@end

