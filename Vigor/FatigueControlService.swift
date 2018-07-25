//
//  FatigueControlService.swift
//  Vigor
//
//  Created by Алексей Россошанский on 25.07.2018.
//  Copyright © 2018 Rentateam. All rights reserved.
//

import Cocoa

class FatigueControlService {
    
    var isStarted = true
    
    func startFC() {
        isStarted = true
        print("analyzing started")
    }
    
    func stopFC() {
        isStarted = false
        print("analyzing finished")
    }
    
    func exit() {
        isStarted = false
        print("left the app")
        NSApplication.shared.terminate(self)
    }
    
    func currentStatus() -> String {
        print("status requested")
        return "100%"
    }
}
