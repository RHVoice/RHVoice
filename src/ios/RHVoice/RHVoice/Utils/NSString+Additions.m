//
//  NSString+Additions.m
//  RHVoice
//
//  Created by Ihor Shevchuk on 29.04.2022.
//

#import "NSString+Additions.h"

@implementation NSString (Additions)
+ (NSString *)temporaryPathWithExtesnion:(NSString *)extesnion
{
    NSString *uuidString = [NSUUID UUID].UUIDString;
    return [NSTemporaryDirectory() stringByAppendingPathComponent:[NSString stringWithFormat:@"%@.%@", uuidString, extesnion]];
}
@end
