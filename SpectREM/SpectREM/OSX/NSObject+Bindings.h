//
//  NSObject+Bindings.h
//  SpectREM
//
//  Created by Michael Daley on 2019-12-02.
//  Copyright © 2019 71Squared Ltd. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface NSObject_Bindings : NSObject

// Used to propogate changes to a property to objects that are bound to that property
-(void)propagateValue:(id)value forBinding:(NSString*)binding;

@end

NS_ASSUME_NONNULL_END
