//
//  SIPCall.cpp
//  PJSIPFramework
//
//  Created by Md. MA-Ariful Jannat on 5/12/24.
//

#include "SIPCall.hpp"

SIPCall::SIPCall(Account &acc, SIPCallDelegate *delegate, std::string customCallId)
: Call(acc, PJSUA_INVALID_ID), delegate(delegate), customCallId(customCallId) { }

SIPCall::~SIPCall() {
    
}

void SIPCall::onCallState(OnCallStateParam &prm) {
    if (delegate) {
        delegate->handleCallState(prm, this);
    }
}

string SIPCall::getCustomCallId() {
    return customCallId;
}

void SIPCall::onCallMediaState(OnCallMediaStateParam &prm) {
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
//                void *window = ci.media[i].videoWindow.getInfo().winHandle.handle.window;
//                updateVideoPtr(window);// TODO: We are not supporting video now
            }
        }
    }
    if(delegate) {
        delegate->onCallMediaState(prm, this);
    }
}

void SIPCall::onCallTransferStatus(OnCallTransferStatusParam &prm) {
    if(delegate) {
        delegate->onCallTransferStatus(prm, this);
    }
}
