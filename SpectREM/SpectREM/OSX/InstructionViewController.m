//
//  InstructionViewController.m
//  SpectREM
//
//  Created by Mike on 10/07/2018.
//  Copyright © 2018 71Squared Ltd. All rights reserved.
//

#import "InstructionViewController.h"

@interface InstructionViewController ()

@property (unsafe_unretained) IBOutlet NSTextView *textView;


@end

@implementation InstructionViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    
    NSString *path = [[NSBundle mainBundle] pathForResource:@"Commands" ofType:@"rtf"];
    [self.textView readRTFDFromFile:path];
}

@end
