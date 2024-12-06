//
//  PJSIPWrapper.m
//  PJSIPFramework
//
//  Created by Md. MA-Ariful Jannat on 5/12/24.
//
#import "PJSIPWrapper.h"
#import "CallManager.hpp"

using namespace std;

class PJSIPInternalDelegate : public CallManagerDelegate {
private:
    __weak PJSIPWrapper *wrapper;
    
public:
    PJSIPInternalDelegate(PJSIPWrapper *wrapper) : wrapper(wrapper) {}
    
    void onCallStateChanged(std::string callId, const int &state, string stateName) override {
        if (wrapper.delegate) {
            // Convert std::string to NSString
            NSString *nsCallId = [NSString stringWithUTF8String:callId.c_str()];
            NSString *nsStatename = [NSString stringWithUTF8String:stateName.c_str()];
            
            dispatch_async(dispatch_get_main_queue(), ^{
                [wrapper.delegate onCallStateChanged:nsCallId state:state stateName:nsStatename];
            });
        }
    }
    
    void onTransferStatusChanged(std::string callId, const int &transferStatus) override {
        if (wrapper.delegate) {
            // Convert std::string to NSString
            NSString *nsCallId = [NSString stringWithUTF8String:callId.c_str()];
            
            dispatch_async(dispatch_get_main_queue(), ^{
                [wrapper.delegate onTransferStatusChanged:nsCallId transferStatus:transferStatus];
            });
        }
    }
    
    void onCallFeatureToggled(string callId, string featureName, bool status) override {
        if (wrapper.delegate) {
            // Convert std::string to NSString
            NSString *nsCallId = [NSString stringWithUTF8String:callId.c_str()];
            NSString *nsFeatureName = [NSString stringWithUTF8String:featureName.c_str()];
            
            dispatch_async(dispatch_get_main_queue(), ^{
                [wrapper.delegate onFeatureStatusToggled:nsCallId featureName:nsFeatureName status:status];
            });
        }
    }
};

@interface PJSIPWrapper ()
@property (nonatomic, assign) CallManager *callManager;
@property (nonatomic, assign) PJSIPInternalDelegate *internalDelegate;
@end

@implementation PJSIPWrapper

+ (instancetype)sharedInstance {
    static PJSIPWrapper *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[PJSIPWrapper alloc] init];
    });
    return sharedInstance;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        self.callManager = new CallManager();
        self.internalDelegate = new PJSIPInternalDelegate(self);
        self.callManager->setDelegate(self.internalDelegate);
    }
    return self;
}

- (void)dealloc {
    delete self.callManager;
    delete self.internalDelegate;
}

- (void)configure:(NSString *)proxy {
    string cProxy = [proxy UTF8String];
    
}

- (void)initializeAndPrepare {
    _callManager->initializeAndPrepare();
}

- (void)setProxyServerAddress:(NSString *)proxy {
    string cProxy = [proxy UTF8String];
    _callManager->setProxyServerAddress(cProxy);
}

// Call management methods
- (void)initiateCallTo:(NSString *)destUri
              username:(NSString *)username
              password:(NSString *)password
                domain:(NSString *)domain
                callId:(NSString*) callId
{
    string cDestUri = [destUri UTF8String];
    string cUsername = [username UTF8String];
    string cPassword = [password UTF8String];
    string cDomain = [domain UTF8String];
    string cCallId = [callId UTF8String];
    EventPayload *payload = new EventPayload(MAKE_CALL, cCallId);
    cout<< "CALL_DEBUG: callId" << callId << endl;
    payload->dest_uri = cDestUri;
    payload->username = cUsername;
    payload->password = cPassword;
    payload->domain = cDomain;
    self.callManager->postEvent(payload);
}

- (void)endCallWithID:(NSString *)callId {
    string cCallId = [callId UTF8String];
    EventPayload* event = new EventPayload(HANGUP_CALL, cCallId);
    _callManager->postEvent(event);
} 

- (void)sendDTMFTone:(nonnull NSString *)data callId:(NSString *)callId {
    string cData = [data UTF8String];
    string cCallId = [callId UTF8String];
    EventPayload* event = new EventPayload(SEND_DTMF_TONE, cCallId);
    event->dtmf_tone = cData;
    _callManager->postEvent(event);
}

- (void)toggleHold:(NSString *)callId status:(bool)holdStatus {
    string cCallId = [callId UTF8String];
    EventPayload* event = new EventPayload(HOLD_UNHOLD_CALL, cCallId);
    event->toggleStatus = holdStatus;
    _callManager->postEvent(event);
}

- (void)toggleMute:(NSString *)callId status:(bool)muteStatus {
    string cCallId = [callId UTF8String];
    EventPayload* event = new EventPayload(MUTE_UNMUTE_CALL, cCallId);
    event->toggleStatus = muteStatus;
    _callManager->postEvent(event);
}

- (void)answerCall:(nonnull NSString *)dest_uri channelId:(nonnull NSString *)channelId mediaAddr:(nonnull NSString *)mediaAddr callId:(NSString *)callId {
    string cDestUri = [dest_uri UTF8String];
    string cChannelId = [channelId UTF8String];
    string cMediaAddr = [mediaAddr UTF8String];
    string cCallId = [callId UTF8String];
    EventPayload *payload = new EventPayload(ANSWER_CALL, cCallId);
    payload->dest_uri = cDestUri;
    payload->channelid = cChannelId;
    payload->mediaAddr = cMediaAddr;
    self.callManager->postEvent(payload);
}

@end
