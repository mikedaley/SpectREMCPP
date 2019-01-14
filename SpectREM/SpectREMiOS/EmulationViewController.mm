//
//  GameViewController.m
//  SpectREMiOS
//
//  Created by Michael Daley on 07/01/2019.
//  Copyright © 2019 71Squared Ltd. All rights reserved.
//

#import "EmulationViewController.h"

#import "AudioQueue.hpp"
#import "AudioCore.h"
#import "ZXSpectrum.hpp"
#import "ZXSpectrum48.hpp"
#import "ZXSpectrum128.hpp"
#import "Tape.hpp"
#import "Debug.hpp"
#import "MetalRenderer.h"
#import "SharedConstants.h"

#pragma mark - Constants

uint32_t const cAUDIO_SAMPLE_RATE = 44100;
uint32_t const cFRAMES_PER_SECOND = 50;
NSString  *const cSESSION_FILE_NAME = @"session.z80";

@implementation EmulationViewController
{
@public
    ZXSpectrum                      *_machine;
    Debug                           *_debugger;
    Tape                            *_tape;
    NSString                        *_mainBundlePath;
    bool                            _configViewVisible;
    
    AudioQueue                      *_audioQueue;
    DebugOpCallbackBlock            _debugBlock;
    
    UIStoryboard                    *_storyBoard;
    
    NSTimer                         *_accelerationTimer;
    
    MTKView                         *_metalView;
    MetalRenderer                   *_metalRenderer;
    
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    _metalView = (MTKView *)self.view;
    _metalView.contentScaleFactor = 2.0;
    _metalView.device = MTLCreateSystemDefaultDevice();
    _metalView.backgroundColor = UIColor.blackColor;

    if(!_metalView.device)
    {
        NSLog(@"Metal is not supported on this device");
        self.view = [[UIView alloc] initWithFrame:self.view.frame];
        return;
    }

    _metalRenderer = [[MetalRenderer alloc] initWithMetalKitView:_metalView];

    if (!_metalRenderer)
    {
        NSLog(@"Renderer failed init");
        return;
    }

    [_metalRenderer mtkView:_metalView drawableSizeWillChange:_metalView.bounds.size];
    _metalView.delegate = _metalRenderer;
    
    _mainBundlePath = [[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/"];
    _storyBoard = [UIStoryboard storyboardWithName:@"Main" bundle:nil];

    // The AudioCore uses the sound buffer to identify when a new frame should be drawn for accurate timing. The AudioQueue
    // is used to help measure usage of the audio buffer
    _audioQueue = new AudioQueue();
    self.audioCore = [[AudioCore alloc] initWithSampleRate:cAUDIO_SAMPLE_RATE framesPerSecond:cFRAMES_PER_SECOND callback:(id <EmulationProtocol>)self];

    [self.audioCore setAudioMasterVolume:1.0];
    [self.audioCore setAudioLowPassFilter:5000.0];
    [self.audioCore setAudioHighPassFilter:1];
    
    //Create a tape instance
    _tape = new Tape(tapeStatusCallback);
    
    [self initMachineWithRomPath:_mainBundlePath machineType:eZXSpectrum48];
    
//    [self restoreSession];
    
    _mainBundlePath = [_mainBundlePath stringByAppendingString:@"session.z80"];
    [self loadFileWithURL:[NSURL URLWithString:_mainBundlePath] addToRecent:NO];
    [self startMachine];
    _machine->emuUseAYSound = true;
}

//- (void)restoreSession
//{
//    if (NSURL *supportDirUrl = [self getSupportDirUrl])
//    {
//        // Load the last session file it if exists
//        supportDirUrl = [supportDirUrl URLByAppendingPathComponent:cSESSION_FILE_NAME];
//        if ([[NSFileManager defaultManager] fileExistsAtPath:supportDirUrl.path])
//        {
//            [self loadFileWithURL:supportDirUrl addToRecent:NO];
//        }
//    }
//}

- (void)loadFileWithURL:(NSURL *)url addToRecent:(BOOL)addToRecent
{
    BOOL success = NO;
    
    NSString *urlPath = [url.pathExtension uppercaseString];
    if (([urlPath isEqualToString:cZ80_EXTENSION] || [urlPath isEqualToString:cSNA_EXTENSION]))
    {
        int snapshotMachineType = _machine->snapshotMachineInSnapshotWithPath([url.path cStringUsingEncoding:NSUTF8StringEncoding]);
        if (_machine->machineInfo.machineType != snapshotMachineType)
        {
//            self.defaults.machineSelectedModel = snapshotMachineType;
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
    
//    if (addToRecent && success)
//    {
//        [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:url];
//    }
    
//    if (!success)
//    {
//        NSWindow *window = [[NSApplication sharedApplication] mainWindow];
//        NSAlert *alert = [NSAlert new];
//        alert.informativeText = [NSString stringWithFormat:@"An error occurred trying to open %@", url.path];
//        [alert addButtonWithTitle:@"OK"];
//        [alert beginSheetModalForWindow:window completionHandler:^(NSModalResponse returnCode) {
//            // No need to do anything
//        }];
//    }
}

#pragma mark - Audio Callback

- (void)audioCallback:(int)inNumberFrames buffer:(int16_t *)buffer
{
    if (_machine)
    {
        const uint32_t b = (cAUDIO_SAMPLE_RATE / cFRAMES_PER_SECOND) * 2;
        
        _audioQueue->read(buffer, (inNumberFrames << 1));
        
        // Check if we have used a frames worth of buffer storage and if so then its time to generate another frame.
        if (_audioQueue->bufferUsed() <= b)
        {
            _machine->generateFrame();
            [_metalRenderer updateTextureData:_machine->getScreenBuffer()];
            _audioQueue->write(_machine->audioBuffer, b);
        }
    }
}

- (void)updateDisplay
{
    [_metalRenderer updateTextureData:_machine->displayBuffer];
}

#pragma mark - View Methods

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
//        [_infoPanelViewController displayMessage:@"ZX Spectrum 48k" duration:5];
    }
    else if (machineType == eZXSpectrum128)
    {
        _machine = new ZXSpectrum128(_tape);
//        [_infoPanelViewController displayMessage:@"ZX Spectrum 128k" duration:5];
    }
    else
    {
        NSLog(@"Unknown machine type!");
        return;
    }
    
    _machine->initialise((char *)[romPath cStringUsingEncoding:NSUTF8StringEncoding]);
    
    _debugger = new Debug;
    _debugger->registerMachine(_machine);
    
    __block EmulationViewController *blockSelf = self;
    
    _debugBlock = (^bool(unsigned short address, uint8_t operation) {
        
        if (blockSelf->_debugger->checkForBreakpoint(address, operation))
        {
            [blockSelf pauseMachine];
            return true;
        }
        return false;
        
    });
    
    _machine->registerDebugOpCallback( _debugBlock );
    
    // Once a machine instance has been created we need to apply the defaults to that instance
//    [self applyDefaults];
    
    [self.audioCore start];
    _machine->resume();
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

static void tapeStatusCallback(int blockIndex, int bytes)
{
    cout << "Tape Callback: " << blockIndex << endl;
    [[NSNotificationCenter defaultCenter] postNotificationName:@"TAPE_CHANGED_NOTIFICATION" object:NULL];
}

- (IBAction)resetMachine:(id)sender {
    _machine->resetMachine(true);
}
@end
