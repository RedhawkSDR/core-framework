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
/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LOG4CXX_HELPERS_STRINGINPUTSTREAM_H
#define _LOG4CXX_HELPERS_STRINGINPUTSTREAM_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <string>
#include <sstream>
#include <vector>
#include <log4cxx/helpers/inputstream.h>


namespace log4cxx
{

        namespace helpers {

          /**
           * InputStream implemented on top of a byte array.
           */
          class LOG4CXX_EXPORT StringInputStream : public InputStream
          {
          private:
	    //LOG4CXX_LIST_DEF(ByteList, unsigned char);
	    //ByteList buf;
	    std::string buf;
	    std::stringstream stream;
	    size_t pos;
	    size_t cnt;

          public:
                  DECLARE_ABSTRACT_LOG4CXX_OBJECT(StringInputStream)
                  BEGIN_LOG4CXX_CAST_MAP()
                          LOG4CXX_CAST_ENTRY(StringInputStream)
                          LOG4CXX_CAST_ENTRY_CHAIN(InputStream)
                  END_LOG4CXX_CAST_MAP()

                  /**
                   * Creates a StringInputStream.
                   *
                   * @param bytes array of bytes to copy into stream.
                   */
		    StringInputStream(const std::string & data);

                   virtual ~StringInputStream();

                  /**
                   * Closes this file input stream and releases any system 
                   * resources associated with the stream.
                   */
                  virtual void close();

                  /**
                   * Reads a sequence of bytes into the given buffer.
                   *
                   * @param buf The buffer into which bytes are to be transferred.
                   * @return the total number of bytes read into the buffer, or -1 if there
                   *         is no more data because the end of the stream has been reached.
                   */
                  virtual int read(ByteBuffer& buf);

          private:

                  StringInputStream(const StringInputStream&);

                  StringInputStream& operator=(const StringInputStream&);

          };

          LOG4CXX_PTR_DEF(StringInputStream);
        } // namespace helpers

}  //namespace log4cxx

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

#endif //_LOG4CXX_HELPERS_BYTEARRAYINPUTSTREAM_H
