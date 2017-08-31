//
//  AppDelegate.m
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "AppDelegate.h"
#import "EmulationViewController.h"

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}

- (BOOL)application:(NSApplication *)sender openFile:(NSString *)filename
{
    // Needed to support opening files from the 'Open Recent' menu option
    NSWindow *window = [[NSApplication sharedApplication] mainWindow];
    EmulationViewController *emulationViewController = (EmulationViewController *)[window contentViewController];
    [emulationViewController loadFileWithURL:[NSURL fileURLWithPath:filename] addToRecent:NO];
    return YES;
}

@end
