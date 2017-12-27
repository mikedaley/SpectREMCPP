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
@property (strong) NSCharacterSet *nonHex;
@property (weak) IBOutlet NSTextField *spiPortTextField;

@end

@implementation ConfigurationViewController

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self)
    {
        _defaults = [Defaults defaults];
        _nonHex = [[NSCharacterSet characterSetWithCharactersInString: @"0123456789ABCDEFabcdef"] invertedSet];
}
    return self;
}

- (void)viewDidLoad
{
    self.spiPortTextField.stringValue = [NSString stringWithFormat:@"%lx", _defaults.spiPort];
}

- (void)controlTextDidChange:(NSNotification *)obj
{
    NSControl *textField = (NSTextField *)[obj object];
    NSString *hexString = [textField stringValue];
    
    NSRange nonHexRange = [hexString rangeOfCharacterFromSet: _nonHex];
    BOOL isHex = (nonHexRange.location == NSNotFound);
    
    if (!isHex)
    {
        textField.stringValue = [hexString substringToIndex:[hexString length] - 1];
    }
}

- (void)controlTextDidEndEditing:(NSNotification *)obj
{
    NSString *hexString = [(NSTextField *)[obj object] stringValue];
    unsigned int result = 0;
    NSScanner *scanner = [NSScanner scannerWithString:hexString];
    [scanner scanHexInt:&result];
    _defaults.spiPort = result;
}

@end
