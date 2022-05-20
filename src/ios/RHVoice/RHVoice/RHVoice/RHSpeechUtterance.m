//
//  RHSpeechUtterance.m
//  RHVoice
//
//  Created by Ihor Shevchuk on 03.05.2022.
//
//  Copyright (C) 2022  Olga Yakovleva <olga@rhvoice.org>

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
