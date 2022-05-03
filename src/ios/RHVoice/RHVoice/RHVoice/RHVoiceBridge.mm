//
//  RHVoiceBridge.m
//  RHVoice
//
//  Created by Ihor Shevchuk on 02.05.2022.
//

#import "RHVoiceBridge.h"

#import <AVFAudio/AVAudioSession.h>
#import <AVFAudio/AVAudioPlayer.h>
#import <AVFAudio/AVSpeechSynthesis.h>

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

- (void)setDataPath:(NSString *)dataPath {
    if(![_dataPath isEqual:dataPath]) {
        _dataPath = dataPath;
        [self createRHEngineWithDataPath:dataPath];
    }
}

- (NSString *)version {
    if(RHEngine.get()) {
        return [NSString stringWithUTF8String:RHEngine->get_version().c_str()];
    }
    return @"Not initilzed!";
}

- (std::shared_ptr<RHVoice::engine>)engine {
    return RHEngine;
}

#pragma mark - Private

- (instancetype)init {
    self = [super init];
    if(self) {
        NSString *dataPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"RHVoiceData" ofType:nil];
        self.dataPath = dataPath;
    }
    return self;
}

- (void)createRHEngineWithDataPath:(NSString *)dataPath {
    try {
        RHVoice::engine::init_params param;
        param.data_path = dataPath.UTF8String;
        std::shared_ptr<RHVoice::engine> newEngine(new RHVoice::engine(param));
        RHEngine = newEngine;
    } catch (RHVoice::no_languages) {
        NSLog(@"No Languages folder is located at: %@", dataPath);
        NSLog(@"Please set  valid 'dataPath' property for %@ object. This folder has to contain 'languages' and 'voices' folders.", NSStringFromClass([self class]));
    }

}

- (const std::set<RHVoice::voice_profile>)voiceProfiles {
    if(RHEngine.get()) {
        return RHEngine->get_voice_profiles();
    }
    return std::set<RHVoice::voice_profile>();
}
@end
