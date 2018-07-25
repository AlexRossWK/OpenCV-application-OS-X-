//
//  FeedbackService.swift
//  Vigor
//
//  Created by Алексей Россошанский on 25.07.2018.
//  Copyright © 2018 Rentateam. All rights reserved.
//

import Cocoa

class FeedbackService {
    
    func sendFeedback(emails: [String], subject: String, text: String) {
        let service = NSSharingService(named: NSSharingService.Name.composeEmail)!
        service.recipients = emails
        service.subject = subject
        
        service.perform(withItems: [""])
        
    }
}
