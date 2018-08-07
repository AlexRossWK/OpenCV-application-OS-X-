//
//  CPPConnect.m
//  Vigor
//
//  Created by Алексей Россошанский on 27.07.2018.
//  Copyright © 2018 Rentateam. All rights reserved.
//

#include "TestThread.h"

#import <Foundation/Foundation.h>
#import "CPPConnectHeader.h"
#import <opencv2/opencv.hpp>

@implementation MyOCPPClass

- (void)startDetect:(NSString *)mac
{
    
    std::string macID = std::string([mac UTF8String]);
    
    SleepDetect sd;
    sd.Start(macID) ;
    
}

- (void)stopDetect
{
    SleepDetect sd;
    sd.Stop();
}

//- (NSString *)getStatus
//{
//    SleepDetect sd;
//    NSString *status = [NSString stringWithCString:sd.GetStatus().c_str()
//                                                encoding:[NSString defaultCStringEncoding]];
//    return status;
//}


@end
