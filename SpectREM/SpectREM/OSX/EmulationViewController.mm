//
//  ViewController.m
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>
#import <QuartzCore/QuartzCore.h>
#import <UserNotifications/UserNotifications.h>

#import "AudioCore.h"
#import "AudioQueue.hpp"
#import "ConfigurationViewController.h"
#import "Debug.hpp"
#import "DebugViewController.h"
#import "EmulationController.hpp"
#import "EmulationViewController.h"
#import "ExportAccessoryViewController.h"
#import "InfoPanelViewController.h"
#import "MetalRenderer.h"
#import "ORSSerial/ORSSerial.h"
#import "SharedConstants.h"
#import "SmartLINK.h"
#import "Tape.hpp"
#import "TapeBrowserViewController.h"
#import "ZXSpectrum.hpp"
#import "ZXSpectrum128.hpp"
#import "ZXSpectrum48.hpp"

#pragma mark - Constants

uint32_t const cAUDIO_SAMPLE_RATE   = 44100;
uint32_t const cFRAMES_PER_SECOND   = 50;
NSString * const cSESSION_FILE_NAME = @"session.z80";

const int cSCREEN_4_3               = 0;
const int cSCREEN_FILL              = 1;

#pragma mark - Private Interface

@interface EmulationViewController()
{
@public
    EmulationController                 * emulationController;

    NSString                            * mainBundlePath_;
    NSURL                               * lastOpenedURL_;
    
    AudioQueue                          * audioQueue_;
    
    NSDictionary                        * keyMappings_;
    
    NSStoryboard                        * storyBoard_;
    
    ConfigurationViewController         * configViewController_;
    bool                                configViewVisible_;
    
    ExportAccessoryViewController       * saveAccessoryController_;
    
    NSWindowController                  * tapeBrowserWindowController_;
    TapeBrowserViewController           * tapeBrowserViewController_;
    
    NSWindowController                  * debugWindowController_;
    DebugViewController                 * debugViewController_;
    
    InfoPanelViewController             * infoPanelViewController_;
    
    MTKView                             * metalView_;
    MetalRenderer                       * metalRenderer_;
    
    // This timer is used when the emulator is run at > 1x speed. Rather than the sound driving the speed, a timer
    // drives the updates
    NSTimer                             * accelerationTimer_;
    
    SmartLink                           * smartLink_;
}
@end

#pragma mark - Implementation

@implementation EmulationViewController

- (void)dealloc
{
    delete emulationController;
    [self.defaults removeObserver:self forKeyPath:MachineAcceleration];
    [self.defaults removeObserver:self forKeyPath:MachineSelectedModel];
    [self.defaults removeObserver:self forKeyPath:MachineTapeInstantLoad];
    [self.defaults removeObserver:self forKeyPath:MachineUseAYSound];
    [self.defaults removeObserver:self forKeyPath:MachineUseSpecDRUM];
}

#pragma mark - Audio Callback

- (void)audioCallback:(int)inNumberFrames buffer:(int16_t *)buffer
{
    if (emulationController->machine)
    {
        const uint32_t b = (cAUDIO_SAMPLE_RATE / (cFRAMES_PER_SECOND * _defaults.machineAcceleration)) * 2;
        
        audioQueue_->read(buffer, (inNumberFrames << 1));
        
        // Check if we have used a frames worth of buffer storage and if so then its time to generate another frame.
        if (audioQueue_->bufferUsed() <= b)
        {
            if (_defaults.machineAcceleration == 1)
            {
                // No point in updating the screen if the screen isn't visible. Also needed to stop the app from stalling when
                // brought to the front
                dispatch_async(dispatch_get_main_queue(), ^{
                    if (self.view.window.occlusionState & NSApplicationOcclusionStateVisible)
                    {
                        [metalRenderer_ updateTextureData:emulationController->getDisplayBuffer()];
                    }
                });
                
                // Generate another frame
                emulationController->generateFrame();
            }
            audioQueue_->write(emulationController->getAudioBuffer(), b);
        }
    }
}

- (void)setupAccelerationTimer
{
    if (_defaults.machineAcceleration > 1)
    {
        [accelerationTimer_ invalidate];
        accelerationTimer_ = [NSTimer timerWithTimeInterval:1.0 / (cFRAMES_PER_SECOND * _defaults.machineAcceleration) repeats:YES block:^(NSTimer * _Nonnull timer) {
            
            emulationController->generateFrame();
            
            if (!(emulationController->getFrameCounter() % static_cast<uint32_t>(_defaults.machineAcceleration)))
            {
                [metalRenderer_ updateTextureData:emulationController->getDisplayBuffer()];
            }
        }];
        
        [[NSRunLoop mainRunLoop] addTimer:accelerationTimer_ forMode:NSRunLoopCommonModes];
    }
    else
    {
        [accelerationTimer_ invalidate];
    }
}

- (void)updateDisplay
{
    [metalRenderer_ updateTextureData:emulationController->getDisplayBuffer()];
    
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
    
    metalView_ = (MTKView *)self.view;
    metalView_.device = MTLCreateSystemDefaultDevice();
    
    if (!metalView_.device)
    {
        NSLog(@"Metal is not supported on this device!");
        return;
    }
    
    metalRenderer_ = [[MetalRenderer alloc] initWithMetalKitView:metalView_];
    
    if (!metalRenderer_)
    {
        NSLog(@"Renderer failed init");
        return;
    }
    
    [metalRenderer_ mtkView:metalView_ drawableSizeWillChange:metalView_.drawableSize];
    metalView_.delegate = metalRenderer_;
    metalView_.nextResponder = self;
    
    _defaults = [Defaults defaults];
    
    mainBundlePath_ = [[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/Contents/Resources/"];
    storyBoard_ = [NSStoryboard storyboardWithName:@"Main" bundle:nil];
    
    smartLink_ = [[SmartLink alloc] init];
    
    // The AudioCore uses the sound buffer to identify when a new frame should be drawn for accurate timing. The AudioQueue
    // is used to help measure usage of the audio buffer
    audioQueue_ = new AudioQueue();
    self.audioCore = [[AudioCore alloc] initWithSampleRate:cAUDIO_SAMPLE_RATE framesPerSecond:cFRAMES_PER_SECOND callback:(id <EmulationProtocol>)self];
    
    emulationController = new EmulationController();
    [self initMachineWithRomPath:mainBundlePath_ machineType:(int)_defaults.machineSelectedModel];

    [self setupConfigView];
    [self setupInfoView];
    [self setupControllers];
    [self setupObservers];
    [self setupNotifications];
    [self setupBindings];
    [self setupKeyMappings];
        
    [self restoreSession];
    
    if (_defaults.machineAcceleration > 1)
    {
        [self setupAccelerationTimer];
    }
}

- (void)viewWillAppear
{
    [super viewWillAppear];
    [self.view.window setTitle:[NSString stringWithFormat:@"SpectREM %@", [NSString stringWithCString:emulationController->getMachineName() encoding:NSUTF8StringEncoding]]];
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
        ZXSpectrum::SnapshotData sessionSnapshot = emulationController->snapshotCreateZ80();
        NSData *data = [NSData dataWithBytes:sessionSnapshot.data length:sessionSnapshot.length];
        [data writeToURL:supportDirUrl atomically:YES];
    }
}

#pragma mark - Notifications

- (void)setupNotifications
{
    [smartLink_ bind:@"serialPort" toObject:configViewController_ withKeyPath:@"serialPort" options:nil];
    
}

#pragma mark - Observers

- (void)setupObservers
{
    [self.defaults addObserver:self forKeyPath:MachineAcceleration options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:MachineSelectedModel options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:MachineTapeInstantLoad options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:MachineUseAYSound options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:MachineUseSpecDRUM options:NSKeyValueObservingOptionNew context:NULL];
    
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
        [self initMachineWithRomPath:mainBundlePath_ machineType:(int)self.defaults.machineSelectedModel];
    }
    else if ([keyPath isEqualToString:MachineTapeInstantLoad])
    {
        emulationController->setInstantTapeLoad([change[NSKeyValueChangeNewKey] boolValue]);
    }
    else if ([keyPath isEqualToString:MachineUseAYSound])
    {
        emulationController->setUseAySound([change[NSKeyValueChangeNewKey] boolValue]);
    }
    else if ([keyPath isEqualToString:MachineUseSpecDRUM])
    {
        emulationController->setUseSpecDrum([change[NSKeyValueChangeNewKey] boolValue]);
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
    [smartLink_ sendSmartlinkAction:cSMARTLINK_RESET];
}

- (IBAction)smartlinkSendSnapshot:(id)sender
{
    ZXSpectrum::SnapshotData snapshot = emulationController->snapshotCreateZ80();
    [smartLink_ sendSnapshot:snapshot.data ofType:SnapshotTypeZ80];
}

#pragma mark - Apply defaults

- (void)applyDefaults
{
    emulationController->setInstantTapeLoad(self.defaults.machineTapeInstantLoad);
    emulationController->setUseAySound(self.defaults.machineUseAYSound);
    emulationController->setUseSpecDrum(self.defaults.machineUseSpecDRUM);
}

#pragma mark - View/Controller Setup

- (void)setupConfigView
{
    configViewController_ = [storyBoard_ instantiateControllerWithIdentifier:@"CONFIG_VIEW_CONTROLLER"];
    [self.configEffectsView setFrameOrigin:(CGPoint){-self.configEffectsView.frame.size.width, 0}];
    self.configScrollView.documentView = configViewController_.view;
}

- (void)setupInfoView
{
    infoPanelViewController_ = [storyBoard_ instantiateControllerWithIdentifier:@"InfoPanelViewController"];
    [infoPanelViewController_.view setFrameOrigin:(NSPoint){self.view.frame.size.width - infoPanelViewController_.view.frame.size.width - 4, -infoPanelViewController_.view.frame.size.height}];
    [self.view addSubview:infoPanelViewController_.view];
}

- (void)setupControllers
{
    saveAccessoryController_ = [storyBoard_ instantiateControllerWithIdentifier:@"SAVE_ACCESSORY_VIEW_CONTROLLER"];
    tapeBrowserWindowController_ = [storyBoard_ instantiateControllerWithIdentifier:@"TAPE_BROWSER_WINDOW"];
    tapeBrowserViewController_ = (TapeBrowserViewController *)tapeBrowserWindowController_.contentViewController;
//    tapeBrowserViewController_.emulationViewController = self;
    tapeBrowserViewController_.emulationController = emulationController;
    
    debugWindowController_ = [storyBoard_ instantiateControllerWithIdentifier:@"DEBUG_WINDOW"];
    debugViewController_ = (DebugViewController *)debugWindowController_.contentViewController;
    debugViewController_.emulationViewController = self;
}

- (void)setupBindings
{
    [smartLink_ bind:@"serialPort" toObject:configViewController_ withKeyPath:@"serialPort" options:nil];
}

#pragma mark - Init/Switch Machine

- (void)initMachineWithRomPath:(NSString *)romPath machineType:(int)machineType
{
    if (self.audioCore)
    {
        [self.audioCore stop];
        while (self.audioCore.isRunning) { };
    }
    
    emulationController->pauseMachine();
    emulationController->createMachineOfType(machineType, [romPath cStringUsingEncoding:NSUTF8StringEncoding]);
    [infoPanelViewController_ displayMessage:[NSString stringWithCString:emulationController->getMachineName() encoding:NSUTF8StringEncoding] duration:5];

    emulationController->setTapeStatusCallback(tapeStatusCallback);
    [self setupDebugger];
    
    // Once a machine instance has been created we need to apply the defaults to that instance
    [self applyDefaults];
    
    [self.audioCore start];
    emulationController->resumeMachine();
    
    [self.view.window setTitle:[NSString stringWithFormat:@"SpectREM %@",
                                [NSString stringWithCString:emulationController->getMachineName()
                                                   encoding:NSUTF8StringEncoding]]];
}

#pragma mark - Debugger

- (void)setupDebugger
{
    EmulationViewController *blockSelf = self;
    std::function<bool(uint16_t, uint8_t)> debugBlock;
    
    debugBlock = ([blockSelf](uint16_t address, uint8_t operation) {
        
        if (blockSelf->emulationController->debugger->checkForBreakpoint(address, operation))
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                [blockSelf sendNotificationsWithTitle:@"EXEC BREAK"
                                             subtitle:nil
                                                 body:[NSString stringWithFormat:@"EXEC BREAK at 0x%04x", address]
                                                sound:[UNNotificationSound defaultSound]];
                [blockSelf->debugViewController_ pauseMachine:nil];
            });
            return true;
        }
        
        return false;
        
    });
    
    emulationController->setDebugCallback( debugBlock );
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

- (void)setupKeyMappings
{
    keyMappings_ = @{
        @(OSX_KEY_LEFT_SHIFT)   : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Shift), //Left Shift
        @(OSX_KEY_RIGHT_SHIFT)  : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Shift), //Right Shift
        @(OSX_KEY_Z)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Z),
        @(OSX_KEY_X)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_X),
        @(OSX_KEY_C)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_C),
        @(OSX_KEY_V)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_V),
        
        @(OSX_KEY_A)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_A),
        @(OSX_KEY_S)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_S),
        @(OSX_KEY_D)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_D),
        @(OSX_KEY_F)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_F),
        @(OSX_KEY_G)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_G),
        
        @(OSX_KEY_Q)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Q),
        @(OSX_KEY_W)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_W),
        @(OSX_KEY_E)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_E),
        @(OSX_KEY_R)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_R),
        @(OSX_KEY_T)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_T),
        
        @(OSX_KEY_1)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_1),
        @(OSX_KEY_2)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_2),
        @(OSX_KEY_3)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_3),
        @(OSX_KEY_4)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_4),
        @(OSX_KEY_5)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_5),
        
        @(OSX_KEY_0)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_0),
        @(OSX_KEY_9)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_9),
        @(OSX_KEY_8)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_8),
        @(OSX_KEY_7)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_7),
        @(OSX_KEY_6)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_6),
        
        @(OSX_KEY_P)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_P),
        @(OSX_KEY_O)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_O),
        @(OSX_KEY_I)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_I),
        @(OSX_KEY_U)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_U),
        @(OSX_KEY_Y)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Y),
        
        @(OSX_KEY_ENTER)        : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Enter),
        @(OSX_KEY_L)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_L),
        @(OSX_KEY_K)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_K),
        @(OSX_KEY_J)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_J),
        @(OSX_KEY_H)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_H),
        
        @(OSX_KEY_SPACE)        : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Space),
        @(OSX_KEY_CONTROL)      : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_SymbolShift), // Control key
        @(OSX_KEY_M)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_M),
        @(OSX_KEY_N)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_N),
        @(OSX_KEY_B)            : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_B),
        
        @(OSX_KEY_RIGHT_SQUARE) : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_InvVideo),
        @(OSX_KEY_LEFT_SQUARE)  : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_TrueVideo),
        @(OSX_KEY_QUOTE)        : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Quote),
        @(OSX_KEY_SEMI_COLON)   : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_SemiColon),
        @(OSX_KEY_COMMA)        : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Comma),
        @(OSX_KEY_MINUS)        : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Minus),
        @(OSX_KEY_PLUS)         : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Plus),
        @(OSX_KEY_PERIOD)       : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Period),
        @(OSX_KEY_TAB)          : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Edit),
        @(OSX_KEY_TILDA)        : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Graph),
        @(OSX_KEY_ESC)          : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Break), // ESC key
        @(OSX_KEY_BACKSPACE)    : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_Backspace),
        @(OSX_KEY_ARROW_UP)     : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_ArrowUp),
        @(OSX_KEY_ARROW_DOWN)   : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_ArrowDown),
        @(OSX_KEY_ARROW_LEFT)   : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_ArrowLeft),
        @(OSX_KEY_ARROW_RIGHT)  : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_ArrowRight),
        @(OSX_KEY_LEFT_ALT)     : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_ExtendMode), // Left Alt
        @(OSX_KEY_RIGHT_ALT)    : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_ExtendMode), // Right Alt
        @(OSX_KEY_CAPSLOCK)     : @((uint32_t)ZXSpectrum::eZXSpectrumKey::Key_CapsLock)
    };
}

- (void)keyDown:(NSEvent *)event
{
    if (!event.isARepeat && !(event.modifierFlags & NSEventModifierFlagCommand) && event.keyCode != 96)
    {
        emulationController->keyboardKeyDown((ZXSpectrum::eZXSpectrumKey)[[keyMappings_ objectForKey:@(event.keyCode)] unsignedIntValue]);
    }
}

- (void)keyUp:(NSEvent *)event
{
    if (event.keyCode == 96) // F5 key
    {
        [self loadFileWithURL:lastOpenedURL_ addToRecent:NO];
        return;
    }
    
    if (!event.isARepeat && !(event.modifierFlags & NSEventModifierFlagCommand))
    {
        emulationController->keyboardKeyUp((ZXSpectrum::eZXSpectrumKey)[[keyMappings_ objectForKey:@(event.keyCode)] unsignedIntValue]);
    }
}

- (void)flagsChanged:(NSEvent *)event
{
    if (!(event.modifierFlags & NSEventModifierFlagCommand))
    {
        switch (event.keyCode)
        {
            case 58:
            case 61:
                if (event.modifierFlags & NSEventModifierFlagOption)
                {
                    emulationController->keyboardKeyDown(ZXSpectrum::eZXSpectrumKey::Key_ExtendMode);
                } else {
                    emulationController->keyboardKeyUp(ZXSpectrum::eZXSpectrumKey::Key_ExtendMode);
                }
                break;
            case 57:
                if ((event.modifierFlags & NSEventModifierFlagCapsLock) || !(event.modifierFlags & NSEventModifierFlagCapsLock))
                {
                    emulationController->keyboardKeyDown(ZXSpectrum::eZXSpectrumKey::Key_CapsLock);
                } else {
                    emulationController->keyboardKeyUp(ZXSpectrum::eZXSpectrumKey::Key_CapsLock);
                }
                break;
            case 56:
            case 60:
                if (event.modifierFlags & NSEventModifierFlagShift)
                {
                    emulationController->keyboardKeyDown(ZXSpectrum::eZXSpectrumKey::Key_Shift);
                } else {
                    emulationController->keyboardKeyUp(ZXSpectrum::eZXSpectrumKey::Key_Shift);
                }
                break;
            case 59:
                if (event.modifierFlags & NSEventModifierFlagControl)
                {
                    emulationController->keyboardKeyDown(ZXSpectrum::eZXSpectrumKey::Key_SymbolShift);
                } else {
                    emulationController->keyboardKeyUp(ZXSpectrum::eZXSpectrumKey::Key_SymbolShift);
                }
            default:
                break;
        }
    }
}

#pragma mark - File Loading

- (void)loadFileWithURL:(NSURL *)url addToRecent:(BOOL)addToRecent
{
    Tape::FileResponse fileResponse;
    
    emulationController->pauseMachine();
    
    NSString *extension = [url.pathExtension uppercaseString];
    if (([extension isEqualToString:cZ80_EXTENSION] || [extension isEqualToString:cSNA_EXTENSION]))
    {
        int snapshotMachineType = emulationController->snapshotMachineInSnapshotWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]);
        if (emulationController->getMachineType() != snapshotMachineType)
        {
            self.defaults.machineSelectedModel = snapshotMachineType;
        }
    }

    fileResponse = emulationController->loadFileWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]);

    if (fileResponse.success)
    {
        lastOpenedURL_ = url;
        if (addToRecent)
        {
            [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:url];
        }
    } else {
        NSAlert *alert = [NSAlert new];
        alert.informativeText = [NSString stringWithFormat:[NSString stringWithCString:fileResponse.responseMsg.c_str()
                                                                              encoding:[NSString defaultCStringEncoding]], url.path];
        [alert addButtonWithTitle:@"OK"];
        [alert setAlertStyle:NSAlertStyleWarning];
        [alert runModal];
    }
    
    emulationController->resumeMachine();
}

#pragma mark - Restore Session

- (void)restoreSession
{
    if (NSURL *supportDirUrl = [self getSupportDirUrl])
    {
        // Load the last session file if it exists
        supportDirUrl = [supportDirUrl URLByAppendingPathComponent:cSESSION_FILE_NAME];
        [self loadFileWithURL:supportDirUrl addToRecent:NO];
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

- (void)tapeSetCurrentBlock:(NSInteger)blockIndex
{
    emulationController->tapePlayer->setCurrentBlock( static_cast<int>(blockIndex) );
    emulationController->tapePlayer->rewindBlock();
    emulationController->tapePlayer->stop();
}

static void tapeStatusCallback(int blockIndex, int bytes)
{
    std::cout << "Tape Callback: " << blockIndex << "\n";
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
    
    if (emulationController->getMachineType() == eZXSpectrum48)
    {
        [[saveAccessoryController_.exportPopup itemAtIndex:cSNA_SNAPSHOT_TYPE] setEnabled:YES];
        savePanel.allowedFileTypes = @[cZ80_EXTENSION, cSNA_EXTENSION];
    }
    else
    {
        [[saveAccessoryController_.exportPopup itemAtIndex:cSNA_SNAPSHOT_TYPE] setEnabled:NO];
        [saveAccessoryController_.exportPopup selectItemAtIndex:cZ80_SNAPSHOT_TYPE];
        savePanel.allowedFileTypes = @[cZ80_EXTENSION];
    }
    
    savePanel.accessoryView = saveAccessoryController_.view;
    
    [savePanel beginWithCompletionHandler:^(NSInteger result) {
        if (result == NSModalResponseOK)
        {
            ZXSpectrum::SnapshotData snapshot;
            NSURL *url = savePanel.URL;
            
            switch (saveAccessoryController_.exportType) {
                case cZ80_SNAPSHOT_TYPE:
                    snapshot = emulationController->snapshotCreateZ80();
                    url = [[url URLByDeletingPathExtension] URLByAppendingPathExtension:cZ80_EXTENSION];
                    break;
                    
                case cSNA_SNAPSHOT_TYPE:
                    snapshot = emulationController->snapshotCreateSNA();
                    url = [[url URLByDeletingPathExtension] URLByAppendingPathExtension:cSNA_EXTENSION];
                    break;
                    
                default:
                    break;
            }
            NSData *data = [NSData dataWithBytes:snapshot.data length:snapshot.length];
            [data writeToURL:url atomically:YES];
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
        emulationController->resetMachine(false);
    }
    else
    {
        emulationController->resetMachine(true);
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
    [debugWindowController_ showWindow:self];
}

- (IBAction)switchHexDecimal:(id)sender
{
    debugViewController_.hexFormat = (debugViewController_.hexFormat) ? NO : YES;
    [debugViewController_ updateViewDetails];
}

- (void)pauseMachine
{
    emulationController->pauseMachine();
    [self.audioCore stop];
}

- (void)startMachine
{
    emulationController->resumeMachine();
    [self.audioCore start];
}

#pragma mark - Tape Menu Items

- (IBAction)showTapeBrowser:(id)sender
{
    [tapeBrowserWindowController_ showWindow:self.view.window];
}

#pragma mark - Getters

- (BOOL)getDisplayReady
{
    return emulationController->isDisplayReady();
}

- (void *)getDebugger
{
    return emulationController->getDebugger();
}

- (BOOL)isEmulatorPaused
{
    return emulationController->isMachinePaused();
}

@end





