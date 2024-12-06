//
//  SipAccount.cpp
//  PJSIPFramework
//
//  Created by Md. MA-Ariful Jannat on 5/12/24.
//

#include "SipAccount.hpp"
#include <iostream>

// Constructor
SIPAccount::SIPAccount()
{
    // Initialization if needed
}

// Destructor
SIPAccount::~SIPAccount()
{
    std::cout<<"Deleting sip account" << std::endl;
    // Invoke shutdown() before deleting any member objects
    shutdown();
}

// Method to handle registration state changes
void SIPAccount::onRegState(OnRegStateParam &prm)
{
    // Handle registration state change logic here
    // You can check prm values to handle the state
}

// Method to handle incoming calls
void SIPAccount::onIncomingCall(OnIncomingCallParam &iprm)
{
    // Handle incoming call logic here
    // You can either answer or reject the call based on iprm
}

// Shutdown function
void SIPAccount::shutdown()
{
    // Add logic for shutting down the SIP account, cleanup, etc.
}
