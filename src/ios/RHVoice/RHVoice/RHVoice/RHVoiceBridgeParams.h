//
//  RHVoiceBridgeInitParams.h
//  RHVoice
//
//  Created by Ihor Shevchuk on 09.05.2022.
//

#import <Foundation/Foundation.h>

#import <RHVoice/RHVoiceLoggerProtocol.h>

NS_ASSUME_NONNULL_BEGIN

@interface RHVoiceBridgeParams : NSObject
@property (nonatomic, weak, nullable) id<RHVoiceLoggerProtocol> logger;
@property (nonatomic, strong, nonnull) NSString *dataPath;
+ (instancetype)defaultParams;
@end

NS_ASSUME_NONNULL_END
