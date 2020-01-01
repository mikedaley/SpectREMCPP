//
//  EmulationViewController_Protocol.h
//  SpectREM
//
//  Created by Michael Daley on 07/01/2019.
//  Copyright © 2019 71Squared Ltd. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol EmulationProtocol <NSObject>

- (void)loadFileWithURL:(NSURL *)url addToRecent:(BOOL)addToRecent;

@optional
- (void)audioCallback:(int)inNumberFrames buffer:(int16_t *)buffer;

@end

NS_ASSUME_NONNULL_END
