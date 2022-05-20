//
//  RHVoiceLogger.h
//  RHVoice
//
//  Created by Ihor Shevchuk on 04.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#import <Foundation/Foundation.h>

#import "RHVoiceLoggerProtocol.h"
#import "RHVoice_common.h"

NS_ASSUME_NONNULL_BEGIN

@interface RHVoiceLogger : NSObject
+ (void)logAtLevel:(RHVoiceLogLevel)level format:(NSString *)format, ... NS_SWIFT_UNAVAILABLE("Please use logAtLevelWithMessage instead");
+ (void)logAtLevel:(RHVoiceLogLevel)level message:(NSString *)message;

+ (void)logAtRHVoiceLevel:(RHVoice_log_level)level message:(NSString *)message;


@end

NS_ASSUME_NONNULL_END
