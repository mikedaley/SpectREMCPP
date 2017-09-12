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

    int             write(signed short *buffer, int count);
    int             read(signed short *buffer, int count);
    int             bufferUsed();

private:
    short           *audioQueueBuffer;
    int             audioQueueBufferRead;
    int             audioQueueBufferWritten;
    int             audioQueueBufferCapacity;

};

#endif /* AudioQueue_hpp */
