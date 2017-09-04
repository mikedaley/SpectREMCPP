//
//  ViewController.m
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "EmulationViewController.h"
#import "AudioCore.h"
#import "EmulationScene.h"
#import "ZXSpectrum.hpp"
#import "ZXSpectrum48.hpp"

#pragma mark - Constants

NSString *const cSNA_EXTENSION = @"SNA";
NSString *const cZ80_EXTENSION = @"Z80";

int const cAUDIO_SAMPLE_RATE = 192000;
float const cFRAMES_PER_SECOND = 50;

static NSString  *const cSESSION_FILE_NAME = @"session.z80";

#pragma mark - Private Interface

@interface EmulationViewController()
{
    EmulationScene      *scene;
    ZXSpectrum          *machine;
    AudioCore           *audioCore;
    dispatch_source_t   displayTimer;
}
@end

#pragma mark - Implementation

@implementation EmulationViewController

- (void)dealloc
{
    delete machine;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    [self initMachineWithRomAtPath:[[NSBundle mainBundle] pathForResource:@"48" ofType:@"ROM"]];
    
    scene = (EmulationScene *)[SKScene nodeWithFileNamed:@"EmulationScene"];
    
    // Remember to do this before presenting the scene or it goes all wierd !!!
    scene.scaleMode = SKSceneScaleModeFill;
    
    scene.nextResponder = self;

    [self.skView presentScene:scene];
    
    [self setupTimersAndQueues];
    
    audioCore = [[AudioCore alloc] initWithSampleRate:cAUDIO_SAMPLE_RATE framesPerSecond:cFRAMES_PER_SECOND machine:machine];
    
    [audioCore start];
    [self startEmulationTimer];
    
    [self restoreSession];
}

- (void)initMachineWithRomAtPath:(NSString *)romPath
{
    machine = new ZXSpectrum48();
    machine->initialise((char *)[romPath cStringUsingEncoding:NSUTF8StringEncoding]);
}

- (void)setupTimersAndQueues
{
    displayTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
    dispatch_source_set_timer(displayTimer, DISPATCH_TIME_NOW, (1.0 / cFRAMES_PER_SECOND) * NSEC_PER_SEC, 0);

    dispatch_source_set_event_handler(displayTimer, ^{
        [scene.emulationScreenTexture modifyPixelDataWithBlock:^(void *pixelData, size_t lengthInBytes) {
            memcpy(pixelData, machine->displayBuffer, lengthInBytes);
        }];
    });
}

#pragma mark - Keyboard

- (void)keyDown:(NSEvent *)event
{
    if (!event.isARepeat && !(event.modifierFlags & NSEventModifierFlagCommand))
    {
        machine->keyboardKeyDown(event.keyCode);
    }
}

- (void)keyUp:(NSEvent *)event
{
    if (!event.isARepeat && !(event.modifierFlags & NSEventModifierFlagCommand))
    {
        machine->keyboardKeyUp(event.keyCode);
    }
}

- (void)flagsChanged:(NSEvent *)event
{
    if (!(event.modifierFlags & NSEventModifierFlagCommand))
    {
        machine->keyboardFlagsChanged(event.modifierFlags, event.keyCode);
    }
}

#pragma mark - Timer

- (void)startEmulationTimer
{
    dispatch_resume(displayTimer);
}

- (void)suspendEmulationTimer
{
    dispatch_suspend(displayTimer);
}

#pragma mark - File Loading

- (void)loadFileWithURL:(NSURL *)url addToRecent:(BOOL)addToRecent
{
    BOOL error = NO;
    
    if ([[url.pathExtension uppercaseString] isEqualToString:@"Z80"])
    {
        if (machine->snapshotZ80LoadWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]))
        {
            if (addToRecent)
            {
                [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:url];
            }
        }
        else
        {
            error = YES;
        }
    }
    else if ([[url.pathExtension uppercaseString] isEqualToString:@"SNA"])
    {
        if (machine->snapshotSNALoadWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]))
        {
            if (addToRecent)
            {
                [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:url];
            }
        }
        else
        {
            error = YES;
        }
    }
    
    if (error)
    {
        NSWindow *window = [[NSApplication sharedApplication] mainWindow];
        NSAlert *alert = [NSAlert new];
        alert.informativeText = [NSString stringWithFormat:@"An error occurred trying to open %@", url.path];
        [alert addButtonWithTitle:@"OK"];
        [alert beginSheetModalForWindow:window completionHandler:^(NSModalResponse returnCode) {
            // No need to do
        }];
    }
}

#pragma mark - View Methods

- (void)viewWillDisappear
{
    NSString *bundleID = [[NSBundle mainBundle] bundleIdentifier];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    NSArray *supportDir = [fileManager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
    
    if (supportDir.count > 0)
    {
        NSURL *supportDirUrl = [[supportDir objectAtIndex:0] URLByAppendingPathComponent:bundleID];
        
        NSError *error = nil;
        if (![fileManager createDirectoryAtURL:supportDirUrl withIntermediateDirectories:YES attributes:nil error:&error])
        {
            NSLog(@"ERROR: creating support directory.");
            return;
        }
        
        supportDirUrl = [supportDirUrl URLByAppendingPathComponent:cSESSION_FILE_NAME];
        ZXSpectrum::snap sessionSnapshot = machine->snapshotCreateZ80();
        NSData *data = [NSData dataWithBytes:sessionSnapshot.data length:sessionSnapshot.length];
        [data writeToURL:supportDirUrl atomically:YES];
    }
}

#pragma mark - Restore Session

- (void)restoreSession
{
    NSString *bundleID = [[NSBundle mainBundle] bundleIdentifier];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    NSArray *supportDir = [fileManager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
    
    if (supportDir.count > 0)
    {
        NSURL *supportDirUrl = [[supportDir objectAtIndex:0] URLByAppendingPathComponent:bundleID];
        
        // Load the last session file it if exists
        supportDirUrl = [supportDirUrl URLByAppendingPathComponent:cSESSION_FILE_NAME];
        if ([fileManager fileExistsAtPath:supportDirUrl.path])
        {
            NSLog(@"Restoring session");
            [self loadFileWithURL:supportDirUrl addToRecent:NO];
        }
        else
        {
            NSLog(@"No session to restore");
        }
    }
}

#pragma mark - File Menu Items

- (IBAction)openFile:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel new];
    openPanel.canChooseDirectories = NO;
    openPanel.allowsMultipleSelection = NO;
    openPanel.allowedFileTypes = @[cSNA_EXTENSION, cZ80_EXTENSION];
    
    [openPanel beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result) {
        if (result == NSModalResponseOK)
        {
            [self loadFileWithURL:openPanel.URLs[0] addToRecent:YES];
        }
    }];
}

#pragma mark - View Menu Items

- (IBAction)setWindowSize:(id)sender
{
    if (([self.view.window styleMask] & NSWindowStyleMaskFullScreen) != NSWindowStyleMaskFullScreen)
    {
        NSMenuItem *menuItem = (NSMenuItem*)sender;
        float width = 320 * menuItem.tag;
        float height = 256 * menuItem.tag;
        [self.view.window.animator setContentSize:(NSSize){width, height}];
    }
}

#pragma mark - Machine Menu Items

- (IBAction)resetMachine:(id)sender
{
    machine->resetMachine();
}


@end





