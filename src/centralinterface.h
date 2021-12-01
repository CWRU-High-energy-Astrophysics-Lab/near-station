//
// Created by robin on 11/24/21.
//

#ifndef FAR_STATION_ROBIN_XBEEINTERFACE_H
#define FAR_STATION_ROBIN_XBEEINTERFACE_H

#endif //FAR_STATION_ROBIN_XBEEINTERFACE_H

#include <string>
#include "msg_outline/Generalmsg.h"
//returns next msg needed to be sent
Generalmsg getmsgFromCentral();

// add incoming msg to queue to be unpacked and process
void addmsgtoCentral(Generalmsg incoming);

string encrypt(const Generalmsg& msg) ;
Generalmsg decrypt(basic_string<char> msg);