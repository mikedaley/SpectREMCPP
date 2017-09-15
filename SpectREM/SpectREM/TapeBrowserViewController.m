//
//  TapeBrowserViewController.m
//  SpectREM
//
//  Created by Michael Daley on 10/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "TapeBrowserViewController.h"
#import "EmulationViewController.h"
#import "TapeCellView.h"

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

- (void)tapeChangedNotification:(NSNotification *)notification
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.tableView reloadData];
    });
}

#pragma mark - Table View Delegate

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [self.emulationViewController tapeNumberOfblocks];
}

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    TapeCellView *view = nil;
    
    if ([tableColumn.identifier isEqualToString:@"StatusColID"])
    {
        view = [tableView makeViewWithIdentifier:@"StatusCellID" owner:nil];
        if (row == [self.emulationViewController tapeCurrentBlock])
        {
            if ([self.emulationViewController tapeIsplaying])
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
        view.textField.stringValue = [self.emulationViewController tapeBlockTypeForIndex:row];
    }
    else if ([tableColumn.identifier isEqualToString:@"FilenameColID"])
    {
        view = [tableView makeViewWithIdentifier:@"FilenameCellID" owner:nil];
        NSString *blockType = [self.emulationViewController tapeBlockTypeForIndex:row];
        if ([blockType isEqualToString:@"Program Header"] || [blockType isEqualToString:@"Byte Header"])
        {
            view.textField.stringValue = [self.emulationViewController tapeFilenameForIndex:row];
        }
        else
        {
            view.textField.stringValue = @"";
        }
    }
    else if ([tableColumn.identifier isEqualToString:@"AutostartColID"])
    {
        view = [tableView makeViewWithIdentifier:@"AutostartCellID" owner:nil];
        if ([self.emulationViewController tapeAutostartLineForIndex:row] != 0)
        {
            view.textField.stringValue = [NSString stringWithFormat:@"%i", [self.emulationViewController tapeAutostartLineForIndex:row]];
        }
        else
        {
            view.textField.stringValue = @"";
        }
    }
    else if ([tableColumn.identifier isEqualToString:@"AddressColID"])
    {
        view = [tableView makeViewWithIdentifier:@"AddressCellID" owner:nil];
        if ([self.emulationViewController tapeBlockStartAddressForIndex:row] != 0)
        {
            view.textField.stringValue = [NSString stringWithFormat:@"%i", [self.emulationViewController tapeBlockStartAddressForIndex:row]];
        }
        else
        {
            view.textField.stringValue = @"";
        }
    }
    else if ([tableColumn.identifier isEqualToString:@"LengthColID"])
    {
        view = [tableView makeViewWithIdentifier:@"LengthCellID" owner:nil];
        view.textField.stringValue = [NSString stringWithFormat:@"%i", [self.emulationViewController tapeBlockLengthForIndex:row]];
    }
    return view;
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
    [self.emulationViewController tapeSetCurrentBlock:self.tableView.selectedRow];
    [self.tableView reloadData];
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    return 30;
}


#pragma mark - Button Methods

- (IBAction)play:(id)sender
{
    [self.emulationViewController startPlayingTape:nil];
}

- (IBAction)stop:(id)sender
{
    [self.emulationViewController stopPlayingTape:nil];
}

- (IBAction)rewind:(id)sender
{
    [self.emulationViewController rewindTape:nil];
}

- (IBAction)eject:(id)sender
{
    [self.emulationViewController ejectTape:nil];
}

- (IBAction)save:(id)sender
{

}


@end
