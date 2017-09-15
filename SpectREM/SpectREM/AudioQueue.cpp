//
//  AudioQueue.cpp
//  SpectREM
//
//  Created by Michael Daley on 12/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#include "AudioQueue.hpp"

#define kExponent 18
#define kUsed ((audioQueueBufferWritten - audioQueueBufferRead) & (audioQueueBufferCapacity - 1))
#define kSpace (audioQueueBufferCapacity - 1 - kUsed)
#define kSize (audioQueueBufferCapacity - 1)

#pragma mark - Audio Queue

AudioQueue::AudioQueue()
{
    audioQueueBufferCapacity = 1 << kExponent;
    audioQueueBuffer = new short[ audioQueueBufferCapacity << 2 ]();
}

AudioQueue::~AudioQueue()
{
    delete[] audioQueueBuffer;
}

// Write the supplied number of bytes into the queues buffer from the supplied buffer pointer
int AudioQueue::write(unsigned short *buffer, int count)
{
    if (!count) {
        return 0;
    }
    
    int t;
    int i;
    
    t = kSpace;
    
    if (count > t)
    {
        count = t;
    } else {
        t = count;
    }
    
    i = audioQueueBufferWritten;
    
    if ((i + count) > audioQueueBufferCapacity)
    {
        memcpy(audioQueueBuffer + i, buffer, (audioQueueBufferCapacity - i) << 1);
        buffer += audioQueueBufferCapacity - i;
        count -= audioQueueBufferCapacity - i;
        i = 0;
    }
    
    memcpy(audioQueueBuffer + i, buffer, count << 1);
    audioQueueBufferWritten = i + count;
    
    return t;
    
}

// Read the supplied number of bytes from the queues buffer into the supplied buffer pointer
int AudioQueue::read(unsigned short *buffer, int count)
{
    int t;
    int i;
    
    t = kUsed;
    
    if (count > t)
    {
        count = t;
    } else {
        t = count;
    }
    
    i = audioQueueBufferRead;
    
    if ((i + count) > audioQueueBufferCapacity)
    {
        memcpy(buffer, audioQueueBuffer + i, (audioQueueBufferCapacity - i) << 1);
        buffer += audioQueueBufferCapacity - i;
        count -= audioQueueBufferCapacity - i;
        i = 0;
    }
    
    memcpy(buffer, audioQueueBuffer + i, count << 1);
    audioQueueBufferRead = i + count;
    
    return t;
}

// Return the number of used samples in the buffer
int AudioQueue::bufferUsed()
{
    return kUsed;
}
