//
//  ConfigurationViewController.m
//  SpectREM
//
//  Created by Mike Daley on 06/09/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "ConfigurationViewController.h"

#pragma mark - Key Path Constants


@interface ConfigurationViewController ()

@property (strong) NSUserDefaultsController *userDefaultController;

@end

@implementation ConfigurationViewController

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self)
    {
        _defaults = [Defaults defaults];
    }
    return self;
}

@end
