//
//  NSString+Additions.m
//  RHVoice
//
//  Created by Ihor Shevchuk on 29.04.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#import "NSString+Additions.h"

@implementation NSString (Additions)

+ (NSString *)RHTemporaryFolderPath {
    return [NSTemporaryDirectory() stringByAppendingPathComponent:@"RHVoice"];
}

+ (NSString *)RHTemporaryPathWithExtesnion:(NSString *)extesnion {
    NSString *uuidString = [NSUUID UUID].UUIDString;
    return [[self RHTemporaryFolderPath] stringByAppendingPathComponent:[NSString stringWithFormat:@"%@.%@", uuidString, extesnion]];
}
@end
