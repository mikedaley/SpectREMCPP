//
//  AudioQueue.hpp
//  SpectREM
//
//  Created by Michael Daley on 12/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#ifndef AudioQueue_hpp
#define AudioQueue_hpp

#include <iostream>

using namespace std;

class AudioQueue
{

    
public:
    AudioQueue();
    ~AudioQueue();

    uint32_t             write(int16_t *buffer, uint32_t count);
    uint32_t             read(int16_t *buffer, uint32_t count);
    uint32_t             bufferUsed();

private:
    int16_t              *audioQueueBuffer;
    uint32_t             audioQueueBufferRead;
    uint32_t             audioQueueBufferWritten;
    uint32_t             audioQueueBufferCapacity;

};

#endif /* AudioQueue_hpp */
