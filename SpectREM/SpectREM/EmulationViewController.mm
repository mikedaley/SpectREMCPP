//
//  ViewController.m
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import <AVFoundation/AVFoundation.h>
#import "EmulationViewController.h"
#import "ZXSpectrum.hpp"
#import "ZXSpectrum48.hpp"
#import "ZXSpectrum128.hpp"
#import "Tape.hpp"
#import "AudioQueue.hpp"
#import "Debug.hpp"

#import "AudioCore.h"
#import "OpenGLView.h"

#import "ConfigurationViewController.h"
#import "ExportAccessoryViewController.h"
#import "TapeBrowserViewController.h"
#import "DebugViewController.h"

#pragma mark - Constants

uint32_t const cAUDIO_SAMPLE_RATE = 44100;
uint32_t const cFRAMES_PER_SECOND = 50;

static NSString  *const cSESSION_FILE_NAME = @"session.z80";

static const int cSCREEN_4_3 = 0;
static const int cSCREEN_FILL = 1;

#pragma mark - Private Interface

@interface EmulationViewController()
{
@public
    ZXSpectrum                      *machine;
    Debug                           *debugger;
    Tape                            *tape;
    dispatch_source_t               displayTimer;
    NSString                        *mainBundlePath;
    bool                            configViewVisible;
    
    AudioQueue                      *audioQueue;
    int16_t                         audioBuffer;
    DebugOpCallbackBlock            debugBlock;
    
    NSStoryboard                    *storyBoard;
    ConfigurationViewController     *configViewController;
    ExportAccessoryViewController   *saveAccessoryController;
    NSWindowController              *tapeBrowserWindowController;
    TapeBrowserViewController       *tapeBrowserViewController;
    NSWindowController              *debugWindowController;
    DebugViewController             *debugViewController;
    
    NSTimer                         *accelerationTimer;
}
@end

#pragma mark - Implementation

@implementation EmulationViewController

- (void)dealloc
{
    if (machine)
    {
        delete machine;
    }
    [self.defaults removeObserver:self forKeyPath:MachineAcceleration];
    [self.defaults removeObserver:self forKeyPath:MachineSelectedModel];
    [self.defaults removeObserver:self forKeyPath:MachineTapeInstantLoad];
    [self.defaults removeObserver:self forKeyPath:MachineUseAYSound];
    [self.defaults removeObserver:self forKeyPath:SPIPort];
    
    [[NSNotificationCenter defaultCenter] removeObserver:cDISPLAY_UPDATE_NOTIFICATION];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _defaults = [Defaults defaults];
    
    mainBundlePath = [[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/Contents/Resources/"];
    storyBoard = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
    
    self.view.nextResponder = self;
    
    // The AudioCore uses the sound buffer to identify when a new frame should be drawn for accurate timing. The AudioQueue
    // is used to help measure usage of the audio buffer
    audioQueue = new AudioQueue();
    self.audioCore = [[AudioCore alloc] initWithSampleRate:cAUDIO_SAMPLE_RATE framesPerSecond:cFRAMES_PER_SECOND callback:self];
    
    //Create a tape instance
    tape = new Tape(tapeStatusCallback);
    
    [self setupConfigView];
    [self setupControllers];
    [self setupObservers];
    [self setupNotifications];
    
    [self initMachineWithRomPath:mainBundlePath machineType:(int)_defaults.machineSelectedModel];

    [self restoreSession];
    
    if (_defaults.machineAcceleration > 1)
    {
        [self setupAccelerationTimer];
    }
}

#pragma mark - Audio Callback

- (void)audioCallback:(int)inNumberFrames buffer:(int16_t *)buffer
{
    if (machine)
    {
        const uint32_t b = (cAUDIO_SAMPLE_RATE / (cFRAMES_PER_SECOND * _defaults.machineAcceleration)) * 2;
        
        audioQueue->read(buffer, (inNumberFrames << 1));
        
        // Check if we have used a frames worth of buffer storage and if so then its time to generate another frame.
        if (audioQueue->bufferUsed() < b)
        {
            if (_defaults.machineAcceleration == 1)
            {
                machine->generateFrame();
                dispatch_async(dispatch_get_main_queue(), ^{
                    [(OpenGLView *)self.glView updateTextureData:machine->displayBuffer];
                });
            }
            audioQueue->write(machine->audioBuffer, b);
        }
    }
}

- (void)setupAccelerationTimer
{
    if (_defaults.machineAcceleration > 1)
    {
        [accelerationTimer invalidate];
        accelerationTimer = [NSTimer timerWithTimeInterval:1.0 / (50.0 * _defaults.machineAcceleration) repeats:YES block:^(NSTimer * _Nonnull timer) {
            
            machine->generateFrame();
            
            if (!(machine->emuFrameCounter % static_cast<uint32_t>(_defaults.machineAcceleration)))
            {
                dispatch_async(dispatch_get_main_queue(), ^{
                    [(OpenGLView *)self.glView updateTextureData:machine->displayBuffer];
                });
            }
            
        }];
        
        [[NSRunLoop mainRunLoop] addTimer:accelerationTimer forMode:NSRunLoopCommonModes];
    }
    else
    {
        [accelerationTimer invalidate];
    }
}

- (void)updateDisplay
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [(OpenGLView *)self.glView updateTextureData:machine->displayBuffer];
    });
}

#pragma mark - View Methods

- (void)viewWillAppear
{
    [self.view.window setTitle:[NSString stringWithFormat:@"SpectREM %@", [NSString stringWithCString:machine->machineInfo.machineName encoding:NSUTF8StringEncoding]]];
}

#pragma mark - Notifications

- (void)setupNotifications
{
    [[NSNotificationCenter defaultCenter] addObserverForName:cDISPLAY_UPDATE_NOTIFICATION object:NULL queue:NULL usingBlock:^(NSNotification * _Nonnull note) {
        [self updateDisplay];
    }];
}

#pragma mark - Observers

- (void)setupObservers
{
    [self.defaults addObserver:self forKeyPath:MachineAcceleration options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:MachineSelectedModel options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:MachineTapeInstantLoad options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:MachineUseAYSound options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:SPIPort options:NSKeyValueObservingOptionNew context:NULL];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context
{
    if ([keyPath isEqualToString:MachineAcceleration])
    {
        [self setupAccelerationTimer];
    }
    else if ([keyPath isEqualToString:MachineSelectedModel])
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
    else if ([keyPath isEqualToString:SPIPort])
    {
        machine->spiPort = [change[NSKeyValueChangeNewKey] unsignedIntegerValue];
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

    debugWindowController = [storyBoard instantiateControllerWithIdentifier:@"DEBUG_WINDOW"];
    debugViewController = (DebugViewController *)debugWindowController.contentViewController;
    debugViewController.emulationViewController = self;

}

#pragma mark - Init/Switch Machine

- (void)initMachineWithRomPath:(NSString *)romPath machineType:(int)machineType
{
    if (self.audioCore)
    {
        [self.audioCore stop];
        while (self.audioCore.isRunning) { };
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
    else
    {
        NSLog(@"Unknown machine type!");
        return;
    }
    
    machine->initialise((char *)[romPath cStringUsingEncoding:NSUTF8StringEncoding]);
    
    debugger = new Debug;
    debugger->registerMachine(machine);
    
    __block EmulationViewController *blockSelf = self;
    
    debugBlock = (^bool(unsigned short address, uint8_t operation) {
        
        if (blockSelf->debugger->checkForBreakpoint(address, operation))
        {
            [blockSelf pauseMachine];
            return true;
        }
        return false;
        
    });
    
    machine->registerDebugOpCallback( debugBlock );
    
    [self applyDefaults];
    
    [self.audioCore start];
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
    
    [openPanel beginWithCompletionHandler:^(NSModalResponse result) {
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
    
    [savePanel beginWithCompletionHandler:^(NSInteger result) {
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
    [Defaults setupDefaultsWithReset:YES];
}

#pragma mark - View Menu Items

- (IBAction)setWindowSize:(id)sender
{
    if (([self.view.window styleMask] & NSWindowStyleMaskFullScreen) != NSWindowStyleMaskFullScreen)
    {
        NSMenuItem *menuItem = (NSMenuItem*)sender;
        float width = (32 + 256 + 32) * menuItem.tag;
        float height = (32 + 192 + 32) * menuItem.tag;
        [self.view.window setContentSize:(NSSize){width, height}];
    }
}

- (IBAction)setScreenRatio:(id)sender
{
    NSMenuItem *menuItem = (NSMenuItem *)sender;
    if (menuItem.tag == cSCREEN_4_3)
    {

    }
    else if (menuItem.tag == cSCREEN_FILL)
    {

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

- (IBAction)resetToSnapLoad:(id)sender
{
    machine->resetToSnapLoad();
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

//        CASpringAnimation *spring = [CASpringAnimation animation];
//        spring.fromValue = [NSValue valueWithRect:self.configEffectsView.layer.presentationLayer.frame];
//        spring.toValue = [NSValue valueWithRect:configFrame];
//        spring.duration = 1;
//        spring.damping = 10;
//        [self.configEffectsView.layer addAnimation:spring forKey:@"position.x"];
//        self.configEffectsView.frame = configFrame;
    }
    else
    {
        configFrame.origin.x = -configFrame.size.width;
        configFrame.origin.y = 0;
    }
    
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        context.duration = 0.3;
        context.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseInEaseOut];
        [self.configEffectsView.animator setAlphaValue:1];
        [self.configEffectsView.animator setFrame:configFrame];
    }  completionHandler:^{
        
    }];

}

#pragma mark - Debug Menu Items

- (IBAction)showDebugger:(id)sender
{
    [debugWindowController showWindow:NULL];
}

- (IBAction)switchHexDecimal:(id)sender
{
    debugViewController.hexFormat = (debugViewController.hexFormat) ? NO : YES;
    [debugViewController updateViewDetails];
}

- (void)pauseMachine
{
    if (machine)
    {
        machine->emuPaused = true;
        [self.audioCore stop];
        [[NSNotificationCenter defaultCenter] postNotificationName:cCPU_PAUSED_NOTIFICATION object:NULL];
    }
}

- (void)startMachine
{
    if (machine)
    {
        machine->emuPaused = false;
        [self.audioCore start];
        [[NSNotificationCenter defaultCenter] postNotificationName:cCPU_RESUMED_NOTIFICATION object:NULL];
    }
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

- (BOOL)getDisplayReady
{
    return machine->displayReady;
}

- (void *)getCurrentMachine
{
    return machine;
}

- (void *)getDebugger
{
    return debugger;
}

- (BOOL)getCPUState
{
    return machine->emuPaused;
}

- (void)pauseCPU
{
    machine->emuPaused = true;
    [[NSNotificationCenter defaultCenter] postNotificationName:@"kCPU_PAUSED_NOTIFICATION" object:NULL];
}

- (void)resumeCPU
{
    machine->emuPaused = false;
    [[NSNotificationCenter defaultCenter] postNotificationName:@"kCPU_RESUMED_NOTIFICATION" object:NULL];
}

@end





