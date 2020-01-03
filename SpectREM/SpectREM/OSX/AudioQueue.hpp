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

class AudioQueue
{

    
public:
    AudioQueue();
    ~AudioQueue();

    void            write(int16_t *buffer, uint32_t count);
    void            read(int16_t *buffer, uint32_t count);
    int             bufferUsed();

private:
    int16_t         *audioQueueBuffer;
    int             audioQueueBufferRead;
    int             audioQueueBufferWritten;
    int             audioQueueBufferCapacity;

};

#endif /* AudioQueue_hpp */
