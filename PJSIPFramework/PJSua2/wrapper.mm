///*
// * Copyright (C) 2012-2012 Teluu Inc. (http://www.teluu.com)
// * Contributed by Emre Tufekci (github.com/emretufekci)
// *
// * This program is free software; you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation; either version 2 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program; if not, write to the Free Software
// * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// */
//
//#import "wrapper.h"
//#import "CustomPJSUA2.hpp"
//
///**
// Create a object from .hpp class & wrapper to be able to use it via Swift
// */
//@implementation CPPWrapper
//PJSua2 pjsua2;
//
////Lib
///**
// Create Lib with EpConfig
// */
//-(void) createLibWrapper
//{
//    return pjsua2.createLib();
//};
//
///**
// Delete lib
// */
//-(void) deleteLibWrapper {
//    pjsua2.deleteLib();
//}
//
////Account
///**
// Create Account via following config(string username, string password, string ip, string port)
// */
//-(void) createAccountWrapper :(NSString*) usernameNS :(NSString*) passwordNS
//                             :(NSString*) registrarNS :(NSString*) portNS
//{
//    std::string username = std::string([[usernameNS componentsSeparatedByString:@"*"][0] UTF8String]);
//    std::string password = std::string([[passwordNS componentsSeparatedByString:@"*"][0] UTF8String]);
//    std::string registrar = std::string([[registrarNS componentsSeparatedByString:@"*"][0] UTF8String]);
//    std::string port = std::string([[portNS componentsSeparatedByString:@"*"][0] UTF8String]);
//    
//    pjsua2.createAccount(username, password, registrar, port);
//}
//
//- (void)setProxyServerAddress:(NSString *)proxyAddr
//{
//    std::string proxyAddress = std::string([proxyAddr UTF8String]);;
//    pjsua2.setProxyServerAddress(proxyAddress);
//}
//
///**
// Unregister account
// */
//-(void) unregisterAccountWrapper {
//    return pjsua2.unregisterAccount();
//}
//
///**
// Answer incoming call
// */
//- (void) answerCallWrapper {
//    pjsua2.answerCall();
//}
//
//- (void)answerCall:(NSString *)dest_uri channelId:(NSString *)channelId mediaAddr:(NSString *)mediaAddr
//{
//    std::string channelIdStr = std::string([channelId UTF8String]);
//    std::string mediaAddrStr = std::string([mediaAddr UTF8String]);;
//    std::string desturiStr = std::string([dest_uri UTF8String]);;
//    pjsua2.answerCall(desturiStr, channelIdStr, mediaAddrStr);
//}
//
//- (void) setUserAccount:(NSString *)userId
//{
//    std::string userIdStr = std::string([userId UTF8String]);
//    pjsua2.setAccount(userIdStr);
//}
///**
// Hangup active call (Incoming/Outgoing/Active)
// */
//- (void) hangupCallWrapper {
//    pjsua2.hangupCall();
//}
//
///**
// Hold the call
// */
//- (void) holdCallWrapper{
//    pjsua2.holdCall();
//}
//
///**
// unhold the call
// */
//- (void) unholdCallWrapper{
//    pjsua2.unholdCall();
//}
//
//-(void) muteCallWrapper {
//    pjsua2.muteCall();
//}
//
//-(void) unMuteCallWrapper {
//    pjsua2.unMuteCall();
//}
//
///**
// Make outgoing call (string dest_uri) -> e.g. makeCall(sip:<SIP_USERNAME@SIP_URI:SIP_PORT>)
// */
//-(void) outgoingCallWrapper :(NSString*) dest_uriNS
//{
//    std::string dest_uri = std::string([[dest_uriNS componentsSeparatedByString:@"*"][0] UTF8String]);
//    pjsua2.outgoingCall(dest_uri);
//}
//
//-(void) transferCallWrapper :(NSString*) dest_uriNS
//{
//    std::string dest_uri = std::string([[dest_uriNS componentsSeparatedByString:@"*"][0] UTF8String]);
//    pjsua2.transferCall(dest_uri);
//}
//
//-(void) sendDTMFTone: (NSString*) dtmf_tone
//{
//    // Extract the first character and convert to std::string
//    std::string tone_str = std::string([[dtmf_tone substringToIndex:1] UTF8String]);
//    pjsua2.sendDTMFTone(tone_str);
//}
//
//// Factory method to create NSString from C++ string
///**
// Get caller id for incoming call, checks account currently registered (ai.regIsActive)
// */
//- (NSString*) incomingCallInfoWrapper {
//    NSString* result = [NSString stringWithUTF8String:pjsua2.incomingCallInfo().c_str()];
//    return result;
//}
//
///**
// Listener (When we have incoming call, this function pointer will notify swift.)
// */
//- (void)incoming_call_wrapper: (void(*)())function {
//    pjsua2.incoming_call(function);
//}
//
///**
// Listener (When we have changes on the call state, this function pointer will notify swift.)
// */
//- (void)call_listener_wrapper: (void(*)(int,int))function {
//    pjsua2.call_listener(function);
//}
//
//
///**
// Listener (When we have changes on the acc reg state, this function pointer will notify swift.)
// (Runs swift code from C++)
// */
//-(void) acc_listener_wrapper: (void(*)(bool))function {
//    pjsua2.acc_listener(function);
//}
//
//-(void) call_transfer_wrapper: (void(*)(bool))function {
//    pjsua2.call_transfer_listener(function);
//}
//
///**
// Listener (When we have video, this function pointer will notify swift.)
// (Runs swift code from C++)
// */
//-(void) update_video_wrapper: (void(*)(void *))function {
//    pjsua2.update_video(function);
//}
//
//@end
