//
//  ViewController.m
//  z80test
//
//  Created by Michael Daley on 30/07/2017.
//  Copyright © 2017 Mike Daley. All rights reserved.
//

#import "ViewController.h"
#import "ZXSpectrum48.hpp"

@interface ViewController()
{
    ZXSpectrum48 *_machine;

    dispatch_queue_t _emulationQueue;
    dispatch_source_t _emulationTimer;
}

@end

@implementation ViewController

- (void)dealloc
{

}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // Do any additional setup after loading the view.
    NSString *romPath = [[NSBundle mainBundle] pathForResource:@"48" ofType:@"ROM"];
    
    _machine = new ZXSpectrum48;
    _machine->initialise((char *)[romPath cStringUsingEncoding:NSUTF8StringEncoding]);
    
    _emulationQueue = dispatch_queue_create("EmulationQueue", nil);
    _emulationTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, _emulationQueue);
    dispatch_source_set_timer(_emulationTimer, DISPATCH_TIME_NOW, 0.02 * NSEC_PER_SEC, 0);

    dispatch_source_set_event_handler(_emulationTimer, ^{
        _machine->runFrame();
    });

    dispatch_resume(_emulationTimer);
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

@end
