//
//  ViewController.m
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "EmulationViewController.h"
#import "EmulationScene.h"
#import "ZXSpectrum48.hpp"

#pragma mark - Private Interface

@interface EmulationViewController()
{
    ZXSpectrum48        _machine;
    
    dispatch_queue_t    _emulationQueue;
    dispatch_source_t   _emulationTimer;
}
@end

#pragma mark - Implementation 

@implementation EmulationViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    [self initMachineWithRomAtPath:[[NSBundle mainBundle] pathForResource:@"48" ofType:@"ROM"]];
    
    EmulationScene *scene = (EmulationScene *)[SKScene nodeWithFileNamed:@"Scene"];
    scene.scaleMode = SKSceneScaleModeAspectFill;
    
    [self.skView presentScene:scene];
    
    self.skView.showsFPS = YES;
    self.skView.showsNodeCount = YES;
    
    [self setupTimersAndQueues];
    [self startEmulationTimer];
}

- (void)initMachineWithRomAtPath:(NSString *)romPath
{
    _machine.initialise((char *)[romPath cStringUsingEncoding:NSUTF8StringEncoding]);
}

- (void)setupTimersAndQueues
{
    _emulationQueue = dispatch_queue_create("EmulationQueue", nil);
    _emulationTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, _emulationQueue);
    dispatch_source_set_timer(_emulationTimer, DISPATCH_TIME_NOW, 0.02 * NSEC_PER_SEC, 0);
    
    dispatch_source_set_event_handler(_emulationTimer, ^{
        _machine.runFrame();
    });    
}

- (void)startEmulationTimer
{
    dispatch_resume(_emulationTimer);
}

- (void)stopEmulationTimer
{
    dispatch_suspend(_emulationTimer);
}

@end
