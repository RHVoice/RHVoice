//
//  NSFileManager+Additions.h
//  RHVoice
//
//  Created by Ihor Shevchuk on 03.05.2022.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface NSFileManager (Additions)
- (void)RHCreateTempFolderIfNeededPath:(NSString *)path;
- (void)RHRemoveTempFolderIfNeededPath:(NSString *)path;
@end

NS_ASSUME_NONNULL_END
