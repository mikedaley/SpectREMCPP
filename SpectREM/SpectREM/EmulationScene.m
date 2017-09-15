//
//  GameScene.m
//  SpectREM
//
//  Created by Mike Daley on 01/08/2017.
//  Copyright © 2017 71Squared Ltd. All rights reserved.
//

#import "EmulationScene.h"
#import "EmulationViewController.h"
#import "EmulationWindowController.h"
#import "Defaults.h"

#pragma mark - Constants

static NSString *const cU_FILTER_VALUE  =           @"u_filterValue";
static NSString *const cU_BORDER_SIZE   =           @"u_borderSize";

#pragma mark - Implementation 

@interface EmulationScene()

@property (strong) Defaults *defaults;

@end


@implementation EmulationScene
{
    SKShader *shader;
}

- (void)didMoveToView:(SKView *)view
{
    _emulationScreenTexture = [SKMutableTexture mutableTextureWithSize:CGSizeMake(320, 256)];
    _emulationScreenTexture.filteringMode = SKTextureFilteringLinear;
    
    _backingTexture = [SKMutableTexture mutableTextureWithSize:CGSizeMake(320, 256)];
    
    _emulationScreen = (SKSpriteNode *)[self childNodeWithName:@"//emulationScreen"];
    _emulationScreen.texture = _emulationScreenTexture;
    
    _backingNode = (SKSpriteNode *)[self childNodeWithName:@"//backingNode"];
    
    shader = [SKShader shaderWithFileNamed:@"PixelShader.fsh"];
    _emulationScreen.shader = shader;
    
    _defaults = [Defaults defaults];
    
    [self setupShaderAttributes];
    [self setupObservers];

}

#pragma mark - Scene Update

-(void)update:(CFTimeInterval)currentTime
{
    [_emulationScreenTexture modifyPixelDataWithBlock:^(void *pixelData, size_t lengthInBytes) {
        memcpy(pixelData, [self.emulationViewController getDisplayBuffer], lengthInBytes);
    }];
}

#pragma marl - Observers

- (void)setupObservers
{
    [self.defaults addObserver:self forKeyPath:DisplayPixelFilterLevel options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:DisplayBorderSize options:NSKeyValueObservingOptionNew context:NULL];
    
    // Apply current defaults
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayBorderSize] forAttributeNamed:cU_BORDER_SIZE];
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayPixelFilterLevel] forAttributeNamed:cU_FILTER_VALUE];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context
{
    if ([keyPath isEqualToString:DisplayBorderSize])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_BORDER_SIZE];
    }

    else if ([keyPath isEqualToString:DisplayPixelFilterLevel])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_FILTER_VALUE];
    }
}

#pragma mark - Shader Setup

- (void)setupShaderAttributes
{
    shader.attributes = @[
                          [SKAttribute attributeWithName:cU_BORDER_SIZE type:SKAttributeTypeFloat],
                          [SKAttribute attributeWithName:cU_FILTER_VALUE type:SKAttributeTypeFloat]
                          ];
}

#pragma mark - Mouse Events

- (void)mouseDown:(NSEvent *)event
{
    [self.view.window performWindowDragWithEvent:event];
}



@end
