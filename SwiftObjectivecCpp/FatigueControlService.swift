//
//  FatigueControlService.swift
//  SwiftObjectivecCpp
//
//  Created by Алексей Россошанский on 25.07.2018.
//  Copyright © 2018 AlexRoss. All rights reserved.
//

import Cocoa

class FatigueControlService {
    
    var isStarted = true
    let objcClass = MyOCPPClass()
    let deviceServie = DeviceService()

    func startFC() {
        print("analyzing started")
        objcClass.startDetect(deviceServie.deviceID(), withArg2: Bundle.main.resourcePath! + "/")
    }
    
    func stopFC() {
        objcClass.stopDetect()
        print("analyzing finished")
    }
    
    func exit() {
        isStarted = false
        print("left the app")
        objcClass.stopDetect()
        NSApplication.shared.terminate(self)
    }
    
    func writeMAC(mac: String) {
//        objcClass.writeMAC(mac)
    }
    
//    func currentStatus() -> String {
//        print("status" + objcClass.getStatus())
//        guard let status = objcClass.getStatus() else {
//            return "NON"
//        }
//        if status == "" {
//            return "ERR"
//        }
//        return status
//    }
}
