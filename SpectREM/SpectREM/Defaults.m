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
NSString * const MachineUseAYSound = @"machineUseAYSound";

#pragma mark - Display

NSString * const DisplayPixelFilterValue = @"displayPixelFilterValue";
NSString * const DisplayBorderSize = @"displayBorderSize";
NSString * const DisplayCurvature = @"displayCurvature";
NSString * const DisplayContrast = @"displayContrast";
NSString * const DisplayBrightness = @"displayBrightness";
NSString * const DisplaySaturation = @"displaySaturation";
NSString * const DisplayScanLines = @"displayScanLines";
NSString * const DisplayScanLineSize = @"displayScanLineSize";
NSString * const DisplayRGBOffset = @"displayRGBOffset";
NSString * const DisplayHorizontalSync = @"displayHorizontalSync";
NSString * const DisplayShowReflection = @"displayShowReflection";
NSString * const DisplayShowVignette = @"displayShowVignette";
NSString * const DisplayVignetteX = @"displayVignetteX";
NSString * const DisplayVignetteY = @"displayVignetteY";

#pragma mark - Audio

NSString * const AudioMasterVolume = @"audioMasterVolume";
NSString * const AudioHighPassFilter = @"audioHighPassFilter";
NSString * const AudioLowPassFilter = @"audioLowPassFilter";

@implementation Defaults

+ (void)setupDefaultsWithReset:(BOOL)reset
{
    NSUserDefaults *userDefaults = [NSUserDefaults standardUserDefaults];

    NSDictionary *defaults = @{
                               MachineSelectedModel : @(0),
                               MachineTapeInstantLoad : @YES,
                               MachineUseAYSound: @YES,
                               
                               DisplayPixelFilterValue : @(0.15),
                               DisplayBorderSize : @(32),
                               DisplayCurvature : @(0.0),
                               DisplayContrast : @(0.75),
                               DisplayBrightness : @(1.0),
                               DisplaySaturation : @(1.0),
                               DisplayScanLines : @(0.0),
                               DisplayScanLineSize : @(960),
                               DisplayRGBOffset : @(0.0),
                               DisplayHorizontalSync : @(0),
                               DisplayShowReflection : @NO,
                               DisplayShowVignette : @NO,
                               DisplayVignetteX : @(1.0),
                               DisplayVignetteY : @(0.25),
                               
                               AudioMasterVolume : @(1.5),
                               AudioHighPassFilter : @(0),
                               AudioLowPassFilter : @(5000)
                               };
    
    for (NSString *key in defaults)
    {
        if (![userDefaults objectForKey:key] || reset)
        {
            [userDefaults setValue:defaults[key] forKey:key];
        }
    }
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

+ (instancetype)reload
{
    static Defaults *defaults = nil;
    defaults = [Defaults new];
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
        _machineUseAYSound = [[userDefaults valueForKey:MachineUseAYSound] boolValue];
        
        _displayPixelFilterValue = [[userDefaults valueForKey:DisplayPixelFilterValue] floatValue];
        _displayBorderSize = [[userDefaults valueForKey:DisplayBorderSize] integerValue];
        _displayCurvature = [[userDefaults valueForKey:DisplayCurvature] floatValue];
        _displayContrast = [[userDefaults valueForKey:DisplayContrast] floatValue];
        _displayBrightness = [[userDefaults valueForKey:DisplayBrightness] floatValue];
        _displaySaturation = [[userDefaults valueForKey:DisplaySaturation] floatValue];
        _displayRGBOffset = [[userDefaults valueForKey:DisplayRGBOffset] floatValue];
        _displayScanLines = [[userDefaults valueForKey:DisplayScanLines] floatValue];
        _displayScanLineSize = [[userDefaults valueForKey:DisplayScanLineSize] floatValue];
        _displayHorizontalSync = [[userDefaults valueForKey:DisplayHorizontalSync] floatValue];
        _displayShowReflection = [[userDefaults valueForKey:DisplayShowReflection] boolValue];
        _displayShowVignette = [[userDefaults valueForKey:DisplayShowVignette] boolValue];
        _displayVignetteX = [[userDefaults valueForKey:DisplayVignetteX] floatValue];
        _displayVignetteY = [[userDefaults valueForKey:DisplayVignetteY] floatValue];

        _audioMasterVolume = [[userDefaults valueForKey:AudioMasterVolume] floatValue];
        _audioHighPassFilter = [[userDefaults valueForKey:AudioHighPassFilter] integerValue];
        _audioLowPassFilter = [[userDefaults valueForKey:AudioLowPassFilter] integerValue];
    }
    return self;
}

#pragma mark - Machine

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

- (void)setMachineUseAYSound:(BOOL)machineUseAYSound
{
    _machineUseAYSound = machineUseAYSound;
    [[NSUserDefaults standardUserDefaults] setBool:machineUseAYSound forKey:MachineUseAYSound];
}

#pragma mark - Display

- (void)setDisplayPixelFilterValue:(CGFloat)displayPixelFilterValue
{
    _displayPixelFilterValue = displayPixelFilterValue;
    [[NSUserDefaults standardUserDefaults] setFloat:displayPixelFilterValue forKey:DisplayPixelFilterValue];
}

- (void)setDisplayBorderSize:(double)displayBorderSize
{
    _displayBorderSize = displayBorderSize;
    [[NSUserDefaults standardUserDefaults] setInteger:displayBorderSize forKey:DisplayBorderSize];
}

- (void)setDisplayCurvature:(CGFloat)displayCurvature
{
    _displayCurvature = displayCurvature;
    [[NSUserDefaults standardUserDefaults] setFloat:displayCurvature forKey:DisplayCurvature];
}

- (void)setDisplayContrast:(CGFloat)displayContrast
{
    _displayContrast = displayContrast;
    [[NSUserDefaults standardUserDefaults] setFloat:displayContrast forKey:DisplayContrast];
}

- (void)setDisplayBrightness:(CGFloat)displayBrightness
{
    _displayBrightness = displayBrightness;
    [[NSUserDefaults standardUserDefaults] setFloat:displayBrightness forKey:DisplayBrightness];
}

- (void)setDisplaySaturation:(CGFloat)displaySaturation
{
    _displaySaturation = displaySaturation;
    [[NSUserDefaults standardUserDefaults] setFloat:displaySaturation forKey:DisplaySaturation];
}

- (void)setDisplayRGBOffset:(CGFloat)displayRGBOffset
{
    _displayRGBOffset = displayRGBOffset;
    [[NSUserDefaults standardUserDefaults] setFloat:displayRGBOffset forKey:DisplayRGBOffset];
}

- (void)setDisplayScanLines:(CGFloat)displayScanLines
{
    _displayScanLines = displayScanLines;
    [[NSUserDefaults standardUserDefaults] setFloat:displayScanLines forKey:DisplayScanLines];
}

- (void)setDisplayScanLineSize:(CGFloat)displayScanLineSize
{
    _displayScanLineSize = displayScanLineSize;
    [[NSUserDefaults standardUserDefaults] setFloat:displayScanLineSize forKey:DisplayScanLineSize];
}

- (void)setDisplayHorizontalSync:(CGFloat)displayHorizontalSync
{
    _displayHorizontalSync = displayHorizontalSync;
    [[NSUserDefaults standardUserDefaults] setFloat:displayHorizontalSync forKey:DisplayHorizontalSync];
}

- (void)setDisplayShowReflection:(BOOL)displayShowReflection
{
    _displayShowReflection = displayShowReflection;
    [[NSUserDefaults standardUserDefaults] setBool:displayShowReflection forKey:DisplayShowReflection];
}

- (void)setDisplayShowVignette:(BOOL)displayShowVignette
{
    _displayShowVignette = displayShowVignette;
    [[NSUserDefaults standardUserDefaults] setBool:displayShowVignette forKey:DisplayShowVignette];
}

- (void)setDisplayVignetteX:(CGFloat)displayVignetteX
{
    _displayVignetteX = displayVignetteX;
    [[NSUserDefaults standardUserDefaults] setFloat:displayVignetteX forKey:DisplayVignetteX];
}

- (void)setDisplayVignetteY:(CGFloat)displayVignetteY
{
    _displayVignetteY = displayVignetteY;
    [[NSUserDefaults standardUserDefaults] setFloat:displayVignetteY forKey:DisplayVignetteY];
}

#pragma mark - Audio

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
