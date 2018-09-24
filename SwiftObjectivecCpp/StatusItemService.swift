//
//  StatusItemService.swift
//  SwiftObjectivecCpp
//
//  Created by Алексей Россошанский on 25.07.2018.
//  Copyright © 2018 AlexRoss. All rights reserved.
//

import Cocoa

class StatusItemService {
    
    lazy var statusItem: NSStatusItem = {
        //Lenth can be NSStatusItem.dynamicLength
        return NSStatusBar.system.statusItem(withLength: 100)
    }()
    
    func setStatusItemButtonTitle(text: String) {
        if let button = self.statusItem.button {
            button.title = text
        }
    }
}
