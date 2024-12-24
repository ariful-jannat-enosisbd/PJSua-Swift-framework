//
//  PJSwiftClient.swift
//  PJSIPFramework
//
//  Created by Md. MA-Ariful Jannat on 6/12/24.
//

import Foundation
class PJSIPSwiftClient: NSObject, PJSIPWrapperDelegate {
    static let shared = PJSIPSwiftClient()
    var delegate: PJSwiftDelegate?
    
    let wrapper: PJSIPWrapper
    var activeCallId: String?
    override init() {
        self.wrapper = PJSIPWrapper.sharedInstance()
        super.init()
        wrapper.delegate = self
        wrapper.initializeAndPrepare();
    }
    
    func setProxyServer(_ serverAddress: String) {
        wrapper.setProxyServerAddress(serverAddress)
    }
    
    func setDefaultAccount(_ username: String, password: String, domain: String) {
        
    }
    
    func setUserlessAccount(_ accountId: String) {
        
    }
    
    @discardableResult
    func callNumber(_ number: String) -> String {
        let callId = UUID().uuidString
        delegate?.onCallStarted(callId, name: number, number: number)
        wrapper.initiateCall(
            to: "sip:\(number)@pbn-voipqa-1-47.practicebynumber.com",
            username: "web_10eb0477",
            password: "782bd162",
            domain: "pbn-voipqa-1-47.practicebynumber.com",
            callId: callId
        )
        return callId
    }
    
    func onFeatureStatusToggled(_ callId: String, featureName: String, status: Bool) {
        if featureName == "TOGGLE_MUTE" {
            delegate?.onMuteStatusUpdated(callId, status: status)
        } else if featureName == "TOGGLE_HOLD" {
            delegate?.onHoldStatusUpdated(callId, status: status)
        }
    }
    
    // Delegate method called from pjsip
    func onTransferStatusChanged(_ callId: String, transferStatus: Int) {
         
    }
    
    func onCallStateChanged(_ callId: String, state: Int, stateName: String) {
        debugPrint("CALL_STATUS: \(callId) : \(state) : \(stateName)")
        switch stateName {
        case "CALLING":
            break;
        case "EARLY":
            delegate?.onCallRinging(callId)
        case "CONNECTING":
            delegate?.onCallConnecting(callId)
        case "CONFIRMED":
            delegate?.onCallConnected(callId)
        case "DISCONNECTED":
            delegate?.onCallEnded(callId)
        default:
            break;
        }
        
    }
    
    @MainActor
    func endCall(withID callId: String) {
        wrapper.endCall(withID: callId)
    }
    
    @MainActor
    func toggleHold(callId: String, status: Bool) {
        wrapper.toggleHold(callId, status: status)
    }
    
    @MainActor
    func toggleMute(callId: String, status: Bool) {
        wrapper.toggleMute(callId, status: status)
    }
    func onExceptionRaised(_ callId: String, forEventType eventType: Int32, errorMessage: String) {
        delegate?.onException(callId, forEventType: Int(eventType), errorMessag: errorMessage)
    }
}
 
struct PJSIPEventType {
    static let makeCall = 1
    static let answerCall = 2
    static let holdUnholdCall = 3
    static let hangupCall = 4
    static let muteUnmuteCall = 5
    static let sendDTMFTone = 6
    static let blindTransferCall = 7
}

protocol PJSwiftDelegate {
    func onCallStarted(_ callid: String, name: String, number: String)
    func onCallConnecting(_ callId: String)
    func onCallRinging(_ callid: String)
    func onCallConnected(_ callId: String)
    func onCallEnded(_ callId: String)
    func onMuteStatusUpdated(_ callId: String, status: Bool)
    func onHoldStatusUpdated(_ callId: String, status: Bool)
    func onException(_ callId: String, forEventType: Int, errorMessag: String)
}
