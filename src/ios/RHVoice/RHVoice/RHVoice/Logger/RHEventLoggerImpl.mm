//
//  RHEventLoggerImpl.cpp
//  RHVoice
//
//  Created by Ihor Shevchuk on 04.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#include "RHEventLoggerImpl.hpp"

#import "RHVoiceLogger.h"

void RHEventLoggerImpl::log(const std::string& tag, RHVoice_log_level level, const std::string& message) const {
    
    NSString *nsTag = [NSString stringWithUTF8String:tag.c_str()];
    NSString *nsMessage = [NSString stringWithUTF8String:message.c_str()];
    
    [RHVoiceLogger logAtRHVoiceLevel:level message:[[NSString alloc] initWithFormat:@"%@ %@",nsTag, nsMessage]];
}
