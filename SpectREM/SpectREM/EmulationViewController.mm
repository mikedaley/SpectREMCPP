//
//  ViewController.m
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import "EmulationViewController.h"
#import "ZXSpectrum.hpp"
#import "ZXSpectrum48.hpp"
#import "ZXSpectrum128.hpp"
#import "ZXSpectrum128_2.hpp"
#import "Tape.hpp"
#import "AudioQueue.hpp"

#import "AudioCore.h"
#import "EmulationScene.h"

#import "ConfigurationViewController.h"
#import "ExportAccessoryViewController.h"
#import "TapeBrowserViewController.h"

#pragma mark - Constants

NSString *const cSNA_EXTENSION = @"SNA";
NSString *const cZ80_EXTENSION = @"Z80";
NSString *const cTAP_EXTENSION = @"TAP";

int const cAUDIO_SAMPLE_RATE = 192000;
float const cFRAMES_PER_SECOND = 50;

static NSString  *const cSESSION_FILE_NAME = @"session.z80";

static const int cSCREEN_4_3 = 0;
static const int cSCREEN_FILL = 1;

#pragma mark - Private Interface

@interface EmulationViewController()
{
@public
    ZXSpectrum                      *machine;
    Tape                            *tape;
    AudioCore                       *audioCore;
    dispatch_source_t               displayTimer;
    NSString                        *mainBundlePath;
    bool                            configViewVisible;
    
    AudioQueue                      *audioQueue;
    short                           audioBuffer;
    
    NSStoryboard                    *storyBoard;
    ConfigurationViewController     *configViewController;
    ExportAccessoryViewController   *saveAccessoryController;
    NSWindowController              *tapeBrowserWindowController;
    TapeBrowserViewController       *tapeBrowserViewController;
    NSLock *lock;

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
    
    mainBundlePath = [[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/Contents/Resources/"];
    
    storyBoard = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
    
    _scene = (EmulationScene *)[SKScene nodeWithFileNamed:@"EmulationScene"];
    _scene.nextResponder = self;
    _scene.emulationViewController = self;
    _scene.scaleMode = SKSceneScaleModeFill;
    [self.skView presentScene:_scene];
    
    // The AudioCore uses the sound buffer to identify when a new frame should be drawn for accurate timing. The AudioQueue
    // is used to help measure usage of the audio buffer
    audioQueue = new AudioQueue();
    audioCore = [[AudioCore alloc] initWithSampleRate:cAUDIO_SAMPLE_RATE framesPerSecond:cFRAMES_PER_SECOND callback:self];
    
    //Create a tape instance
    tape = new Tape(tapeStatusCallback);
    
    [Defaults setupDefaults];
    _defaults = [Defaults defaults];
    
    lock = [NSLock new];
    
    [self initMachineWithRomPath:mainBundlePath machineType:(int)_defaults.machineSelectedModel];

    [self setupConfigView];
    [self setupControllers];
    [self setupObservers];
    [self restoreSession];
}

- (void)audioCallback:(int)inNumberFrames buffer:(unsigned short *)buffer
{
    if (machine)
    {
        // Update the queue with the reset buffer
        audioQueue->read(buffer, (inNumberFrames * 2));
        
        // Check if we have used a frames worth of buffer storage and if so then its time to generate another frame.
        if (audioQueue->bufferUsed() < 7680)
        {
            dispatch_sync(dispatch_get_main_queue(), ^{
                [lock lock];
                machine->generateFrame();
                [lock unlock];
            });
            audioQueue->write(machine->audioBuffer, 7680);
        }
    }
}

- (void)viewWillAppear
{
    [self.view.window setTitle:[NSString stringWithFormat:@"SpectREM %@", [NSString stringWithCString:machine->machineInfo.machineName encoding:NSUTF8StringEncoding]]];
}

#pragma mark - Observers

- (void)setupObservers
{
    [self.defaults addObserver:self forKeyPath:MachineSelectedModel options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:MachineTapeInstantLoad options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:MachineUseAYSound options:NSKeyValueObservingOptionNew context:NULL];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context
{
    if ([keyPath isEqualToString:MachineSelectedModel])
    {
        [self initMachineWithRomPath:mainBundlePath machineType:(int)self.defaults.machineSelectedModel];
    }
    else if ([keyPath isEqualToString:MachineTapeInstantLoad])
    {
        machine->emuTapeInstantLoad = [change[NSKeyValueChangeNewKey] boolValue];
    }
    else if ([keyPath isEqualToString:MachineUseAYSound])
    {
        machine->emuUseAYSound = [change[NSKeyValueChangeNewKey] boolValue];
    }
}

#pragma mark - Apply defaults

- (void)applyDefaults
{
    machine->emuTapeInstantLoad = self.defaults.machineTapeInstantLoad;
    machine->emuUseAYSound = self.defaults.machineUseAYSound;
}

#pragma mark - View/Controller Setup

- (void)setupConfigView
{
    configViewController = [storyBoard instantiateControllerWithIdentifier:@"CONFIG_VIEW_CONTROLLER"];
    [self.configEffectsView setFrameOrigin:(CGPoint){-self.configEffectsView.frame.size.width, 0}];
    self.configScrollView.documentView = configViewController.view;
}

- (void)setupControllers
{
    saveAccessoryController = [storyBoard instantiateControllerWithIdentifier:@"SAVE_ACCESSORY_VIEW_CONTROLLER"];
    tapeBrowserWindowController = [storyBoard instantiateControllerWithIdentifier:@"TAPE_BROWSER_WINDOW"];
    tapeBrowserViewController = (TapeBrowserViewController *)tapeBrowserWindowController.contentViewController;
    tapeBrowserViewController.emulationViewController = self;
}

#pragma mark - Init/Switch Machine

- (void)initMachineWithRomPath:(NSString *)romPath machineType:(int)machineType
{
    [self.scene setPaused:YES];
    
    if (audioCore)
    {
        [audioCore stop];
        while (audioCore.isRunning) { };
    }
    
    if (machine) {
        machine->pause();
        delete machine;
    }
    
    if (machineType == eZXSpectrum48)
    {
        machine = new ZXSpectrum48(tape);
    }
    else if (machineType == eZXSpectrum128)
    {
        machine = new ZXSpectrum128(tape);
    }
    else if (machineType == eZXSpectrum128_2)
    {
        machine = new ZXSpectrum128_2(tape);
    }
    else
    {
        NSLog(@"Unknown machine type!");
        return;
    }
    
    machine->initialise((char *)[romPath cStringUsingEncoding:NSUTF8StringEncoding]);
    
    [self applyDefaults];
    
    [audioCore start];
    [self.scene setPaused:NO];
    machine->resume();
    
    [self.view.window setTitle:[NSString stringWithFormat:@"SpectREM %@",
                                [NSString stringWithCString:machine->machineInfo.machineName
                                                   encoding:NSUTF8StringEncoding]]];
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

#pragma mark - File Loading

- (void)loadFileWithURL:(NSURL *)url addToRecent:(BOOL)addToRecent
{
    BOOL success = NO;
    
    NSString *urlPath = [url.pathExtension uppercaseString];
    if (([urlPath isEqualToString:cZ80_EXTENSION] || [urlPath isEqualToString:cSNA_EXTENSION]))
    {
        int snapshotMachineType = machine->snapshotMachineInSnapshotWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]);
        if (machine->machineInfo.machineType != snapshotMachineType)
        {
            self.defaults.machineSelectedModel = snapshotMachineType;
        }
    }
    
    if ([[url.pathExtension uppercaseString] isEqualToString:cZ80_EXTENSION])
    {
        success = machine->snapshotZ80LoadWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]);
    }
    else if ([[url.pathExtension uppercaseString] isEqualToString:cSNA_EXTENSION])
    {
        success = machine->snapshotSNALoadWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]);
    }
    else if ([[url.pathExtension uppercaseString] isEqualToString:cTAP_EXTENSION])
    {
        success = tape->loadWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]);
        [[NSNotificationCenter defaultCenter] postNotificationName:@"TAPE_CHANGED_NOTIFICATION" object:NULL];
    }
    
    if (addToRecent && success)
    {
        [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:url];
    }
    
    if (!success)
    {
        NSWindow *window = [[NSApplication sharedApplication] mainWindow];
        NSAlert *alert = [NSAlert new];
        alert.informativeText = [NSString stringWithFormat:@"An error occurred trying to open %@", url.path];
        [alert addButtonWithTitle:@"OK"];
        [alert beginSheetModalForWindow:window completionHandler:^(NSModalResponse returnCode) {
            // No need to do anything
        }];
    }
}

#pragma mark - View Methods

- (void)viewWillDisappear
{
    if (NSURL *supportDirUrl = [self getSupportDirUrl])
    {
        NSError *error = nil;
        if (![[NSFileManager defaultManager] createDirectoryAtURL:supportDirUrl withIntermediateDirectories:YES attributes:nil error:&error])
        {
            NSLog(@"ERROR: creating support directory.");
            return;
        }
        
        supportDirUrl = [supportDirUrl URLByAppendingPathComponent:cSESSION_FILE_NAME];
        ZXSpectrum::Snap sessionSnapshot = machine->snapshotCreateZ80();
        NSData *data = [NSData dataWithBytes:sessionSnapshot.data length:sessionSnapshot.length];
        [data writeToURL:supportDirUrl atomically:YES];
    }
}

#pragma mark - Restore Session

- (void)restoreSession
{
    if (NSURL *supportDirUrl = [self getSupportDirUrl])
    {
        // Load the last session file it if exists
        supportDirUrl = [supportDirUrl URLByAppendingPathComponent:cSESSION_FILE_NAME];
        if ([[NSFileManager defaultManager] fileExistsAtPath:supportDirUrl.path])
        {
            [self loadFileWithURL:supportDirUrl addToRecent:NO];
        }
    }
}

- (NSURL *)getSupportDirUrl
{
    NSString *bundleID = [[NSBundle mainBundle] bundleIdentifier];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    NSArray *supportDir = [fileManager URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask];
    
    if (supportDir.count > 0)
    {
        return [[supportDir objectAtIndex:0] URLByAppendingPathComponent:bundleID];
    }
    
    return nil;
}

#pragma mark - Tape Browser Methods

- (NSInteger)tapeNumberOfblocks
{
    return tape->numberOfTapeBlocks();
}

- (NSString *)tapeBlockTypeForIndex:(NSInteger)blockIndex
{
    return @(tape->blocks[ blockIndex ]->getBlockName().c_str());
}

- (NSString *)tapeFilenameForIndex:(NSInteger)blockIndex
{
    return @(tape->blocks[ blockIndex ]->getFilename().c_str());
}

- (int)tapeAutostartLineForIndex:(NSInteger)blockIndex
{
    int lineNumber = tape->blocks [blockIndex ]->getAutoStartLine();
    return (lineNumber == 32768) ? 0 : lineNumber;
}

- (unsigned short)tapeBlockStartAddressForIndex:(NSInteger)blockIndex
{
    return tape->blocks[ blockIndex ]->getStartAddress();
}

- (unsigned short)tapeBlockLengthForIndex:(NSInteger)blockIndex
{
    return tape->blocks[ blockIndex ]->getDataLength();
}

- (NSInteger)tapeCurrentBlock
{
    return tape->currentBlockIndex;
}

- (BOOL)tapeIsplaying
{
    return tape->playing;
}

- (void)tapeSetCurrentBlock:(NSInteger)blockIndex
{
    tape->setSelectedBlock( static_cast<int>(blockIndex) );
    tape->rewindBlock();
    tape->stopPlaying();
}

static void tapeStatusCallback(int blockIndex, int bytes)
{
    cout << "Tape Callback: " << blockIndex << endl;
    [[NSNotificationCenter defaultCenter] postNotificationName:@"TAPE_CHANGED_NOTIFICATION" object:NULL];
}

#pragma mark - File Menu Items

- (IBAction)openFile:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel new];
    openPanel.canChooseDirectories = NO;
    openPanel.allowsMultipleSelection = NO;
    openPanel.allowedFileTypes = @[cSNA_EXTENSION, cZ80_EXTENSION, cTAP_EXTENSION];
    
    [openPanel beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result) {
        if (result == NSModalResponseOK)
        {
            [self loadFileWithURL:openPanel.URLs[0] addToRecent:YES];
        }
    }];
}

- (IBAction)exportSnapshot:(id)sender
{
    NSSavePanel *savePanel = [NSSavePanel new];
    
    if (machine->machineInfo.machineType == eZXSpectrum48)
    {
        [[saveAccessoryController.exportPopup itemAtIndex:cSNA_SNAPSHOT_TYPE] setEnabled:YES];
        savePanel.allowedFileTypes = @[cZ80_EXTENSION, cSNA_EXTENSION];
    }
    else
    {
        [[saveAccessoryController.exportPopup itemAtIndex:cSNA_SNAPSHOT_TYPE] setEnabled:NO];
        [saveAccessoryController.exportPopup selectItemAtIndex:cZ80_SNAPSHOT_TYPE];
        savePanel.allowedFileTypes = @[cZ80_EXTENSION];
    }
    
    savePanel.accessoryView = saveAccessoryController.view;
    
    [savePanel beginSheetModalForWindow:self.view.window completionHandler:^(NSInteger result) {
        if (result == NSModalResponseOK)
        {
            ZXSpectrum::Snap snapshot;
            NSURL *url = savePanel.URL;
            
            if (saveAccessoryController.exportType == cZ80_SNAPSHOT_TYPE)
            {
                snapshot = machine->snapshotCreateZ80();
                url = [[url URLByDeletingPathExtension] URLByAppendingPathExtension:cZ80_EXTENSION];
                NSData *data = [NSData dataWithBytes:snapshot.data length:snapshot.length];
                [data writeToURL:url atomically:YES];
            }
            else if (saveAccessoryController.exportType == cSNA_SNAPSHOT_TYPE)
            {
                snapshot = machine->snapshotCreateSNA();
                url = [[url URLByDeletingPathExtension] URLByAppendingPathExtension:cSNA_EXTENSION];
                NSData *data = [NSData dataWithBytes:snapshot.data length:snapshot.length];
                [data writeToURL:url atomically:YES];
            }
        }
    }];
}

- (IBAction)resetPreferences:(id)sender
{
    [[NSUserDefaultsController sharedUserDefaultsController] revertToInitialValues:NULL];
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

- (IBAction)setScreenRatio:(id)sender
{
    NSMenuItem *menuItem = (NSMenuItem *)sender;
    if (menuItem.tag == cSCREEN_4_3)
    {
        self.scene.scaleMode = SKSceneScaleModeAspectFit;
    }
    else if (menuItem.tag == cSCREEN_FILL)
    {
        self.scene.scaleMode = SKSceneScaleModeFill;
    }
}

#pragma mark - Machine Menu Items

- (IBAction)resetMachine:(id)sender
{
    NSMenuItem *menuItem = (NSMenuItem *)sender;
    if (menuItem.tag == 0)
    {
        machine->resetMachine(false);
    }
    else
    {
        machine->resetMachine(true);
    }
}

- (IBAction)selectMachine:(id)sender
{
    NSMenuItem *menuItem = (NSMenuItem *)sender;
    if (menuItem.tag != self.defaults.machineSelectedModel)
    {
        self.defaults.machineSelectedModel = menuItem.tag;
    }
}

- (IBAction)showConfigPanel:(id)sender
{
    NSRect configFrame = self.configEffectsView.frame;
    configFrame.origin.y = 0;
    if (configFrame.origin.x < 0)
    {
        configFrame.origin.x = 0;
        configFrame.origin.y = 0;
    }
    else
    {
        configFrame.origin.x = -configFrame.size.width;
        configFrame.origin.y = 0;
    }
    
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        context.duration = 0.5;
        context.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
        [self.configEffectsView.animator setAlphaValue:1];
        [self.configEffectsView.animator setFrame:configFrame];
    }  completionHandler:^{
        
    }];
}

#pragma mark - Tape Menu Items

- (IBAction)showTapeBrowser:(id)sender
{
    [tapeBrowserWindowController showWindow:self.view.window];
}

- (IBAction)startPlayingTape:(id)sender
{
    tape->startPlaying();
}

- (IBAction)stopPlayingTape:(id)sender
{
    tape->stopPlaying();
}

- (IBAction)rewindTape:(id)sender
{
    tape->rewindTape();
}

- (IBAction)ejectTape:(id)sender
{
    tape->eject();
}

- (IBAction)saveTape:(id)sender
{
    NSSavePanel *savePanel = [NSSavePanel new];
    savePanel.allowedFileTypes = @[ cTAP_EXTENSION ];
    [savePanel beginSheetModalForWindow:tapeBrowserWindowController.window completionHandler:^(NSInteger result) {
        if (result == NSModalResponseOK)
        {
            vector<unsigned char> tapeData = tape->getTapeData();
            NSMutableData *saveData = [NSMutableData new];
            [saveData appendBytes:tapeData.data() length:tapeData.size()];
            [saveData writeToURL:savePanel.URL atomically:YES];
        }
    }];
}

#pragma mark - Getters

- (void *)getDisplayBuffer
{
    return machine->displayBuffer;
}

- (void *)getRGBADisplayBuffer
{
    return machine->displayRGBABuffer;
}

- (BOOL)getDisplayReady
{
    return machine->displayReady;
}
@end





