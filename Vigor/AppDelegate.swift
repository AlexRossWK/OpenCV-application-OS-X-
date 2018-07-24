//
//  AppDelegate.swift
//  Vigor
//
//  Created by Алексей Россошанский on 24.07.2018.
//  Copyright © 2018 Rentateam. All rights reserved.
//

import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
    
    class var shared: AppDelegate {
        return NSApplication.shared.delegate as! AppDelegate
    }
    
    let statusItem = NSStatusBar.system.statusItem(withLength:NSStatusItem.variableLength)
    let status = ""
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        //Menu bar item
        if let button = statusItem.button {
            button.title = "VIGOR" + " " + status
        }
        //Menu
        constructMenu()
    }
    
    func applicationWillTerminate(_ aNotification: Notification) {
        
    }
    
    
}

extension AppDelegate {
    func constructMenu() {
        let menu = NSMenu()
        
        menu.addItem(NSMenuItem(title: "ID", action: nil, keyEquivalent: ""))
        menu.addItem(NSMenuItem(title: deviceID(), action: nil, keyEquivalent: ""))
        menu.addItem(NSMenuItem.separator())
        menu.addItem(NSMenuItem(title: "Start/Stop", action: #selector(AppDelegate.startStopAnalizing(_:)), keyEquivalent: "s"))
        menu.addItem(NSMenuItem.separator())
        menu.addItem(NSMenuItem(title: "Feedback", action: #selector(AppDelegate.sendFeedback(_:)), keyEquivalent: "f"))
        menu.addItem(NSMenuItem.separator())
        menu.addItem(NSMenuItem(title: "Exit", action: #selector(NSApplication.terminate(_:)), keyEquivalent: "q"))
        
        statusItem.menu = menu
    }
    
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
        return "error"
    }
    
}

extension AppDelegate {
    
    @objc func startStopAnalizing(_ sender: Any?) {
        
    }
    
    @objc func sendFeedback(_ sender: Any?) {
        
    }
    
}
