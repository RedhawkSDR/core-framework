/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include "bulkio_p.h"
#include "bulkio_attachable_base.h"

namespace bulkio {
  
  /**
   * Wrap Callback functions as SriListerer objects
   */
  class StaticSriCallback : public SriListener
  {
  public:
    virtual void operator() ( BULKIO::StreamSRI& sri)
    {
      (*func_)(sri);
    }

    StaticSriCallback ( SriListenerCallbackFn func) :
      func_(func)
    {
    }

  private:

    SriListenerCallbackFn func_;
  };
  
  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::InAttachablePort(std::string port_name, 
                                     InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::Callback *attach_detach_cb,
                                     bulkio::sri::Compare  sriCmp,
                                     bulkio::time::Compare timeCmp,
                                     SRICallback *newSriCB,
                                     SRICallback *sriChangeCB):
      Port_Provides_base_impl(port_name),
      sriChanged(false),
      sri_cmp(sriCmp),
      time_cmp(timeCmp),
      attach_detach_callback(attach_detach_cb),
      newSRICallback(),
      sriChangeCallback()
  {
    this->stats = new linkStatistics(port_name);

    if ( newSriCB ) {
      newSRICallback = *newSriCB;
    }
    
    if ( sriChangeCB ) {
      sriChangeCallback = *sriChangeCB;
    }
  }


  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::InAttachablePort(std::string port_name, 
                                     LOGGER_PTR    logger,
                                     InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::Callback *attach_detach_cb,
                                     bulkio::sri::Compare sriCmp,
                                     bulkio::time::Compare timeCmp,
                                     SRICallback *newSriCB,
                                     SRICallback *sriChangeCB): 
      Port_Provides_base_impl(port_name),
      sriChanged(false),
      sri_cmp(sriCmp),
      time_cmp(timeCmp),
      attach_detach_callback(attach_detach_cb),
      logger(logger),
      newSRICallback(),
      sriChangeCallback()
  {
      stats = new linkStatistics(port_name);
      LOG_DEBUG( logger, "bulkio::InAttachablePort CTOR port:" << port_name );

      if ( newSriCB ) {
        newSRICallback = *newSriCB;
      }
      
      if ( sriChangeCB ) {
        sriChangeCallback = *sriChangeCB;
      }
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::~InAttachablePort() {
      if (stats) delete stats;
  }


  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  bool InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::hasSriChanged () {
    return sriChanged;
  }
  
  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  void InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::setNewSriListener( SRICallback newCallback ) {
    newSRICallback = newCallback;
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  void InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::setNewSriListener( SRICallbackFn newCallback ) {
    newSRICallback = SRICallback(newCallback);
  }
  
  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  void InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::setSriChangeListener( SRICallback newCallback ) {
    newSRICallback = newCallback;
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  void InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::setSriChangeListener( SRICallbackFn newCallback ) {
    newSRICallback = SRICallback(newCallback);
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  void InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::setLogger( LOGGER_PTR newLogger ) 
  {
    logger = newLogger;
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  void InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::setNewAttachDetachCallback( Callback *newCallback) {
    attach_detach_callback = newCallback;
  }

  //
  // Port Statistics Interface
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  void InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::enableStats(bool enable)  
  {
    if (stats) stats->setEnabled(enable);
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  void InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::setBitSize(double bitSize) {
    if (stats) stats->setBitSize(bitSize);
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  void InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::updateStats(unsigned int elementsReceived, float queueSize, bool EOS, std::string streamID) {
    boost::mutex::scoped_lock lock(statUpdateLock);
    if (stats) stats->update(elementsReceived, queueSize, EOS, streamID );
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  BULKIO::PortStatistics * InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::statistics()
  {
    boost::mutex::scoped_lock lock(statUpdateLock);
    if (stats) {
      BULKIO::PortStatistics_var recStat = new BULKIO::PortStatistics(stats->retrieve());
      return recStat._retn();
    }
    return NULL;
  }


  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  BULKIO::PortUsageType InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::state()
  {
    // Blocks when values are updating
    READ_LOCK lock(attachmentLock);
    
    if (attachedStreamMap.size() == 0) {
      return BULKIO::IDLE;
    } else if (attachedStreamMap.size() == 1) {
      return BULKIO::BUSY;
    } else {
      return BULKIO::ACTIVE;
    }
  }


  //
  // updateSRI Interface
  //

  //
  // activeSRIs - returns a sequence of BULKIO::StreamSRI objectsPort
  //
  // @return BULKIO::StreamSRISequence - list of active SRI objects for this port
  //                                     the caller is responsible for freeing the memory returned
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  BULKIO::StreamSRISequence * InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::activeSRIs()
  {
    READ_LOCK lock(sriUpdateLock);
    BULKIO::StreamSRISequence seq_rtn;
    SriMap::iterator currH;
    int i = 0;
    for (currH = currentHs.begin(); currH != currentHs.end(); currH++) {
      i++;
      seq_rtn.length(i);
      seq_rtn[i-1] = currH->second.first;
    }
    BULKIO::StreamSRISequence_var retSRI = new BULKIO::StreamSRISequence(seq_rtn);

    // NOTE: You must delete the object that this function returns!
    return retSRI._retn();
  }


  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  void InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::pushSRI(const BULKIO::StreamSRI& H, const BULKIO::PrecisionUTCTime& T)
  {
    TRACE_ENTER(logger, "InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::pushSRI" );

    // Shared mutex allows concurrent reads
    //   Upgrades to unique lock while modifying values
    boost::upgrade_lock< boost::shared_mutex > lock(sriUpdateLock);
    
    bool foundSRI = false;
    BULKIO::StreamSRI tmpH = H;
    SriMap::iterator sriIter;

    sriIter = currentHs.begin();
    while (sriIter != currentHs.end()) {
      if (strcmp(H.streamID, (*sriIter).first.c_str()) == 0) {
        foundSRI = true;
        break;
      }
      sriIter++;
    }
    if (!foundSRI) {
      if ( newSRICallback ) { 
        LOG_DEBUG(logger, "pushSRI: About to call user-defined 'newSRICallback' method")
        newSRICallback(tmpH);
        LOG_DEBUG(logger, "pushSRI: Returned from user-defined 'newSRICallback' method")
      }

      {
        // Upgrade lock to unique - Blocks all reads
        boost::upgrade_to_unique_lock< boost::shared_mutex > uniqueLock(lock);
        currentHs.insert(std::make_pair(CORBA::string_dup(H.streamID), std::make_pair(H, T)));
        sriChanged = true;
      }

    } else {
      bool schanged = false;
      if ( sri_cmp != NULL ) {
        schanged = sri_cmp( (*sriIter).second.first, H );
      }
      bool tchanged = false;
      if ( time_cmp != NULL ) {
        tchanged = time_cmp( (*sriIter).second.second, T );
      }
      sriChanged = !schanged || !tchanged;
 
      if ( sriChanged && sriChangeCallback ) {
        LOG_DEBUG(logger, "pushSRI: About to call user-defined 'sriChangeCallback' method")
        sriChangeCallback(tmpH);
        LOG_DEBUG(logger, "pushSRI: Returned from user-defined 'sriChangeCallback' method")
      } 
  
      {
        // Upgrade lock to unique - Blocks all reads
        boost::upgrade_to_unique_lock< boost::shared_mutex > uniqueLock(lock);
        (*sriIter).second = std::make_pair(H, T);
      }
    }

    TRACE_EXIT(logger, "InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::pushSRI" );
  }

  //
  // Attachable Interface
  //

  //
  // attach
  //
  // Request an attach operation to the provided stream definition.  Each requestor is required
  // to provide a subscriber id to track attachment requests.  Upon successfull
  // completion, this method returns an attachment identifier.  This identifier is required
  // to perform the detach operation
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  char* InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::attach(const StreamDefinition& stream, const char* userid)
    throw (typename PortType::AttachError, typename PortType::StreamInputError) 
  {
    TRACE_ENTER(logger, "InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::attach" );
    LOG_DEBUG( logger, "ATTACHABLE PORT: ATTACH REQUEST, STREAM/USER: " << stream.id <<  "/" << userid );

    std::string attachId("");

    if ( attach_detach_callback ) {
      try {
        LOG_DEBUG( logger, "ATTACHABLE PORT: CALLING ATTACH CALLBACK, STREAM/USER: " << stream.id <<  "/" << userid );
        attachId = attach_detach_callback->attach(stream, userid);
      }
      catch(...) {
        LOG_ERROR( logger, "ATTACHABLE PORT: ATTACH CALLBACK EXCEPTION, STREAM/USER: " << stream.id <<  "/" << userid );
        throw typename PortType::AttachError("Callback Failed.");
      }
    }
    if ( attachId.size() == 0 ) {
      attachId = ossie::generateUUID();
    }

    // Unique lock of shared_mutex - Blocks all other reads/writes
    boost::unique_lock< boost::shared_mutex > lock(attachmentLock);

    attachedStreamMap.insert(std::make_pair(attachId, new StreamDefinition(stream)));
    attachedUsers.insert(std::make_pair(attachId, std::string(userid)));

    LOG_DEBUG( logger, "ATTACHABLE PORT, ATTACH COMPLETED, ID:" << attachId << 
               " STREAM/USER" << stream.id <<  "/" << userid );

    TRACE_EXIT(logger, "InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::attach" );
    return CORBA::string_dup(attachId.c_str());
  }


  //
  // detach
  //
  // Process a request to detach from a stream for the provided attachment identifier.
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  void InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::detach(const char* attachId)
  {
    TRACE_ENTER(logger, "InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::detach" );
    LOG_DEBUG( logger, "ATTACHABLE PORT: DETACH REQUESTED,  ID:" << attachId   );
    if ( attach_detach_callback )  {

      try {
        LOG_DEBUG( logger, "ATTACHABLE PORT: CALLING DETACH CALLBACK, ID:" << attachId );
        attach_detach_callback->detach(attachId);
        LOG_DEBUG( logger, "ATTACHABLE PORT: RETURNED FROM DETACH CALLBACK, ID:" << attachId );
      }
      catch(typename PortType::DetachError &ex) {
        throw ex;
      }
      catch(...) {
        LOG_ERROR( logger, "ATTACHABLE PORT: DETACH CALLBACK EXCEPTION ID:" << attachId  );
        throw typename PortType::DetachError("Unknown issue occured in the detach callback!");
      }
    }
    
    // Unique lock of shared_mutex - Blocks all other reads/writes
    boost::unique_lock< boost::shared_mutex > lock(attachmentLock);

    //
    // remove item from attachStreamMap
    //
    try {
      typename AttachedStreams::iterator itr = attachedStreamMap.find(attachId);
      if ( itr != attachedStreamMap.end() )
        delete itr->second;

      attachedStreamMap.erase(attachId);
      attachedUsers.erase(attachId);
    }
    catch(...) {
      throw typename PortType::DetachError("Unknown Attach ID");
    }

    LOG_DEBUG( logger, "ATTACHABLE PORT: DETACH SUCCESS,  ID:" << attachId  );

    TRACE_EXIT(logger, "InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::detach" );
  }


  //
  // getStreamDefinition
  //
  // @return   StreamDefinition Return the stream definition for this attachId
  //           NULL attachId was not found
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  StreamDefinition* InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::getStreamDefinition(const char* attachId)
  {
    // Blocks when values are updating
    READ_LOCK lock(attachmentLock);

    typename AttachedStreams::iterator portIter2;
    portIter2 = attachedStreamMap.begin();
    // use: attachedPorts[(*portIter).first] :instead
    while (portIter2 != attachedStreamMap.end()) {
      if (strcmp((*portIter2).first.c_str(), attachId) == 0) {
        return (*portIter2).second;
      }
      portIter2++;
    }
    return NULL;
  }

  //
  // getUser
  //
  // @return char * the user id that made the attachId request
  //         NULL attachId was not found
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  char* InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::getUser(const char* attachId)
  {
    // Blocks when values are updating
    READ_LOCK lock(attachmentLock);

    AttachedUsers::iterator portIter2;
    portIter2 = attachedUsers.begin();
    while (portIter2 != attachedUsers.end()) {
      if (strcmp((*portIter2).first.c_str(), attachId) == 0) {
        return CORBA::string_dup((*portIter2).second.c_str());
      }
      portIter2++;
    }
    return NULL;
  }

  //
  // attachedSRIs
  //
  // @return BULKIO::StreamSRISequence Returns a list of StreamSRI objects for each attached 
  //                                   stream definition. The caller is responsible for freeing
  //                                   the provided objects
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  BULKIO::StreamSRISequence* InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::attachedSRIs()
  {
    READ_LOCK lock(sriUpdateLock);
    BULKIO::StreamSRISequence_var sris = new BULKIO::StreamSRISequence();
    sris->length(currentHs.size());
    SriMap::iterator sriIter;
    unsigned int idx = 0;

    sriIter = currentHs.begin();
    while (sriIter != currentHs.end()) {
      sris[idx++] = (*sriIter).second.first;
      sriIter++;
    }
    return sris._retn();
  }

  //
  // attachedStreams
  //
  // @return StreamSequence Returns a list of attached Stream Defintions. The caller
  //                                 is responsible for freeing the provided objects
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  StreamSequence* InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::attachedStreams()
  {
    // Blocks when values are updating
    READ_LOCK lock(attachmentLock);
    
    StreamSequence* seq = new StreamSequence();
    seq->length(attachedStreamMap.size());
    typename AttachedStreams::iterator portIter2;
    portIter2 = attachedStreamMap.begin();
    unsigned int i = 0;
    while (portIter2 != attachedStreamMap.end()) {
      (*seq)[i++] = *((*portIter2).second);
      portIter2++;
    }
    return seq;
  }


  //
  // attachmentIds
  //
  // @return BULKIO::StringSequence Return the current list of attachment identifiers. The caller
  //                                is responsible for freeing the provided objects
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  BULKIO::StringSequence* InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::attachmentIds()
  {
    // Blocks when values are updating
    READ_LOCK lock(attachmentLock);
    
    BULKIO::StringSequence* seq = new BULKIO::StringSequence();
    seq->length(attachedStreamMap.size());
    typename AttachedStreams::iterator portIter2;
    portIter2 = attachedStreamMap.begin();
    unsigned int i = 0;
    while (portIter2 != attachedStreamMap.end()) {
      (*seq)[i++] = CORBA::string_dup((*portIter2).first.c_str());
      portIter2++;
    }
    return seq;
  }



  //
  // usageState
  //
  // If the number of attached streams == 0 
  //      return typename PortType::IDLE;
  // If the number of attached streams == 1
  //      return typename PortType::BUSY;
  // other
  //    return typename PortType::ACTIVE;
  //
  // @return   typename PortType::InputUsageState return the current state of the port base on how many attach requests have been received
  //            
  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  typename PortType::InputUsageState InAttachablePort<StreamDefinition,PortType,StreamSequence,POAType>::usageState()
  {
    // Blocks when values are updating
    READ_LOCK lock(attachmentLock);
    
    if (attachedStreamMap.size() == 0) {
      return PortType::IDLE;
    } else if (attachedStreamMap.size() == 1) {
      return PortType::BUSY;
    } else {
      return PortType::ACTIVE;
    }
  }

  //
  // class StreamAttachment
  //
      //
      // Constructors
      // 
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamAttachment::StreamAttachment(const std::string& connection_id, 
                       const std::string& attach_id, 
                       typename PortType::_ptr_type input_port) :
          _connectionId(connection_id),
          _attachId(attach_id)
      {
          setInputPort(input_port);
      }
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamAttachment::StreamAttachment(const std::string& connection_id, 
                       typename PortType::_ptr_type input_port) :
          _connectionId(connection_id),
          _attachId("")
      {
          setInputPort(input_port);
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamAttachment::setLogger(LOGGER_PTR newLogger) {
          logger = newLogger;
      }
  
      // 
      // detach
      //
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamAttachment::detach() {
          if (not _attachId.empty()){
              try {
                  _inputPort->detach(_attachId.c_str());
                  _attachId = "";
              } catch( typename PortType::DetachError& ex) { 
                  std::cout << "A detach error occured - msg: " << ex.msg << std::endl; 
              } catch (std::exception& ex) {
                  std::cout << "An unknown error occurred while attaching: " << ex.what() << std::endl;
              } catch( CORBA::SystemException& ex ) {
                  std::cout << "A CORBA::SystemException occurred: " << ex.NP_minorString() << " " << ex._name() << ex.completed() << std::endl;
              } catch ( const CORBA::Exception& ex ) {
                  std::cout << "A CORBA::Exception occurred: " << ex._name() << std::endl;
              } catch (...) {
                  std::cout << "An unknown error occurred while attaching" << std::endl;
              }
          }
      }

      //
      // Getters
      //
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      std::string OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamAttachment::getConnectionId() { 
          return _connectionId; 
      }
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      std::string OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamAttachment::getAttachId() { 
          return _attachId; 
      }
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      typename PortType::_ptr_type OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamAttachment::getInputPort() { 
          return _inputPort; 
      }

      //
      // Setters
      //
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamAttachment::setConnectionId(const std::string& connection_id) {
          _connectionId = connection_id;
      }
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamAttachment::setAttachId(const std::string& attach_id) {
          _attachId = attach_id;
      }
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamAttachment::setInputPort(typename PortType::_ptr_type input_port) {
          _inputPort = PortType::_duplicate(input_port);
      }

    //
    // End StreamAttachment
    //


    //
    // class Stream 
    //
      //
      // Constructors
      //
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::Stream():
          _name(""),
          _streamId(""),
          _sri(bulkio::sri::create()),
          _time(bulkio::time::utils::create())
      {
      }
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::Stream(const StreamDefinition& stream_def,
             const std::string name,
             const std::string stream_id) :
          _streamDefinition(stream_def),
          _name(name),
          _streamId(stream_id),
          _sri(bulkio::sri::create()),
          _time(bulkio::time::utils::create())
      {
      }
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::Stream(const StreamDefinition& stream_def,
             const std::string name,
             const std::string stream_id,
             BULKIO::StreamSRI sri,
             BULKIO::PrecisionUTCTime time) :
          _streamDefinition(stream_def),
          _name(name),
          _streamId(stream_id),
          _sri(sri),
          _time(time)
      {
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::Stream(const Stream& obj) :
         _streamDefinition(obj._streamDefinition),
         _name(obj._name),
         _streamId(obj._streamId),
         _streamAttachments(obj._streamAttachments),
         _sri(obj._sri),
         _time(obj._time)
      {
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::setLogger(LOGGER_PTR newLogger) {
          logger = newLogger;
      }
      
      //
      // detach
      //
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::detachAll() {
          boost::mutex::scoped_lock lock(attachmentsUpdateLock);
          StreamAttachmentIter iter;
          for (iter = _streamAttachments.begin(); iter != _streamAttachments.end(); /*NoIncrement*/) {
              iter->detach();
              iter = _streamAttachments.erase(iter);
          }
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::detachByConnectionId(const std::string& connectionId) {
          boost::mutex::scoped_lock lock(attachmentsUpdateLock);
          StreamAttachmentIter iter;
          for (iter = _streamAttachments.begin(); iter != _streamAttachments.end(); /*NoIncrement*/) {
              if (iter->getConnectionId() == connectionId) {
                  iter->detach();
                  _streamAttachments.erase(iter);
              } else {
                  ++iter;
              } 
          }
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::detachByAttachId(const std::string& attachId) {
          boost::mutex::scoped_lock lock(attachmentsUpdateLock);
          StreamAttachmentIter iter;
          for (iter = _streamAttachments.begin(); iter != _streamAttachments.end(); /*NoIncrement*/) {
              if (iter->getAttachId() == attachId) {
                  iter->detach();
                  _streamAttachments.erase(iter);
              } else {
                  ++iter;
              } 
          }
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::detachByAttachIdConnectionId(const std::string& attachId, const std::string& connectionId) {
          boost::mutex::scoped_lock lock(attachmentsUpdateLock);
          StreamAttachmentIter iter;
          for (iter = _streamAttachments.begin(); iter != _streamAttachments.end(); /*NoIncrement*/) {
              if ((iter->getAttachId() == attachId) && (iter->getConnectionId() == connectionId)) {
                  iter->detach();
                  _streamAttachments.erase(iter);
              } else {
                  ++iter;
              } 
          }
      }


      //
      // Getters
      //
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      StreamDefinition& OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::getStreamDefinition() {
          return _streamDefinition;
      }
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      std::string OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::getName() {
          return _name;
      }
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      std::string OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::getStreamId() {
          return _streamId;
      }
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      typename OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamAttachmentList OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::getStreamAttachments() {
          return _streamAttachments;
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      BULKIO::StreamSRI OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::getSRI() {
          return _sri;
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      BULKIO::PrecisionUTCTime OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::getTime() {
          return _time;
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      std::set<std::string> OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::getConnectionIds() {
          StreamAttachmentIter iter;
          std::set<std::string> connIds;
          for (iter = _streamAttachments.begin(); iter != _streamAttachments.end(); iter++) {
              connIds.insert(iter->getConnectionId());
          }
          return connIds;
      }
      
      //
      // Setters
      //
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::setStreamDefintion(StreamDefinition& stream_definition) {
          _streamDefinition = stream_definition;
      }
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::setName(const std::string& name) {
          _name = name;
      }
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::setStreamId(const std::string& stream_id) {
          _streamId = stream_id;
      }
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::setSRI(BULKIO::StreamSRI sri) {
          _sri = sri;
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::setTime(BULKIO::PrecisionUTCTime time) {
          _time = time;
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::createNewAttachment(const std::string& connectionId, 
                               typename PortType::_ptr_type port) {
          boost::mutex::scoped_lock lock(attachmentsUpdateLock);
          try {
              char* attachId = port->attach(_streamDefinition, _name.c_str()); 
              StreamAttachment attachment(connectionId, std::string(attachId), port);
              _streamAttachments.push_back(attachment);
              //port->pushSRI(_sri, _time);
          } catch (typename PortType::AttachError& ex) {
              std::cout << "AttachError occurred: " << ex.msg << std::endl;
          } catch (typename PortType::StreamInputError& ex) {
              std::cout << "StreamInputError occurred: " << ex.msg << std::endl;
          } catch (std::exception& ex) {
              std::cout << "An unknown error occurred while attaching: " << ex.what() << std::endl;
          } catch( CORBA::SystemException& ex ) {
              std::cout << "A CORBA::SystemException occurred: " << ex.NP_minorString() << " " << ex._name() << ex.completed() << std::endl;
          } catch ( const CORBA::Exception& ex ) {
              std::cout << "A CORBA::Exception occurred: " << ex._name() << std::endl;
          } catch (...) {
              std::cout << "An unknown error occurred while attaching" << std::endl;
          }
      }
      
      //
      // State Validators
      //
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      bool OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::hasAttachId(const std::string& attachId) {
          StreamAttachmentIter iter;
          for (iter = _streamAttachments.begin(); iter != _streamAttachments.end(); iter++) {
              if (iter->getAttachId() == attachId) {
                  return true;
              } 
          }
          return false;
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      bool OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::hasConnectionId(const std::string& connectionId) {
          StreamAttachmentIter iter;
          for (iter = _streamAttachments.begin(); iter != _streamAttachments.end(); iter++) {
              if (iter->getConnectionId() == connectionId) {
                  return true;
              } 
          }
          return false;
      }
              
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      bool OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::isValid() {
          bool hasStreamDefinition = (strlen(_streamDefinition.id) > 0);
          bool hasStreamId = (not _streamId.empty());
          return (hasStreamDefinition && hasStreamId);
      }

      //
      // Search methods 
      //
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      typename OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamAttachmentList OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::findAttachmentsByAttachId(const std::string& attachId) {
          StreamAttachmentIter iter;
          StreamAttachmentList foundAttachments;
          for (iter = _streamAttachments.begin(); iter != _streamAttachments.end(); iter++) {
              if (iter->getAttachId() == attachId) {
                  foundAttachments.push_back(*iter);
              } 
          }
          return foundAttachments;
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      typename OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamAttachmentList OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::findAttachmentsByConnectionId(const std::string& connectionId) {
          StreamAttachmentIter iter;
          StreamAttachmentList foundAttachments;
          for (iter = _streamAttachments.begin(); iter != _streamAttachments.end(); iter++) {
              if (iter->getConnectionId() == connectionId) {
                  foundAttachments.push_back(*iter);
              } 
          }
          return foundAttachments;
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream::updateAttachments(OutAttachablePort::StreamAttachmentList expectedAttachments) {
          StreamAttachmentIter iter;
          std::vector<std::string> expectedConnectionIds;
          std::vector<std::string> connectionsToRemove;;
          std::vector<std::string>::iterator connIter;
          bool validConnection;
          
          // Add new attachments that do not already exist
          for (iter = expectedAttachments.begin(); iter != expectedAttachments.end(); iter++) {
              validConnection = this->hasConnectionId(iter->getConnectionId());
              
              // Attempt to create new attachment if one doesn't already exist
              if (not validConnection) {
                try {
                    this->createNewAttachment(iter->getConnectionId(), iter->getInputPort());
                    validConnection = true;
                } catch (typename PortType::AttachError& ex) {
                    LOG_ERROR( logger, __FUNCTION__ << ": AttachError occurred: " << ex.msg);
                } catch (typename PortType::StreamInputError& ex) {
                    LOG_ERROR( logger, __FUNCTION__ << ": StreamInputError occurred: " << ex.msg);
                } catch(...) {
                    LOG_ERROR( logger, __FUNCTION__ << ": Unknown attachment error occured: " << iter->getConnectionId() );
                }
              }

              if (validConnection) {
                  expectedConnectionIds.push_back(iter->getConnectionId());
              }
          }
          
          // Iterate through attachments and compare to expected connectionIds
          for (iter = this->_streamAttachments.begin(); iter != this->_streamAttachments.end(); iter++) {
              std::string existingConnectionId(iter->getConnectionId());
              
              bool detachConnection = true;
              for (connIter = expectedConnectionIds.begin(); connIter != expectedConnectionIds.end(); connIter++) {
                  if (existingConnectionId == (*connIter)) {
                      detachConnection = false;
                      break;
                  }
              }
              if (detachConnection) {
                  // Store off and apply detach outside of this loop
                  // Removing now will mess up iterator
                  connectionsToRemove.push_back(existingConnectionId);
              }
          }

          // Detach all connections that need to be removed
          for (connIter = connectionsToRemove.begin(); connIter != connectionsToRemove.end(); connIter++) {
              this->detachByConnectionId((*connIter));
          }
      }
    
    //
    // class StreamContainer {
    //
      //
      // Constructors
      //
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::StreamContainer() {
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::setLogger(LOGGER_PTR newLogger) {
          logger = newLogger;
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::addStream(Stream& newStream) {
          _streams.push_back(newStream);    
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::removeStreamByStreamId(const std::string& streamId) {
          StreamIter iter;
          for (iter = _streams.begin(); iter != _streams.end();) {
              if (iter->getStreamId() == streamId) {
                  iter->detachAll();
                  iter = _streams.erase(iter);  // Remove from container
              } else {
                  ++iter;
              }
          }
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      bool OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::hasStreamId(const std::string& streamId) {
          StreamIter iter;
          for (iter=_streams.begin(); iter != _streams.end(); iter++) {
              if (iter->getStreamId() == streamId) {
                  return true;
              }
          }
          return false;
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      bool OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::hasAttachments() {
          StreamIter iter;
          for (iter=_streams.begin(); iter != _streams.end(); iter++) {
              if (iter->getStreamAttachments().empty()) {
                  continue;
              }
              return true;
          }
          return false;
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      std::vector<std::string> OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::getStreamIds() {
          std::vector<std::string> streamIds;
          StreamIter iter;
          for (iter=_streams.begin(); iter != _streams.end(); iter++) {
              streamIds.push_back(iter->getStreamId());
          }
          return streamIds;
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      std::vector<std::string> OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::getAttachIds() {
          std::vector<std::string> attachIds;
          StreamIter iter;
          StreamAttachmentIter attachedIter;
          for (iter=_streams.begin(); iter != _streams.end(); iter++) {
              StreamAttachmentList attached = iter->getStreamAttachments();
              for (attachedIter= attached.begin(); attachedIter != attached.end(); attachedIter++)
                  attachIds.push_back(attachedIter->getAttachId());
          }
          return attachIds;
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      typename OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamList OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::getStreams() {
          return _streams;
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::addConnectionToAllStreams(const std::string& connectionId, typename PortType::_ptr_type port) {
          StreamIter iter;
          if (_streams.empty()) {
             //LOG_INFO(this->logger, "NO STREAMS DEFINED. NO ATTACHMENTS WERE MADE")
          }

          for (iter=_streams.begin(); iter != _streams.end(); iter++) {
              if (not iter->hasConnectionId(connectionId)) {
                  try {
                      iter->createNewAttachment(connectionId, port);
                  } catch (typename PortType::AttachError& ex) {
                      LOG_ERROR( logger, __FUNCTION__ << ": AttachError occurred: " << ex.msg);
                  } catch (typename PortType::StreamInputError& ex) {
                      LOG_ERROR( logger, __FUNCTION__ << ": StreamInputError occurred: " << ex.msg);
                  } catch(...) {
                      LOG_ERROR( logger, __FUNCTION__ << ": Unknown attachment error occured: " << connectionId );
                  }
              }
          }
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::addConnectionToStream(const std::string& connectionId, typename PortType::_ptr_type port, const std::string& streamId) {
          StreamIter iter;
          for (iter=_streams.begin(); iter != _streams.end(); iter++) {
              // Skip invalid streamIds
              if (iter->getStreamId() != streamId) {
                  continue;
              }
              if (not iter->hasConnectionId(connectionId)) {
                  try {
                      iter->createNewAttachment(connectionId, port);
                  } catch (typename PortType::AttachError& ex) {
                      LOG_ERROR( logger, __FUNCTION__ << ": AttachError occurred: " << ex.msg);
                  } catch (typename PortType::StreamInputError& ex) {
                      LOG_ERROR( logger, __FUNCTION__ << ": StreamInputError occurred: " << ex.msg);
                  } catch(...) {
                      LOG_ERROR( logger, __FUNCTION__ << ": Unknown attachment error occured: " << connectionId );
                  }
              }
          }
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::updateSRIForAllStreams(OutPortSriMap currentSRIs) {
          StreamIter iter;
          for (iter=_streams.begin(); iter != _streams.end(); iter++) {
              OutPortSriMap::iterator sri_iter=  currentSRIs.find(iter->getStreamId());
              if (sri_iter == currentSRIs.end()) {
                  continue;
              }
              iter->setSRI(sri_iter->second.sri);
              iter->setTime(sri_iter->second.time);
          }
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::updateStreamSRI(const std::string& streamId, const BULKIO::StreamSRI sri) {
          StreamIter iter;
          for (iter=_streams.begin(); iter != _streams.end(); iter++) {
              // Skip invalid streamIds
              if (iter->getStreamId() != streamId) {
                  continue;
              }
              iter->setSRI(sri);
          }
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::updateStreamTime(const std::string& streamId, const BULKIO::PrecisionUTCTime time) {
          StreamIter iter;
          for (iter=_streams.begin(); iter != _streams.end(); iter++) {
              // Skip invalid streamIds
              if (iter->getStreamId() != streamId) {
                  continue;
              }
              iter->setTime(time);
          }
      }
        
      //  
      // Detach
      //
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::detachByAttachIdConnectionId(const std::string& attachId, const std::string& connectionId) {
          StreamIter iter;
          for (iter = _streams.begin(); iter != _streams.end(); iter++) {
              try {
                  iter->detachByAttachIdConnectionId(attachId, connectionId);
                  //LOG_DEBUG(logger, "ATTACHABLE PORT: DETACH COMPLETD ID:" << attachId  );
              }
              catch(...) {
                  //LOG_WARN(logger, "UNABLE TO DETACH ATTACHID/CONNECTIONID: " << attachId << "/" << connectionId);
              }
          }
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::detachByConnectionId(const std::string& connectionId) {
          StreamIter iter;
          for (iter = _streams.begin(); iter != _streams.end(); iter++) {
              try {
                  if (iter->hasConnectionId(connectionId))
                      iter->detachByConnectionId(connectionId);
              }
              catch(...) {
                  //LOG_WARN(logger, "UNABLE TO DETACH ATTACHID/CONNECTIONID: " << attachId << "/" << connectionId);
              }
          }
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::detachByAttachId(const std::string& attachId) {
          StreamIter iter;
          for (iter = _streams.begin(); iter != _streams.end(); iter++) {
              try {
                  iter->detachByAttachId(attachId);
                  LOG_DEBUG(logger, "ATTACHABLE PORT: DETACH COMPLETD ID: " << attachId  );
              }
              catch(...) {
                  LOG_WARN(logger, "UNABLE TO DETACH ATTACHID: " << attachId );
              }
          }
      }
     
      // 
      // Search methods
      //
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      typename OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream* OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::findByStreamId(const std::string& streamId) {
          StreamIter iter;
          for (iter = _streams.begin(); iter != _streams.end(); iter++) {
              if (iter->getStreamId() == streamId) {
                  return &(*iter);
              }
          }
          return NULL;
      }
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      typename OutAttachablePort<StreamDefinition,PortType,StreamSequence>::Stream* OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::findByAttachId(const std::string& attachId) {
          StreamIter iter;
          for (iter = _streams.begin(); iter != _streams.end(); iter++) {
              if (iter->hasAttachId(attachId)) {
                  return &(*iter);
              }
          }
          return NULL;
      }
      
      
      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::printState(const std::string& title) {
          StreamIter iter;
          StreamAttachmentIter attachmentIter;
          LOG_DEBUG( logger, " PORT STATE: " << title)
          for (iter = _streams.begin(); iter != _streams.end(); iter++) {
            
              StreamAttachmentList streamAttachments = iter->getStreamAttachments();
              printBlock("Stream", iter->getStreamId(),0);
              for (attachmentIter = streamAttachments.begin();
                   attachmentIter != streamAttachments.end();
                   attachmentIter++) {
                  printBlock("Attachment", attachmentIter->getAttachId(), 1);
              }
          }
          LOG_DEBUG( logger, "")
      }

      template <typename StreamDefinition, typename PortType, typename StreamSequence>
      void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::StreamContainer::printBlock(const std::string& title, const std::string& id, size_t indents) {
          std::string indentStr("");
          for (size_t ii=0; ii < indents; ii++) {
            indentStr.append("   ");
          }
          std::string line("---------------");

          LOG_DEBUG( logger, indentStr << " |" << line) ;
          LOG_DEBUG( logger, indentStr << " |" << title);
          LOG_DEBUG( logger, indentStr << " |   '" << id << "'");
          LOG_DEBUG( logger, indentStr << " |" << line);
      }
  //
  // End StreamContainer
  //
      
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  OutAttachablePort<StreamDefinition,PortType,StreamSequence>::OutAttachablePort(std::string port_name,
                      ConnectionEventListener *connectCB,
                      ConnectionEventListener *disconnectCB ):
    Port_Uses_base_impl(port_name),
    _connectCB(),
    _disconnectCB()
  {
    recConnectionsRefresh = false;
    recConnections.length(0);
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  OutAttachablePort<StreamDefinition,PortType,StreamSequence>::OutAttachablePort(std::string port_name,
                      LOGGER_PTR  logger,
                      ConnectionEventListener *connectCB,
                      ConnectionEventListener *disconnectCB ):
    Port_Uses_base_impl(port_name),
    logger(logger),
    _connectCB(),
    _disconnectCB()
  {
    if ( connectCB ) {
      _connectCB = boost::shared_ptr< ConnectionEventListener >( connectCB, null_deleter() );
    }

    if ( disconnectCB ) {
      _disconnectCB = boost::shared_ptr< ConnectionEventListener >( disconnectCB, null_deleter() );
    }

    recConnectionsRefresh = false;
    recConnections.length(0);

    LOG_DEBUG( logger, "bulkio::OutAttachablePort<StreamDefinition,PortType,StreamSequence>::CTOR port:" << this->name );
    this->streamContainer.setLogger(logger);
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  OutAttachablePort<StreamDefinition,PortType,StreamSequence>::~OutAttachablePort() {};
 
  //
  // Allow users to set own logger
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::setLogger( LOGGER_PTR newLogger )
  {
    logger = newLogger;
    this->streamContainer.setLogger(logger);
  }

  //
  // Port Statistics Interface
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  BULKIO::UsesPortStatisticsSequence * OutAttachablePort<StreamDefinition,PortType,StreamSequence>::statistics() 
  {
    boost::mutex::scoped_lock lock(updatingPortsLock);
    BULKIO::UsesPortStatisticsSequence_var recStat = new BULKIO::UsesPortStatisticsSequence();
    recStat->length(outConnections.size());
    for (unsigned int i = 0; i < outConnections.size(); i++) {
      recStat[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
      recStat[i].statistics = stats[outConnections[i].second].retrieve();
    }
    return recStat._retn();
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  BULKIO::PortUsageType OutAttachablePort<StreamDefinition,PortType,StreamSequence>::state() 
  {
    boost::mutex::scoped_lock lock(updatingPortsLock);
    if (outConnections.size() > 0) {
      return BULKIO::ACTIVE;
    } else {
      return BULKIO::IDLE;
    }

    return BULKIO::BUSY;
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::enableStats(bool enable)
  {
    for (unsigned int i = 0; i < outConnections.size(); i++) {
      stats[outConnections[i].second].setEnabled(enable);
    }
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::setBitSize(double bitSize)
  {
    for (unsigned int i = 0; i < outConnections.size(); i++) {
      stats[outConnections[i].second].setBitSize(bitSize);
    }
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::updateStats(unsigned int elementsReceived, unsigned int queueSize, bool EOS, std::string streamID)
  {
    for (unsigned int i = 0; i < outConnections.size(); i++) {
      stats[outConnections[i].second].update(elementsReceived, queueSize, EOS, streamID);
    }
  }

  //
  // Uses Port Interface
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::connectPort(CORBA::Object_ptr connection, const char* connectionId)
  {
    TRACE_ENTER(logger, "OutAttachablePort<StreamDefinition,PortType,StreamSequence>::connectPort" );

    {
      boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
      
      //this->streamContainer.printState("Before connectPort");
      typename PortType::_var_type port;
      try{
        port = PortType::_narrow(connection);
      }
      catch(...) {
        LOG_ERROR( logger, "CONNECT FAILED: UNABLE TO NARROW ENDPOINT,  USES PORT:" << this->name );
        throw CF::Port::InvalidPort(1, "Unable to narrow");
      }

      std::string connId(connectionId);
      Stream foundStream;
      bool portListed = false;
      std::vector<connection_descriptor_struct >::iterator ftPtr;

      for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {
        // Skip irrelavent port entries
        if (ftPtr->port_name != this->name)  
          continue;

        // Process filterTable entry
        portListed = true;
        if (ftPtr->connection_id == connId) {
          OutPortSriMap::iterator sri_iter=  currentSRIs.find( ftPtr->stream_id );
          if ( sri_iter != currentSRIs.end() ) {
            this->streamContainer.updateStreamSRI(ftPtr->stream_id, sri_iter->second.sri);
            this->streamContainer.updateStreamTime(ftPtr->stream_id, sri_iter->second.time);
          }
          this->streamContainer.addConnectionToStream(connId, port, ftPtr->stream_id);
        }
      }
     
      if (not portListed) {
        this->streamContainer.updateSRIForAllStreams(currentSRIs); 
        this->streamContainer.addConnectionToAllStreams(connId, port);
      }

      outConnections.push_back(std::make_pair(port, connectionId));
      this->active = true;
      this->recConnectionsRefresh = true;
      this->refreshSRI = true;    
      LOG_DEBUG( logger, "CONNECTION ESTABLISHED,  PORT/CONNECTION_ID:" << this->name << "/" << connectionId );
    }

    if ( _connectCB ) (*_connectCB)(connectionId);
    this->streamContainer.printState("After connectPort");

    TRACE_EXIT(logger, "OutAttachablePort<StreamDefinition,PortType,StreamSequence>::connectPort" );
  }
  
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::disconnectPort(const char* connectionId)
  {
    {
      boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

      try {
        // Detach everything that's associated to connectionId
        std::string connId(connectionId);
        this->streamContainer.detachByConnectionId(connId);
      }
      catch(...) {
        LOG_ERROR(logger," Unable to detach for CONNECTION: " << connectionId );
      }

      for (unsigned int i = 0; i < outConnections.size(); i++) {
        if (outConnections[i].second == connectionId) {
          LOG_DEBUG( logger, "DISCONNECT, PORT/CONNECTION: "  << this->name << "/" << connectionId );
          outConnections.erase(outConnections.begin() + i);
          break;
        }
      }

      OutPortSriMap::iterator cSRIs =  currentSRIs.begin();
      for (; cSRIs!=currentSRIs.end(); cSRIs++) {
        // remove connection id from sri connections list
        cSRIs->second.connections.erase( connectionId );
      }

      if (outConnections.size() == 0) {
        this->active = false;
      }
      recConnectionsRefresh = true;
    }
    if ( _disconnectCB ) (*_disconnectCB)(connectionId);

  }
  
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  ExtendedCF::UsesConnectionSequence * OutAttachablePort<StreamDefinition,PortType,StreamSequence>::connections()
  {
    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
    if (recConnectionsRefresh) {
      recConnections.length(outConnections.size());
      for (unsigned int i = 0; i < outConnections.size(); i++) {
        recConnections[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
        recConnections[i].port = CORBA::Object::_duplicate(outConnections[i].first);
      }
      recConnectionsRefresh = false;
    }
    ExtendedCF::UsesConnectionSequence_var retVal = new ExtendedCF::UsesConnectionSequence(recConnections);
    return retVal._retn();
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::updateConnectionFilter(const std::vector<bulkio::connection_descriptor_struct> &_filterTable)
  {
    SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in
    filterTable = _filterTable;
    
    //1. loop over fitlerTable
    //   A. ignore other port names
    //   B. create mapping of streamid->connections(attachments)
    //
    
    bool hasPortEntry = false;
    std::map<std::string, bool> streamsFound;
    std::map<std::string, bool>::iterator streamsFoundIter;
    std::map<std::string, StreamAttachmentList> streamAttachmentsMap;
    typename std::map<std::string, StreamAttachmentList>::iterator streamAttachmentsIter;

    //this->streamContainer.printState("Before Filter Table Update");

    // Populate found streams vector
    std::vector<std::string> knownStreamIds(this->streamContainer.getStreamIds());
    std::vector<std::string>::iterator strIter;
    for (strIter=knownStreamIds.begin(); strIter!=knownStreamIds.end(); strIter++) {
        streamsFound[(*strIter)] = false;
    }

    // Iterate through each filterTable entry and capture state
    std::vector<bulkio::connection_descriptor_struct >::iterator ftPtr;
    for (ftPtr = filterTable.begin(); ftPtr != filterTable.end(); ftPtr++) {
      // Skip entries that unrelated to this port
      if (ftPtr->port_name != this->name) {
          continue;
      }
      
      // Lookup the port from the connectionId
      hasPortEntry = true;
      typename PortType::_ptr_type connectedPort = this->getConnectedPort(ftPtr->connection_id);
      if (connectedPort->_is_nil()) {
          LOG_DEBUG( logger, "Unable to find connected port with connectionId: " << ftPtr->connection_id);
          continue;
      }
      
      // Keep track of which attachments are SUPPOSED to exist
      if (this->streamContainer.hasStreamId(ftPtr->stream_id)) {
        // Indicate that current stream has entry
        streamsFound[ftPtr->stream_id] = true;

        // Create a temporary attachment that we'll use as a reference
        StreamAttachment expectedAttachment(ftPtr->connection_id, connectedPort);
        streamAttachmentsMap[ftPtr->stream_id].push_back(expectedAttachment);
      }
    }
        
    // Iterate through all attachment associations defined by filterEntries
    for (streamAttachmentsIter  = streamAttachmentsMap.begin();
         streamAttachmentsIter != streamAttachmentsMap.end();
         streamAttachmentsIter++) {
        std::string streamId = (*streamAttachmentsIter).first;
        Stream* foundStream = this->streamContainer.findByStreamId(streamId);
        if (foundStream) {
            foundStream->updateAttachments(streamAttachmentsIter->second);
        } else {
            LOG_WARN( logger, "Unable to locate stream definition for streamId: " << streamId);
        }
    }

    if (hasPortEntry) {
        // If there's a valid port entry, we need to detach unmentioned streams
        for (streamsFoundIter=streamsFound.begin(); streamsFoundIter != streamsFound.end(); streamsFoundIter++) {
            if (not streamsFoundIter->second) {
                Stream* stream = this->streamContainer.findByStreamId(streamsFoundIter->first);
                if (stream) stream->detachAll();
            }
        }
    } else {
        // No port entry == All connections on
        for (ConnectionsIter iter = outConnections.begin(); iter != outConnections.end(); ++iter) {
            this->streamContainer.addConnectionToAllStreams(iter->second, iter->first);
        }
    }
       
    this->updateSRIForAllConnections();    
    this->streamContainer.printState("After Filter Table Update");
  };

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void  OutAttachablePort<StreamDefinition,PortType,StreamSequence>::updateSRIForAllConnections() {
    // Iterate through stream objects in container
    //   Check if currentSRI has stream entry
    //     Yes: Check that ALL connections are listed in currentSRI entry
    //          Update currentSRI
    //     No:  PushSRI on all attachment ports
    //          Update currentSRI

    // Initialize variables
    std::string streamId;
    std::set<std::string> streamConnIds;
    std::set<std::string> currentSRIConnIds;
    std::set<std::string>::iterator connIdIter;
    OutPortSriMap::iterator sriMapIter;
    StreamList streams = this->streamContainer.getStreams(); // Returns copy of stream objects
   
    // Iterate through all registered streams
    for (StreamIter iter = streams.begin(); iter != streams.end(); iter++) {
      streamId = iter->getStreamId();
      streamConnIds = iter->getConnectionIds();

      // Check if currentSRIs has entry for StreamId
      sriMapIter = this->currentSRIs.find(streamId);
      if (sriMapIter != currentSRIs.end()) {

        // Check if all connections on the streams have pushed SRI
        currentSRIConnIds = sriMapIter->second.connections;
        for (connIdIter = streamConnIds.begin(); connIdIter != streamConnIds.end(); connIdIter++) {
           
          // If not found, pushSRI and update currentSRIs container
          if (currentSRIConnIds.find(*connIdIter) == currentSRIConnIds.end()) {

            // Grab the port
            typename PortType::_ptr_type connectedPort = this->getConnectedPort(*connIdIter);
            if (connectedPort->_is_nil()) {
                LOG_DEBUG( logger, "Unable to find connected port with connectionId: " << (*connIdIter));
                continue;
            }
            // Push sri and update sriMap 
            connectedPort->pushSRI(sriMapIter->second.sri, sriMapIter->second.time);
            sriMapIter->second.connections.insert(*connIdIter);
          }
        }
      }
    }
  }
  
  //
  // Attach listener interfaces for connect and disconnect events
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void   OutAttachablePort<StreamDefinition,PortType,StreamSequence>::setNewConnectListener( ConnectionEventListener *newListener ) {
    _connectCB =  boost::shared_ptr< ConnectionEventListener >(newListener, null_deleter());
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void   OutAttachablePort<StreamDefinition,PortType,StreamSequence>::setNewConnectListener( ConnectionEventCallbackFn  newListener ) {
    _connectCB =  boost::make_shared< StaticConnectionListener >( newListener );
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void   OutAttachablePort<StreamDefinition,PortType,StreamSequence>::setNewDisconnectListener( ConnectionEventListener *newListener ) {
    _disconnectCB =  boost::shared_ptr< ConnectionEventListener >(newListener, null_deleter());
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void   OutAttachablePort<StreamDefinition,PortType,StreamSequence>::setNewDisconnectListener( ConnectionEventCallbackFn  newListener ) {
    _disconnectCB =  boost::make_shared< StaticConnectionListener >( newListener );
  }

  //
  // pushSRI to allow for insertion of SRI context into data stream
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::pushSRI(const BULKIO::StreamSRI& H, const BULKIO::PrecisionUTCTime& T)
  {
    TRACE_ENTER(logger, "OutAttachablePort<StreamDefinition,PortType,StreamSequence>::pushSRI" );

    boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

    std::string sid( H.streamID );
    OutPortSriMap::iterator sri_iter=  currentSRIs.find( sid );
    if ( sri_iter == currentSRIs.end() ) {
      // need to use insert since we do not have default CTOR for AttachableSriMapStruct
      AttachableSriMapStruct sri_ctx(H,T);
      currentSRIs.insert(std::make_pair(H.streamID, sri_ctx));
      sri_iter=  currentSRIs.find( sid );
    }
    else {
      // overwrite the SRI 
      sri_iter->second.sri = H;

      // reset connections list to be empty
      sri_iter->second.connections.clear();
    }

    if (this->active) {
      bool portListed = false;
      std::vector<bulkio::connection_descriptor_struct >::iterator ftPtr;
      for (ConnectionsIter i = outConnections.begin(); i != outConnections.end(); ++i) {
         for (ftPtr=filterTable.begin(); ftPtr!= filterTable.end(); ftPtr++) {

            if (ftPtr->port_name == this->name) portListed=true;

            if ( (ftPtr->port_name == this->name) and
               (ftPtr->connection_id == i->second) and
               (strcmp(ftPtr->stream_id.c_str(),H.streamID) == 0 ) ){
               LOG_DEBUG(logger,"pushSRI - PORT:" << this->name << " CONNECTION:" << ftPtr->connection_id << " SRI streamID:" << H.streamID << " Mode:" << H.mode << " XDELTA:" << 1.0/H.xdelta );  

               try {
                  i->first->pushSRI(H, T);
                  sri_iter->second.connections.insert( i->second );
               } catch(...) {
                  LOG_ERROR( logger, "PUSH-SRI FAILED, PORT/CONNECTION: " << this->name << "/" << i->second );
               }
            }
         }
      }

      if (!portListed) {
         for (ConnectionsIter i = outConnections.begin(); i != outConnections.end(); ++i) {
            std::string connectionId = CORBA::string_dup(i->second.c_str());
            LOG_DEBUG(logger,"pushSRI -2- PORT:" << this->name << " CONNECTION:" << connectionId << " SRI streamID:" << H.streamID << " Mode:" << H.mode << " XDELTA:" << 1.0/H.xdelta );
            try {
               i->first->pushSRI(H, T);
               sri_iter->second.connections.insert( i->second );
            } catch(...) {
               LOG_ERROR( logger, "PUSH-SRI FAILED, PORT/CONNECTION: " << this->name << "/" << i->second );
          }
        }
      }
    }

    TRACE_EXIT(logger, "OutAttachablePort<StreamDefinition,PortType,StreamSequence>::pushSRI" );
    return;
  }


  //
  // getStreamDefinition
  //
  // @return   StreamDefinition Return the stream definition for this attachId
  //           NULL attachId was not found
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  StreamDefinition* OutAttachablePort<StreamDefinition,PortType,StreamSequence>::getStreamDefinition(const char* attachId)
  {
    Stream* foundStream = this->streamContainer.findByAttachId(std::string(attachId));
    if (foundStream != NULL) {
        return new StreamDefinition(foundStream->getStreamDefinition());
    }
    return NULL;
  }


  //
  // getUser
  //
  // @return char * the user id that made the attachId request
  //         NULL attachId was not found
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  char* OutAttachablePort<StreamDefinition,PortType,StreamSequence>::getUser(const char* attachId)
  {
    // ADD DEPRECATED MESSAGE HERE
    return NULL;
  }

  //
  // usageState
  //
  // If the number of attached streams == 0 
  //      return typename PortType::IDLE;
  // If the number of attached streams == 1
  //      return typename PortType::BUSY;
  // other
  //    return typename PortType::ACTIVE;
  //
  // @return   typename PortType::InputUsageState return the current state of the port base on how many attach requests have been received
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  typename PortType::InputUsageState OutAttachablePort<StreamDefinition,PortType,StreamSequence>::usageState()
  {
    if (this->streamContainer.hasAttachments()) {
      return PortType::IDLE;
    } else {
      return PortType::ACTIVE;
    }
  }

  //
  // attachedStreams
  //
  // @return StreamSequence Returns a last Stream Defintion that was made. The caller
  //                                    is responsible for freeing the provided objects
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  StreamSequence* OutAttachablePort<StreamDefinition,PortType,StreamSequence>::attachedStreams()
  {
    unsigned int i = 0;
    StreamIter iter;
    StreamSequence* seq = new StreamSequence();

    StreamList streams = this->streamContainer.getStreams();
    seq->length(streams.size());
    for (iter = streams.begin(); iter != streams.end(); iter++) {
      (*seq)[i++] = iter->getStreamDefinition();
    }

    return seq;
  }

  //
  // attachmentIds
  //
  // @return BULKIO::StringSequence Return the current list of attachment identifiers. The caller
  //                                is responsible for freeing the provided objects
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  BULKIO::StringSequence* OutAttachablePort<StreamDefinition,PortType,StreamSequence>::attachmentIds()
  {
    BULKIO::StringSequence* seq = new BULKIO::StringSequence();
    std::vector<std::string> attachedIds = this->streamContainer.getAttachIds();
    std::vector<std::string>::iterator iter;

    seq->length(attachedIds.size());
    unsigned int i = 0;
    for (iter = attachedIds.begin(); iter != attachedIds.end(); iter++) {
      (*seq)[i++] = CORBA::string_dup((*iter).c_str());
    }
    return seq;
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  BULKIO::StringSequence* OutAttachablePort<StreamDefinition,PortType,StreamSequence>::attachmentIds(const std::string& streamId)
  {
    BULKIO::StringSequence* seq = new BULKIO::StringSequence();
    Stream* foundStream = this->streamContainer.findByStreamId(streamId);
    unsigned int i = 0;
    if (foundStream) {
      StreamAttachmentIter iter;
      StreamAttachmentList attachments = foundStream->getStreamAttachments();
      seq->length(attachments.size());
      for (iter = attachments.begin(); iter != attachments.end(); iter++) {
        (*seq)[i++] = CORBA::string_dup(iter->getAttachId().c_str());
      }
    }
    return seq;
  }

  //
  //  Attachable Interface
  //

  //
  // attach
  //
  // Send out a request to attach to the provided stream definition.  The end point servicing
  // this request will provide an attachment identifier that can be used by the detach request
  //    
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  char* OutAttachablePort<StreamDefinition,PortType,StreamSequence>::attach(const StreamDefinition& stream, const char* userid) 
      throw (typename PortType::AttachError, typename PortType::StreamInputError)
  {
    // TODO: ADD DEPRECATION WARNING HERE
    this->streamContainer.removeStreamByStreamId(std::string(stream.id));
    user_id = userid;
    this->addStream(stream); 
    return CORBA::string_dup("");
  }

  //
  // updateStream
  //
  // Allows users to update an active stream 
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  bool OutAttachablePort<StreamDefinition,PortType,StreamSequence>::updateStream(const StreamDefinition& stream) {
    
    {
      // Scope lock all BUT addStream to avoid deadlock
      boost::mutex::scoped_lock lock(updatingPortsLock);

      std::string streamId(stream.id);
      if (not this->streamContainer.hasStreamId(streamId)) {
        return false;
      }

      this->streamContainer.removeStreamByStreamId(std::string(stream.id));
    }
    return this->addStream(stream);
  }


  //
  // addStream
  //
  // Registers stream definition as an active stream. Does NOT allow streams
  // with identical stream.ids.  Returns false if stream already exists
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  bool OutAttachablePort<StreamDefinition,PortType,StreamSequence>::addStream(const StreamDefinition& stream)
  {
    TRACE_ENTER(logger, "OutAttachablePort<StreamDefinition,PortType,StreamSequence>::addStream" );
    boost::mutex::scoped_lock lock(updatingPortsLock);
    //this->streamContainer.printState("Before stream added");

    std::string attachId;

    // If the stream already exists, return false
    if (this->streamContainer.hasStreamId(std::string(stream.id))) {
        return false;
    }

    // Create a new stream for attachment
    Stream* newStream = new Stream(stream, std::string(user_id), std::string(stream.id));

    // Iterate through connections and apply filterTable
    bool portListed = false;
    std::vector<bulkio::connection_descriptor_struct >::iterator ftPtr;
    for (ConnectionsIter i = outConnections.begin(); i != outConnections.end(); ++i) {
       for (ftPtr=filterTable.begin(); ftPtr!= filterTable.end(); ftPtr++) {

          if (ftPtr->port_name == this->name) portListed=true;

          if ( (ftPtr->port_name == this->name) and
             (ftPtr->connection_id == i->second) and
             (strcmp(ftPtr->stream_id.c_str(),stream.id) == 0 ) ){
             LOG_DEBUG(logger,"attach - PORT:" << this->name << " CONNECTION:" << ftPtr->connection_id << " SRI streamID:" << stream.id); 

             try {
                // Create a new attachment for valid filterTable entry
                OutPortSriMap::iterator sri_iter=  currentSRIs.find( std::string(stream.id) );
                if ( sri_iter != currentSRIs.end() ) {
                    newStream->setSRI(sri_iter->second.sri);
                    newStream->setTime(sri_iter->second.time);
                }
                
                try {
                    newStream->createNewAttachment(i->second, i->first);
                } catch (typename PortType::AttachError& ex) {
                    LOG_ERROR( logger, __FUNCTION__ << ": AttachError occurred: " << ex.msg);
                } catch (typename PortType::StreamInputError& ex) {
                    LOG_ERROR( logger, __FUNCTION__ << ": StreamInputError occurred: " << ex.msg);
                } catch(...) {
                    LOG_ERROR( logger, __FUNCTION__ << ": Unknown attachment error occured: " << this->name << "/" << i->second );
                }

             } catch(...) {
                LOG_ERROR( logger, "UNABLE TO CREATE ATTACHMENT, PORT/CONNECTION: " << this->name << "/" << i->second );
             }
          }
       }
    }
    
    if (not portListed) {
      OutPortSriMap::iterator sri_iter=  currentSRIs.find( std::string(stream.id) );
      if ( sri_iter != currentSRIs.end() ) {
         newStream->setSRI(sri_iter->second.sri);
         newStream->setTime(sri_iter->second.time);
      }
      // Route new stream to all connections
      for (ConnectionsIter i = outConnections.begin(); i != outConnections.end(); ++i) {
        try {
            newStream->createNewAttachment(i->second, i->first);
        } catch (typename PortType::AttachError& ex) {
            LOG_ERROR( logger, __FUNCTION__ << ": AttachError occurred: " << ex.msg);
        } catch (typename PortType::StreamInputError& ex) {
            LOG_ERROR( logger, __FUNCTION__ << ": StreamInputError occurred: " << ex.msg);
        } catch(...) {
            LOG_ERROR( logger, __FUNCTION__ << ": Unknown attachment error occured: " << this->name << "/" << i->second );
        }
      }
    }

    LOG_DEBUG(logger, "ATTACHABLE PORT: CREATED NEW STREAM :" << stream.id );

    this->streamContainer.addStream(*newStream);
    delete newStream;
    this->streamContainer.printState("End of Attach");

    TRACE_EXIT(logger, "OutAttachablePort<StreamDefinition,PortType,StreamSequence>::attach" );
    return true;
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::removeStream(const std::string& streamId)
  {
    TRACE_ENTER(logger, "OutAttachablePort<StreamDefinition,PortType,StreamSequence>::removeStream" );
    boost::mutex::scoped_lock lock(updatingPortsLock);

    //this->streamContainer.printState("Beginning of RemoveStream");
    this->streamContainer.removeStreamByStreamId(streamId);
    this->streamContainer.printState("End of RemoveStream");

    TRACE_EXIT(logger, "OutAttachablePort<StreamDefinition,PortType,StreamSequence>::removeStream" );
  }

  //
  // detach
  //
  // Send a request to detach from a stream definition for the provided attachment identifier.
  //
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::detach(const char* attach_id )
  {
    TRACE_ENTER(logger, "OutAttachablePort<StreamDefinition,PortType,StreamSequence>::detach" );
    boost::mutex::scoped_lock lock(updatingPortsLock);
    std::string attachId(attach_id);

    //this->streamContainer.printState("Beginning of Detach");
    this->streamContainer.detachByAttachId(std::string(attachId));
    this->streamContainer.printState("End of Detach");
    
    TRACE_EXIT(logger, "OutAttachablePort<StreamDefinition,PortType,StreamSequence>::detach" );
  }

  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  void OutAttachablePort<StreamDefinition,PortType,StreamSequence>::detach(const char* attach_id, const char *connection_id )
  {
    TRACE_ENTER(logger, "OutAttachablePort<StreamDefinition,PortType,StreamSequence>::detach" );
    boost::mutex::scoped_lock lock(updatingPortsLock);

    //this->streamContainer.printState("Beginning of Detach");
    std::string attachId(attach_id);
    std::string connectionId(connection_id);

    if (connectionId.empty()) {
      LOG_WARN(logger, "UNABLE TO DETACH SPECIFIC CONNECTION ID: CONNECTION ID IS <BLANK>");
    } else {
      this->streamContainer.detachByAttachIdConnectionId(attachId, connectionId);
      LOG_DEBUG(logger, "ATTACHABLE PORT: DETACH COMPLETED ID:" << attachId  );
    }
    this->streamContainer.printState("End of Detach");
    TRACE_EXIT(logger, "OutAttachablePort<StreamDefinition,PortType,StreamSequence>::detach" );
  }

  // Grab port by connectionId
  template <typename StreamDefinition, typename PortType, typename StreamSequence>
  typename PortType::_ptr_type OutAttachablePort<StreamDefinition,PortType,StreamSequence>::getConnectedPort(const std::string& connectionId) {
    for (ConnectionsIter iter = outConnections.begin(); iter != outConnections.end(); ++iter) {
      if (iter->second == connectionId) {
        return iter->first;
      }
    }
    return PortType::_nil();
  }

  template class InAttachablePort<BULKIO::VITA49StreamDefinition, 
                   BULKIO::dataVITA49, 
                   BULKIO::VITA49StreamSequence, 
                   POA_BULKIO::dataVITA49>;

  template class OutAttachablePort<BULKIO::VITA49StreamDefinition, 
                    BULKIO::dataVITA49, 
                    BULKIO::VITA49StreamSequence>;
  
  template class InAttachablePort<BULKIO::SDDSStreamDefinition, 
                   BULKIO::dataSDDS, 
                   BULKIO::SDDSStreamSequence, 
                   POA_BULKIO::dataSDDS>;

  template class OutAttachablePort<BULKIO::SDDSStreamDefinition, 
                    BULKIO::dataSDDS, 
                    BULKIO::SDDSStreamSequence>;
  

}  // end of bulkio namespace

