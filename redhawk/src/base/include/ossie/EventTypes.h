#ifndef  __RH_CF_EVENT_TYPES__
#define  __RH_CF_EVENT_TYPES__
#include <COS/CosLifeCycle.hh>
#include <ossie/CF/StandardEvent.h>
#include <ossie/CF/EventChannelManager.h>


namespace ossie {

  namespace events {

    typedef  CosLifeCycle::GenericFactory                      EventChannelFactory;
    typedef  CosLifeCycle::GenericFactory_ptr                  EventChannelFactory_ptr;
    typedef  CosLifeCycle::GenericFactory_var                  EventChannelFactory_var;

    typedef  CF::EventChannelManager                           EventChannelManager;
    typedef  CF::EventChannelManager_ptr                       EventChannelManager_ptr;
    typedef  CF::EventChannelManager_var                       EventChannelManager_var;

    typedef  POA_CF::EventChannelInfoIterator                  EventChannelInfoIteratorBase;
    typedef  CF::EventChannelManager::EventChannelInfo         EventChannelInfo;
    typedef  CF::EventChannelManager::EventChannelInfoList     EventChannelInfoList;
    typedef  CF::EventChannelInfoIterator                      EventChannelInfoIterator;
    typedef  CF::EventChannelManager::EventRegistrant          EventRegistrant;
    typedef  CF::EventChannelManager::EventRegistrantList      EventRegistrantList;
    typedef  CF::EventRegistrantIterator                       EventRegistrantIterator;

    typedef  CF::EventChannel                                  EventChannel; 
    typedef  CF::EventChannel_ptr                              EventChannel_ptr;
    typedef  CF::EventChannel_var                              EventChannel_var;

    typedef  CF::EventPublisher                                EventPublisher; 
    typedef  CF::EventPublisher_ptr                            EventPublisher_ptr;
    typedef  CF::EventPublisher_var                            EventPublisher_var;
    typedef  CosEventComm::PushSupplier                        EventPublisherSupplier;
    typedef  CosEventComm::PushSupplier_ptr                    EventPublisherSupplier_ptr;
    typedef  CosEventComm::PushSupplier_var                    EventPublisherSupplier_var;
    typedef  POA_CosEventComm::PushSupplier                    EventPublisherSupplierPOA;

    typedef  CF::EventSubscriber                               EventSubscriber; 
    typedef  CF::EventSubscriber_var                           EventSubscriber_var;
    typedef  CF::EventSubscriber_ptr                           EventSubscriber_ptr;
    typedef  CosEventComm::PushConsumer                        EventSubscriberConsumer;
    typedef  CosEventComm::PushConsumer_ptr                    EventSubscriberConsumer_ptr;
    typedef  CosEventComm::PushConsumer_var                    EventSubscriberConsumer_var;
    typedef  POA_CosEventComm::PushConsumer                    EventSubscriberConsumerPOA;

    typedef  CF::EventChannelManager::EventRegistration        EventRegistration;
    typedef  CF::EventChannelManager::EventRegistration_var    EventRegistration_var;
    typedef  CF::EventChannelManager::EventChannelReg          EventChannelReg;
    typedef  CF::EventChannelManager::EventChannelReg_var      EventChannelReg_var;
    typedef  CF::EventChannelManager::EventChannelReg*         EventChannelReg_ptr;

  };  // end of event namespace

namespace event {
    // this is added for API compatibility reasons
    using namespace events;
}

}; // end of ossie namespace

#endif
