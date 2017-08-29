//
//  ViewController.m
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "EmulationViewController.h"
#import "EmulationScene.h"
#import "ZXSpectrum.hpp"
#import "ZXSpectrum48.hpp"

#pragma mark - Constants

NSString *const cSNA_EXTENSION = @"SNA";
NSString *const cZ80_EXTENSION = @"Z80";

static NSString  *const cSESSION_FILE_NAME = @"session.z80";

#pragma mark - Private Interface

@interface EmulationViewController()
{
    EmulationScene      *_scene;
    ZXSpectrum          *_machine;
    dispatch_queue_t    _emulationQueue;
    dispatch_source_t   _emulationTimer;
}
@end

#pragma mark - Implementation

@implementation EmulationViewController

- (void)dealloc
{
    delete _machine;
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    [self initMachineWithRomAtPath:[[NSBundle mainBundle] pathForResource:@"48" ofType:@"ROM"]];
    
    _scene = (EmulationScene *)[SKScene nodeWithFileNamed:@"EmulationScene"];
    
    // Remember to do this before presenting the scene or it goes all wierd !!!
    _scene.scaleMode = SKSceneScaleModeFill;
    
    _scene.nextResponder = self;

    [self.skView presentScene:_scene];
    
    [self setupTimersAndQueues];
    [self startEmulationTimer];
    [self restoreSession];
}

- (void)initMachineWithRomAtPath:(NSString *)romPath
{
    _machine = new ZXSpectrum48();
    _machine->initialise((char *)[romPath cStringUsingEncoding:NSUTF8StringEncoding]);
}

- (void)setupTimersAndQueues
{
    _emulationQueue = dispatch_queue_create("EmulationQueue", nil);
    _emulationTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, _emulationQueue);
    dispatch_source_set_timer(_emulationTimer, DISPATCH_TIME_NOW, 0.02 * NSEC_PER_SEC, 0);
    
    // Basic emulation timer. To be replaced with sound based timing
    dispatch_source_set_event_handler(_emulationTimer, ^{
        
        _machine->runFrame();

        dispatch_async(dispatch_get_main_queue(), ^{
            
            [_scene.emulationScreenTexture modifyPixelDataWithBlock:^(void *pixelData, size_t lengthInBytes) {

                memcpy(pixelData, _machine->displayBuffer, lengthInBytes);

            }];
        });
        
    });
}

#pragma mark - Keyboard

- (void)keyDown:(NSEvent *)event
{
    if (!event.isARepeat)
    {
        _machine->keyDown(event.keyCode);
    }
}

- (void)keyUp:(NSEvent *)event
{
    if (!event.isARepeat)
    {
        _machine->keyUp(event.keyCode);
    }
}

- (void)flagsChanged:(NSEvent *)event
{
    if (!(event.modifierFlags & NSEventModifierFlagCommand))
    {
        _machine->keyFlagsChanged(event.modifierFlags, event.keyCode);
    }
}

#pragma mark - Timer

- (void)startEmulationTimer
{
    dispatch_resume(_emulationTimer);
}

- (void)suspendEmulationTimer
{
    dispatch_suspend(_emulationTimer);
}

#pragma mark - File Loading

- (void)loadFileWithURL:(NSURL *)url
{
    if ([[url.pathExtension uppercaseString] isEqualToString:@"Z80"])
    {
        if (_machine->loadZ80SnapshotWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]))
        {
            [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:url];
        }
    }
    else if ([[url.pathExtension uppercaseString] isEqualToString:@"SNA"])
    {
        if (_machine->loadSnapshotWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]))
        {
            [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:url];
        }
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
        ZXSpectrum::snap sessionSnapshot = _machine->createZ80Snapshot();
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
            [self loadFileWithURL:supportDirUrl];
        }
        else
        {
            NSLog(@"No session to restore");
        }
    }
}

#pragma mark - Menu Actions

- (IBAction)openFile:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel new];
    openPanel.canChooseDirectories = NO;
    openPanel.allowsMultipleSelection = NO;
    openPanel.allowedFileTypes = @[cSNA_EXTENSION, cZ80_EXTENSION];
    
    [openPanel beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result) {
        if (result == NSModalResponseOK)
        {
            [self loadFileWithURL:openPanel.URLs[0]];
        }
    }];
}

- (IBAction)resetMachine:(id)sender
{
    _machine->reset();
}


@end
