//
//  CPPConnectHeader.h
//  SwiftObjectivecCpp
//
//  Created by Алексей Россошанский on 27.07.2018.
//  Copyright © 2018 AlexRoss. All rights reserved.
//

#ifndef CPPConnectHeader_h
#define CPPConnectHeader_h


#endif /* CPPConnectHeader_h */

#import <Foundation/Foundation.h>


@interface MyOCPPClass: NSObject

-(void)startDetect:(NSString *)mac withArg2:(NSString *)resPath;
-(void)stopDetect;
//-(NSString *)getStatus;
//-(void)writeMAC:(NSString *)mac;
@end
