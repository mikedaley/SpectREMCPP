 //
//  ViewController.m
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>
#import <AVFoundation/AVFoundation.h>
#import <UserNotifications/UserNotifications.h>
#import "EmulationViewController.h"
#import "ZXSpectrum.hpp"
#import "ZXSpectrum48.hpp"
#import "ZXSpectrum128.hpp"
#import "Tape.hpp"
#import "AudioQueue.hpp"
#import "Debug.hpp"

#import "AudioCore.h"
#import "SmartLINK.h"
#import "ORSSerial/ORSSerial.h"

#import "ConfigurationViewController.h"
#import "ExportAccessoryViewController.h"
#import "TapeBrowserViewController.h"
#import "DebugViewController.h"
#import "InfoPanelViewController.h"

#import "MetalRenderer.h"

#import "SharedConstants.h"

#pragma mark - Constants

uint32_t const cAUDIO_SAMPLE_RATE = 44100;
uint32_t const cFRAMES_PER_SECOND = 50;
NSString  *const cSESSION_FILE_NAME = @"session.z80";

const int cSCREEN_4_3 = 0;
const int cSCREEN_FILL = 1;

#pragma mark - Private Interface

@interface EmulationViewController()
{
@public
    ZXSpectrum                      *_machine;
    Debug                           *_debugger;
    Tape                            *_tape;
    NSString                        *_mainBundlePath;
    bool                            _configViewVisible;
    NSURL                           *_lastOpenedURL;
    
    AudioQueue                      *_audioQueue;
    
    NSStoryboard                    *_storyBoard;
    ConfigurationViewController     *_configViewController;
    ExportAccessoryViewController   *_saveAccessoryController;
    NSWindowController              *_tapeBrowserWindowController;
    TapeBrowserViewController       *_tapeBrowserViewController;
    NSWindowController              *_debugWindowController;
    DebugViewController             *_debugViewController;
    InfoPanelViewController         *_infoPanelViewController;
    
    NSTimer                         *_accelerationTimer;
    
    MTKView                         *_metalView;
    MetalRenderer                   *_metalRenderer;
    
    SmartLink                       *_smartLink;
}
@end

#pragma mark - Implementation

@implementation EmulationViewController

- (void)dealloc
{
    if (_machine)
    {
        delete _machine;
    }
    [self.defaults removeObserver:self forKeyPath:MachineAcceleration];
    [self.defaults removeObserver:self forKeyPath:MachineSelectedModel];
    [self.defaults removeObserver:self forKeyPath:MachineTapeInstantLoad];
    [self.defaults removeObserver:self forKeyPath:MachineUseAYSound];
    [self.defaults removeObserver:self forKeyPath:MachineUseSpecDRUM];
    [self.defaults removeObserver:self forKeyPath:SPIPort];
}

#pragma mark - Audio Callback

- (void)audioCallback:(int)inNumberFrames buffer:(int16_t *)buffer
{
    if (_machine)
    {
        const uint32_t b = (cAUDIO_SAMPLE_RATE / (cFRAMES_PER_SECOND * _defaults.machineAcceleration)) * 2;

        _audioQueue->read(buffer, (inNumberFrames << 1));

        // Check if we have used a frames worth of buffer storage and if so then its time to generate another frame.
        if (_audioQueue->bufferUsed() <= b)
        {
            if (_defaults.machineAcceleration == 1)
            {
                _machine->generateFrame();
                
                // No point in updating the screen if the screen isn't visible. Also needed to stop the app from stalling when
                // brought to the front
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (self.view.window.occlusionState & NSApplicationOcclusionStateVisible)
                    {
                        [_metalRenderer updateTextureData:_machine->getScreenBuffer()];
                    }
                });
            }
            _audioQueue->write(_machine->audioBuffer, b);
        }
    }
}

- (void)setupAccelerationTimer
{
    if (_defaults.machineAcceleration > 1)
    {
        [_accelerationTimer invalidate];
        _accelerationTimer = [NSTimer timerWithTimeInterval:1.0 / (cFRAMES_PER_SECOND * _defaults.machineAcceleration) repeats:YES block:^(NSTimer * _Nonnull timer) {
            
            _machine->generateFrame();
            
            if (!(_machine->emuFrameCounter % static_cast<uint32_t>(_defaults.machineAcceleration)))
            {
                [_metalRenderer updateTextureData:_machine->getScreenBuffer()];
            }
        }];
        
        [[NSRunLoop mainRunLoop] addTimer:_accelerationTimer forMode:NSRunLoopCommonModes];
    }
    else
    {
        [_accelerationTimer invalidate];
    }
}

- (void)updateDisplay
{
    [_metalRenderer updateTextureData:_machine->displayBuffer];
//    if (_debugger && _debugViewController) {
//        if (!_debugViewController.view.isHidden) {
//            dispatch_async(dispatch_get_main_queue(), ^{
//                [_debugViewController updateViewDetails];
//            });
//        }
//    }
}

#pragma mark - View Methods

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _metalView = (MTKView *)self.view;
    _metalView.device = MTLCreateSystemDefaultDevice();
    
    if (!_metalView.device)
    {
        NSLog(@"Metal is not supported on this device!");
        return;
    }
    
    _metalRenderer = [[MetalRenderer alloc] initWithMetalKitView:_metalView];
    
    if (!_metalRenderer)
    {
        NSLog(@"Renderer failed init");
        return;
    }
    
    [_metalRenderer mtkView:_metalView drawableSizeWillChange:_metalView.drawableSize];
    _metalView.delegate = _metalRenderer;
    _metalView.nextResponder = self;
    
    _defaults = [Defaults defaults];
    
    _mainBundlePath = [[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/Contents/Resources/"];
    _storyBoard = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
    
    _smartLink = [[SmartLink alloc] init];

    // The AudioCore uses the sound buffer to identify when a new frame should be drawn for accurate timing. The AudioQueue
    // is used to help measure usage of the audio buffer
    _audioQueue = new AudioQueue();
    self.audioCore = [[AudioCore alloc] initWithSampleRate:cAUDIO_SAMPLE_RATE framesPerSecond:cFRAMES_PER_SECOND callback:(id <EmulationProtocol>)self];
    
    //Create a tape instance
    _tape = new Tape(tapeStatusCallback);
    
    [self setupConfigView];
    [self setupInfoView];
    [self setupControllers];
    [self setupObservers];
    [self setupNotifications];
    [self setupBindings];
    
    [self initMachineWithRomPath:_mainBundlePath machineType:(int)_defaults.machineSelectedModel];
    
    [self restoreSession];
        
    if (_defaults.machineAcceleration > 1)
    {
        [self setupAccelerationTimer];
    }
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    [self.view.window setTitle:[NSString stringWithFormat:@"SpectREM %@", [NSString stringWithCString:_machine->machineInfo.machineName encoding:NSUTF8StringEncoding]]];
}

- (void)viewWillDisappear
{
    [super viewWillDisappear];
    
    if (NSURL *supportDirUrl = [self getSupportDirUrl])
    {
        NSError *error = nil;
        if (![[NSFileManager defaultManager] createDirectoryAtURL:supportDirUrl withIntermediateDirectories:YES attributes:nil error:&error])
        {
            NSLog(@"ERROR: creating support directory.");
            return;
        }
        
        supportDirUrl = [supportDirUrl URLByAppendingPathComponent:cSESSION_FILE_NAME];
        ZXSpectrum::Snap sessionSnapshot = _machine->snapshotCreateZ80();
        NSData *data = [NSData dataWithBytes:sessionSnapshot.data length:sessionSnapshot.length];
        [data writeToURL:supportDirUrl atomically:YES];
    }
}

#pragma mark - Notifications

- (void)setupNotifications
{
    [_smartLink bind:@"serialPort" toObject:_configViewController withKeyPath:@"serialPort" options:nil];

}

#pragma mark - Observers

- (void)setupObservers
{
    [self.defaults addObserver:self forKeyPath:MachineAcceleration options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:MachineSelectedModel options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:MachineTapeInstantLoad options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:MachineUseAYSound options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:MachineUseSpecDRUM options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:SPIPort options:NSKeyValueObservingOptionNew context:NULL];
    
    [self.defaults addObserver:self forKeyPath:AudioMasterVolume options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:AudioHighPassFilter options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:AudioLowPassFilter options:NSKeyValueObservingOptionNew context:NULL];
    
    [self.audioCore setAudioHighPassFilter:[[self.defaults valueForKey:AudioHighPassFilter] doubleValue]];
    [self.audioCore setAudioLowPassFilter:[[self.defaults valueForKey:AudioLowPassFilter] doubleValue]];
    [self.audioCore setAudioMasterVolume:[[self.defaults valueForKey:AudioMasterVolume] doubleValue]];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context
{
    if ([keyPath isEqualToString:MachineAcceleration])
    {
        [self setupAccelerationTimer];
    }
    else if ([keyPath isEqualToString:MachineSelectedModel])
    {
        [self initMachineWithRomPath:_mainBundlePath machineType:(int)self.defaults.machineSelectedModel];
    }
    else if ([keyPath isEqualToString:MachineTapeInstantLoad])
    {
        _machine->emuTapeInstantLoad = [change[NSKeyValueChangeNewKey] boolValue];
    }
    else if ([keyPath isEqualToString:MachineUseAYSound])
    {
        _machine->emuUseAYSound = [change[NSKeyValueChangeNewKey] boolValue];
    }
    else if ([keyPath isEqualToString:MachineUseSpecDRUM])
    {
        _machine->emuUseSpecDRUM = [change[NSKeyValueChangeNewKey] boolValue];
    }
    else if ([keyPath isEqualToString:SPIPort])
    {
        _machine->spiPort = [change[NSKeyValueChangeNewKey] unsignedIntegerValue];
    }
    else if ([keyPath isEqualToString:AudioMasterVolume])
    {
        [self.audioCore setAudioMasterVolume:[change[NSKeyValueChangeNewKey] doubleValue]];
    }
    else if ([keyPath isEqualToString:AudioLowPassFilter])
    {
        [self.audioCore setAudioLowPassFilter:[change[NSKeyValueChangeNewKey] doubleValue]];
    }
    else if ([keyPath isEqualToString:AudioHighPassFilter])
    {
        [self.audioCore setAudioHighPassFilter:[change[NSKeyValueChangeNewKey] doubleValue]];
    }
}

#pragma mark - SmartLINK

- (IBAction)smartlinkReset:(id)sender
{
    [_smartLink sendSmartlinkAction:cSMARTLINK_RESET];
}

- (IBAction)smartlinkSendSnapshot:(id)sender
{
     ZXSpectrum::Snap snapshot = _machine->snapshotCreateZ80();
    [_smartLink sendSnapshot:snapshot.data ofType:SnapshotTypeZ80];
}

#pragma mark - Apply defaults

- (void)applyDefaults
{
    _machine->emuTapeInstantLoad = self.defaults.machineTapeInstantLoad;
    _machine->emuUseAYSound = self.defaults.machineUseAYSound;
    _machine->emuUseSpecDRUM = self.defaults.machineUseSpecDRUM;
}

#pragma mark - View/Controller Setup

- (void)setupConfigView
{
    _configViewController = [_storyBoard instantiateControllerWithIdentifier:@"CONFIG_VIEW_CONTROLLER"];
    [self.configEffectsView setFrameOrigin:(CGPoint){-self.configEffectsView.frame.size.width, 0}];
    self.configScrollView.documentView = _configViewController.view;
}

- (void)setupInfoView
{
    _infoPanelViewController = [_storyBoard instantiateControllerWithIdentifier:@"InfoPanelViewController"];
    [_infoPanelViewController.view setFrameOrigin:(NSPoint){self.view.frame.size.width - _infoPanelViewController.view.frame.size.width - 4, -_infoPanelViewController.view.frame.size.height}];
    [self.view addSubview:_infoPanelViewController.view];
}

- (void)setupControllers
{
    _saveAccessoryController = [_storyBoard instantiateControllerWithIdentifier:@"SAVE_ACCESSORY_VIEW_CONTROLLER"];
    _tapeBrowserWindowController = [_storyBoard instantiateControllerWithIdentifier:@"TAPE_BROWSER_WINDOW"];
    _tapeBrowserViewController = (TapeBrowserViewController *)_tapeBrowserWindowController.contentViewController;
    _tapeBrowserViewController.emulationViewController = self;

    _debugWindowController = [_storyBoard instantiateControllerWithIdentifier:@"DEBUG_WINDOW"];
    _debugViewController = (DebugViewController *)_debugWindowController.contentViewController;
    _debugViewController.emulationViewController = self;
}

- (void)setupBindings
{
    [_smartLink bind:@"serialPort" toObject:_configViewController withKeyPath:@"serialPort" options:nil];
}

#pragma mark - Init/Switch Machine

- (void)initMachineWithRomPath:(NSString *)romPath machineType:(int)machineType
{
    if (self.audioCore)
    {
        [self.audioCore stop];
        while (self.audioCore.isRunning) { };
    }
    
    if (_machine) {
        _machine->pause();
        delete _machine;
    }
    
    if (machineType == eZXSpectrum48)
    {
        _machine = new ZXSpectrum48(_tape);
        [_infoPanelViewController displayMessage:@"ZX Spectrum 48k" duration:5];
    }
    else if (machineType == eZXSpectrum128)
    {
        _machine = new ZXSpectrum128(_tape);
        [_infoPanelViewController displayMessage:@"ZX Spectrum 128k" duration:5];
    }
    else
    {
        NSLog(@"Unknown machine type!");
        return;
    }
    
    _machine->initialise((char *)[romPath cStringUsingEncoding:NSUTF8StringEncoding]);
    
    _debugger = new Debug;
    _debugger->attachMachine(_machine);
    
    std::function<bool(uint16_t, uint8_t)> debugBlock;
    EmulationViewController *blockSelf = self;
    
    debugBlock = ([blockSelf](uint16_t address, uint8_t operation) {
        
        if (blockSelf->_debugger->checkForBreakpoint(address, operation))
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                [blockSelf sendNotificationsWithTitle:@"EXEC BREAK"
                                        subtitle:nil
                                            body:[NSString stringWithFormat:@"EXEC BREAK at 0x%04x", address]
                                           sound:[UNNotificationSound defaultCriticalSound]];
                [blockSelf->_debugViewController pauseMachine:nil];
            });
            return true;
        }
        
        return false;
        
    });
    
    _machine->registerDebugOpCallback( debugBlock );
    
    // Once a machine instance has been created we need to apply the defaults to that instance
    [self applyDefaults];
    
    [self.audioCore start];
    _machine->resume();
    
    [self.view.window setTitle:[NSString stringWithFormat:@"SpectREM %@",
                                [NSString stringWithCString:_machine->machineInfo.machineName
                                                   encoding:NSUTF8StringEncoding]]];
}

#pragma mark - Local Notifications

- (void)sendNotificationsWithTitle:(NSString *)title subtitle:(NSString *)subtitle body:(NSString *)body sound:(UNNotificationSound *)sound
{
    UNMutableNotificationContent *userNote = [UNMutableNotificationContent new];
    userNote.title = title;
    userNote.subtitle = subtitle;
    userNote.body = body;
    userNote.sound = sound;
    UNNotificationRequest *request = [UNNotificationRequest requestWithIdentifier:[[NSUUID UUID] UUIDString] content:userNote trigger:nil];
    [[UNUserNotificationCenter currentNotificationCenter] addNotificationRequest:request withCompletionHandler:nil];
}

#pragma mark - Keyboard

- (void)keyDown:(NSEvent *)event
{
    if (!event.isARepeat && !(event.modifierFlags & NSEventModifierFlagCommand) && event.keyCode != 96)
    {
        _machine->keyboardKeyDown(event.keyCode);
    }
}

- (void)keyUp:(NSEvent *)event
{
    if (event.keyCode == 96) // F5 key
    {
        [self loadFileWithURL:_lastOpenedURL addToRecent:NO];
        return;
    }
        
    if (!event.isARepeat && !(event.modifierFlags & NSEventModifierFlagCommand))
    {
        _machine->keyboardKeyUp(event.keyCode);
    }
}

- (void)flagsChanged:(NSEvent *)event
{
    if (!(event.modifierFlags & NSEventModifierFlagCommand))
    {
//        _machine->keyboardFlagsChanged(event.modifierFlags, event.keyCode);
    }
}

#pragma mark - File Loading

- (void)loadFileWithURL:(NSURL *)url addToRecent:(BOOL)addToRecent
{
    BOOL success = NO;
    
    NSString *urlPath = [url.pathExtension uppercaseString];
    if (([urlPath isEqualToString:cZ80_EXTENSION] || [urlPath isEqualToString:cSNA_EXTENSION]))
    {
        int snapshotMachineType = _machine->snapshotMachineInSnapshotWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]);
        if (_machine->machineInfo.machineType != snapshotMachineType)
        {
            self.defaults.machineSelectedModel = snapshotMachineType;
        }
    }
    
    if ([[url.pathExtension uppercaseString] isEqualToString:cZ80_EXTENSION])
    {
        success = _machine->snapshotZ80LoadWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]);
    }
    else if ([[url.pathExtension uppercaseString] isEqualToString:cSNA_EXTENSION])
    {
        success = _machine->snapshotSNALoadWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]);
    }
    else if ([[url.pathExtension uppercaseString] isEqualToString:cTAP_EXTENSION])
    {
        success = _tape->loadWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]);
        [[NSNotificationCenter defaultCenter] postNotificationName:@"TAPE_CHANGED_NOTIFICATION" object:NULL];
    }
    
    if (success)
    {
        _lastOpenedURL = url;
        if (addToRecent)
        {
            [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:url];
        }
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

#pragma mark - Restore Session

- (void)restoreSession
{
    if (NSURL *supportDirUrl = [self getSupportDirUrl])
    {
        // Load the last session file if it exists
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
    return _tape->numberOfTapeBlocks();
}

- (NSString *)tapeBlockTypeForIndex:(NSInteger)blockIndex
{
    return @(_tape->blocks[ blockIndex ]->getBlockName().c_str());
}

- (NSString *)tapeFilenameForIndex:(NSInteger)blockIndex
{
    return @(_tape->blocks[ blockIndex ]->getFilename().c_str());
}

- (int)tapeAutostartLineForIndex:(NSInteger)blockIndex
{
    int lineNumber = _tape->blocks [blockIndex ]->getAutoStartLine();
    return (lineNumber == 32768) ? 0 : lineNumber;
}

- (unsigned short)tapeBlockStartAddressForIndex:(NSInteger)blockIndex
{
    return _tape->blocks[ blockIndex ]->getStartAddress();
}

- (unsigned short)tapeBlockLengthForIndex:(NSInteger)blockIndex
{
    return _tape->blocks[ blockIndex ]->getDataLength();
}

- (NSInteger)tapeCurrentBlock
{
    return _tape->currentBlockIndex;
}

- (BOOL)tapeIsplaying
{
    return _tape->playing;
}

- (void)tapeSetCurrentBlock:(NSInteger)blockIndex
{
    _tape->setSelectedBlock( static_cast<int>(blockIndex) );
    _tape->rewindBlock();
    _tape->stopPlaying();
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
    
    if (_machine->machineInfo.machineType == eZXSpectrum48)
    {
        [[_saveAccessoryController.exportPopup itemAtIndex:cSNA_SNAPSHOT_TYPE] setEnabled:YES];
        savePanel.allowedFileTypes = @[cZ80_EXTENSION, cSNA_EXTENSION];
    }
    else
    {
        [[_saveAccessoryController.exportPopup itemAtIndex:cSNA_SNAPSHOT_TYPE] setEnabled:NO];
        [_saveAccessoryController.exportPopup selectItemAtIndex:cZ80_SNAPSHOT_TYPE];
        savePanel.allowedFileTypes = @[cZ80_EXTENSION];
    }
    
    savePanel.accessoryView = _saveAccessoryController.view;
    
    [savePanel beginWithCompletionHandler:^(NSInteger result) {
        if (result == NSModalResponseOK)
        {
            ZXSpectrum::Snap snapshot;
            NSURL *url = savePanel.URL;
            
            if (_saveAccessoryController.exportType == cZ80_SNAPSHOT_TYPE)
            {
                snapshot = _machine->snapshotCreateZ80();
                url = [[url URLByDeletingPathExtension] URLByAppendingPathExtension:cZ80_EXTENSION];
                NSData *data = [NSData dataWithBytes:snapshot.data length:snapshot.length];
                [data writeToURL:url atomically:YES];
            }
            else if (_saveAccessoryController.exportType == cSNA_SNAPSHOT_TYPE)
            {
                snapshot = _machine->snapshotCreateSNA();
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
        _machine->resetMachine(false);
    }
    else
    {
        _machine->resetMachine(true);
    }
}

- (IBAction)resetToSnapLoad:(id)sender
{
    _machine->resetToSnapLoad();
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
        context.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
        [self.configEffectsView.animator setAlphaValue:1];
        [self.configEffectsView.animator setFrame:configFrame];
    }  completionHandler:^{
        
    }];
}

#pragma mark - Debug Menu Items

- (IBAction)showDebugger:(id)sender
{
    [_debugWindowController showWindow:self];
}

- (IBAction)switchHexDecimal:(id)sender
{
    _debugViewController.hexFormat = (_debugViewController.hexFormat) ? NO : YES;
    [_debugViewController updateViewDetails];
}

- (void)pauseMachine
{
    if (_machine)
    {
        _machine->emuPaused = true;
        [self.audioCore stop];
    }
}

- (void)startMachine
{
    if (_machine)
    {
        _machine->emuPaused = false;
        [self.audioCore start];
    }
}

#pragma mark - Tape Menu Items

- (IBAction)showTapeBrowser:(id)sender
{
    [_tapeBrowserWindowController showWindow:self.view.window];
}

- (IBAction)startPlayingTape:(id)sender
{
    _tape->startPlaying();
}

- (IBAction)stopPlayingTape:(id)sender
{
    _tape->stopPlaying();
}

- (IBAction)rewindTape:(id)sender
{
    _tape->rewindTape();
}

- (IBAction)ejectTape:(id)sender
{
    _tape->eject();
}

- (IBAction)saveTape:(id)sender
{
    NSSavePanel *savePanel = [NSSavePanel new];
    savePanel.allowedFileTypes = @[ cTAP_EXTENSION ];
    [savePanel beginSheetModalForWindow:_tapeBrowserWindowController.window completionHandler:^(NSInteger result) {
        if (result == NSModalResponseOK)
        {
            vector<unsigned char> tapeData = _tape->getTapeData();
            NSMutableData *saveData = [NSMutableData new];
            [saveData appendBytes:tapeData.data() length:tapeData.size()];
            [saveData writeToURL:savePanel.URL atomically:YES];
        }
    }];
}

#pragma mark - Getters

- (void *)getDisplayBuffer
{
    return _machine->displayBuffer;
}

- (BOOL)getDisplayReady
{
    return _machine->displayReady;
}

- (void *)getCurrentMachine
{
    return _machine;
}

- (void *)getDebugger
{
    return _debugger;
}

- (BOOL)isEmulatorPaused
{
    return _machine->emuPaused;
}

@end





