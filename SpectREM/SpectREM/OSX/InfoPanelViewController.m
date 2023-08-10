//
//  InfoPanelViewController.m
//  SpectREM
//
//  Created by Mike on 10/09/2018.
//  Copyright © 2018 71Squared Ltd. All rights reserved.
//

#import "InfoPanelViewController.h"
#import <QuartzCore/QuartzCore.h>

@interface InfoPanelViewController ()

@end

@implementation InfoPanelViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.view.wantsLayer = YES;
    [(NSVisualEffectView*)self.view setBlendingMode:NSVisualEffectBlendingModeWithinWindow];
    [(NSVisualEffectView*)self.view setMaterial:NSVisualEffectMaterialHUDWindow];
    self.view.layer.cornerRadius = 6;
    [self.view setFrameOrigin:(NSPoint){0, -self.view.frame.size.height}];
    self.panelHidden = YES;
}

- (void)displayMessage:(NSString *)message duration:(NSInteger)duration
{
    NSPoint origin = self.view.frame.origin;
    if (self.view.frame.origin.y < 0)
    {
        origin.y = 4;
    }
    else
    {
        origin.y = -self.view.frame.size.height;
    }
    
    self.infoText.stringValue = message;
    
    if (self.panelHidden)
    {
        [self animateToOrigin:origin completionHandler:^{
            self.panelHidden = NO;
            dispatch_async(dispatch_get_main_queue(), ^{
                [self performSelector:@selector(hideMessage) withObject:self afterDelay:duration];
            });
        }];
    }
}

- (void)hideMessage
{
    NSPoint origin = self.view.frame.origin;
    origin.y = -self.view.frame.size.height;
    self.panelHidden = YES;
    [self animateToOrigin:origin completionHandler:^{
    }];
}

- (void)animateToOrigin:(NSPoint)origin completionHandler:(void (^)(void))completionHandler
{
    [NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
        context.duration = 0.35;
        context.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut];
        [self.view.animator setAlphaValue:1];
        [self.view.animator setFrameOrigin:origin];
    }  completionHandler:^{
        completionHandler();
    }];
}

@end
