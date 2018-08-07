//
//  MenuService.swift
//  Vigor
//
//  Created by Алексей Россошанский on 25.07.2018.
//  Copyright © 2018 Rentateam. All rights reserved.
//

import Cocoa

class MenuService {
    
    lazy var menu: NSMenu = {
        return NSMenu()
    }()
    
    let deviceService = DeviceService()
    
    func constructMenu(statusItem: NSStatusItem) {
                
        menu.addItem(NSMenuItem(title: "ID", action: nil, keyEquivalent: ""))
        menu.addItem(NSMenuItem(title: deviceService.deviceID(), action: nil, keyEquivalent: ""))
        menu.addItem(NSMenuItem.separator())
        menu.addItem(NSMenuItem(title: "Start/Stop", action: #selector(AppDelegate.startStopAnalizing(_:)), keyEquivalent: "s"))
        menu.addItem(NSMenuItem.separator())
        menu.addItem(NSMenuItem(title: "Feedback", action: #selector(AppDelegate.sendFeedback(_:)), keyEquivalent: "f"))
        menu.addItem(NSMenuItem.separator())
        menu.addItem(NSMenuItem(title: "Exit", action: #selector(NSApplication.terminate(_:)), keyEquivalent: "q"))
        menu.addItem(NSMenuItem.separator())
//        menu.addItem(NSMenuItem(title: "0", action: nil, keyEquivalent: ""))
//        menu.addItem(NSMenuItem.separator())
        statusItem.menu = menu
    }
}
