//
//  Defaults.m
//  SpectREM
//
//  Created by Mike Daley on 15/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "Defaults.h"

#pragma mark - Machine

NSString * const MachineSelectedModel = @"machineSelectedModel";
NSString * const MachineTapeInstantLoad = @"machineTapeInstantLoad";

#pragma mark - Display

NSString * const DisplayPixelFilterLevel = @"displayPixelFilterLevel";
NSString * const DisplayBorderSize = @"displayBorderSize";

#pragma mark - Audio

NSString * const AudioMasterVolume = @"audioMasterVolume";
NSString * const AudioHighPassFilter = @"audioHighPassFilter";
NSString * const AudioLowPassFilter = @"audioLowPassFilter";

@implementation Defaults

+ (void)setupDefaults
{
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
    
    NSDictionary *defaults = @{
                               MachineSelectedModel : @(0),
                               MachineTapeInstantLoad : @YES,
                               DisplayPixelFilterLevel : @(0.0),
                               DisplayBorderSize : @(32),
                               AudioMasterVolume : @(1.5),
                               AudioHighPassFilter : @(0),
                               AudioLowPassFilter : @(5000)
                               };
    
    [userDefaults registerDefaults:defaults];
}

+ (instancetype)defaults
{
    static Defaults *defaults = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        defaults = [Defaults new];
    });
    return defaults;
}

- (instancetype)init
{
    self = [super init];
    if (self)
    {
        NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];
        
        _machineSelectedModel = [[userDefaults valueForKey:MachineSelectedModel] integerValue];
        _machineTapeInstantLoad = [[userDefaults valueForKey:MachineTapeInstantLoad] boolValue];
        
        _displayPixelFilterLevel = [[userDefaults valueForKey:DisplayPixelFilterLevel] floatValue];
        _displayBorderSize = [[userDefaults valueForKey:DisplayBorderSize] integerValue];
        
        _audioMasterVolume = [[userDefaults valueForKey:AudioMasterVolume] floatValue];
        _audioHighPassFilter = [[userDefaults valueForKey:AudioHighPassFilter] integerValue];
        _audioLowPassFilter = [[userDefaults valueForKey:AudioLowPassFilter] integerValue];
    }
    return self;
}

- (void)setMachineSelectedModel:(NSInteger)machineSelectedModel
{
    _machineSelectedModel = machineSelectedModel;
    [[NSUserDefaults standardUserDefaults] setInteger:machineSelectedModel forKey:MachineSelectedModel];
}

- (void)setMachineTapeInstantLoad:(BOOL)machineTapeInstantLoad
{
    _machineTapeInstantLoad = machineTapeInstantLoad;
    [[NSUserDefaults standardUserDefaults] setBool:machineTapeInstantLoad forKey:MachineTapeInstantLoad];
}

- (void)setDisplayPixelFilterLevel:(CGFloat)displayPixelFilterLevel
{
    _displayPixelFilterLevel = displayPixelFilterLevel;
    [[NSUserDefaults standardUserDefaults] setFloat:displayPixelFilterLevel forKey:DisplayPixelFilterLevel];
}

- (void)setDisplayBorderSize:(double)displayBorderSize
{
    _displayBorderSize = displayBorderSize;
    [[NSUserDefaults standardUserDefaults] setInteger:displayBorderSize forKey:DisplayBorderSize];
}

- (void)setAudioMasterVolume:(CGFloat)audioMasterVolume
{
    _audioMasterVolume = audioMasterVolume;
    [[NSUserDefaults standardUserDefaults] setFloat:audioMasterVolume forKey:AudioMasterVolume];
}

- (void)setAudioHighPassFilter:(NSInteger)audioHighPassFilter
{
    _audioHighPassFilter = audioHighPassFilter;
    [[NSUserDefaults standardUserDefaults] setInteger:audioHighPassFilter forKey:AudioHighPassFilter];
}

- (void)setAudioLowPassFilter:(NSInteger)audioLowPassFilter
{
    _audioLowPassFilter = audioLowPassFilter;
    [[NSUserDefaults standardUserDefaults] setInteger:audioLowPassFilter forKey:AudioLowPassFilter];
}

@end
