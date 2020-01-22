//
//  TapeBrowserViewController.m
//  SpectREM
//
//  Created by Michael Daley on 10/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "TapeBrowserViewController.h"
#import "TapeCellView.h"
#import "EmulationController.hpp"
#import "SharedConstants.h"

@interface TapeBrowserViewController ()

@property (weak) IBOutlet NSTableView *tableView;

@end

@implementation TapeBrowserViewController

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self)
    {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(tapeChangedNotification:) name:@"TAPE_CHANGED_NOTIFICATION" object:NULL];
    }
    return self;
}

- (BOOL)acceptsFirstResponder
{
    return NO;
}

- (void)tapeChangedNotification:(NSNotification *)notification
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.tableView reloadData];
    });
}

#pragma mark - Table View Delegate

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    if (self.emulationController)
    {
        return self.emulationController->getNumberOfTapeBlocks();
    }
    return 0;
}

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    TapeCellView *view = nil;
    
    if ([tableColumn.identifier isEqualToString:@"StatusColID"])
    {
        view = [tableView makeViewWithIdentifier:@"StatusCellID" owner:nil];
        if (row == self.emulationController->getCurrentTapeBlock())
        {
            if (self.emulationController->isTapePlaying())
            {
                view.imageView.image = [NSImage imageNamed:NSImageNameStatusAvailable];
            }
            else
            {
                view.imageView.image = [NSImage imageNamed:NSImageNameStatusUnavailable];
            }
        }
        else
        {
            view.imageView.image = nil;
        }
    }
    else if ([tableColumn.identifier isEqualToString:@"BlockTypeColID"])
    {
        view = [tableView makeViewWithIdentifier:@"BlockTypeCellID" owner:nil];
        view.textField.stringValue = [NSString stringWithCString:self.emulationController->tapeBlockTypeForIndex(static_cast<int>(row)).c_str() encoding:NSUTF8StringEncoding] ;
    }
    else if ([tableColumn.identifier isEqualToString:@"FilenameColID"])
    {
        view = [tableView makeViewWithIdentifier:@"FilenameCellID" owner:nil];
        NSString *blockType = [NSString stringWithCString:self.emulationController->tapeBlockTypeForIndex(static_cast<int>(row)).c_str() encoding:NSUTF8StringEncoding] ;
        if ([blockType isEqualToString:@"Program Header"] || [blockType isEqualToString:@"Byte Header"])
        {
            view.textField.stringValue = [NSString stringWithCString:self.emulationController->tapeFilenameForIndex(static_cast<int>(row)).c_str() encoding:NSUTF8StringEncoding] ;
        }
        else
        {
            view.textField.stringValue = @"";
        }
    }
    else if ([tableColumn.identifier isEqualToString:@"AutostartColID"])
    {
        view = [tableView makeViewWithIdentifier:@"AutostartCellID" owner:nil];
        if (self.emulationController->tapeAutostartLineForIndex(static_cast<int>(row)) != 0)
        {
            view.textField.stringValue = [NSString stringWithFormat:@"%i", self.emulationController->tapeAutostartLineForIndex(static_cast<int>(row))];
        }
        else
        {
            view.textField.stringValue = @"";
        }
    }
    else if ([tableColumn.identifier isEqualToString:@"AddressColID"])
    {
        view = [tableView makeViewWithIdentifier:@"AddressCellID" owner:nil];
        if (self.emulationController->tapeBlockStartAddressForIndex(static_cast<int>(row)) != 0)
        {
            view.textField.stringValue = [NSString stringWithFormat:@"%i", self.emulationController->tapeBlockStartAddressForIndex(static_cast<int>(row))];
        }
        else
        {
            view.textField.stringValue = @"";
        }
    }
    else if ([tableColumn.identifier isEqualToString:@"LengthColID"])
    {
        view = [tableView makeViewWithIdentifier:@"LengthCellID" owner:nil];
        view.textField.stringValue = [NSString stringWithFormat:@"%i", self.emulationController->tapeBlockLengthForIndex(static_cast<int>(row))];
    }
    return view;
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
    self.emulationController->setCurrentTapeBlockIndex(static_cast<int>(self.tableView.selectedRow));
    [self.tableView reloadData];
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    return 30;
}


#pragma mark - Button Methods

- (IBAction)playTape:(id)sender
{
    self.emulationController->playTape();
}

- (IBAction)stopTape:(id)sender
{
    self.emulationController->stopTape();
}

- (IBAction)rewindTape:(id)sender
{
    self.emulationController->rewindTape();
}

- (IBAction)ejectTape:(id)sender
{
    self.emulationController->ejectTape();
}

- (IBAction)saveTape:(id)sender
{
    NSSavePanel *savePanel = [NSSavePanel new];
    savePanel.allowedFileTypes = @[ cTAP_EXTENSION ];
    [savePanel beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result) {
        if (result == NSModalResponseOK)
        {
            std::vector<unsigned char> tapeData = self.emulationController->getTapeData();
            NSMutableData *saveData = [NSMutableData new];
            [saveData appendBytes:tapeData.data() length:tapeData.size()];
            [saveData writeToURL:savePanel.URL atomically:YES];
        }
    }];
}


@end
