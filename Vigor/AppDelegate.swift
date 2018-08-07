//
//  AppDelegate.swift
//  Vigor
//
//  Created by Алексей Россошанский on 24.07.2018.
//  Copyright © 2018 Rentateam. All rights reserved.
//

import Cocoa
import AVFoundation


@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
    
    
    let menuService = MenuService()
    let deviceService = DeviceService()
    let feedbackService = FeedbackService()
    let fatigueControlService = FatigueControlService()
    let statusItemService = StatusItemService()
    let backendService = BackendService()
    
    //Backgrounding test
    var time = 0
    //    var timer1 = Timer()
    var timer2 = Timer()
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        
        //Menu
        menuService.constructMenu(statusItem: statusItemService.statusItem)
        
        //Status item title
        statusItemService.setStatusItemButtonTitle(text: "VIGOR" + " " + "NON")
        
        //Status request
        startStatusPeriodicRequest()
        
        //Start detect fatigue
        DispatchQueue.global(qos: .userInitiated).async {
            self.fatigueControlService.startFC()
        }
        
        //Backgrounding test
        //        timer1 = Timer.scheduledTimer(withTimeInterval: 1, repeats: true, block: { [weak self] (_) in
        //            self?.time += 1
        //            self?.menuService.menu.items[9].title = "\(self?.time ?? 0)"
        //        })
        //        RunLoop.main.add(timer1, forMode: .commonModes)
        
    }
    
}

//Periodic status request
extension AppDelegate {
    
    private func startStatusPeriodicRequest() {
        timer2 = Timer.scheduledTimer(withTimeInterval: 4, repeats: true, block: { [weak self] (_) in
            self?.backendService.getFatigueRating(success: { (fatigueRating) in
                DispatchQueue.main.async {
                    if let rating = fatigueRating {
                        self?.statusItemService.setStatusItemButtonTitle(text: "VIGOR" + " " +  rating + "%")
                    } else {
                        self?.statusItemService.setStatusItemButtonTitle(text: "VIGOR" + " " + "NON")
                    }
                }
            }, failure: {
                DispatchQueue.main.async {
                    self?.statusItemService.setStatusItemButtonTitle(text: "VIGOR" + " " +  "ERR")
                }
            })
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
            fatigueControlService.isStarted = true
            DispatchQueue.global(qos: .userInitiated).async {
                self.fatigueControlService.startFC()
            }
            startStatusPeriodicRequest()
            
            self.statusItemService.setStatusItemButtonTitle(text: "VIGOR" + " " + "NON")
            
        default:
            fatigueControlService.isStarted = false
            fatigueControlService.stopFC()
            
            stopStatusPeriodicRequest()
            
            self.statusItemService.setStatusItemButtonTitle(text: "VIGOR" + " " +  "OFF")
            
        }
    }
    
    @objc func sendFeedback(_ sender: Any?) {
        feedbackService.sendFeedback(emails: ["ik@woodenshark.com"], subject: "Fatigue control feedback: \(deviceService.deviceID())", text: "")
        
    }
    
    @objc func exitApp(_ sender: Any?) {
        fatigueControlService.exit()
    }
}

