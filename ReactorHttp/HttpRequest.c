#include <stdio.h>
#include <strings.h>
#include <string.h>
#include "HttpRequest.h"


#define HeaderSize 12

struct HttpRequest* httpRequestInit(){
    struct HttpRequest* request = (struct HttpRequest*)malloc(sizeof(struct HttpRequest));
    httpRequestReset(request);
    request->reqHeaders = (struct RequestHeader*)malloc(sizeof(struct RequestHeader) * HeaderSize);

    return request;
}

void httpRequestReset(struct HttpRequest* req) {
    req->curState = ParseReqLine;
    req->method = NULL;
    req->url = NULL;
    req->version= NULL;
    req->reqHeadersNum = 0;
}

void httpRequestResetEx(struct HttpRequest* req) {
    free(req->url);
    free(req->method);
    free(req->version);
    if (req->reqHeaders != NULL){
        for (int i = 0; i < req->reqHeadersNum; ++i) {
            free(req->reqHeaders[i].key);
            free(req->reqHeaders[i].value);
        }
        free(req->reqHeaders);
    }
    httpRequestReset(req);
}

void httpRequestDestroy(struct HttpRequest* req) {
    if (req != NULL) {
        httpRequestResetEx(req);
        free(req);
    }
}

enum HttpRequestState HttpRequestState(struct HttpRequest* request) {
    return request->curState;
}

void HttpRequestAddHeader(struct HttpRequest* request, const char* key, const char* value) {
    request->reqHeaders[request->reqHeadersNum].key = key;
    request->reqHeaders[request->reqHeadersNum].value = value;
    request->reqHeadersNum++;
}

char HttpRequestGetHeader(struct HttpRequest* request, const char* key) {
    if (request != NULL) {
        for (int i = 0; i < request->reqHeadersNum; ++i) {
            if (strncasecmp(request->reqHeaders[i].key, key, strlen(key)) == 0) {
                return request->reqHeaders[i].value;
            }
        }
    }
    return NULL;
}


