/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifdef  HAVE_LOG4CXX
#include <string.h>
#include <iostream>
#include <algorithm>
#include <log4cxx/logger.h>
#include <log4cxx/helpers/bytearrayinputstream.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/loglog.h>
#include "StringInputStream.h"


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace std;

IMPLEMENT_LOG4CXX_OBJECT(StringInputStream)

StringInputStream::StringInputStream(const std::string &data) :
buf(data),
  stream(buf), 
  pos(0), 
  cnt(0) 
{ 
  // for testing
  //LogLog::setInternalDebugging(true);
}



StringInputStream::~StringInputStream() {
}


void StringInputStream::close() {
}

#if 0
int StringInputStream::read(ByteBuffer& dst) {
  if ( pos >= buf.size() ) {
    //LogLog::debug( "line(" << cnt << ") >>> EOF" );
    return -1;
  } else {
    std::string delimiter("\n");
    size_t tpos;
    std::string token;
    tpos = buf.find(delimiter);
    if ( tpos != std::string::npos ) {
        token = buf.substr(tpos+delimiter.length());
	//LogLog::debug( "token(" << token.size() << ")>>>" << token <<  "<<< " );
	buf.erase(0, buf.find(delimiter) + delimiter.length());
    }
    else {
      token  = buf;
      buf.erase(0,buf.size());
    }
    if ( token.size() > 0 ) {
      size_t bytesCopied = min(dst.remaining(), token.size());
      //LogLog::debug( "read:  pos:bytesCopied " << pos << ":" << bytesCopied );
      memcpy(dst.current(), &buf[pos], bytesCopied);
      pos += bytesCopied;
      dst.position(dst.position() + bytesCopied);
      return bytesCopied;
    }
    
    return -1;
  }
    
  return -1;

}
#endif


int StringInputStream::read(ByteBuffer& dst) {
  if ( pos >= buf.size() ) {
    //std::cout << "line(" << cnt << ") >>> EOF" << std::endl;
    return -1;
  } else {
    // grab next line from the string stream
    std::string line;
    std::getline( stream, line );
    if ( !stream.good() )  {
      //std::cout << "line(" << cnt << ") >>> EOF" << std::endl;
      return -1;
    }

    // add back end of line for 
    line=line+"\n";
    size_t bytesCopied = min(dst.remaining(), line.size());
    std::ostringstream os;
    os << "DST (pos,bytestocopy): " << pos << ":" << bytesCopied << " SRC (" << cnt << ":" << line.size() << ")>>>" << line <<  "<<< ";    
    //LogLog::debug( LogString(os.str()) );
    memcpy(dst.current(), line.c_str(), bytesCopied);
    pos += bytesCopied;
    cnt++;
    dst.position(dst.position() + bytesCopied);
    return bytesCopied;
  }
    
  return -1;

}

#endif
