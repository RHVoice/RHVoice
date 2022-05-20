//
//  RHEventLoggerImpl.hpp
//  RHVoice
//
//  Created by Ihor Shevchuk on 04.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#ifndef RHEventLoggerImpl_hpp
#define RHEventLoggerImpl_hpp

#include <stdio.h>

#include "core/event_logger.hpp"

class RHEventLoggerImpl : public RHVoice::event_logger
{
public:
    void log(const std::string& tag, RHVoice_log_level level, const std::string& message) const override;
};

#endif /* RHeventLoggerImpl_hpp */
