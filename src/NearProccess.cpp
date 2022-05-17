//
// Created by robin on 10/21/21.
//
#include "t2li.h"
#include "NearProccess.h"
#include <mutex>
#include <thread>
#include <iostream>

#include <filesystem>
#include <utility>
#include "packinterface.h"
#include "networkcom.h"


std::mutex mu;
std::mutex mu2;
std::mutex mu3;
std::mutex mu4;
std::mutex mu5;
bool restartingpi = false;

bool getRestart() {
    return restartingpi;
}

std::string user;

bool init() { //set baud rates and check file system layout
    msgToProccess = std::priority_queue<Generalmsg>();
    msgToPack = std::priority_queue<Generalmsg>();
    msgToUnPack = std::priority_queue<std::string>();
    msgToSend = std::priority_queue<std::string>();
    msgToCentral = std::priority_queue<Generalmsg>();
    msgFromCentral = std::priority_queue<Generalmsg>();

    user = "/home/" + std::string(getenv("USER"));
    try {
        std::filesystem::current_path(user);
        std::filesystem::current_path(user + "/data");
        std::filesystem::current_path(user + "/data/log");
        std::filesystem::current_path(user + "/data/temp");
        std::filesystem::current_path(user);


    }
    catch (std::filesystem::filesystem_error const &ex) {
        std::cout << "File system not as expected, please check\n";
        return false;
    }

    return true;
}

int main() {//this is called on pi boot
    if (init()) {

        std::thread processThread(npt);

        //Xbee thread
        std::thread xbeeThread(xbeeLoop);
        //server thread
        std::thread serverThread(startServer);

        serverThread.join();
        processThread.join();


    }
    return 1;
}

bool npt() {
    NearProccess node = NearProccess();

    return node.start();


}

int NearProccess::start() {

    while (!restartingpi) {

        std::string msg = getmsgToUnpack();




        if (!msg.empty()) {
            std::cout<< "testpoint1" <<std::endl;
            addmsgtoProccess(msg);

        }

        if (!msgtoProccessEmpty()) {
            std::cout<< "testpoint2" <<std::endl;
             Generalmsg msg = getmsgToProccess();
            std::string type = msg.getID();


             if (type == "T3LI") {
                 addmsgtoPack(msg);// sent to storGE




             } else if (type.substr(0, 2) == "CMD") {


                 if (msg.getID() == "CMDN") {
                     restartingpi = true;
                 }
                 else{
                     addmsgtoPack(msg);
                 }
             }
              else if (type == "HIST") {

                 addmsgtoPack(msg);
                 //history request, send history
             } else if (type == "T2LI") {
                 addmsgtoCentral(msg);

             } else if (type == "LOGB") {
                 addmsgtoPack(msg);
                 // log request, send log file.
             } else {

                 std::cout<< type <<std::endl;
                 std::cout << encrypt(msg);
                 //add a report to log
             }
         }
         while(ismsgFromNetIn()!=true){

             addmsgtoProccess(encrypt(getmsgToNetIn()));
         }




    }
    return restartingpi;
}


bool msgtoProccessEmpty() {
    mu.lock();
    bool temp = msgToProccess.empty();

    mu.unlock();

    return temp;
}


//function to msgToProccess
void addmsgtoProccess(std::string incoming) {

    Generalmsg msg = decrypt(incoming);

    mu.lock();
    msgToProccess.push(msg);
    mu.unlock();
}

Generalmsg NearProccess::getmsgToProccess() {
    Generalmsg msg;
    mu.lock();

    if (!msgToProccess.empty()) {
        msg = msgToProccess.top();
        msgToProccess.pop();
    }

    mu.unlock();
    return msg;
}

// functions to msgToPack
void NearProccess::addmsgtoPack(const Generalmsg &outgoing) {
    mu2.lock();
    msgToPack.push(outgoing);
    mu2.unlock();

};

std::string getmsgToPack() {
    mu2.lock();
    std::string pack = encrypt(msgToPack.top());
    msgToPack.pop();
    mu2.unlock();
    return pack;

}

//function to msgToSend
void addmsgtoSend(std::string outgoing) {
    mu3.lock();
    msgToSend.push(outgoing);
    mu3.unlock();

};

std::string getmsgToSend() {
    mu3.lock();
    std::string pack;
    if (!msgToSend.empty()) {
        pack = msgToSend.top();
        msgToSend.pop();
    }
    mu3.unlock();
    return pack;

}

//Functions to msgToUnpack
void addmsgtoUnpack(std::string incoming) {

    mu4.lock();

    msgToUnPack.push(incoming);
    mu4.unlock();

};

std::string getmsgToUnpack() {
    mu4.lock();
    std::string pack;
    if (!msgToUnPack.empty()) {

        pack = msgToUnPack.top();
        msgToUnPack.pop();
    }
    mu4.unlock();


    return pack;


}


//Functions to msgToUnpack
void addmsgtoCentral(Generalmsg incoming) {
    mu5.lock();
    msgToCentral.push(incoming);
    mu5.unlock();

};

Generalmsg getmsgFromCentral() {
    mu5.lock();
    Generalmsg pack = (msgFromCentral.top());
    msgToCentral.pop();
    mu5.unlock();
    return pack;

}


//piCommand
int NearProccess::bashCmd(const std::string &cmd) {
    return system(cmd.c_str());
}


std::string encrypt(const Generalmsg &generalmsg) {
    return std::string(generalmsg.getID() + "[" + generalmsg.getRev() + "]:" + std::to_string(generalmsg.getSize()) +
                  generalmsg.getPayload());
}

Generalmsg decrypt(std::string input) {
    Generalmsg msg;
    std::string type = input.substr(0, 4);
    unsigned long headerend = input.find(':');
    try {
        std::string payload = input.substr(headerend+1);
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
    }
    catch (const std::out_of_range){
        msg = Generalmsg("ERRO", "REV0", input, 12);
    }

    return msg;
}
