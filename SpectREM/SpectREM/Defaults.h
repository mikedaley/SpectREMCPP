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

extern NSString * const MachineSelectedModel;
@property (nonatomic, assign) NSInteger machineSelectedModel;

extern NSString * const MachineTapeInstantLoad;
@property (nonatomic, assign) BOOL machineTapeInstantLoad;

extern NSString * const MachineUseAYSound;
@property(nonatomic, assign) BOOL machineUseAYSound;

#pragma mark - Display

extern NSString * const DisplayPixelFilterLevel;
@property (nonatomic, assign) CGFloat displayPixelFilterLevel;

extern NSString * const DisplayBorderSize;
@property (nonatomic, assign) CGFloat displayBorderSize;

#pragma mark - Audio

extern NSString * const AudioMasterVolume;
@property (nonatomic, assign) CGFloat audioMasterVolume;

extern NSString * const AudioHighPassFilter;
@property (nonatomic, assign) NSInteger audioHighPassFilter;

extern NSString * const AudioLowPassFilter;
@property (nonatomic, assign) NSInteger audioLowPassFilter;

#pragma mark - Init

+ (instancetype)defaults;
+ (void)setupDefaults;

@end
