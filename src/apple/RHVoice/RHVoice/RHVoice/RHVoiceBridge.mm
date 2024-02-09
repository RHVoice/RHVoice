//
//  RHVoiceBridge.m
//  RHVoice
//
//  Created by Ihor Shevchuk on 02.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#import "RHVoiceBridge.h"

#import "RHVoiceBridge+Private.h"

#import <AVFAudio/AVAudioSession.h>
#import <AVFAudio/AVAudioPlayer.h>
#import <AVFAudio/AVSpeechSynthesis.h>

#import "NSFileManager+Additions.h"
#import "NSString+Additions.h"

#include "RHEventLoggerImpl.hpp"
#include "core/engine.hpp"

@interface RHVoiceBridge () {
    std::shared_ptr<RHVoice::engine> RHEngine;
}
@end

@implementation RHVoiceBridge

#pragma mark - Public

+ (instancetype)sharedInstance {
    static RHVoiceBridge *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[RHVoiceBridge alloc] init];
    });
    return sharedInstance;
}

- (void)setParams:(RHVoiceBridgeParams *)params {
    if(![self.params isEqual:params]) {
        _params = params;
    }
}

- (NSString *)version {
    if([self engine].get()) {
        return [NSString stringWithUTF8String:[self engine]->get_version().c_str()];
    }
    return @"Not initilzed!";
}

- (std::shared_ptr<RHVoice::engine>)engine {
    if(RHEngine.get() == nil) {
        [self createRHEngineWithParams:self.params];
    }
    
    return RHEngine;
}

#pragma mark - Private

+ (void)load {
    [[NSFileManager defaultManager] RHRemoveTempFolderIfNeededPath:[NSString RHTemporaryFolderPath]];
}

- (instancetype)init {
    self = [super init];
    if(self) {
        self.params = [RHVoiceBridgeParams defaultParams];
    }
    return self;
}

- (void)createRHEngineWithParams:(RHVoiceBridgeParams *)params {
    try {
        RHVoice::engine::init_params param;
        param.data_path = params.dataPath.UTF8String;
        if(params.logger != nil && [params.logger respondsToSelector:@selector(logAtLevel:message:)]) {
            param.logger = std::make_shared<RHEventLoggerImpl>();
        }
        
        RHEngine = std::make_shared<RHVoice::engine>(param);
    } catch (RHVoice::no_languages) {
        NSLog(@"No Languages folder is located at: %@", params.dataPath);
        NSLog(@"Please set  valid 'dataPath' property for %@ object. This folder has to contain 'languages' and 'voices' folders.", NSStringFromClass([self class]));
    }
}

- (const std::set<RHVoice::voice_profile>)voiceProfiles {
    if([self engine].get()) {
        return [self engine]->get_voice_profiles();
    }
    return std::set<RHVoice::voice_profile>();
}
@end
