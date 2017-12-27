//
//  Defaults.h
//  SpectREM
//
//  Created by Mike Daley on 15/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface Defaults : NSObject

#pragma mark - Machine

extern NSString * const MachineAcceleration;
@property (nonatomic, assign) CGFloat machineAcceleration;

extern NSString * const MachineSelectedModel;
@property (nonatomic, assign) NSInteger machineSelectedModel;

extern NSString * const MachineTapeInstantLoad;
@property (nonatomic, assign) BOOL machineTapeInstantLoad;

extern NSString * const MachineUseAYSound;
@property(nonatomic, assign) BOOL machineUseAYSound;

#pragma mark - Display

extern NSString * const DisplayPixelFilterValue;
@property (nonatomic, assign) CGFloat displayPixelFilterValue;

extern NSString * const DisplayBorderSize;
@property (nonatomic, assign) CGFloat displayBorderSize;

extern NSString * const DisplayCurvature;
@property (nonatomic, assign) CGFloat displayCurvature;

extern NSString * const DisplayContrast;
@property (nonatomic, assign) CGFloat displayContrast;

extern NSString * const DisplayBrightness;
@property (nonatomic, assign) CGFloat displayBrightness;

extern NSString * const DisplaySaturation;
@property (nonatomic, assign) CGFloat displaySaturation;

extern NSString * const DisplayScanLines;
@property (nonatomic, assign) CGFloat displayScanLines;

extern NSString * const DisplayScanLineSize;
@property (nonatomic, assign) CGFloat displayScanLineSize;

extern NSString * const DisplayRGBOffset;
@property (nonatomic, assign) CGFloat displayRGBOffset;

extern NSString * const DisplayHorizontalSync;
@property (nonatomic, assign) CGFloat displayHorizontalSync;

extern NSString * const DisplayShowReflection;
@property (nonatomic, assign) BOOL displayShowReflection;

extern NSString * const DisplayShowVignette;
@property (nonatomic, assign) BOOL displayShowVignette;

extern NSString * const DisplayVignetteX;
@property (nonatomic, assign) CGFloat displayVignetteX;

extern NSString * const DisplayVignetteY;
@property (nonatomic, assign) CGFloat displayVignetteY;

#pragma mark - Audio

extern NSString * const AudioMasterVolume;
@property (nonatomic, assign) CGFloat audioMasterVolume;

extern NSString * const AudioHighPassFilter;
@property (nonatomic, assign) NSInteger audioHighPassFilter;

extern NSString * const AudioLowPassFilter;
@property (nonatomic, assign) NSInteger audioLowPassFilter;

#pragma mark - SPI

extern NSString * const SPIPort;
@property (nonatomic, assign) NSUInteger spiPort;

#pragma mark - Init

+ (instancetype)defaults;
+ (instancetype)reload;
+ (void)setupDefaultsWithReset:(BOOL)reset;

@end
