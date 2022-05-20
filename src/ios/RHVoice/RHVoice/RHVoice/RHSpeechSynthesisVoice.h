//
//  RHSpeechSynthesisVoice.h
//  RHVoice
//
//  Created by Ihor Shevchuk on 03.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef enum RHSpeechSynthesisVoiceGender : NSInteger {
    RHSpeechSynthesisVoiceGenderUnknown,
    RHSpeechSynthesisVoiceGenderMale,
    RHSpeechSynthesisVoiceGenderFemale
} RHSpeechSynthesisVoiceGender;

@interface RHSpeechSynthesisVoice : NSObject
@property (nonatomic, readonly, strong) NSString *name;
@property (nonatomic, readonly, strong) NSString *language;
@property (nonatomic, readonly) RHSpeechSynthesisVoiceGender gender;
- (instancetype)init NS_UNAVAILABLE;
+ (NSArray<RHSpeechSynthesisVoice *> *)speechVoices;
@end

NS_ASSUME_NONNULL_END
