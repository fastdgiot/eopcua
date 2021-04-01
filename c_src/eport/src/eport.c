/*----------------------------------------------------------------
* Copyright (c) 2021 Faceplate
*
* This file is provided to you under the Apache License,
* Version 2.0 (the "License"); you may not use this file
* except in compliance with the License.  You may obtain
* a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an
* "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
* KIND, either express or implied.  See the License for the
* specific language governing permissions and limitations
* under the License.
----------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "eport.h"

int read_exact(byte *buf, int len);
int write_exact(byte *buf, int len);
int read_cmd(byte **buf);
int write_cmd(byte *buf, int len);

void eport_loop(eport_request_handler callback){

    byte *buf;
    int len = 0;
    len = read_cmd(&buf);

    if (len == EOF)
    {
        free(buf);
        fprintf(stdout,"EXIT port\r\n");
        exit(EXIT_FAILURE);
    }
    
    fprintf(stdout,"message received: %s\r\n",(char *)buf);
    free(buf);

    cJSON *testJSON = cJSON_Parse( "{\"x1\":1}" );
    fprintf(stdout,"calback with a JSON\r\n");
    callback( testJSON );
    cJSON_Delete( testJSON );

}

//----------Read/Write helpers----------------------------------------
int read_exact(byte *buf, int len) {
    int i, got=0;
    do {
        if ((i = read(IN_DESC, buf+got, len-got)) <= 0){
            return EOF;
        }
        got += i;
    } while (got<len);

  return(len);
}

int write_exact(byte *buf, int len) {
    int i, wrote = 0;
    do {
        if ((i = write(OUT_DESC, buf+wrote, len-wrote)) <= 0)
        return (i);
        wrote += i;
    } while (wrote<len);

    return (len);
}

int read_cmd(byte **buf) {
    byte lbuf[HEADER_LENGTH];
    int len;
    int i;

    // Step 1. Read the length of the message
    if (read_exact(lbuf, HEADER_LENGTH) != HEADER_LENGTH) return(-1);

    // Convert the length from big-endian to little-endian?
    len = 0;
    for (i = HEADER_LENGTH -1; i >= 0; i--){
        len = len | (lbuf[i] << (8 * (HEADER_LENGTH - i -1)) );
    }

    // Step 2. Read the message itself
    *buf = malloc(len); // dynamically allocate the memory for the message
    if (*buf == NULL){
        return -1;
    }
    return read_exact(*buf, len);
}

int write_cmd(byte *buf, int len){
    byte li;

    li = (len >> 8) & 0xff;
    write_exact(&li, 1);
    
    li = len & 0xff;
    write_exact(&li, 1);

    return write_exact(buf, len);
}