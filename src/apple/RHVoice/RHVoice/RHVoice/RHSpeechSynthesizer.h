//
//  RHSpeechSynthesizer.h
//  RHVoice
//
//  Created by Ihor Shevchuk on 03.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#import <Foundation/Foundation.h>

#import <RHVoice/RHSpeechUtterance.h>

@class RHSpeechSynthesizer;

@protocol RHSpeechSynthesizerDelegate <NSObject>
- (void)speechSynthesizer:(RHSpeechSynthesizer *_Nonnull)speechSynthesizer
                didFinish:(RHSpeechUtterance *_Nonnull)utterance;

- (void)speechSynthesizer:(RHSpeechSynthesizer *_Nonnull)speechSynthesizer
      didFailToSynthesize:(RHSpeechUtterance *_Nonnull)utterance
                withError:(NSError *_Nullable)error;
@end

NS_ASSUME_NONNULL_BEGIN

@interface RHSpeechSynthesizer : NSObject
@property (nonatomic, weak) id<RHSpeechSynthesizerDelegate> delegate;
@property (nonatomic, readonly) BOOL isSpeaking;
- (void)speak:(RHSpeechUtterance *)utterance;
- (void)synthesizeUtterance:(RHSpeechUtterance *)utterance
               toFileAtPath:(NSString *)path;
- (void)stopAndCancel;
@end

NS_ASSUME_NONNULL_END
