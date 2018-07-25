//
//  StatusItemService.swift
//  Vigor
//
//  Created by Алексей Россошанский on 25.07.2018.
//  Copyright © 2018 Rentateam. All rights reserved.
//

import Cocoa

class StatusItemService {
    
    lazy var statusItem: NSStatusItem = {
        return NSStatusBar.system.statusItem(withLength:NSStatusItem.variableLength)
    }()
    
    func setStatusItemButtonTitle(text: String) {
        if let button = self.statusItem.button {
            button.title = text
        }
    }
}
