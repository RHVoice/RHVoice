//
//  NSFileManager+Additions.m
//  RHVoice
//
//  Created by Ihor Shevchuk on 03.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#import "NSFileManager+Additions.h"

@implementation NSFileManager (Additions)
- (void)RHCreateTempFolderIfNeededPath:(NSString *)path {
    if(![self fileExistsAtPath:path]) {
        NSError *error;
        if(![self createDirectoryAtPath:path withIntermediateDirectories:NO attributes:nil error:&error]) {
        }
    }
}

- (void)RHRemoveTempFolderIfNeededPath:(NSString *)path {
    if([self fileExistsAtPath:path]) {
        NSError *error;
        if(![self removeItemAtPath:path error:&error]) {
            
        }
    }
}
@end
