//
//  RHSpeechUtterance.h
//  RHVoice
//
//  Created by Ihor Shevchuk on 03.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#import <Foundation/Foundation.h>

#import <RHVoice/RHSpeechSynthesisVoice.h>

NS_ASSUME_NONNULL_BEGIN

@interface RHSpeechUtterance : NSObject

@property (nonatomic, strong, nullable) NSString *text;
@property (nonatomic, strong) RHSpeechSynthesisVoice *voice;
@property (nonatomic, assign) double rate;

- (instancetype)initWithText:(NSString * _Nullable )text;
@end

NS_ASSUME_NONNULL_END
