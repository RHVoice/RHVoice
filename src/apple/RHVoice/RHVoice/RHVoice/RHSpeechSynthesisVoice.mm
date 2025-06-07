//
//  RHSpeechSynthesisVoice.m
//  RHVoice
//
//  Created by Ihor Shevchuk on 03.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#import "RHSpeechSynthesisVoice.h"

#import "RHVoiceBridge+Private.h"

@interface RHSpeechSynthesisVoice() {
    RHVoice::voice_profile voice_profile;
}
@end

@implementation RHSpeechSynthesisVoice

#pragma mark - Public

- (NSString *)name {
    return [NSString stringWithUTF8String:voice_profile.primary()->get_name().c_str()];
}

- (NSString *)language {
    return [NSString stringWithUTF8String:voice_profile.primary()->get_alpha2_country_code().c_str()];
}

- (RHSpeechSynthesisVoiceGender)gender {
    return [RHSpeechSynthesisVoice genderFromRHVoiceGender:voice_profile.primary()->get_gender()];
}

+ (NSArray<RHSpeechSynthesisVoice *> *)speechVoices {

    NSMutableArray *result = [[NSMutableArray alloc] initWithCapacity:[[RHVoiceBridge sharedInstance] voiceProfiles].size()];
    
    for(auto profile : [[RHVoiceBridge sharedInstance] voiceProfiles]) {
        [result addObject:[[RHSpeechSynthesisVoice alloc] initWith:profile]];
    }
    return [result copy];
}

#pragma mark - Private

- (instancetype)initWith:(RHVoice::voice_profile &)voice_profile {
    self = [super init];
    if(self) {
        self->voice_profile = voice_profile;
    }
    return self;
}

- (const RHVoice::voice_profile&)voiceProfile; {
    return voice_profile;
}

+ (RHSpeechSynthesisVoiceGender)genderFromRHVoiceGender:(RHVoice_voice_gender)gender {
    switch (gender) {
        case RHVoice_voice_gender_unknown:
            return RHSpeechSynthesisVoiceGenderUnknown;
        case RHVoice_voice_gender_male:
            return RHSpeechSynthesisVoiceGenderMale;
        case RHVoice_voice_gender_female:
            return RHSpeechSynthesisVoiceGenderFemale;
    }
}
@end
