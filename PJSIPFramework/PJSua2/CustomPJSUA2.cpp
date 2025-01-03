/*
 * Copyright (C) 2012-2012 Teluu Inc. (http://www.teluu.com)
 * Contributed by Emre Tufekci (github.com/emretufekci)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "CustomPJSUA2.hpp"
#include <iostream>
#include <list>

using namespace pj;

class MyEndpoint : public Endpoint
{
public:
    virtual void onTimer(const OnTimerParam &prm);
};

// Subclass to extend the Account and get notifications etc.
class MyAccount : public Account
{
public:
    std::string dest_uri;
    std::string dtmf_tone;
    std::string transfer_dest;
    
    //below are for incoming call
    std::string channelid;
    std::string mediaAddr;
    
    MyAccount() {}
    ~MyAccount()
    {
        // Invoke shutdown() first..
        shutdown();
        // ..before deleting any member objects.
    }
    
    // This is getting for register status!
    virtual void onRegState(OnRegStateParam &prm);
    
    // This is getting for incoming call (We can either answer or hangup the incoming call)
    virtual void onIncomingCall(OnIncomingCallParam &iprm);
};

// Subclass to extend the Call and get notifications etc.
class MyCall : public Call
{
public:
    MyCall(Account &acc, int call_id = PJSUA_INVALID_ID)
    : Call(acc, call_id)
    { }
    ~MyCall()
    { }
    
    // Notification when call's state has changed.
    virtual void onCallState(OnCallStateParam &prm);
    
    // Notification when call's media state has changed.
    virtual void onCallMediaState(OnCallMediaStateParam &prm);
    
    virtual void onCallTransferStatus(OnCallTransferStatusParam &prm);
};

enum {
    MAKE_CALL = 1,
    ANSWER_CALL = 2,
    HOLD_CALL = 3,
    UNHOLD_CALL = 4,
    HANGUP_CALL = 5,
    MUTE_CALL = 6,
    UNMUTE_CALL = 7,
    SEND_DTMF_TONE = 8,
    TRANSFER_CALL = 9,
    SEND_INVITE_TO_ANSWER_CALL = 10,
};

Call *call = NULL;
Endpoint *ep = NULL;
MyAccount *acc = NULL;
std::string proxyServerAddress = "";

// Listen swift code via function pointers
void (*incomingCallPtr)() = 0;
void (*callStatusListenerPtr)(int, int) = 0;
void (*accStatusListenerPtr)(bool) = 0;
void (*callTransferStatusListenerPtr)(bool) = 0;
void (*updateVideoPtr)(void *) = 0;

//Getter & Setter function
std::string callerId;

void MyEndpoint::onTimer(const OnTimerParam &prm)
{
    /* IMPORTANT:
     * We need to call PJSIP API from a separate thread since
     * PJSIP API can potentially block the main/GUI thread.
     * And make sure we don't use Apple's Dispatch / gcd since
     * it's incompatible with POSIX threads.
     * In this example, we take advantage of PJSUA2's timer thread
     * to perform call operations. For a more complex application,
     * it is recommended to create your own separate thread
     * instead for this purpose.
     */
    long code = (long) prm.userData;
    if (code == MAKE_CALL) {
        CallOpParam prm(true); // Use default call settings
        prm.opt.videoCount = 0;
        try {
            call = new MyCall(*acc);
            call->makeCall(acc->dest_uri, prm);
        } catch(Error& err) {
            callStatusListenerPtr(-1, -1);
            std::cout << err.info() << std::endl;
        }
    } else if (code == ANSWER_CALL) {
        if (call && call->getInfo().state == PJSIP_INV_STATE_INCOMING) {  // Check if there's an incoming call
            CallOpParam op(true);
            op.statusCode = PJSIP_SC_OK;
            call->answer(op);
        } else {
            // Handle cases where there's no incoming call
            std::cerr << "No incoming call to answer." << std::endl;
        }
    } else if (code == SEND_INVITE_TO_ANSWER_CALL) {
        CallOpParam prm(true);
        prm.opt.videoCount = 0;
        
        // Create SIP message headers
        SipHeaderVector headers;
        
        // Add Channel ID header
        SipHeader channelHeader;
        channelHeader.hName = "X-Channel-Id";
        channelHeader.hValue = acc->channelid;
        headers.push_back(channelHeader);
        
        // Add Media IPv4 Address header
        SipHeader ipHeader;
        ipHeader.hName = "X-Media-Ipv4-Addr";
        ipHeader.hValue = acc->mediaAddr;
        headers.push_back(ipHeader);
        
        // Assign headers to call parameters
        prm.txOption.headers = headers;
        try {
            call = new MyCall(*acc);
            call->makeCall(acc->dest_uri, prm);
        } catch(Error& err) {
            callStatusListenerPtr(-1, -1);
            std::cout << err.info() << std::endl;
        }
    } else if (code == HOLD_CALL) {
        if (call != NULL) {
            CallOpParam op(true);
            
            try {
                call->setHold(op);
            } catch(Error& err) {
                std::cout << "Hold error: " << err.info() << std::endl;
            }
        }
    } else if (code == UNHOLD_CALL) {
        if (call != NULL) {
            CallOpParam op(true);
            op.opt.flag = PJSUA_CALL_UNHOLD;
            
            try {
                call->reinvite(op);
            } catch(Error& err) {
                std::cout << "Unhold/Reinvite error: " << err.info() << std::endl;
            }
        }
    } else if (code == HANGUP_CALL) {
        if(!call) { return; }
        if (call != NULL) {
            CallInfo ci;
//            try {
//                // Attempt to get call information
//                ci = call->getInfo();
//            } catch (const std::exception& e) {
//                // Log the exception message for debugging
//                std::cerr << "Error retrieving call info: " << e.what() << std::endl;
//                ci.state = PJSIP_INV_STATE_DISCONNECTED; // Set state to disconnected
//            } catch (...) {
//                // Handle any other exceptions
//                std::cerr << "Unknown error occurred while retrieving call info." << std::endl;
//                ci.state = PJSIP_INV_STATE_DISCONNECTED; // Set state to disconnected
//            }

            // Check if the call is still in progress before attempting to hang up
            if (call->isActive()) {
                CallOpParam op(true);
                op.statusCode = PJSIP_SC_DECLINE;

                try {
                    // Attempt to hang up the call
                    call->hangup(op);
                } catch (const std::exception& e) {
                    // Log any errors during hangup
                    std::cerr << "Error hanging up the call: " << e.what() << std::endl;
                } catch (...) {
                    // Handle any other exceptions during hangup
                    std::cerr << "Unknown error occurred while hanging up the call." << std::endl;
                }
            } else {
                std::cout << "Call is already disconnected, no action taken." << std::endl;
            }
        } else {
            std::cerr << "Call pointer is null, cannot hang up." << std::endl;
        }
    } else if (code == MUTE_CALL) {
        if (call != NULL) {
            CallInfo ci = call->getInfo();
            if (ci.media.size() > 0 && ci.media[0].type == PJMEDIA_TYPE_AUDIO) {
                AudioMedia *aud_med = (AudioMedia *)call->getMedia(0);
                // Mute the audio transmission (stop sending audio from the mic)
                if (aud_med != NULL) {
                    try {
                        ep->audDevManager().getCaptureDevMedia().stopTransmit(*aud_med);
                        //                        aud_med->adjustTxLevel(0);  // Set the transmit level to 0 (mute)
                        std::cout << "Call muted." << std::endl;
                    } catch (Error &err) {
                        std::cout << "Error muting call: " << err.info() << std::endl;
                    }
                }
            }
        }
    } else if (code == UNMUTE_CALL) {
        if (call != NULL) {
            CallInfo ci = call->getInfo();
            if (ci.media.size() > 0 && ci.media[0].type == PJMEDIA_TYPE_AUDIO) {
                AudioMedia *aud_med = (AudioMedia *)call->getMedia(0);
                // Mute the audio transmission (stop sending audio from the mic)
                if (aud_med != NULL) {
                    try {
                        ep->audDevManager().getCaptureDevMedia().startTransmit(*aud_med);
                        //                        aud_med->adjustTxLevel(0);  // Set the transmit level to 0 (mute)
                        std::cout << "Call Un Muted." << std::endl;
                    } catch (Error &err) {
                        std::cout << "Error unMuting call: " << err.info() << std::endl;
                    }
                }
            }
        }
    } else if (code == SEND_DTMF_TONE) {
        try {
            CallOpParam prm(true); // Use default call settings
            if (call != NULL && !acc->dtmf_tone.empty()) {  // Check if dtmf_tone is not empty
                std::string dtmfTone = acc->dtmf_tone;  // Get the DTMF tone
                call->dialDtmf(dtmfTone);  // Send the DTMF tone
                std::cout << "DTMF tone sent" << std::endl;
            } else {
                if (call == NULL) {
                    std::cout << "Call object is NULL" << std::endl;
                }
                if (acc->dtmf_tone.empty()) {
                    std::cout << "DTMF tone is empty" << std::endl;
                }
            }
        } catch (Error &err) {
            std::cout << "Error sending DTMF tone: " << err.info() << std::endl;
        }
    } else if (code == TRANSFER_CALL) {
        try {
            if (call != NULL && !acc->transfer_dest.empty()) {  // Ensure call is not NULL and transfer destination is present
                CallOpParam op(true);
                op.statusCode = PJSIP_SC_DECLINE;  // Set the status code for the transfer
                std::string destinationNumber = acc->transfer_dest;
                
                // Log the transfer details
                std::cout << "Initiating call transfer to: " << destinationNumber << std::endl;
                
                call->xfer(destinationNumber, op);
                
                // Log successful transfer initiation
                std::cout << "Call transfer initiated to: " << destinationNumber << std::endl;
            } else {
                // Handle case when call is NULL or transfer destination is empty
                if (call == NULL) {
                    std::cout << "Error: Call object is NULL. Cannot transfer call." << std::endl;
                }
                if (acc->transfer_dest.empty()) {
                    std::cout << "Error: Transfer destination is empty. Cannot transfer call." << std::endl;
                }
            }
        } catch (Error &err) {
            // Log any errors during the call transfer process
            std::cout << "Error transferring call: " << err.info() << std::endl;
        }
    }
}

void MyAccount::onRegState(OnRegStateParam &prm)
{
    AccountInfo ai = getInfo();
    std::cout << (ai.regIsActive? "*** Register: code=" : "*** Unregister: code=")
    << prm.code << std::endl;
    accStatusListenerPtr(ai.regIsActive);
}

void MyAccount::onIncomingCall(OnIncomingCallParam &iprm)
{
    incomingCallPtr();
    call = new MyCall(*this, iprm.callId);
}

void setCallerId(std::string callerIdStr)
{
    callerId = callerIdStr;
}

std::string getCallerId()
{
    return callerId;
}

void MyCall::onCallState(OnCallStateParam &prm)
{
    CallInfo ci = getInfo();
    std::cout << "SIP_CALLING: Call state changed: " << ci.state << std::endl;
    callStatusListenerPtr(ci.state, ci.lastStatusCode);
    setCallerId(ci.remoteUri);
    PJSua2 pjsua2;
    pjsua2.incomingCallInfo();
    if (ci.state == PJSIP_INV_STATE_CALLING) {
        // Set NullDev during the calling state
        ep->audDevManager().setNullDev();
    } else if (ci.state == PJSIP_INV_STATE_EARLY) {
        // Ringing state - set proper audio devices
        ep->audDevManager().setCaptureDev(PJSUA_SND_DEFAULT_CAPTURE_DEV);
        ep->audDevManager().setPlaybackDev(PJSUA_SND_DEFAULT_PLAYBACK_DEV);
    } else if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
        // Cleanup when call is disconnected
        delete call;
        call = NULL;
        return;
    }
}

void MyCall::onCallMediaState(OnCallMediaStateParam &prm)
{
    CallInfo ci = getInfo();
    // Iterate all the call medias
    for (unsigned i = 0; i < ci.media.size(); i++) {
        if (ci.media[i].status == PJSUA_CALL_MEDIA_ACTIVE ||
            ci.media[i].status == PJSUA_CALL_MEDIA_REMOTE_HOLD)
        {
            if (ci.media[i].type==PJMEDIA_TYPE_AUDIO) {
                AudioMedia *aud_med = (AudioMedia *)getMedia(i);
                
                // Connect the call audio media to sound device
                AudDevManager& mgr = Endpoint::instance().audDevManager();
                aud_med->startTransmit(mgr.getPlaybackDevMedia());
                mgr.getCaptureDevMedia().startTransmit(*aud_med);
            } else if (ci.media[i].type==PJMEDIA_TYPE_VIDEO) {
                void *window = ci.media[i].videoWindow.getInfo().winHandle.handle.window;
                updateVideoPtr(window);
            }
        }
    }
}

void MyCall::onCallTransferStatus(OnCallTransferStatusParam &prm)
{
    if (prm.statusCode == PJSIP_SC_OK) {
        std::cout << "SIP_CALLING: Call transfer successful" << std::endl;
    } else {
        std::cout << "SIP_CALLING: Call transfer failed" << std::endl;
    }
    callTransferStatusListenerPtr(prm.statusCode == PJSIP_SC_OK);
}

/**
 Create Lib with EpConfig
 */
void PJSua2::createLib()
{
    if (ep != NULL) {
        return; // Library is already initialized
    }
    
    ep = new MyEndpoint;
    try {
        ep->libCreate();
    } catch (Error& err){
        std::cout << "Startup error: " << err.info() << std::endl;
    }
    
    //LibInit
    try {
        EpConfig ep_cfg;
        ep->libInit( ep_cfg );
    } catch(Error& err) {
        std::cout << "Initialization error: " << err.info() << std::endl;
    }
    
    // Create SIP transport
    try {
        TransportConfig tcfg;
        tcfg.port = 5060;
        ep->transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
    } catch(Error& err) {
        std::cout << "Transport creation error: " << err.info() << std::endl;
    }
    
    // Start the library (worker threads etc)
    try {
        ep->libStart();
    } catch(Error& err) {
        std::cout << "Startup error: " << err.info() << std::endl;
    }
    setAccount("sip:localhost"); //TODO: remove it if not necessary
}

/**
 Delete lib
 */
void PJSua2::deleteLib()
{
    // Here we don't have anything else to do..
    pj_thread_sleep(500);
    
    // Delete the account. This will unregister from server
    delete acc;
    
    ep->libDestroy();
    delete ep;
}

/**
 Create Account via following config(string username, string password, string ip, string port)
 */
void PJSua2::createAccount(std::string username, std::string password,
                           std::string registrar, std::string port)
{
    // Configure an AccountConfig
    AccountConfig acfg;
    acfg.idUri = "sip:" + username + "@" + registrar + ":" + port;;
    acfg.regConfig.registrarUri = ""; // "sip:" + registrar + ":" + port;;
    acfg.regConfig.timeoutSec = 3600;
    
    AuthCredInfo cred("digest", "*", username, 0, password);
    acfg.sipConfig.authCreds.push_back(cred);
    if(proxyServerAddress.length() > 0) {
        acfg.sipConfig.proxies.push_back(proxyServerAddress);
    }
    acfg.videoConfig.autoShowIncoming = true;
    acfg.videoConfig.autoTransmitOutgoing = true;
    acfg.regConfig.registerOnAdd = false;
    try {
        if(!acc) {
            acc = new MyAccount;
            acc->create(acfg);
        } else {
            acc->modify(acfg);
        }
    } catch (Error& err){
        std::cout << "Account update error: " << err.info() << std::endl;
    }
//    if (!acc) {
//        // Create the account
//        acc = new MyAccount;
//        try {
//            acc->create(acfg);
//        } catch(Error& err) {
//            std::cout << "Account creation error: " << err.info() << std::endl;
//        }
//    } else {
//        // Modify the account
//        try {
//            //Update the registration
//            acc->modify(acfg);
////            acc->setRegistration(true);
//        } catch(Error& err) {
//            std::cout << "Account modify error: " << err.info() << std::endl;
//        }
//    }
}

void PJSua2::setAccount(std::string userId) {
    AccountConfig acfg;
    acfg.idUri =  userId; //"sip:100@pbn-voipqa-1-47.practicebynumber.com";
    acfg.mediaConfig.srtpUse = PJMEDIA_SRTP_DISABLED;
    acfg.mediaConfig.srtpSecureSignaling = 0;
    if(proxyServerAddress.length() > 0) {
        acfg.sipConfig.proxies.push_back(proxyServerAddress);
    }
    acfg.regConfig.registerOnAdd = false;
    acfg.regConfig.disableRegOnModify = true;
    try {
        if(!acc) {
            acc = new MyAccount;
            acc->create(acfg, true);
        } else {
            acc->modify(acfg);
            acc->setDefault();
        }
    } catch(Error& err) {
        std::cout << "Account creation error: " << err.info() << std::endl;
    }
}

/**
 Unregister account
 */
void PJSua2::unregisterAccount()
{
    if(!acc) { return; }
    acc->setRegistration(false);
}

/**
 Make outgoing call (string dest_uri) -> e.g. makeCall(sip:<SIP_USERNAME@SIP_IP:SIP_PORT>)
 */
void PJSua2::outgoingCall(std::string dest_uri)
{
    if(!acc) { return; }
    acc->dest_uri = dest_uri;
    ep->utilTimerSchedule(0, (Token)MAKE_CALL);
}

void PJSua2::transferCall(std::string dest_uri)
{
    if(!acc) { return; }
    acc->transfer_dest = dest_uri;
    ep->utilTimerSchedule(0, (Token)TRANSFER_CALL);
}

/**
 send a DTMF tone to the other caller
 */
void PJSua2::sendDTMFTone(std::string dtmf_tone)
{
    if(!acc) { return; }
    acc->dtmf_tone = dtmf_tone;
    ep->utilTimerSchedule(0, (Token)SEND_DTMF_TONE);
}

/**
 Answer incoming call
 */
void PJSua2::answerCall()
{
    ep->utilTimerSchedule(0, (Token)ANSWER_CALL);
}

void PJSua2::answerCall(std::string dest_uri, std::string channelId, std::string mediaAddr)
{
    if(!acc) { return; }
    acc->dest_uri = dest_uri;
    acc->channelid = channelId;
    acc->mediaAddr = mediaAddr;
    ep->utilTimerSchedule(0, (Token)SEND_INVITE_TO_ANSWER_CALL);
}

void PJSua2::setProxyServerAddress(std::string proxyAddr)
{
    proxyServerAddress = proxyAddr;
}


/**
 Hangup active call (Incoming/Outgoing/Active)
 */
void PJSua2::hangupCall()
{
    if(!ep) { return; }
    ep->utilTimerSchedule(0, (Token)HANGUP_CALL);
}

/**
 Hold the call
 */
void PJSua2::holdCall()
{
    ep->utilTimerSchedule(0, (Token)HOLD_CALL);
}

/**
 Unhold the call
 */
void PJSua2::unholdCall()
{
    ep->utilTimerSchedule(0, (Token)UNHOLD_CALL);
}

void PJSua2::muteCall()
{
    ep->utilTimerSchedule(0, (Token)MUTE_CALL);
}

void PJSua2::unMuteCall()
{
    ep->utilTimerSchedule(0, (Token)UNMUTE_CALL);
}

/**
 Get caller id for incoming call, checks account currently registered (ai.regIsActive)
 */
std::string PJSua2::incomingCallInfo()
{
    return getCallerId();
}

/**
 Listener (When we have incoming call, this function pointer will notify swift.)
 */
void PJSua2::incoming_call(void (* funcpntr)())
{
    incomingCallPtr = funcpntr;
}

/**
 Listener (When we have changes on the call state, this function pointer will notify swift.)
 */
void PJSua2::call_listener(void (* funcpntr)(int, int))
{
    callStatusListenerPtr = funcpntr;
}

/**
 Listener (When we have changes on the acc reg state, this function pointer will notify swift.)
 */
void PJSua2::acc_listener(void (* funcpntr)(bool))
{
    accStatusListenerPtr = funcpntr;
}

void PJSua2::call_transfer_listener(void (* funcpntr)(bool))
{
    callTransferStatusListenerPtr = funcpntr;
}

/**
 Listener (When we have video, this function pointer will notify swift.)
 */
void PJSua2::update_video(void (*funcpntr)(void *))
{
    updateVideoPtr = funcpntr;
}
