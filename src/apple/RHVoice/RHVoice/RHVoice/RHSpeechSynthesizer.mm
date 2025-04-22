//
//  RHSpeechSynthesizer.m
//  RHVoice
//
//  Created by Ihor Shevchuk on 03.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#import "RHSpeechSynthesizer.h"

#import <AVFAudio/AVAudioPlayer.h>

#import "RHSpeechSynthesisVoice+Private.h"
#import "RHVoiceBridge+Private.h"

#import "NSString+Additions.h"
#import "NSFileManager+Additions.h"

#include "RHVoiceWrapper.h"

#define CALLDELEGATE_WITH_ERROR_IF_NEEDED_AND_EXIT(errorObject) \
    if((errorObject) != nil) { \
        [self callDelegateWithError:error forUtterance:utterance]; \
        return; \
    }

@interface RHSpeechSynthesizer() <AVAudioPlayerDelegate> {
    BOOL _isSpeaking;
    NSOperationQueue *_operationQueue;
}
@property (strong, atomic) AVAudioPlayer *player;
@property (strong, atomic) RHSpeechUtterance *currentUtterance;
@property (strong, readonly) NSOperationQueue *operationQueue;

@end

@implementation RHSpeechSynthesizer

@synthesize operationQueue;
#pragma mark - Public

- (void)speak:(RHSpeechUtterance *)utterance {
    
    __weak RHSpeechSynthesizer *weakSelf = self;
    [self.operationQueue addOperationWithBlock:^{
        [weakSelf speakInternal:utterance];
    }];
}

- (void)synthesizeUtterance:(RHSpeechUtterance *)utterance
               toFileAtPath:(NSString *)path {
    __weak RHSpeechSynthesizer *weakSelf = self;
    [self.operationQueue addOperationWithBlock:^{
        [weakSelf synthesizeInternalUtterance:utterance
                                 toFileAtPath:path];
    }];
}

- (BOOL)isSpeaking {
    return _isSpeaking;
}

- (void)stopAndCancel {
    [self.operationQueue cancelAllOperations];
    [self.player stop];
    [self cleanUp];
}

#pragma mark - Private

- (NSOperationQueue *)operationQueue {
    @synchronized (self) {
        if(_operationQueue == nil) {
            _operationQueue = [[NSOperationQueue alloc] init];
            _operationQueue.name = [NSString stringWithFormat:@"%@Queue", NSStringFromClass([self class])];
            _operationQueue.maxConcurrentOperationCount = 1;
        }
        return _operationQueue;
    }
}

- (void)cleanUp {
    [[NSFileManager defaultManager] removeItemAtURL:self.player.url error:nil];
    self.player = nil;
    
    if(self.currentUtterance != nil) {
        if([self.delegate respondsToSelector:@selector(speechSynthesizer:didFinish:)]) {
            [self.delegate speechSynthesizer:self didFinish:self.currentUtterance];
        }
    }
    
    self.currentUtterance = nil;
#if TARGET_OS_IPHONE
    [[AVAudioSession sharedInstance] setActive:NO error:nil];
#endif
}

- (void)speakInternal:(RHSpeechUtterance *)utterance {
    
    if(utterance.text.length == 0) {
        if([self.delegate respondsToSelector:@selector(speechSynthesizer:didFinish:)]) {
            [self.delegate speechSynthesizer:self didFinish:utterance];
        }
        return;
    }
    
    _isSpeaking = YES;
    [[NSFileManager defaultManager] RHCreateTempFolderIfNeededPath:[NSString RHTemporaryFolderPath]];
    NSString *path = [NSString RHTemporaryPathWithExtesnion:@"wav"];
    
    [self synthesizeInternalUtterance:utterance
                         toFileAtPath:path];

    NSError *error = nil;
    NSURL *url = [NSURL fileURLWithPath:path];
    self.player = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:&error];
#if TARGET_OS_IPHONE
    CALLDELEGATE_WITH_ERROR_IF_NEEDED_AND_EXIT(error);
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:&error];
    CALLDELEGATE_WITH_ERROR_IF_NEEDED_AND_EXIT(error);
    [[AVAudioSession sharedInstance] setActive:YES error:&error];
    CALLDELEGATE_WITH_ERROR_IF_NEEDED_AND_EXIT(error);
#endif
    
    self.currentUtterance = utterance;
    self.player.delegate = self;
    self.player.volume = 1;
    self.player.rate = 1;
    [self.player prepareToPlay];
    [self.player play];
    
    while (self.player.isPlaying)
    {
        [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.3]];
    }
    
    _isSpeaking = NO;
}

- (void)synthesizeInternalUtterance:(RHSpeechUtterance *)utterance
                       toFileAtPath:(NSString *)path {
    if(utterance.text.length == 0) {
        if([self.delegate respondsToSelector:@selector(speechSynthesizer:didFinish:)]) {
            [self.delegate speechSynthesizer:self didFinish:utterance];
        }
        return;
    }
    
    RHVoice::voice_profile profile;
    
    RHSpeechSynthesisVoice *voice = utterance.voice;
    
    if(voice == nil) {
        RHSpeechSynthesisVoice *defaultVoice = [[RHSpeechSynthesisVoice speechVoices] firstObject];
        if(defaultVoice == nil) {
            NSError *error = [NSError errorWithDomain:NSStringFromClass([self class]) code:1 userInfo:nil];
            CALLDELEGATE_WITH_ERROR_IF_NEEDED_AND_EXIT(error);
        }
        voice = defaultVoice;
    }
    
    profile = voice.voiceProfile;
    audio_player player(path.UTF8String);
    player.set_buffer_size(20);
    player.set_sample_rate(24000);
    
    std::unique_ptr<document> doc;
    
    std::string textToSpeak = utterance.text.UTF8String;
    doc = document::create_from_plain_text([RHVoiceBridge sharedInstance].engine,
                                           textToSpeak.begin(),textToSpeak.end(),
                                           content_text,
                                           profile);
    doc->speech_settings.relative.rate = utterance.rate;
    
    doc->set_owner(player);
    doc->synthesize();
    player.finish();
}

- (void)callDelegateWithError:(NSError *)error
                 forUtterance:(RHSpeechUtterance *)utterance {
    if([self.delegate respondsToSelector:@selector(speechSynthesizer:didFailToSynthesize:withError:)]) {
        [self.delegate speechSynthesizer:self didFailToSynthesize:utterance withError:error];
    }
    [self cleanUp];
}

#pragma mark - AVAudioPlayerDelegate

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag {
    [self cleanUp];
}

- (void)audioPlayerDecodeErrorDidOccur:(AVAudioPlayer *)player error:(NSError * __nullable)error {
    [self callDelegateWithError:error forUtterance:self.currentUtterance];
}
@end
