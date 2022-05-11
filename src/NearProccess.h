//
// Created by robin on 10/22/21.
//

#include <iostream>


#include <queue>
#include <thread>

#include "msg_outline/Generalmsg.h"
#include "msg_outline/T3msg.h"
#include "msg_outline/T2msg.h"
#include "msg_outline/Cmdmsg.h"
#include "msg_outline/MsgHistory.h"
#include "msg_outline/Logmsg.h"
#include "xbeeinterface.h"
#include "centralinterface.h"


#ifndef FAR_STATION_ROBIN_EAD_H
#define FAR_STATION_ROBIN_EAD_H

#endif //FAR_STATION_ROBIN_EAD_H


priority_queue<Generalmsg> msgToProccess;
priority_queue<Generalmsg> msgToPack;
priority_queue<string> msgToSend;
priority_queue<string> msgToUnPack;
priority_queue<Generalmsg> msgToCentral;
priority_queue<Generalmsg> msgFromCentral;
;
bool msgtoProccessEmpty();
//string encrypt(const Generalmsg& msg) ;
//Generalmsg decrypt(basic_string<char> msg);
bool send_t3();

bool npt();


class NearProccess{
public:

   char *const EKITPORT = getenv("EKITPORT"); //enviormental varible

    int USB{};


int start();





static Generalmsg getmsgToProccess();
static void addmsgtoPack(const Generalmsg& outgoing);



//piCommand
static int bashCmd(const string& cmd);



string getT2();

string getT3();




};
