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
    
    //Backgrounding test
    var time = 0
    var timer1 = Timer()
    var timer2 = Timer()
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        
        //Menu
        menuService.constructMenu(statusItem: statusItemService.statusItem)
        
        //Status item title
        statusItemService.setStatusItemButtonTitle(text: "VIGOR" + " " + fatigueControlService.currentStatus())
        
        //Backgrounding test
        timer1 = Timer.scheduledTimer(withTimeInterval: 1, repeats: true, block: { [weak self] (_) in
            self?.time += 1
            self?.menuService.menu.items[9].title = "\(self?.time ?? 0)"
        })
        RunLoop.main.add(timer1, forMode: .commonModes)
        
        //Status request
        startStatusPeriodicRequest()
        
    }
    
    func applicationWillTerminate(_ aNotification: Notification) {
        
    }
    
    
}

//Periodic status request
extension AppDelegate {
    private func startStatusPeriodicRequest() {
        timer2 = Timer.scheduledTimer(withTimeInterval: 5, repeats: true, block: { [weak self] (_) in
            self?.statusItemService.setStatusItemButtonTitle(text: "VIGOR" + " " + (self?.fatigueControlService.currentStatus() ?? ""))
        })
        RunLoop.main.add(timer2, forMode: .commonModes)
    }
    
    private func stopStatusPeriodicRequest() {
        timer2.invalidate()
    }
}

//Fatigue Control Methods
extension AppDelegate {
    @objc func startStopAnalizing(_ sender: Any?) {
        switch fatigueControlService.isStarted {
        case false:
            fatigueControlService.startFC()
            startStatusPeriodicRequest()
        default:
            fatigueControlService.stopFC()
            stopStatusPeriodicRequest()
        }
    }
    
    @objc func sendFeedback(_ sender: Any?) {
        feedbackService.sendFeedback(emails: ["ik@woodenshark.com"], subject: "Fatigue control feedback: \(deviceService.deviceID())", text: "")
    }
    
    @objc func exitApp(_ sender: Any?) {
        fatigueControlService.exit()
    }
}
