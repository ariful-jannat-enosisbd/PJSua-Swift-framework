//
//  CallManager.cpp
//  PJSIPFramework
//
//  Created by Md. MA-Ariful Jannat on 5/12/24.
//

#include "CallManager.hpp"
#include <pjsua2.hpp>
#include <iostream>
#include <unordered_map>
#include <optional>

using namespace pj;
using namespace std;

CallManager::CallManager() {
    std::cout << "CallManager initialized." << std::endl;
}

CallManager::~CallManager() {
    
    for (auto& call : activeCalls) {
        delete call.second;
    }
    std::cout << "CallManager destroyed." << std::endl;
}

void CallManager::initializeAndPrepare() {
    if (isInitialized) {
        return;
    }
    isInitialized = true;

    // Create and initialize library
    try {
        libCreate();
        EpConfig ep_cfg;
        libInit(ep_cfg);

        // Create SIP transport
        TransportConfig tcfg;
        tcfg.port = 5060;
        transportCreate(PJSIP_TRANSPORT_UDP, tcfg);

        // Start the library
        libStart();
    } catch (Error& err) {
        std::cout << "Error: " << err.info() << std::endl;
    }
}

void CallManager::setProxyServerAddress(const std::string proxyAddress) {
    proxyServer = proxyAddress;
}

void CallManager::setDelegate(CallManagerDelegate* del) {
    this->delegate = del;
}

void CallManager::postEvent(EventPayload *event) {
    // Add logging if necessary
    utilTimerSchedule(0, event);
}

void CallManager::initiateCallInternal(EventPayload* payload) {
    try {
        string domain = payload->domain.value();
        string username = payload->username.value();
        string password = payload->password.value();
        string destUri = payload->dest_uri.value();
        AccountConfig acfg;
        acfg.idUri = "sip:" + username + "@" + domain;//+ ":5060";
        acfg.regConfig.registrarUri = ""; // "sip:" + registrar + ":" + port;
        acfg.regConfig.timeoutSec = 3600;
        
        // If proxy server is configured, add it to the config
        if (proxyServer.length() > 0) {
            acfg.sipConfig.proxies.push_back(proxyServer);
            // Adding authentication credentials
            AuthCredInfo proxyCred("digest", "*", username, 0, password);
            acfg.sipConfig.authCreds.push_back(proxyCred);
        }
        acfg.mediaConfig.srtpUse = PJMEDIA_SRTP_DISABLED;
        acfg.mediaConfig.srtpSecureSignaling = 0;
        acfg.videoConfig.autoShowIncoming = false;
        acfg.videoConfig.autoTransmitOutgoing = true;
        acfg.regConfig.registerOnAdd = false;
        acfg.regConfig.disableRegOnModify = true;
        
        // Create account and handle errors
        std::cout << "Creating account..." << std::endl;
        SIPAccount* acc = new SIPAccount;
        acc->create(acfg, true);
        acc->setDefault();
        acc->getInfo();
        std::cout << "Account created successfully." << std::endl;
        
        // Making the call
        std::cout << "Making call to: " << destUri << std::endl;
        SIPCall* call = new SIPCall(*acc, this, payload->callId);
        CallOpParam prm(true); // Use default call settings
        prm.opt.videoCount = 0;
        call->makeCall(destUri, prm);
        activeCalls[call->getCustomCallId()] = call;
    } catch (Error& err) {
        std::cout << "Error occurred: " << err.info() << std::endl;
    } catch (std::exception& e) {
        std::cout << "Exception occurred: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown error occurred." << std::endl;
    }
}

void CallManager::answerCall(EventPayload *payload) {
    try {
        AccountConfig acfg;
        acfg.idUri =  payload->username.value(); //"sip:100@pbn-voipqa-1-47.practicebynumber.com";
        acfg.mediaConfig.srtpUse = PJMEDIA_SRTP_DISABLED;
        acfg.mediaConfig.srtpSecureSignaling = 0;
        if(proxyServer.length() > 0) {
            acfg.sipConfig.proxies.push_back(proxyServer);
        }
        acfg.regConfig.registerOnAdd = false;
        acfg.regConfig.disableRegOnModify = true;
        SIPAccount* acc = new SIPAccount;
        acc->create(acfg, true);
        
        CallOpParam prm(true);
        prm.opt.videoCount = 0;
        
        // Create SIP message headers
        SipHeaderVector headers;
        
        // Add Channel ID header
        SipHeader channelHeader;
        channelHeader.hName = "X-Channel-Id";
        channelHeader.hValue = payload->channelid.value();
        headers.push_back(channelHeader);
        
        // Add Media IPv4 Address header
        SipHeader ipHeader;
        ipHeader.hName = "X-Media-Ipv4-Addr";
        ipHeader.hValue = payload->mediaAddr.value();
        headers.push_back(ipHeader);
        
        // Assign headers to call parameters
        prm.txOption.headers = headers;
        SIPCall* call = new SIPCall(*acc, this, payload->callId);
        call->makeCall(payload->dest_uri.value(), prm);
    } catch(Error& err) {
        //        callStatusListenerPtr(-1, -1);//TODO: report exception
        std::cout << err.info() << std::endl;
    }
}

SIPCall* CallManager::getCallWithId(string callId) {
    auto it = activeCalls.find(callId);  // Find the call with the given callId
    if (it != activeCalls.end()) {
        SIPCall* call = it->second;
        if (call) {
            return call;
        }
    }
    // If not found, throw an exception
    throw std::runtime_error("Call with ID " + callId + " not found.");
}

void CallManager::sendDTMFTone(EventPayload *payload) {
    SIPCall* call = getCallWithId(payload->callId);
    call->dialDtmf(payload->dtmf_tone.value());
}

void CallManager::toggleHold(EventPayload *payload) {
    SIPCall* call = getCallWithId(payload->callId);
    CallOpParam op(true);
    if(payload->toggleStatus) {
        call->setHold(op);
    } else {
        op.opt.flag = PJSUA_CALL_UNHOLD;
        call->reinvite(op);
    }
    delegate->onCallFeatureToggled(payload->callId, "TOGGLE_HOLD", payload->toggleStatus);
}

void CallManager::toggleMute(EventPayload *payload) {
    SIPCall* call = getCallWithId(payload->callId);
    CallInfo callInfo = call->getInfo();
    
    // Iterate through media streams
    for (unsigned int i = 0; i < callInfo.media.size(); ++i) {
        Media* media = call->getMedia(i);
        CallMediaInfo mediaInfo = callInfo.media[i];
        if (mediaInfo.type == PJMEDIA_TYPE_AUDIO && media != nullptr && mediaInfo.status == PJSUA_CALL_MEDIA_ACTIVE) {
            // Cast media to AudioMedia
            AudioMedia *audioMedia = dynamic_cast<AudioMedia*>(media);
            if (audioMedia) {
                if (payload->toggleStatus) {
                    audDevManager().getCaptureDevMedia().stopTransmit(*audioMedia);
                } else {
                    audDevManager().getCaptureDevMedia().startTransmit(*audioMedia);
                }
            } else {
                // Handle the case where the media is not AudioMedia
                std::cerr << "Failed to cast media to AudioMedia." << std::endl;
            }
        }
    }
    delegate->onCallFeatureToggled(payload->callId, "TOGGLE_MUTE", payload->toggleStatus);
}

void CallManager::hangupCall(EventPayload *payload) {
    SIPCall* call = getCallWithId(payload->callId);
    CallOpParam prm(true);
    prm.statusCode = PJSIP_SC_DECLINE;
    call->hangup(prm);
}

void CallManager::onTimer(const OnTimerParam &prm) {
    EventPayload* payload = static_cast<EventPayload*>(prm.userData);
    // functions inside this can throw exception if the call with associated id not found
    try {
        if (payload->type == MAKE_CALL) {
            initiateCallInternal(payload);
        } else if(payload->type == ANSWER_CALL) {
            answerCall(payload);
        } else if (payload->type == SEND_DTMF_TONE) {
            sendDTMFTone(payload);
        } else if(payload->type == HOLD_UNHOLD_CALL) {
            toggleHold(payload);
        } else if (payload->type == MUTE_UNMUTE_CALL) {
            toggleMute(payload);
        } else if(payload->type == HANGUP_CALL) {
            hangupCall(payload);
        }
    } catch (const std::runtime_error& e) {
        std::cout << "Error: " << e.what() << std::endl;
        // Handle the error (e.g., logging, recovery, etc.)
    }
}

// SIPCallDelegate implementation
void CallManager::handleCallState(OnCallStateParam &prm, SIPCall* call) {
    
    CallInfo ci = call->getInfo();
    string callId = call->getCustomCallId();
    
    cout<< "Call Status: " + call->getCustomCallId() << ci.stateText << endl;
    
    delegate->onCallStateChanged(callId, ci.state, ci.stateText);
    
    if(ci.state == PJSIP_INV_STATE_DISCONNECTED) {
        // Delete and cleanup the call
        activeCalls[callId] = nullptr;
        delete call; // Be cautious about deleting within the method
    }
}

/// We can do additional task here but media setting is done inside the SIPCALL class so we don't need to handle this
void CallManager::onCallMediaState(OnCallMediaStateParam &prm, SIPCall* call) {
    std::cout << "Handling media state for Call ID: " << call->getId() << std::endl;
}

void CallManager::onCallTransferStatus(OnCallTransferStatusParam &prm, SIPCall* call) {
    delegate->onTransferStatusChanged(call->getCustomCallId(), prm.statusCode == PJSIP_SC_OK);
}