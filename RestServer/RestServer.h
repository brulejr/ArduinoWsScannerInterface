/*
  RestServer.h - RESTful Web Server
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

#ifndef _RestServer_h
#define _RestServer_h

#include <stdlib.h>
#include <inttypes.h>
#include <WProgram.h>
#include <Ethernet.h>

#define BUFFER_SIZE 255
#define MAX_HANDLERS 8

#define HTTP_OK 200
#define HTTP_CREATED 201
#define HTTP_ACCEPTED 202
#define HTTP_NO_CONTENT 204
#define HTTP_BAD_REQUEST 400
#define HTTP_UNAUTHORIZED 401
#define HTTP_FORBIDDEN 403
#define HTTP_NOT_FOUND 404
#define HTTP_METHOD_NOT_ALLOWED 405
#define HTTP_INTERNAL_SERVER_ERROR 500
#define HTTP_NOT_IMPLEMENTED 501
#define HTTP_SERVICE_UNAVAILABLE 503

#define TYPE_APPLICATION_JSON "application/json"

struct RestRequest {
  char method[8];
  char command[16];
  char data[32];
};

extern "C" {
  typedef void (*restHandler)(RestRequest *request, Client *client);
}

struct RestHandlerDef {
  char* name;
  restHandler handler;
};

class RestServer {  

public:
  RestServer(int serverPort);
  void attach(char *name, restHandler handler);
  void begin();
  void generate_header(Client *client, 
                       int code = HTTP_OK, 
                       char *contentType = TYPE_APPLICATION_JSON);
  void generate_response(Client *client, 
                         char *content = "",
                         int code = HTTP_OK, 
                         char *contentType = TYPE_APPLICATION_JSON);
  void process();

protected:
  char buffer[BUFFER_SIZE];
  int bufferIndex;
  int handlerCount;
  int serverPort;
  RestHandlerDef handlers[MAX_HANDLERS];
  Server *server;

  bool buffer_client_stream(Client *client);
  RestHandlerDef* find_handler(RestRequest *request);
  void handle_request(RestRequest *request, Client *client);
  void parse_client_buffer(RestRequest *request);
  void reset();

};

#endif // _RestServer_h
