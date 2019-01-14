//
//  AudioCore.m
//  ZXRetroEmulator
//
//  Created by Mike Daley on 03/09/2016.
//  Copyright © 2016 71Squared Ltd. All rights reserved.
//

#import <AVFoundation/AVFoundation.h>

#import "AudioCore.h"
#import "ZXSpectrum.hpp"
#import "Defaults.h"

#pragma mark - Private interface

@interface AudioCore ()
{
@public
    int             _samplesPerFrame;
    UInt32          _formatBytesPerFrame;
    UInt32          _formatChannelsPerFrame;
    UInt32          _formatBitsPerChannel;
    UInt32          _formatFramesPerPacket;
    UInt32          _formatBytesPerPacket;    
}

// Reference to the emulation queue that is being used to drive the emulation
@property (assign) dispatch_queue_t emulationQueue;

// Properties used to store the CoreAudio graph and nodes, including the high and low pass effects nodes
@property (assign) AUGraph      graph;
@property (assign) AUNode       outNode;
@property (assign) AUNode       mixerNode;
@property (assign) AUNode       converterNode;
@property (assign) AUNode       lowPassNode;
@property (assign) AUNode       highPassNode;
@property (assign) AudioUnit    convertUnit;
@property (assign) AudioUnit    mixerUnit;
@property (assign) AudioUnit    lowPassFilterUnit;
@property (assign) AudioUnit    highPassFilterUnit;

@property (strong) Defaults *defaults;

// Signature of the CoreAudio render callback. This is called by CoreAudio when it needs more data in its buffer.
// By using AudioQueue we can generate another new frame of data at 50.08 fps making sure that the audio stays in
// sync with the frames.
static OSStatus renderAudio(void *inRefCon,
                            AudioUnitRenderActionFlags *ioActionFlags,
                            const AudioTimeStamp *inTimeStamp,
                            UInt32 inBusNumber,
                            UInt32 inNumberFrames,
                            AudioBufferList *ioData);

@end

#pragma mark - Implementation

@implementation AudioCore

- (void)dealloc
{
    NSLog(@"Deallocating AudioCore");
    Boolean running;
    AUGraphIsRunning(_graph, &running);
    if (running)
    {
        CheckError(AUGraphStop(_graph), "AUGraphStop");
    }
    CheckError(AUGraphUninitialize(_graph), "AUGraphUninitilize");
    CheckError(AUGraphClose(_graph), "AUGraphClose");
    
}

- (instancetype)initWithSampleRate:(int)sampleRate framesPerSecond:(float)fps callback:(id <EmulationProtocol>)callback
{
    self = [super init];
    if (self)
    {
//        _defaults = [Defaults defaults];
        
        NSLog(@"Initialising AudioCore");
        
        _samplesPerFrame = sampleRate / fps;
    
#if TARGET_OS_IPHONE
        AVAudioSession *session = [AVAudioSession sharedInstance];
        NSError *error;
        [session setCategory:AVAudioSessionCategoryPlayback error:&error];
        [session setPreferredSampleRate:sampleRate error:&error];
        [session setPreferredIOBufferDuration:0.01 error:&error];
        [session setActive:YES error:&error];
#endif
        CheckError(NewAUGraph(&_graph), "NewAUGraph");
        
        // Output Node
        AudioComponentDescription componentDescription;
        componentDescription.componentType = kAudioUnitType_Output;
#if TARGET_OS_IPHONE
        componentDescription.componentSubType = kAudioUnitSubType_RemoteIO;
#else
        componentDescription.componentSubType = kAudioUnitSubType_DefaultOutput;
#endif
        componentDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
        
        AudioComponent comp = AudioComponentFindNext(NULL, &componentDescription);
        if (comp == NULL)
        {
            NSLog(@"NO DEFAULT OUTPUT NODE");
        }
        
        CheckError(AUGraphAddNode(_graph, &componentDescription, &_outNode), "AUGraphAddNode[kAudioUnitSubType_DefaultOutput]");
        
        // Mixer node
        componentDescription.componentType = kAudioUnitType_Mixer;
#if TARGET_OS_IPHONE
        componentDescription.componentSubType = kAudioUnitSubType_MultiChannelMixer;
#else
        componentDescription.componentSubType = kAudioUnitSubType_StereoMixer;
#endif
        componentDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
        CheckError(AUGraphAddNode(_graph, &componentDescription, &_mixerNode), "AUGraphAddNode[kAudioUnitSubType_StereoMixer]");
        CheckError(AUGraphConnectNodeInput(_graph, _mixerNode, 0, _outNode, 0), "AUGraphConnectNodeInput[kAudioUnitSubType_StereoMixer]");

        // Low pass effect node
        componentDescription.componentType = kAudioUnitType_Effect;
        componentDescription.componentSubType = kAudioUnitSubType_LowPassFilter;
        componentDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
        CheckError(AUGraphAddNode(_graph, &componentDescription, &_lowPassNode), "AUGraphAddNode[kAudioUnitSubType_LowPassFilter]");
        CheckError(AUGraphConnectNodeInput(_graph, _lowPassNode, 0, _mixerNode, 0), "AUGraphConnectNodeInput[kAudioUnitSubType_LowPassFilter]");
        
        // High pass effect node
        componentDescription.componentType = kAudioUnitType_Effect;
        componentDescription.componentSubType = kAudioUnitSubType_HighPassFilter;
        componentDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
        CheckError(AUGraphAddNode(_graph, &componentDescription, &_highPassNode), "AUGraphAddNode[kAudioUnitSubType_HighPassFilter]");
        CheckError(AUGraphConnectNodeInput(_graph, _highPassNode, 0, _lowPassNode, 0), "AUGraphConnectNodeInput[kAudioUnitSubType_HighPassFilter]");
        
        // Converter node
        componentDescription.componentType = kAudioUnitType_FormatConverter;
        componentDescription.componentSubType = kAudioUnitSubType_AUConverter;
        componentDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
        CheckError(AUGraphAddNode(_graph, &componentDescription, &_converterNode), "AUGraphAddNode[kAudioUnitSubType_AUConverter]");
        CheckError(AUGraphConnectNodeInput(_graph, _converterNode, 0, _highPassNode, 0), "AUGraphConnectNodeInput[kAudioUnitSubType_AUConverter]");

        CheckError(AUGraphOpen(_graph), "AUGraphOpen");
        
        // Buffer format
        _formatBitsPerChannel = 16;
        _formatChannelsPerFrame = 2;
        _formatBytesPerFrame = 4;
        _formatFramesPerPacket = 1;
        _formatBytesPerPacket = 4;

        AudioStreamBasicDescription bufferFormat;
        bufferFormat.mFormatID = kAudioFormatLinearPCM;
        bufferFormat.mFormatFlags = kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian;
        bufferFormat.mSampleRate = sampleRate;
        bufferFormat.mBitsPerChannel = _formatBitsPerChannel;
        bufferFormat.mChannelsPerFrame = _formatChannelsPerFrame;
        bufferFormat.mBytesPerFrame = _formatBytesPerFrame;
        bufferFormat.mFramesPerPacket = _formatFramesPerPacket;
        bufferFormat.mBytesPerPacket = _formatBytesPerPacket;
        
        CheckError(AUGraphNodeInfo(_graph, _converterNode, NULL, &_convertUnit), "AUGraphNodeInfo");
        CheckError(AudioUnitSetProperty(_convertUnit, kAudioUnitProperty_StreamFormat,
                                        kAudioUnitScope_Input, 0, &bufferFormat,
                                        sizeof(bufferFormat)), "AudioUnitSetProperty[kAudioUnitProperty_StreamFormat]");

        // Set the frames per slice property on the converter node
        uint framesPerSlice = sampleRate / fps;
//        framesPerSlice = 1024;
        CheckError(AudioUnitSetProperty(_convertUnit, kAudioUnitProperty_MaximumFramesPerSlice,
                                        kAudioUnitScope_Global, 0, &framesPerSlice,
                                        sizeof(framesPerSlice)), "AudioUnitSetProperty[kAudioUnitProperty_MaximumFramesPerSlice]");

        // define the callback for rendering audio
        AURenderCallbackStruct renderCallbackStruct;
        renderCallbackStruct.inputProc = renderAudio;
        renderCallbackStruct.inputProcRefCon = (__bridge void*)callback;
        
        // Attach the audio callback to the converterNode
        CheckError(AUGraphSetNodeInputCallback(_graph, _converterNode, 0, &renderCallbackStruct), "AUGraphNodeInputCallback");
        
        CheckError(AUGraphInitialize(_graph), "AUGraphInitialize");
        
        // Get a reference to the graphics autio units
        AUGraphNodeInfo(_graph, _mixerNode, 0, &_mixerUnit);
        AUGraphNodeInfo(_graph, _lowPassNode, 0, &_lowPassFilterUnit);
        AUGraphNodeInfo(_graph, _highPassNode, 0, &_highPassFilterUnit);
        
    }
    return self;
}

#pragma mark - Audio Core Controler

- (void)stop
{
    Boolean running;
    AUGraphIsRunning(_graph, &running);
    if (running)
    {
        CheckError(AUGraphStop(_graph), "AUGraphStop");
    }
}

- (void)start
{
    Boolean running;
    AUGraphIsRunning(_graph, &running);
    if (!running)
    {
        CheckError(AUGraphStart(_graph), "AUGraphStart");
        AUGraphIsRunning(_graph, &running);
    }
}

- (BOOL)isRunning
{
    Boolean running;
    AUGraphIsRunning(_graph, &running);
    return running;
}

- (void)setAudioMasterVolume:(float)audioMasterVolume
{
    AudioUnitSetParameter(_mixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Output, 0, audioMasterVolume, 0);
}

- (void)setAudioLowPassFilter:(float)audioLowPassFilter
{
    AudioUnitSetParameter(_lowPassFilterUnit, 0, kAudioUnitScope_Global, 0, audioLowPassFilter, 0);
}

- (void)setAudioHighPassFilter:(float)audioHighPassFilter
{
    AudioUnitSetParameter(_highPassFilterUnit, 0, kAudioUnitScope_Global, 0, audioHighPassFilter, 0);
}

#pragma mark - Audio Render

static OSStatus renderAudio(void *inRefCon,
                            AudioUnitRenderActionFlags *ioActionFlags,
                            const AudioTimeStamp *inTimeStamp,
                            UInt32 inBusNumber,
                            UInt32 inNumberFrames,
                            AudioBufferList *ioData)
{
    const id <EmulationProtocol> callback = (__bridge id <EmulationProtocol>)inRefCon;
    
    // Grab the buffer that core audio has passed in.
    int16_t *buffer = static_cast<int16_t *>(ioData->mBuffers[0].mData);
    
    // Reset the buffer to prevent any odd noises being played when a machine starts up
    memset(buffer, 0, ioData->mBuffers[0].mDataByteSize);
    
    [callback audioCallback:inNumberFrames buffer:buffer];
    
    // Set the size of the buffer to be the number of frames requested by the Core Audio callback. This is
    // multiplied by the number of bytes per frame which is 4.
    ioData->mBuffers[0].mDataByteSize = (inNumberFrames << 2);
    
    return noErr;
}

static void CheckError(OSStatus error, const char *operation)
{
    if (error == noErr)
    {
        return;
    }

    cout << "AUDIO ERROR:  " << operation << endl;
    exit(1);
}

@end
