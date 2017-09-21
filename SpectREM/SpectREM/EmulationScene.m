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

static NSString * const cU_DISPLAY_FILTER_VALUE  =  @"u_displayFilterValue";
static NSString * const cU_DISPLAY_BORDER_SIZE =    @"u_displayBorderSize";
static NSString * const cU_DISPLAY_CURVATURE =      @"u_displayCurvature";
static NSString * const cU_DISPLAY_CONTRAST =       @"u_displayContrast";
static NSString * const cU_DISPLAY_BRIGHTNESS =     @"u_displayBrightness";
static NSString * const cU_DISPLAY_SATURATION =     @"u_displaySaturation";
static NSString * const cU_DISPLAY_SCAN_LINES =     @"u_displayScanLines";
static NSString * const cU_DISPLAY_RGB_OFFSET =     @"u_displayRGBOffset";
static NSString * const cU_DISPLAY_HORIZONTAL_SYNC =@"u_displayHorizontalSync";
static NSString * const cU_DISPLAY_SHOW_REFLECTION =@"u_displayShowReflection";
static NSString * const cU_DISPLAY_SHOW_VIGNETTE =  @"u_displayShowVignette";
static NSString * const cU_DISPLAY_VIGNETTE_X =     @"u_displayVignetteX";
static NSString * const cU_DISPLAY_VIGNETTE_Y =     @"u_displayVignetteY";

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
    [self.defaults addObserver:self forKeyPath:DisplayPixelFilterValue options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:DisplayBorderSize options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:DisplayCurvature options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:DisplayContrast options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:DisplayBrightness options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:DisplaySaturation options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:DisplayScanLines options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:DisplayRGBOffset options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:DisplayHorizontalSync options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:DisplayShowReflection options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:DisplayShowVignette options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:DisplayVignetteX options:NSKeyValueObservingOptionNew context:NULL];
    [self.defaults addObserver:self forKeyPath:DisplayVignetteY options:NSKeyValueObservingOptionNew context:NULL];

    // Apply current defaults
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayBorderSize] forAttributeNamed:cU_DISPLAY_BORDER_SIZE];
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayPixelFilterValue] forAttributeNamed:cU_DISPLAY_FILTER_VALUE];
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayCurvature] forAttributeNamed:cU_DISPLAY_CURVATURE];
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayContrast] forAttributeNamed:cU_DISPLAY_CONTRAST];
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayBrightness] forAttributeNamed:cU_DISPLAY_BRIGHTNESS];
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displaySaturation] forAttributeNamed:cU_DISPLAY_SATURATION];
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayScanLines] forAttributeNamed:cU_DISPLAY_SCAN_LINES];
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayRGBOffset] forAttributeNamed:cU_DISPLAY_RGB_OFFSET];
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayHorizontalSync] forAttributeNamed:cU_DISPLAY_HORIZONTAL_SYNC];
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayShowReflection] forAttributeNamed:cU_DISPLAY_SHOW_REFLECTION];
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayShowVignette] forAttributeNamed:cU_DISPLAY_SHOW_VIGNETTE];
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayVignetteX] forAttributeNamed:cU_DISPLAY_VIGNETTE_X];
    [_emulationScreen setValue:[SKAttributeValue valueWithFloat:self.defaults.displayVignetteY] forAttributeNamed:cU_DISPLAY_VIGNETTE_Y];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSKeyValueChangeKey,id> *)change context:(void *)context
{
    if ([keyPath isEqualToString:DisplayBorderSize])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_DISPLAY_BORDER_SIZE];
    }

    else if ([keyPath isEqualToString:DisplayPixelFilterValue])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_DISPLAY_FILTER_VALUE];
    }

    else if ([keyPath isEqualToString:DisplayCurvature])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_DISPLAY_CURVATURE];
    }

    else if ([keyPath isEqualToString:DisplayContrast])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_DISPLAY_CONTRAST];
    }

    else if ([keyPath isEqualToString:DisplayBrightness])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_DISPLAY_BRIGHTNESS];
    }

    else if ([keyPath isEqualToString:DisplaySaturation])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_DISPLAY_SATURATION];
    }

    else if ([keyPath isEqualToString:DisplayScanLines])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_DISPLAY_SCAN_LINES];
    }

    else if ([keyPath isEqualToString:DisplayRGBOffset])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_DISPLAY_RGB_OFFSET];
    }

    else if ([keyPath isEqualToString:DisplayHorizontalSync])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_DISPLAY_HORIZONTAL_SYNC];
    }

    else if ([keyPath isEqualToString:DisplayShowReflection])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_DISPLAY_SHOW_REFLECTION];
    }
    
    else if ([keyPath isEqualToString:DisplayShowVignette])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_DISPLAY_SHOW_VIGNETTE];
    }

    else if ([keyPath isEqualToString:DisplayVignetteX])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_DISPLAY_VIGNETTE_X];
    }

    else if ([keyPath isEqualToString:DisplayVignetteY])
    {
        [_emulationScreen setValue:[SKAttributeValue valueWithFloat:[change[NSKeyValueChangeNewKey] floatValue]] forAttributeNamed:cU_DISPLAY_VIGNETTE_Y];
    }
}

#pragma mark - Shader Setup

- (void)setupShaderAttributes
{
    shader.attributes = @[
                          [SKAttribute attributeWithName:cU_DISPLAY_BORDER_SIZE type:SKAttributeTypeFloat],
                          [SKAttribute attributeWithName:cU_DISPLAY_FILTER_VALUE type:SKAttributeTypeFloat],
                          [SKAttribute attributeWithName:cU_DISPLAY_CURVATURE type:SKAttributeTypeFloat],
                          [SKAttribute attributeWithName:cU_DISPLAY_SATURATION type:SKAttributeTypeFloat],
                          [SKAttribute attributeWithName:cU_DISPLAY_BRIGHTNESS type:SKAttributeTypeFloat],
                          [SKAttribute attributeWithName:cU_DISPLAY_CONTRAST type:SKAttributeTypeFloat],
                          [SKAttribute attributeWithName:cU_DISPLAY_SCAN_LINES type:SKAttributeTypeFloat],
                          [SKAttribute attributeWithName:cU_DISPLAY_RGB_OFFSET type:SKAttributeTypeFloat],
                          [SKAttribute attributeWithName:cU_DISPLAY_HORIZONTAL_SYNC type:SKAttributeTypeFloat],
                          [SKAttribute attributeWithName:cU_DISPLAY_SHOW_REFLECTION type:SKAttributeTypeFloat],
                          [SKAttribute attributeWithName:cU_DISPLAY_SHOW_VIGNETTE type:SKAttributeTypeFloat],
                          [SKAttribute attributeWithName:cU_DISPLAY_VIGNETTE_X type:SKAttributeTypeFloat],
                          [SKAttribute attributeWithName:cU_DISPLAY_VIGNETTE_Y type:SKAttributeTypeFloat]
                          ];
}

#pragma mark - Mouse Events

- (void)mouseDown:(NSEvent *)event
{
    [self.view.window performWindowDragWithEvent:event];
}



@end
