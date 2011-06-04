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

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "RestServer.h"
#include <Ethernet.h>

/******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * Constructors
 ******************************************************************************/

//------------------------------------------------------------------------------
//
// Constructs a web service client
//
RestServer::RestServer(int serverPort) {
  this->serverPort = serverPort;
  this->handlerCount = 0;
}

/******************************************************************************
 * User API
 ******************************************************************************/

//------------------------------------------------------------------------------
void RestServer::attach(char *name, restHandler handler) {
  Serial.print("Attaching [");
  Serial.print(name);
  Serial.println("]...");
  RestHandlerDef rec = {};
  rec.name = name;
  rec.handler = handler;
  handlers[handlerCount++] = rec;
}

//------------------------------------------------------------------------------
void RestServer::begin() {
  this->server = new Server(this->serverPort);
}

//------------------------------------------------------------------------------
bool RestServer::buffer_client_stream(Client *client) {

  // read a character from client stream
  char c = client->read();

  //  add character to buffer if not carriage return / linefeed
  //  if we run out of buffer, overwrite the end
  if (c != '\n' && c != '\r') {
    buffer[bufferIndex++] = c;
    buffer[bufferIndex] = '\0';
    if (bufferIndex >= BUFFER_SIZE) {
      bufferIndex -= 1;
    }
    return true;
  } else {
    return false;
  }
  
}

//------------------------------------------------------------------------------
RestHandlerDef* RestServer::find_handler(RestRequest *request) {
  if (handlerCount > 0) {
    for (int i = 0; i < handlerCount; i++) {
      if (strcmp(handlers[i].name, request->command) == 0) {
        return &handlers[i];
      }
    }
  }
  return NULL;
}	

//------------------------------------------------------------------------------
void RestServer::generate_header(Client *client, int code, char *contentType)
{
  client->print("HTTP/1.1 ");
  client->print(code);
  client->println(" OK");
  client->print("Content-Type: ");
  client->println(contentType);
  client->println();
}

//------------------------------------------------------------------------------
void RestServer::generate_response(Client *client, 
                                   char * content, 
                                   int code, 
                                   char *contentType)
{
  generate_header(client, code, contentType);
  client->println(content);
}

//------------------------------------------------------------------------------
void RestServer::handle_request(RestRequest *request, Client *client) {

  // find request handler
  RestHandlerDef* handler = find_handler(request);

  // execute corresponding handler, or generate 404 response if none defined
  if (handler != NULL) {
    handler->handler(request, client);
  } else {
    client->println("HTTP/1.1 404 OK");
    client->println("Content-Type: application/json");
    client->println();
    client->println("{ \"status\": \"NO HANDLER\" }");
  }

}

//------------------------------------------------------------------------------
void RestServer::parse_client_buffer(RestRequest* request) {

  // convert buffer into a string for further processing
  String s = String(buffer);

  // extract http method
  s.substring(0, s.indexOf(' ')).toCharArray(request->method, 8);;
  
  // reduce string to uris
  s = s.substring(s.indexOf('/'), s.indexOf(' ', s.indexOf('/')));
  
  // parse parameters from uri
  s.toCharArray(buffer, BUFFER_SIZE);
  String(strtok(buffer, "/")).toCharArray(request->command, 16);
  String(strtok(NULL, "/")).toCharArray(request->data, 32);
}

//------------------------------------------------------------------------------
void RestServer::process() {

  // listen for incoming clients
  Client client = server->available();
  if (client) {

   // handle client connection
    while (client.connected()) {
      if (client.available()) {

        // buffer character from client. if necessary to buffer, the continue
        // immediately to next character
        if (buffer_client_stream(&client)) {
          continue;
        }

	Serial.print("raw buffer: ");
        Serial.println(buffer);

        // parse buffer of client stream
	RestRequest request = {};
        parse_client_buffer(&request);
 	Serial.print("http method: [");
        Serial.print(request.method);
	Serial.println("]");
        Serial.print("command: [");
        Serial.print(request.command);
	Serial.println("]");
        Serial.print("data: [");
        Serial.print(request.data);
	Serial.println("]");

	// handle request
	handle_request(&request, &client);

	break;
      }
    }

    // close client connection
    client.stop();

    // reset processing buffer
    reset();

  }
}

//------------------------------------------------------------------------------
void RestServer::reset() {
  buffer[0] = '\0';
  bufferIndex = 0;
}

//------------------------------------------------------------------------------
void* operator new(size_t size) { 
  return malloc(size);
}

//------------------------------------------------------------------------------
void operator delete(void* ptr) { 
  if (ptr) free(ptr);
} 

