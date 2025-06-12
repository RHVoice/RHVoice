//
//  RHVoiceLoggerProtocol.h
//  RHVoice
//
//  Created by Ihor Shevchuk on 04.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#ifndef RHVoiceLoggerProtocol_h
#define RHVoiceLoggerProtocol_h

typedef enum RHVoiceLogLevel : NSInteger {
    RHVoiceLogLevelTrace,
    RHVoiceLogLevelDebug,
    RHVoiceLogLevelInfo,
    RHVoiceLogLevelWarning,
    RHVoiceLogLevelError
} RHVoiceLogLevel;


@protocol RHVoiceLoggerProtocol <NSObject>
- (void)logAtLevel:(RHVoiceLogLevel)level message:(NSString *)message;
@end

#endif /* RHVoiceLoggerProtocol_h */
