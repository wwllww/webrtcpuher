#include <utility>
#include "RTCLivePusher.h"
#include "DeviceManager.h"
#include "MediaStream.h"

 MediaStreamImpl::MediaStreamImpl(const std::string& id, RTCLivePusher *peerconnect)
    : m_id(id), m_peerconnect(peerconnect){}
 
 MediaStreamImpl::~MediaStreamImpl() 
 {
   auto audioTrack = m_peerconnect->getAudioTrack();
   if (audioTrack) {
     audioTrack->RemoveSink(this);
   }

   auto videoTrack = m_peerconnect->getVideoTrack();
   if (videoTrack) {
     videoTrack->RemoveSink(&m_dataObserver);
   }
 }


const char* MediaStreamImpl::id() const
{
   return m_id.c_str();
 }