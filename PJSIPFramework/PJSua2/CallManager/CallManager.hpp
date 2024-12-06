//
//  CallManager.hpp
//  PJSIPFramework
//
//  Created by Md. MA-Ariful Jannat on 5/12/24.
//
#ifndef CALLMANAGER_HPP
#define CALLMANAGER_HPP

#include <unordered_map>
#include <string>
#include <iostream>
#include "SIPCall.hpp"
#include <pjsua2.hpp>
#include "SipAccount.hpp"

using namespace pj;
using namespace std;

enum CallAction {
    MAKE_CALL = 1,
    ANSWER_CALL = 2,
    HOLD_UNHOLD_CALL = 3,
    HANGUP_CALL = 4,
    MUTE_UNMUTE_CALL = 5,
    SEND_DTMF_TONE = 6,
    TRANSFER_CALL = 7,
};

class EventPayload {
public:
    // Use optional to make fields nullable
    CallAction type;
    string callId;
    
    optional<string> dest_uri;
    optional<string> dtmf_tone;
    optional<string> transfer_dest;
    
    // Fields for incoming calls
    optional<string> channelid;
    optional<string> mediaAddr;
    
    optional<string> username;
    optional<string> password;
    optional<string> domain;
    bool toggleStatus = false;
    
    // Constructor with default values
    EventPayload(CallAction type, string callId)
    : type(type), callId(callId) { }
};

class CallManagerDelegate {
public:
    virtual void onCallStateChanged(string callId, const int &state, string stateName) = 0;
    virtual void onTransferStatusChanged(string callId, const int &transferStatus) = 0;
    virtual void onCallFeatureToggled(string callId, string featureName, bool status) = 0;
};


class CallManager : public Endpoint, public SIPCallDelegate {
private:
    unordered_map<string, SIPCall*> activeCalls;  // Map of active calls
    unordered_map<int, SIPAccount*> sipAccounts;  // Map of active calls
    string proxyServer;  // Optional proxy server configuration
    CallManagerDelegate* delegate;
    bool isInitialized = false;
    void initiateCallInternal(EventPayload *payload);
    void answerCall(EventPayload *payload);
    void toggleHold(EventPayload *payload);
    void toggleMute(EventPayload *payload);
    void sendDTMFTone(EventPayload *payload);
    void hangupCall(EventPayload *payload);
    SIPCall* getCallWithId(string callId);

public:
    CallManager();
    ~CallManager();
    
    void initializeAndPrepare();
    void setProxyServerAddress(const string proxyAddress);

    void setDelegate(CallManagerDelegate* del);
    void onTimer(const OnTimerParam &prm) override;
    // Implemented methods from SIPCallDelegate
    void handleCallState(OnCallStateParam &prm, SIPCall* call) override;
    void onCallMediaState(OnCallMediaStateParam &prm, SIPCall* call) override;
    void onCallTransferStatus(OnCallTransferStatusParam &prm, SIPCall* call) override;
    void postEvent(EventPayload *event);
};

#endif /* CALLMANAGER_HPP */
