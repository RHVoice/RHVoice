//
//  RHVoiceBridge.h
//  RHVoice
//
//  Created by Ihor Shevchuk on 02.05.2022.
//

#import <Foundation/Foundation.h>

#import <RHVoice/RHVoiceBridgeParams.h>

NS_ASSUME_NONNULL_BEGIN

@interface RHVoiceBridge : NSObject
@property (nonatomic, strong) RHVoiceBridgeParams *params;
- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)sharedInstance;

- (NSString *)version;
@end

NS_ASSUME_NONNULL_END
