//
//  RHVoiceBridgeInitParams.m
//  RHVoice
//
//  Created by Ihor Shevchuk on 09.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#import "RHVoiceBridgeParams.h"

@implementation RHVoiceBridgeParams
+ (instancetype)defaultParams {
    RHVoiceBridgeParams *result = [[RHVoiceBridgeParams alloc] init];
    result.dataPath = [[NSBundle bundleForClass:[self class]] pathForResource:@"RHVoiceData" ofType:nil];
    return result;
}

- (BOOL)isEqual:(id)object {
    RHVoiceBridgeParams *anotherObject = object;
    if(![anotherObject isKindOfClass:[self class]]) {
        return NO;
    }
    
    return [anotherObject.dataPath isEqualToString:self.dataPath] &&
           [anotherObject.logger isEqual:self.logger];
}

- (NSUInteger)hash {
    return [self.dataPath hash] ^ [self.logger hash];
}
@end
