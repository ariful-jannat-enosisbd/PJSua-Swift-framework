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
    BLIND_TRANSFER_CALL = 7,
};

class EventPayload {
public:
    // Use optional to make fields nullable
    CallAction type;
    string callId;
    
    optional<string> destUri;
    optional<string> dtmf_tone;
    optional<string> transfer_dest;
    
    // Fields for incoming calls
    optional<string> channelid;
    optional<string> mediaAddr;
    bool toggleStatus = false;
    bool useDefaultAccount = false;
    
    optional<string> username;
    optional<string> password;
    optional<string> domain;
    
    // Constructor with default values
    EventPayload(CallAction type, string callId)
    : type(type), callId(callId) { }
};

/**
 * @class CallManagerDelegate
 * @brief Interface for handling call-related events and state changes.
 *
 * This class defines the methods that a delegate must implement to handle various events such as
 * call state changes, transfer status updates, and call feature toggles.
 */
class CallManagerDelegate {
public:
    /**
     * @brief Notifies the delegate of a change in the call state.
     *
     * This method is called when the state of a call changes (e.g., from ringing to connected).
     *
     * @param callId The unique identifier for the call.
     * @param state The numeric state of the call (e.g., 0 for idle, 1 for active).
     * @param stateName A descriptive name for the call state (e.g., "RINGING", "CONNECTED").
     */
    virtual void onCallStateChanged(string callId, const int &state, string stateName) = 0;

    /**
     * @brief Notifies the delegate about a change in the transfer status of the call.
     *
     * This method is called when a call transfer action has completed or failed.
     *
     * @param callId The unique identifier for the call being transferred.
     * @param transferStatus The numeric status of the transfer (e.g., 0 for failed, 1 for successful).
     */
    virtual void onTransferStatusChanged(string callId, const int &transferStatus) = 0;

    /**
     * @brief Notifies the delegate about a change in the status of a call feature.
     *
     * This method is called when a call feature (such as mute or hold) is toggled.
     *
     * @param callId The unique identifier for the call.
     * @param featureName The name of the feature being toggled (e.g., "MUTE", "HOLD").
     * @param status The new status of the feature (true for enabled, false for disabled).
     */
    virtual void onCallFeatureToggled(string callId, string featureName, bool status) = 0;
    virtual void onExceptionRaised(string callId, int forEventType, string message) = 0;
};


/**
 * @class CallManager
 * @brief Manages SIP calls and interacts with the SIP stack.
 *
 * The CallManager class is responsible for managing active SIP calls, handling call-related
 * events (e.g., call initiation, answering, holding, muting), and providing updates on call state.
 * It also communicates with a delegate to notify changes and events regarding the calls.
 * This class implements the `SIPCallDelegate` interface to handle SIP call state updates.
 */
class CallManager : public Endpoint, public SIPCallDelegate {
private:
    /**
     * @brief A map of active SIP calls, keyed by call ID.
     * This allows tracking of multiple calls by their unique call ID.
     */
    unordered_map<string, SIPCall*> activeCalls;

    /**
     * @brief A map of active SIP accounts, keyed by account ID.
     * This stores the active SIP accounts for managing multiple account configurations.
     */
    SIPAccount* defaultAccount;

    /**
     * @brief An optional proxy server address for SIP communication.
     * This can be configured if using a SIP proxy server.
     */
    string proxyServer;

    /**
     * @brief The delegate to notify about call state changes and other events.
     * The delegate must implement the `CallManagerDelegate` interface.
     */
    CallManagerDelegate* delegate;

    /**
     * @brief Flag to indicate if the CallManager has been initialized.
     */
    bool isInitialized = false;

    /**
     * @brief Initiates an outbound call with the given payload.
     *
     * @param payload The event payload containing details about the call to initiate.
     */
    void initiateCallInternal(EventPayload *payload);
    void initCallWithDefaultAccount(EventPayload *payload);
    void initiateCall(SIPAccount* account, string destUri, string callId);

    /**
     * @brief Answers an incoming call with the provided payload.
     *
     * @param payload The event payload containing details about the incoming call.
     */
    void answerCall(EventPayload *payload);

    /**
     * @brief Toggles the hold state of an active call.
     *
     * @param payload The event payload containing the call information and hold/unhold action.
     */
    void toggleHold(EventPayload *payload);

    /**
     * @brief Toggles the mute state of an active call.
     *
     * @param payload The event payload containing the call information and mute/unmute action.
     */
    void toggleMute(EventPayload *payload);

    /**
     * @brief Sends a DTMF tone during the active call.
     *
     * @param payload The event payload containing the DTMF tone information.
     */
    void sendDTMFTone(EventPayload *payload);

    /**
     * @brief Hangs up an active call.
     *
     * @param payload The event payload containing the call information.
     */
    void hangupCall(EventPayload *payload);

    /**
     * @brief Performs a blind transfer of an active call.
     *
     * @param payload The event payload containing the target address for the call transfer.
     */
    void blindTransferCall(EventPayload *payload);

    /**
     * @brief Retrieves the SIPCall object for a specific call using the call ID.
     *
     * @param callId The unique identifier for the call.
     * @return The SIPCall object corresponding to the provided call ID, or nullptr if not found.
     */
    SIPCall* getCallWithId(string callId);

public:
    /**
     * @brief Constructs a CallManager object.
     * Initializes the necessary resources and sets up the environment for managing calls.
     */
    CallManager();

    /**
     * @brief Destroys the CallManager object and releases resources.
     * Cleans up any active calls and accounts.
     */
    ~CallManager();
    
    /**
     * @brief Initializes and prepares the CallManager for use.
     * This method should be called after construction to set up necessary configurations.
     */
    void initializeAndPrepare();

    /**
     * @brief Sets the address of the proxy server for SIP communication.
     *
     * @param proxyAddress The address of the SIP proxy server (e.g., "sip.example.com").
     */
    void setProxyServerAddress(const string proxyAddress);

    /**
     * @brief Sets the delegate for receiving call state updates and events.
     *
     * @param del The delegate that implements the `CallManagerDelegate` interface.
     */
    void setDelegate(CallManagerDelegate* del);

    /**
     * @brief Handles timer-based events.
     * This method is called periodically by the timer to process events.
     *
     * @param prm The parameters related to the timer event.
     */
    void onTimer(const OnTimerParam &prm) override;

    /**
     * @brief Handles changes in the call state, as notified by the SIPCallDelegate.
     *
     * @param prm The parameters related to the call state change.
     * @param call The SIPCall object that represents the call.
     */
    void handleCallState(OnCallStateParam &prm, SIPCall* call) override;

    /**
     * @brief Handles changes in the media state of a call, as notified by the SIPCallDelegate.
     *
     * @param prm The parameters related to the media state change.
     * @param call The SIPCall object that represents the call.
     */
    void onCallMediaState(OnCallMediaStateParam &prm, SIPCall* call) override;

    /**
     * @brief Handles changes in the transfer status of a call, as notified by the SIPCallDelegate.
     *
     * @param prm The parameters related to the transfer status change.
     * @param call The SIPCall object that represents the call.
     */
    void onCallTransferStatus(OnCallTransferStatusParam &prm, SIPCall* call) override;

    /**
     * @brief Posts an event to the event queue for processing.
     *
     * @param event The event payload containing details about the event to be processed.
     */
    void postEvent(EventPayload *event);
    
    void setDefaultAccount(string userId, string password);
};

#endif /* CALLMANAGER_HPP */
