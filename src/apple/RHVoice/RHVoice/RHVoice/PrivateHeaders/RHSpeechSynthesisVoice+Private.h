//
//  RHSpeechSynthesisVoice+Private.h
//  RHVoice
//
//  Created by Ihor Shevchuk on 03.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#ifndef RHSpeechSynthesisVoice_Private_h
#define RHSpeechSynthesisVoice_Private_h

#import "RHSpeechSynthesisVoice.h"

#include "core/voice_profile.hpp"

@interface RHSpeechSynthesisVoice(private_additions)

- (const RHVoice::voice_profile&)voiceProfile;

@end

#endif /* RHSpeechSynthesisVoice_Private_h */
