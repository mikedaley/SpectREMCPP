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

#pragma mark - Private interface

@interface AudioCore ()
{
    
@public
    int             samplesPerFrame;
    UInt32          formatBytesPerFrame;
    UInt32          formatChannelsPerFrame;
    UInt32          formatBitsPerChannel;
    UInt32          formatFramesPerPacket;
    UInt32          formatBytesPerPacket;    
}

// Reference to the emulation queue that is being used to drive the emulation
@property (assign) dispatch_queue_t emulationQueue;

// Properties used to store the CoreAudio graph and nodes, including the high and low pass effects nodes
@property (assign) AUGraph graph;
@property (assign) AUNode outNode;
@property (assign) AUNode mixerNode;
@property (assign) AUNode converterNode;
@property (assign) AUNode lowPassNode;
@property (assign) AUNode highPassNode;
@property (assign) AudioUnit convertUnit;
@property (assign) AudioUnit mixerUnit;
@property (assign) AudioUnit lowPassFilterUnit;
@property (assign) AudioUnit highPassFilterUnit;

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

- (instancetype)initWithSampleRate:(int)sampleRate framesPerSecond:(float)fps machine:(ZXSpectrum *)machine
{
    self = [super init];
    if (self)
    {
        samplesPerFrame = sampleRate / fps;
    
        CheckError(NewAUGraph(&_graph), "NewAUGraph");
        
        // Output Node
        AudioComponentDescription componentDescription;
        componentDescription.componentType = kAudioUnitType_Output;
        componentDescription.componentSubType = kAudioUnitSubType_DefaultOutput;
        componentDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
        CheckError(AUGraphAddNode(_graph, &componentDescription, &_outNode), "AUGraphAddNode[kAudioUnitSubType_DefaultOutput]");
        
        // Mixer node
        componentDescription.componentType = kAudioUnitType_Mixer;
        componentDescription.componentSubType = kAudioUnitSubType_StereoMixer;
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
        formatBitsPerChannel = 16;
        formatChannelsPerFrame = 2;
        formatBytesPerFrame = 4;
        formatFramesPerPacket = 1;
        formatBytesPerPacket = 4;

        AudioStreamBasicDescription bufferFormat;
        bufferFormat.mFormatID = kAudioFormatLinearPCM;
        bufferFormat.mFormatFlags = kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian;
        bufferFormat.mSampleRate = sampleRate;
        bufferFormat.mBitsPerChannel = formatBitsPerChannel;
        bufferFormat.mChannelsPerFrame = formatChannelsPerFrame;
        bufferFormat.mBytesPerFrame = formatBytesPerFrame;
        bufferFormat.mFramesPerPacket = formatFramesPerPacket;
        bufferFormat.mBytesPerPacket = formatBytesPerPacket;
        
        CheckError(AUGraphNodeInfo(_graph, _converterNode, NULL, &_convertUnit), "AUGraphNodeInfo");
        CheckError(AudioUnitSetProperty(_convertUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &bufferFormat, sizeof(bufferFormat)), "AudioUnitSetProperty[kAudioUnitProperty_StreamFormat]");
        
        // Set the frames per slice property on the converter node
        uint32 framesPerSlice = 882;
        CheckError(AudioUnitSetProperty(_convertUnit, kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Input, 0, &framesPerSlice, sizeof(framesPerSlice)), "AudioUnitSetProperty[kAudioUnitProperty_MaximumFramesPerSlice]");

        // define the callback for rendering audio
        AURenderCallbackStruct renderCallbackStruct;
        renderCallbackStruct.inputProc = renderAudio;
        renderCallbackStruct.inputProcRefCon = machine;
        
        // Attach the audio callback to the converterNode
        CheckError(AUGraphSetNodeInputCallback(_graph, _converterNode, 0, &renderCallbackStruct), "AUGraphNodeInputCallback");
        
        CheckError(AUGraphInitialize(_graph), "AUGraphInitialize");
        
        // Get a reference to the graphics autio units
        AUGraphNodeInfo(_graph, _mixerNode, 0, &_mixerUnit);
        AUGraphNodeInfo(_graph, _lowPassNode, 0, &_lowPassFilterUnit);
        AUGraphNodeInfo(_graph, _highPassNode, 0, &_highPassFilterUnit);

        
        AudioUnitSetParameter(_lowPassFilterUnit, 0, kAudioUnitScope_Global, 0, 3500, 0);
        AudioUnitSetParameter(_highPassFilterUnit, 0, kAudioUnitScope_Global, 0, 1, 0);
        AudioUnitSetParameter(_mixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Output, 0, 1, 0);
    }
    return self;
}

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
    }
}

- (BOOL)isRunning
{
    Boolean running;
    AUGraphIsRunning(_graph, &running);
    return running;
}

#pragma mark - Audio Render

static OSStatus renderAudio(void *inRefCon, AudioUnitRenderActionFlags *ioActionFlags,
                            const AudioTimeStamp *inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames,
                            AudioBufferList *ioData)
{
    ZXSpectrum *machine = (ZXSpectrum *)inRefCon;
    
    // Grab the buffer that core audio has passed in and reset its contents to 0.
    // The format being used has 4 bytes per frame so multiply inNumberFrames by 4
    short *buffer = (short *)ioData->mBuffers[0].mData;
    memset(buffer, 0, inNumberFrames << 2);
    
    // Update the queue with the reset buffer
    machine->audioQueueRead(buffer, (inNumberFrames << 1));
    
    // Check if we have used a frames worth of buffer storage and if so then its time to generate another frame.
    if (machine->audioQueueBufferUsed() < 7680)
    {
        machine->generateFrame();
        machine->audioQueueWrite(machine->audioBuffer, 7680);
    }
    
    // Set the size of the buffer to be the number of frames requested by the Core Audio callback. This is
    // multiplied by the number of bytes per frame which is 4.
    ioData->mBuffers[0].mDataByteSize = (inNumberFrames << 2);
    
    return noErr;
}

// Routine to help detect and display OSStatus errors generated when using the Core Audio API
// It works out of the error is a C string to be displayed or an integer value. This allows them
// to be logged in a consistent manor.
// Taken from "Learning Core Audio" by Chris Adams and Kevin Avila
static void CheckError(OSStatus error, const char *operation)
{
    if (error == noErr)
    {
        return;
    }
    
    char str[20];
    *(UInt32 *) (str + 1) = CFSwapInt32HostToBig(error);
    if (isprint(str[1]) && isprint(str[2]) && isprint(str[3]) && isprint(str[4]))
    {
        str[0] = str[5] = '\'';
        str[6] = '\0';
    }
    else
    {
        sprintf(str, "%d", (int)error);
    }
    
    fprintf(stderr, "[Error] %s (%s)\n", operation, str);
    exit(1);
}

@end
