//
//  PJSIPWrapper.h
//  PJSIPFramework
//
//  Created by Md. MA-Ariful Jannat on 5/12/24.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol PJSIPWrapperDelegate <NSObject>
@required
- (void)onCallStateChanged:(NSString*)callId state:(NSInteger) state stateName:(NSString*) stateName;
- (void)onTransferStatusChanged:(NSString*) callId transferStatus:(NSInteger) transferStatus;
- (void)onFeatureStatusToggled:(NSString*) callId featureName:(NSString*) featureName status:(bool) status;
- (void)onExceptionRaised:(NSString*) callId forEventType:(int) eventType errorMessage:(NSString*) errorMessage;
@end

@interface PJSIPWrapper : NSObject

// Singleton instance
+ (instancetype)sharedInstance;

// Set the delegate
@property (nonatomic, weak) id<PJSIPWrapperDelegate> delegate;

- (void) setProxyServerAddress: (NSString *) proxy;
- (void) initializeAndPrepare;

- (void)setDefaultAccount:(NSString *)userId password:(nullable NSString *)password;


/// Initiates a call to the specified destination URI.
///
/// @param destUri The URI of the call destination.
/// @param username The username for authentication.
/// @param password The password for authentication.
/// @param domain The domain of the SIP server.
/// @param callId A unique identifier for the call.
- (void)initiateCallTo:(NSString *)destUri
              username:(NSString *)username
              password:(NSString *)password
                domain:(NSString *)domain
                callId:(NSString *)callId;

- (void) initCall:(NSString *) destUri callId: (NSString *) callId;

/// Answers an incoming call.
///
/// @param dest_uri The URI of the call destination.
/// @param channelId The channel identifier for the call.
/// @param mediaAddr The media address for call communication.
/// @param callId A unique identifier for the call.
- (void)answerCall:(NSString *)dest_uri
          channelId:(NSString *)channelId
          mediaAddr:(NSString *)mediaAddr
             callId:(NSString *)callId;

/// Toggles the hold status of a call.
///
/// @param callId The unique identifier for the call.
/// @param holdStatus A boolean indicating whether to hold (true) or resume (false) the call.
- (void)toggleHold:(NSString *)callId
             status:(bool)holdStatus;

/// Toggles the mute status of a call.
///
/// @param callId The unique identifier for the call.
/// @param muteStatus A boolean indicating whether to mute (true) or unmute (false) the call.
- (void)toggleMute:(NSString *)callId
             status:(bool)muteStatus;

/// Sends a DTMF tone during a call.
///
/// @param data The DTMF tone data to send.
/// @param callId The unique identifier for the call.
- (void)sendDTMFTone:(NSString *)data
               callId:(NSString *)callId;

- (void) blindTransferCall:(NSString *) destUri
                    callId:(NSString *)callId;

/// Ends a specific call
///
/// @param callId The id of the call that should disconnect
- (void)endCallWithID:(NSString *)callId;

@end

NS_ASSUME_NONNULL_END
