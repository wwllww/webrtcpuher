/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// This file provides an example of unity native plugin APIs.

#ifndef EXAMPLES_UNITYPLUGIN_UNITY_PLUGIN_APIS_H_
#define EXAMPLES_UNITYPLUGIN_UNITY_PLUGIN_APIS_H_

#include <stdint.h>

//#define USING_FUNCTION_CB

/// webrtc::PeerConnectionInterface::IceConnectionState
enum ICE_CONNECTION_STATE {
  ICE_STATE_NEW = 0,
  ICE_STATE_CHECKING,
  ICE_STATE_CONNECTED,
  ICE_STATE_COMPLETED,
  ICE_STATE_FAILED,
  ICE_STATE_DISCONNECTED,
  ICE_STATE_CLOSED,
  ICE_STATE_MAX
};

enum PEERCONNECTIONSTATE {
  kNew,
  kConnecting,
  kConnected,
  kDisconnected,
  kFailed,
  kClosed
};

// Definitions of callback functions.
#ifdef USING_FUNCTION_CB

#include <functional>

typedef std::function<void(const uint8_t* data_y,
                           const uint8_t* data_u,
                           const uint8_t* data_v,
                           const uint8_t* data_a,
                           int stride_y,
                           int stride_u,
                           int stride_v,
                           int stride_a,
                           uint32_t width,
                           uint32_t height)>
    I420FRAMEREADY_CALLBACK;
typedef std::function<void()> LOCALDATACHANNELREADY_CALLBACK;
typedef std::function<void(const char* msg)> DATAFROMEDATECHANNELREADY_CALLBACK;
typedef std::function<void(const char* msg)> FAILURE_CALLBACK;
typedef std::function<void(const char* type, const char* sdp)>
    LOCALSDPREADYTOSEND_CALLBACK;
typedef std::function<
    void(const char* candidate, const int sdp_mline_index, const char* sdp_mid)>
    ICECANDIDATEREADYTOSEND_CALLBACK;
typedef std::function<void(ICE_CONNECTION_STATE state, const char* offer)>
    ICECSTATECHANGE_CALLBACK;
typedef std::function<void(const void* audio_data,
                           int bits_per_sample,
                           int sample_rate,
                           int number_of_channels,
                           int number_of_frames)>
    AUDIOBUSREADY_CALLBACK;

#else

#define Callback_def_0(name)                    \
  struct name {                                 \
    typedef void (*Func)(void*);                \
	name(Func f = nullptr, void* v=nullptr) : func(f), context(v) {} \
    void operator()() { if (func) func(context); }        \
	explicit operator bool() const { return func != nullptr; }\
    \
    Func func;                                  \
    void* context;                              \
  }

#define Callback_def_1(name, t1, p1)              \
  struct name {                                   \
	typedef void (*Func)(t1, void*);              \
	name(Func f = nullptr, void* v=nullptr) : func(f), context(v) {} \
    void operator()(t1 p1) { if (func) func(p1, context); } \
    explicit operator bool() const { return func != nullptr; }\
                                                  \
    Func func;                                    \
    void* context;                                \
  }
#define Callback_def_2(name, t1, p1, t2, p2)                 \
  struct name {                                              \
	typedef void (*Func)(t1, t2, void*);                  \
    name(Func f = nullptr, void* v = nullptr) : func(f), context(v) {} \
    void operator()(t1 p1, t2 p2) { if (func) func(p1, p2, context); } \
    explicit operator bool() const { return func != nullptr; }\
                                                             \
    Func func;                                               \
    void* context;                                           \
  }
#define Callback_def_3(name, t1, p1, t2, p2, t3, p3)                    \
  struct name {                                                         \
	typedef void (*Func)(t1, t2, t3, void*);              \
    name(Func f = nullptr, void* v = nullptr) : func(f), context(v) {} \
    void operator()(t1 p1, t2 p2, t3 p3) { if (func) func(p1, p2, p3, context); } \
    explicit operator bool() const { return func != nullptr; }\
                                                                        \
    Func func;                                                          \
    void* context;                                                      \
  }

#define Callback_def_5(name, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5) \
  struct name {                                                      \
	typedef void (*Func)(t1, t2, t3, t4, t5, void*);                 \
    name(Func f = nullptr, void* v = nullptr) : func(f), context(v) {} \
    void operator()(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5) {             \
     if (func)                                                      \
        func(p1, p2, p3, p4, p5, context);                             \
    }                                                                \
    explicit operator bool() const { return func != nullptr; }\
                                                                     \
    Func func;                                                       \
    void* context;                                                   \
  }

#define Callback_def_6(name, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5,t6,p6)   \
  struct name {                                                        \
    typedef void (*Func)(t1, t2, t3, t4, t5,t6,void*);                   \
    name(Func f = nullptr, void* v = nullptr) : func(f), context(v) {} \
    void operator()(t1 p1, t2 p2, t3 p3, t4 p4, t5 p5,t6 p6) {               \
      if (func)                                                        \
        func(p1, p2, p3, p4, p5, p6,context);                             \
    }                                                                  \
    explicit operator bool() const { return func != nullptr; }         \
                                                                       \
    Func func;                                                         \
    void* context;                                                     \
  }
#define Callback_def_11(name, t1, p1, t2, p2, t3, p3, t4, p4, t5, p5, t6, p6, \
                        t7, p7, t8, p8, t9, p9, t10, p10,t11,p11)                     \
  struct name {                                                               \
	typedef void (*Func)(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10,t11,void*);     \
    name(Func f = nullptr, void* v=nullptr) : func(f), context(v) {} \
    void operator()(t1 p1,                                                    \
                    t2 p2,                                                    \
                    t3 p3,                                                    \
                    t4 p4,                                                    \
                    t5 p5,                                                    \
                    t6 p6,                                                    \
                    t7 p7,                                                    \
                    t8 p8,                                                    \
                    t9 p9,                                                    \
                    t10 p10,                                                  \
                    t11 p11) {                                                \
      if (func)                                                               \
        func(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11,context);                 \
    }                                                                         \
    explicit operator bool() const { return func != nullptr; }\
                                                                              \
    Func func;                                                                \
    void* context;                                                            \
  }

Callback_def_11(I420FRAMEREADY_CALLBACK,
                const uint8_t*,                data_y,
                const uint8_t*,                data_u,
                const uint8_t*,                data_v,
                const uint8_t*,                data_a,
                int,                stride_y,
                int,                stride_u,
                int,                stride_v,
                int,                stride_a,
                uint32_t,                width,
                uint32_t,                height,
                int64_t,            render_time);
  Callback_def_0(LOCALDATACHANNELREADY_CALLBACK);
Callback_def_1(DATAFROMEDATECHANNELREADY_CALLBACK, const char*, msg);
Callback_def_1(FAILURE_CALLBACK, const char*, msg);
Callback_def_2(LOCALSDPREADYTOSEND_CALLBACK,
               const char*,               type,
               const char*,               sdp);
Callback_def_3(ICECANDIDATEREADYTOSEND_CALLBACK,
               const char*,               candidate,
               const int,                 sdp_mline_index,
               const char*,               sdp_mid);
Callback_def_2(ICECSTATECHANGE_CALLBACK,
               ICE_CONNECTION_STATE,      state, 
			   const char*,               offer);
Callback_def_5(AUDIOBUSREADY_CALLBACK,
               const void*,               audio_data,
               int,               bits_per_sample,
               int,               sample_rate,
               int,               number_of_channels,
               int,               number_of_frames);

Callback_def_6(AUDIOBUSREADY_CALLBACK_WITHTYPE,
               const void*,    audio_data,
               int,            bits_per_sample,
               int,            sample_rate,
               int,            number_of_channels,
               int,            number_of_frames,
	           bool,           brecord_data);

Callback_def_3(CONNECTSTATECHANGE_CALLBACK,
               PEERCONNECTIONSTATE,state,
               uint32_t, uid,
               bool, bClose);

Callback_def_2(SEI_CALLBACK,
               const char*,
               data,
               uint32_t,
               len);

#endif

#endif  // EXAMPLES_UNITYPLUGIN_UNITY_PLUGIN_APIS_H_
