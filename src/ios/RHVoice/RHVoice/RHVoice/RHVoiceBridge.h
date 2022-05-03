//
//  RHVoiceBridge.h
//  RHVoice
//
//  Created by Ihor Shevchuk on 02.05.2022.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface RHVoiceBridge : NSObject
@property (strong, nonatomic) NSString *dataPath;

- (instancetype)init NS_UNAVAILABLE;
+ (instancetype)sharedInstance;

- (NSString *)version;
@end

NS_ASSUME_NONNULL_END
