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

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "WebServiceClient.h"
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
WebServiceClient::WebServiceClient(byte *serverAddr, 
                                   int serverPort,
                                   char *userAgent)
{
  this->serverAddr = serverAddr;
  this->serverPort = serverPort;
  this->userAgent = userAgent;
}

/******************************************************************************
 * User API
 ******************************************************************************/

//------------------------------------------------------------------------------
void WebServiceClient::call(char *uri, 
                            char *content,
                            void (*successHandler)(int rc),
                            void (*failureHandler)(int rc),
                            int requestTimeout,
			    char *contentType)
{

  Client wsclient(this->serverAddr, this->serverPort);
  if (wsclient.connect()) {

    // Send HTTP POST to the server
    sprintf(this->buffer, "POST %s HTTP/1.1", uri);
    wsclient.println(this->buffer);
    sprintf(this->buffer, 
            "Host: %d.%d.%d.%d:%d", 
            this->serverAddr[0], this->serverAddr[1], this->serverAddr[2], 
            this->serverAddr[3], this->serverPort);
    wsclient.println(this->buffer);
    sprintf(this->buffer, "User-Agent: %s", this->userAgent);
    wsclient.println(this->buffer);
    sprintf(this->buffer, "Content-Length: %d", strlen(content));
    wsclient.println(this->buffer);
    sprintf(this->buffer, "Content-Type: %s", contentType);
    wsclient.println(this->buffer);
    sprintf(this->buffer, "Accept: %s", contentType);
    wsclient.println(this->buffer);
    wsclient.println();

    // Post web service input data
    wsclient.println(content);
    
    // Pause for web service to complete
    unsigned long startTime = millis();
    while ((!wsclient.available()) && ((millis() - startTime) < requestTimeout));
    
    // Read the response
    int rc = parse_http_status(&wsclient);
    if (rc == HTTP_STATUS_SUCCESS) {
      if (successHandler != NULL) {
        successHandler(rc);
      }
    } else {
      if (failureHandler != NULL) {
        failureHandler(rc);
      }
    }
    flush_content(&wsclient);
    
    // Disconnect from the server
    wsclient.flush();
    wsclient.stop();
    
  } else {
      if (failureHandler != NULL) {
        failureHandler(-1);
      }
  }

}

//------------------------------------------------------------------------------
void WebServiceClient::flush_content(Client *wsclient) {
  while (wsclient->available()) {
    wsclient->read();
  }
}

//------------------------------------------------------------------------------
int WebServiceClient::parse_http_status(Client *wsclient) {
  int parsePos = 0;
  int statusCode = -1;
  while (wsclient->available() && parsePos < 2) {
    char c = wsclient->read();
    if (parsePos < 1) {
      if (c == ' ') parsePos++;
    } 
    else if (parsePos == 1) {
      if (c >= '0' && c <= '9') {
        if (statusCode == -1) {
          statusCode = c - '0';
        } 
        else {
          statusCode *= 10;
          statusCode += c - '0';
        }
      } 
      else {
        parsePos++;
      }
    } 
    else {
      break;
    }
  }
  return statusCode;
}


