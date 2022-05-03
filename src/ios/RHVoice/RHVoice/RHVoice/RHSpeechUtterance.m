//
//  RHSpeechUtterance.m
//  RHVoice
//
//  Created by Ihor Shevchuk on 03.05.2022.
//

#import "RHSpeechUtterance.h"

@implementation RHSpeechUtterance

- (instancetype)init {
    return [self initWithText:nil];
}

- (instancetype)initWithText:(NSString * _Nullable )text {
    self = [super init];
    if(self) {
        self.text = text;
        self.rate = 1;
    }
    return self;
}
@end
