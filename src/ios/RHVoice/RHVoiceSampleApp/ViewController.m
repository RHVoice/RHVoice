//
//  ViewController.m
//  RHVoiceSampleApp
//
//  Created by Ihor Shevchuk on 03.05.2022.
//

#import "ViewController.h"

#import <RHVoice/RHVoiceBridge.h>

@interface ViewController ()
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    NSLog(@"Version:%@", [[RHVoiceBridge sharedInstance] version]);
}


@end
