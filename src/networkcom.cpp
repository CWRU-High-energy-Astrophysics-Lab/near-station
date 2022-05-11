//
// Created by robin on 12/20/21.
//

#include "networkcom.h"
#include "client.h"
#include "server.h"

std::mutex munet;
bool working = true;
std::priority_queue<std::string> msgToT2PI;

std::priority_queue<std::string> msgToStorage;// send every thing that is not T2
std::priority_queue<std::string> msgNetIn;


bool netThread() {
    // start on thead of incoming
    std::thread serverthead(startServer);

    //client loop
    while (working) {
        if (!ismsgToT2PIEmpty()) {
            send("computePi", "2000", getmsgToT2PI());
        }
        if (!ismsgToStorageEmpty()) {
            send("storagePi", "2010", getmsgToStorage());
        }
    }
    serverthead.join();
    return working;
}

void startServer() {
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
    bool temp = msgNetIn.empty();
    munet.unlock();
    return temp;
}

bool ismsgToStorageEmpty() {
    munet.lock();
    bool temp = msgToT2PI.empty();
    munet.unlock();
    return temp;
}


std::string encryptnet(const Generalmsg &generalmsg) {
    return std::string(generalmsg.gedID() + "[" + generalmsg.getRev() + "]:" + std::to_string(generalmsg.getSize()) +
                  generalmsg.getPayload());
}

Generalmsg decryptnet(std::string input) {
    Generalmsg msg;
    std::string type = input.substr(0, 3);
    unsigned long headerend = input.find(':');
    std::string payload = input.substr(headerend);
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
std::string getmsgToT2PI() {
    munet.lock();
    std::string pack = msgToT2PI.top();
    msgToT2PI.pop();
    munet.unlock();
    return pack;
}

// add incoming msg to queue to be unpacked and process
void addmsgtoT2PI(Generalmsg incoming) {
    munet.lock();
    msgToT2PI.push(encryptnet(incoming));
    munet.unlock();
}


//returns next msg needed to be sent
std::string getmsgToStorage() {
    munet.lock();
    std::string pack = msgToStorage.top();
    msgToStorage.pop();
    munet.unlock();
    return pack;
}

// add incoming msg to queue to be unpacked and process
void addmsgtoStorage(Generalmsg incoming) {
    munet.lock();
    msgToStorage.push(encryptnet(incoming));
    munet.unlock();
}


//returns next msg needed to be sent
Generalmsg getmsgToNetIn() {
    munet.lock();

    std::string pack = msgNetIn.top();

    msgNetIn.pop();
    munet.unlock();
    return decryptnet(pack);


}

// add incoming msg to queue to be unpacked and process
void addmsgtoNetIn(std::string incoming) {
    munet.lock();
    msgNetIn.push(incoming);
    munet.unlock();
}
