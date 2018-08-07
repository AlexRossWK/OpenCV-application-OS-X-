//
//  BackendService.swift
//  Vigor
//
//  Created by Алексей Россошанский on 02.08.2018.
//  Copyright © 2018 Rentateam. All rights reserved.
//

import Cocoa

class BackendService {
    
    let deviceService = DeviceService()
    
    func getFatigueRating(success: @escaping (String?) -> Void, failure: @escaping () -> Void) {
        var request = URLRequest(url: URL(string: "http://eye-server.woodenshark.com/api/v1/vivacity")!)
        request.addValue("\(deviceService.deviceID())", forHTTPHeaderField: "X-Device-ID")
        URLSession.shared.dataTask(with: request, completionHandler: {
            (data, response, error) -> Void in
            
            if error == nil, let json = data {
                do {
                    let decoder = JSONDecoder()
                    let fatigue = try decoder.decode(FatigueModel.self, from: json)
                    if  let vivacity = fatigue.vivacity {
                        success("\(vivacity)")
                    } else {
                        success(nil)
                    }
                } catch {
                    failure()
                    return
                }
            } else {
                failure()
                return
            }
        }).resume()
    }
}
