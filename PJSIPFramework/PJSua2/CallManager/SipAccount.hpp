//
//  SipAccount.hpp
//  PJSIPFramework
//
//  Created by Md. MA-Ariful Jannat on 5/12/24.
//



#include <stdio.h>
#include <pjsua2.hpp>  // Assuming this is the correct header for PJSUA2

using namespace pj;

using namespace pj;

class SIPAccount : public Account
{
public:
    // Constructor and Destructor
    SIPAccount();
    ~SIPAccount();
    
    // Virtual methods for registration state and incoming call handling
    virtual void onRegState(OnRegStateParam &prm) override;
    virtual void onIncomingCall(OnIncomingCallParam &iprm) override;
    
private:
    // Any private members, if needed
    void shutdown();
};
