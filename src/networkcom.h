//
// Created by robin on 12/20/21.
//

#include "msg_outline/Generalmsg.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <queue>
#include <mutex>
#include "msg_outline/T3msg.h"
#include "msg_outline/T2msg.h"
#include "msg_outline/Cmdmsg.h"
#include "msg_outline/MsgHistory.h"
#include "msg_outline/Logmsg.h"


#ifndef NEAR_STATION_NETWORKCOM_H
#define NEAR_STATION_NETWORKCOM_H

#endif //NEAR_STATION_NETWORKCOM_H



void startServer();

bool ismsgToT2PIEmpty();
bool ismsgFromNetIn();
bool ismsgToStorageEmpty();

//returns next msg needed to be sent
string getmsgToT2PI();

// add incoming msg to queue to be unpacked and process
void addmsgtoT2PI(Generalmsg incoming);


//returns next msg needed to be sent
string getmsgToStorage();

// add incoming msg to queue to be unpacked and process
void addmsgtoStorage(Generalmsg incoming);

//returns next msg needed to be sent
Generalmsg getmsgToNetIn();

// add incoming msg to queue to be unpacked and process
void addmsgtoNetIn (string incoming);