/*
** Taiga
** Copyright (C) 2010-2013, Eren Okka
** 
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "herro.h"
#include "herro_types.h"

#include "base/encryption.h"

namespace sync {
namespace herro {

Service::Service() {
  host_ = L"api.herro.co";

  canonical_name_ = L"herro";
  name_ = L"Herro";
}

////////////////////////////////////////////////////////////////////////////////

void Service::BuildRequest(Request& request, HttpRequest& http_request) {
  http_request.host = host_;

  // Herro is supposed to return a JSON response for each and every request,
  // so that's what we expect from it
  http_request.header[L"Accept"] = L"application/json";
  http_request.header[L"Accept-Charset"] = L"utf-8";

  if (RequestNeedsAuthentication(request.type)) {
    // Herro uses an API token instead of password. The token has to be manually
    // generated by the user from their dashboard.
    // TODO: Make sure username and token are available
    http_request.header[L"Authorization"] = L"Basic " +
        Base64Encode(request.data[canonical_name_ + L"-username"] + L":" +
                     request.data[canonical_name_ + L"-token"]);
  }

  switch (request.type) {
    case kGetLibraryEntries:
      // Not sure Herro supports gzip compression, but let's put this here anyway
      // TODO: Make sure username is available
      http_request.header[L"Accept-Encoding"] = L"gzip";
      break;
  }

  switch (request.type) {
    BUILD_HTTP_REQUEST(kAddLibraryEntry, AddLibraryEntry);
    BUILD_HTTP_REQUEST(kAuthenticateUser, AuthenticateUser);
    BUILD_HTTP_REQUEST(kDeleteLibraryEntry, DeleteLibraryEntry);
    BUILD_HTTP_REQUEST(kGetLibraryEntries, GetLibraryEntries);
    BUILD_HTTP_REQUEST(kGetMetadataById, GetMetadataById);
    BUILD_HTTP_REQUEST(kSearchTitle, SearchTitle);
    BUILD_HTTP_REQUEST(kUpdateLibraryEntry, UpdateLibraryEntry);
  }
}

void Service::HandleResponse(Response& response, HttpResponse& http_response) {
  if (RequestSucceeded(response, http_response)) {
    switch (response.type) {
      HANDLE_HTTP_RESPONSE(kAddLibraryEntry, AddLibraryEntry);
      HANDLE_HTTP_RESPONSE(kAuthenticateUser, AuthenticateUser);
      HANDLE_HTTP_RESPONSE(kDeleteLibraryEntry, DeleteLibraryEntry);
      HANDLE_HTTP_RESPONSE(kGetLibraryEntries, GetLibraryEntries);
      HANDLE_HTTP_RESPONSE(kGetMetadataById, GetMetadataById);
      HANDLE_HTTP_RESPONSE(kSearchTitle, SearchTitle);
      HANDLE_HTTP_RESPONSE(kUpdateLibraryEntry, UpdateLibraryEntry);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Request builders

void Service::AuthenticateUser(Request& request, HttpRequest& http_request) {
  // Not available
}

void Service::GetLibraryEntries(Request& request, HttpRequest& http_request) {
  http_request.path =
      L"/list/anime/" + request.data[canonical_name_ + L"-username"];
}

void Service::GetMetadataById(Request& request, HttpRequest& http_request) {
  http_request.path = L"/anime/" + request.data[canonical_name_ + L"-id"];

  if (request.data.count(L"characters"))
    http_request.path += L"?characters=true";
}

void Service::SearchTitle(Request& request, HttpRequest& http_request) {
  http_request.path = L"/search/anime/" + request.data[L"title"];
}

void Service::AddLibraryEntry(Request& request, HttpRequest& http_request) {
  request.data[L"action"] = L"add";
  UpdateLibraryEntry(request, http_request);
}

void Service::DeleteLibraryEntry(Request& request, HttpRequest& http_request) {
  request.data[L"action"] = L"delete";
  UpdateLibraryEntry(request, http_request);
}

void Service::UpdateLibraryEntry(Request& request, HttpRequest& http_request) {
  http_request.method = L"POST";
  http_request.header[L"Content-Type"] = L"application/json";

  if (!request.data.count(L"action"))
    request.data[L"action"] = L"update";

  http_request.path = L"/list/anime/" + request.data[L"action"];

  http_request.body = L"{\n";
  http_request.body += L"\"_id\": \"" + request.data[canonical_name_ + L"-id"] + L"\"\n";
  // TODO: Optional parameters: status, progress, score
  // TODO: Find and use a proper JSON library
  http_request.body += L"}";
}

////////////////////////////////////////////////////////////////////////////////
// Response handlers

void Service::AuthenticateUser(Response& response, HttpResponse& http_response) {
  // Not available
}

void Service::GetLibraryEntries(Response& response, HttpResponse& http_response) {
  // TODO: Parse JSON data and update library
}

void Service::GetMetadataById(Response& response, HttpResponse& http_response) {
  // TODO: Parse JSON data and update database
}

void Service::SearchTitle(Response& response, HttpResponse& http_response) {
  // TODO: Parse JSON data and update database
}

void Service::AddLibraryEntry(Response& response, HttpResponse& http_response) {
  // Nothing to do here
}

void Service::DeleteLibraryEntry(Response& response, HttpResponse& http_response) {
  // Nothing to do here
}

void Service::UpdateLibraryEntry(Response& response, HttpResponse& http_response) {
  // Nothing to do here
}

////////////////////////////////////////////////////////////////////////////////

bool Service::RequestNeedsAuthentication(RequestType request_type) const {
  switch (request_type) {
    case kAddLibraryEntry:
    case kAuthenticateUser:
    case kDeleteLibraryEntry:
    case kUpdateLibraryEntry:
      return true;
  }

  return false;
}

bool Service::RequestSucceeded(Response& response,
                               const HttpResponse& http_response) {
  switch (http_response.code) {
    // OK
    case 200:
      return true;

    // Error
    case 500:
      // TODO: Read "response" from JSON data for the error description
      response.data[L"error"] = name() + L" returned an error";
      return false;

    default:
      response.data[L"error"] = name() + L" returned an unknown response";
      return false;
  }
}

}  // namespace herro
}  // namespace sync