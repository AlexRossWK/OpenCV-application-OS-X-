//
//  AppDelegate.swift
//  Vigor
//
//  Created by Алексей Россошанский on 24.07.2018.
//  Copyright © 2018 Rentateam. All rights reserved.
//

import Cocoa
import AVFoundation
import ServiceManagement


@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
    
    
    let menuService = MenuService()
    let deviceService = DeviceService()
    let feedbackService = FeedbackService()
    let fatigueControlService = FatigueControlService()
    let statusItemService = StatusItemService()
    let backendService = BackendService()
    
    let bgQueue = DispatchQueue.global(qos: .userInitiated)
    
    //Backgrounding test
    var time = 0
    //    var timer1 = Timer()
    var timer2 = Timer()
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        
        //StartUp
        if SMLoginItemSetEnabled("rentateam.Vigor.com" as CFString, true) {
            print("Successfully added login item.")
        } else {
            print("Failed to add login item.")
        }
        
        let launcherAppId = "rentateam.LaunchApplication.com"
        let runningApps = NSWorkspace.shared.runningApplications
        let isRunning = !runningApps.filter { $0.bundleIdentifier == launcherAppId }.isEmpty
        
        SMLoginItemSetEnabled(launcherAppId as CFString, true)
        
        if isRunning {
            DistributedNotificationCenter.default().post(name: .killLauncher,
                                                         object: Bundle.main.bundleIdentifier!)
        }
        
        // let stringPathToModel = Bundle.main.path(forResource: "main_clnf_general", ofType: "txt", inDirectory: "model")
        // let stringPathToModelClassifier = Bundle.main.path(forResource: "haarcascade_frontalface_alt", ofType: "xml", inDirectory: "classifiers")
        
        // let stringToResources = Bundle.main.resourcePath
        
        //Observers for sleep
        NSWorkspace.shared.notificationCenter.addObserver(self,
                                                          selector: #selector(wakeUpListener(aNotification:)),
                                                          name: NSWorkspace.didWakeNotification,
                                                          object: nil)
        NSWorkspace.shared.notificationCenter.addObserver(self,
                                                          selector: #selector(sleepListener(aNotification:)),
                                                          name: NSWorkspace.willSleepNotification,
                                                          object: nil)
        
        //Menu
        menuService.constructMenu(statusItem: statusItemService.statusItem)
        
        //Status item title
        statusItemService.setStatusItemButtonTitle(text: "VIGOR" + " " + "NON")
        
        //Status request
        startStatusPeriodicRequest()
        
        //Start detect fatigue
        bgQueue.async {
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
                        self?.showOffifStopped()
                    } else {
                        self?.statusItemService.setStatusItemButtonTitle(text: "VIGOR" + " " + "NON")
                        self?.showOffifStopped()
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
            bgQueue.async {
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

extension AppDelegate {
    @objc func wakeUpListener(aNotification: NSNotification) {
        print("wake up")
        
        if fatigueControlService.isStarted {
            DispatchQueue.main.asyncAfter(deadline: .now() + 3) {
                self.fatigueControlService.isStarted = true
                self.bgQueue.async {
                    self.fatigueControlService.startFC()
                }
                self.startStatusPeriodicRequest()
            }
        }
    }
    
    @objc func sleepListener(aNotification : NSNotification) {
        print("sleep")
        bgQueue.sync {
//            fatigueControlService.isStarted = false
            fatigueControlService.stopFC()
        }
    }
}
//A little crutch to test
extension AppDelegate {
    func showOffifStopped() {
        if !fatigueControlService.isStarted {
            self.statusItemService.setStatusItemButtonTitle(text: "VIGOR" + " " +  "OFF")
        }
    }
}

