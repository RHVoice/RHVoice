//
//  RHVoiceLogger.m
//  RHVoice
//
//  Created by Ihor Shevchuk on 04.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#import "RHVoiceLogger.h"

#import "RHVoiceBridge.h"

@implementation RHVoiceLogger

+ (void)logAtLevel:(RHVoiceLogLevel)level format:(NSString *)format, ... {
    
    id<RHVoiceLoggerProtocol> logger = [RHVoiceBridge sharedInstance].params.logger;
    if(logger != nil && [logger respondsToSelector:@selector(logAtLevel:message:)]) {
        va_list args;
        va_start(args, format);
        NSString *formattedString = [[NSString alloc] initWithFormat:format arguments:args];
        [logger logAtLevel:level message:formattedString];
        va_end(args);
    }
}

+ (void)logAtLevel:(RHVoiceLogLevel)level message:(NSString *)message {
    id<RHVoiceLoggerProtocol> logger = [RHVoiceBridge sharedInstance].params.logger;
    if(logger != nil && [logger respondsToSelector:@selector(logAtLevel:message:)]) {
        [logger logAtLevel:level message:message];
    }
}

+ (void)logAtRHVoiceLevel:(RHVoice_log_level)level message:(NSString *)message {
    [self logAtLevel:[self internalLevelToPublic:level] message:message];
}

+ (RHVoiceLogLevel)internalLevelToPublic:(RHVoice_log_level)internal {
    
    switch (internal) {
        case RHVoice_log_level_trace:
            return RHVoiceLogLevelTrace;
        case RHVoice_log_level_debug:
            return RHVoiceLogLevelDebug;
        case RHVoice_log_level_info:
            return RHVoiceLogLevelInfo;
        case RHVoice_log_level_warning:
            return RHVoiceLogLevelWarning;
        case RHVoice_log_level_error:
            return RHVoiceLogLevelError;
    }
}


@end

