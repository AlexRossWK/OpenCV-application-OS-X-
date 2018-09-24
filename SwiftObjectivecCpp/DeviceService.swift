//
//  DeviceService.swift
//  SwiftObjectivecCpp
//
//  Created by Алексей Россошанский on 25.07.2018.
//  Copyright © 2018 AlexRoss. All rights reserved.
//

import Foundation

class DeviceService {
    
    func deviceID() -> String {
        let theTask = Process()
        let taskOutput = Pipe()
        theTask.launchPath = "/sbin/ifconfig"
        theTask.standardOutput = taskOutput
        theTask.standardError = taskOutput
        theTask.arguments = ["en0"]
        
        theTask.launch()
        theTask.waitUntilExit()
        
        let taskData = taskOutput.fileHandleForReading.readDataToEndOfFile()
        
        if let stringResult = NSString(data: taskData, encoding: String.Encoding.utf8.rawValue) {
            if stringResult != "ifconfig: interface en0 does not exist" {
                let f = stringResult.range(of: "ether")
                if f.location != NSNotFound {
                    let sub = stringResult.substring(from: f.location + f.length)
                    let mac = String(sub[sub.index(after: sub.startIndex)..<sub.index(sub.startIndex, offsetBy: 18)])
                    return mac
                }
            }
        }
        return "00:00:00:00:00:00"
    }
}
