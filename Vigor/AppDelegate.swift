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
    
    
    let menuService = MenuService()
    let deviceService = DeviceService()
    let feedbackService = FeedbackService()
    let fatigueControlService = FatigueControlService()
    let statusItemService = StatusItemService()
    
    
    let menu = NSMenu()

    //Backgrounding test
    var time = 0
    var timer1 = Timer()
    var timer2 = Timer()
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        
        //Menu
        menuService.constructMenu(statusItem: statusItemService.statusItem, menu: menu)
        
        //Status item title
        statusItemService.setStatusItemButtonTitle(text: "VIGOR" + " " + fatigueControlService.currentStatus())
        
        //Backgrounding test
        timer1 = Timer.scheduledTimer(withTimeInterval: 1, repeats: true, block: { [weak self] (_) in
            self?.time += 1
            self?.menu.items[9].title = "\(self?.time ?? 0)"
        })
        RunLoop.main.add(timer1, forMode: .commonModes)
        
        //Status request
        timer2 = Timer.scheduledTimer(withTimeInterval: 5, repeats: true, block: { [weak self] (_) in
            self?.statusItemService.setStatusItemButtonTitle(text: "VIGOR" + " " + (self?.fatigueControlService.currentStatus() ?? ""))
        })
        RunLoop.main.add(timer2, forMode: .commonModes)
        
    }
    
    func applicationWillTerminate(_ aNotification: Notification) {
        
    }
    
    
}

extension AppDelegate {
    
    @objc func startStopAnalizing(_ sender: Any?) {
        switch fatigueControlService.isStarted {
        case false:
            fatigueControlService.startFC()
        default:
            fatigueControlService.stopFC()
        }
    }
    
    @objc func sendFeedback(_ sender: Any?) {
        feedbackService.sendFeedback(emails: ["ik@woodenshark.com"], subject: "Fatigue control feedback: \(deviceService.deviceID())", text: "")
    }
    
    @objc func exitApp(_ sender: Any?) {
        fatigueControlService.exit()
    }
    
}
