//
// Created by robin on 12/20/21.
//

#include "networkcom.h"
#include "client.h"
#include "server.h"
mutex munet;
bool working = true;
priority_queue<string> msgToT2PI;

priority_queue<string> msgToStorage;// send every thing that is not T2
priority_queue<string> msgNetIn;


bool netThread(){
   // start on thead of incoming
    thread serverthead(startServer);

   //client loop
    while (working){
        if(!ismsgToT2PIEmpty()){
            send("computePi","2000",getmsgToT2PI());
        }
        if (!ismsgToStorageEmpty()){
            send("storagePi","2010",getmsgToStorage());
        }
    }
    serverthead.join();
    return working;
}
void startServer(){
    recieve();
}
bool ismsgToT2PIEmpty() {
    munet.lock();
    bool temp = msgToT2PI.empty();
    munet.unlock();
    return temp;
}
bool ismsgFromNetIn() {
    munet.lock();
    bool temp =msgNetIn.empty();
    munet.unlock();
    return temp;
}
bool ismsgToStorageEmpty() {
    munet.lock();
    bool temp = msgToT2PI.empty();
    munet.unlock();
    return temp;
}



string encryptnet(const Generalmsg& generalmsg) {
    return string(generalmsg.gedID() + "[" + generalmsg.getRev() + "]:" + to_string(generalmsg.getSize()) +
                  generalmsg.getPayload());
}

Generalmsg decryptnet(basic_string<char> input) {
    Generalmsg msg;
    string type = input.substr(0, 3);
    unsigned long headerend = input.find(':');
    string payload = input.substr(headerend);
    if (type == "T3LI") {
        msg = T3msg(payload);
    } else if (type.substr(0, 2) == "CMD") {
        msg = Cmdmsg(payload, type.at(3));
    } else if (type == "HIST") {
        msg = MsgHistory(payload);
    } else if (type == "T2LI") {
        msg = T2msg(payload);
    } else if (type == "LOGA") {
        msg = Logmsg(payload);
    } else {
        msg = Generalmsg(type, "REV0", payload, 12);
        //add a report to log
    }


    return msg;
}

//returns next msg needed to be sent
string getmsgToT2PI(){
    munet.lock();
    string pack = msgToT2PI.top();
    msgToT2PI.pop();
    munet.unlock();
    return pack;
}

// add incoming msg to queue to be unpacked and process
void addmsgtoT2PI(Generalmsg incoming){
    munet.lock();
    msgToT2PI.push(encryptnet(incoming));
    munet.unlock();
}


//returns next msg needed to be sent
string getmsgToStorage(){
    munet.lock();
    string pack = msgToStorage.top();
    msgToStorage.pop();
    munet.unlock();
    return pack;
}

// add incoming msg to queue to be unpacked and process
void addmsgtoStorage(Generalmsg incoming){
    munet.lock();
    msgToStorage.push(encryptnet(incoming));
    munet.unlock();
}


//returns next msg needed to be sent
Generalmsg getmsgToNetIn(){
    munet.lock();
    string pack = msgNetIn.top();
    msgNetIn.pop();
    munet.unlock();
    return decryptnet(pack);
}

// add incoming msg to queue to be unpacked and process
void addmsgtoNetIn(string incoming){
    munet.lock();
    msgNetIn.push(incoming);
    munet.unlock();
}
