/*
  WebServiceClient.h - Web Service Client library
  Copyright (c) 2011 Jon R. Brule.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _WebServiceClient_h
#define _WebServiceClient_h

#include <inttypes.h>
#include <WProgram.h>
#include <Ethernet.h>

#define MAX_BUFFER_SIZE 64
#define WS_CONTENT_TYPE "application/json"
#define WS_SERVER_TIMEOUT 15000
#define WS_USER_AGENT "Arduino Ethernet Shield"

extern "C" {
  typedef void (*failureHandler)(int rc);
  typedef void (*successHandler)(int rc);
}

class WebServiceClient {  

public:
  WebServiceClient(byte *serverAddr, int serverPort);
  void call(char *uri, 
            char *content,
            void (*successHandler)(int rc),
            void (*failureHandler)(int rc));

protected:
  void flush_content(Client *wsclient);
  int parse_http_status(Client *wsclient);

  byte *serverAddr;
  char buffer[MAX_BUFFER_SIZE];
  int serverPort;

};

#endif // _WebServiceClient_h
