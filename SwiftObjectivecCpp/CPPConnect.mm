//
//  CPPConnect.m
//  SwiftObjectivecCpp
//
//  Created by Алексей Россошанский on 27.07.2018.
//  Copyright © 2018 AlexRoss. All rights reserved.
//

#include "TestThread.h"

#import <Foundation/Foundation.h>
#import "CPPConnectHeader.h"

@implementation MyOCPPClass

- (void)startDetect:(NSString *)mac withArg2:(NSString *)resPath
{
    
    std::string macID = std::string([mac UTF8String]);
    std::string pathToResources = std::string([resPath UTF8String]);
    SleepDetect sd;
    sd.Start(macID, pathToResources) ;
    
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

- (void)writeMAC:(NSString *)mac
{
    
}

@end
