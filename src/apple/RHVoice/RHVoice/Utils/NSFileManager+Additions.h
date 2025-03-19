//
//  NSFileManager+Additions.h
//  RHVoice
//
//  Created by Ihor Shevchuk on 03.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface NSFileManager (Additions)
- (void)RHCreateTempFolderIfNeededPath:(NSString *)path;
- (void)RHRemoveTempFolderIfNeededPath:(NSString *)path;
@end

NS_ASSUME_NONNULL_END
