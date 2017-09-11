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
    [self.tableView reloadData];
}

#pragma mark - Table View Delegate

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
    return [self.emulationViewController numberOfblocks];
}

-(NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    TapeCellView *view = [tableView makeViewWithIdentifier:@"BlockTypeCellID" owner:nil];
    if (view)
    {
        view.textField.stringValue = [ self.emulationViewController blockNameForTapeBlockIndex:row ];
        if (row == [self.emulationViewController selectedTapeBlock])
        {
            if ([self.emulationViewController isplaying])
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
    return view;
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification
{
    [self.emulationViewController setSelectedTapeBlock:self.tableView.selectedRow];
    [self.tableView reloadData];
}

- (CGFloat)tableView:(NSTableView *)tableView heightOfRow:(NSInteger)row
{
    return 30;
}


@end
