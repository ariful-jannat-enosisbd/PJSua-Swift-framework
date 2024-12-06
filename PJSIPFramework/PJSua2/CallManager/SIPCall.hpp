//
//  SIPCall.hpp
//  PJSIPFramework
//
//  Created by Md. MA-Ariful Jannat on 5/12/24.
//
#ifndef SIPCall_hpp
#define SIPCall_hpp

#include <stdio.h>
#include <pjsua2.hpp>  // Assuming this is the correct header for PJSUA2

using namespace pj;

class SIPCall;

class SIPCallDelegate {
public:
    virtual void handleCallState(OnCallStateParam &prm, SIPCall* call) = 0;
    virtual void onCallMediaState(OnCallMediaStateParam &prm, SIPCall* call) = 0;
    virtual void onCallTransferStatus(OnCallTransferStatusParam &prm, SIPCall* call) = 0;
};

class SIPCall : public Call {
private:
    SIPCallDelegate *delegate;  // Pointer to CallManager
    std::string customCallId;
public:
    SIPCall(Account &acc, SIPCallDelegate *delegate, std::string customCallId);
    ~SIPCall();
    string getCustomCallId();
    
    virtual void onCallState(OnCallStateParam &prm) override;
    virtual void onCallMediaState(OnCallMediaStateParam &prm) override;
    virtual void onCallTransferStatus(OnCallTransferStatusParam &prm) override;
};

#endif /* SIPCall_hpp */
