//
// Created by robin on 10/21/21.
//

#include "NearProccess.h"
#include <mutex>
#include <thread>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <utility>
#include <unistd.h>
#include "packinterface.h"
#include "networkcom.h"


std::mutex mu;
std::mutex mu2;
std::mutex mu3;
std::mutex mu4;

bool restartingpi = false;

bool getRestart() {
    return restartingpi;
}

std::string user;

bool init() { //set baud rates and check file system layout
    nearprocess::msgToProccess = std::priority_queue<Generalmsg>();
    nearprocess::msgToPack = std::priority_queue<Generalmsg>();
    nearprocess::msgToUnPack = std::priority_queue<std::string>();
    nearprocess::msgToSend = std::priority_queue<std::string>();



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

        std::thread processThread(nearprocess::npt);

        //Xbee thread
        std::thread xbeeThread(xbeeLoop);
        //server thread
        std::thread serverThread(netThread);

        serverThread.join();
        processThread.join();
        xbeeThread.join();

        return 1;
    } else{
        return 2;
    }

}

bool nearprocess::npt() {


    return nearprocess::start();


}

int nearprocess::start() {

    while (!restartingpi) {

        std::string umsg = getmsgToUnpack();


        if (!umsg.empty()) {
            addmsgtoProccess(umsg);


        }

        if (!nearprocess::msgtoProccessEmpty()) {

            Generalmsg msg = getmsgToProccess();
            std::string type = msg.getID();
            std::cout<< encrypt(msg)<<std::endl;
            std::ofstream myfile;
            myfile.open("send.txt", std::ios::app);
            myfile << encrypt(msg)<<'\n' ;
            myfile.flush();
            myfile.close();
            if (type == "T3LI") {
                addmsgtoPack(msg);// sent to storGE

            } else if (type.substr(0, 2) == "CMD") {


                if (msg.getID() == "CMDN") {
                    restartingpi = true;
                } else {
                    addmsgtoPack(msg);
                }
            } else if (type == "HIST") {

                addmsgtoPack(msg);
                //history request, send history
            } else if (type == "T2LI") {

                //addmsgtoT2PI(msg);
                T3msg msg1 =T3msg(":t3 request");
                addmsgtoSend(encrypt(msg1));

            } else if (type == "LOGB") {
                addmsgtoPack(msg);
                // log request, send log file.
            } else {
                //std::cout<< "testpoint3"<< std::endl;
                //std::cout<< "recieved" <<std::endl;

                //add a report to log
            }
        }
        //addmsgtoSend(getmsgToPack());

        while (!ismsgFromNetIn()) {

            addmsgtoProccess(encrypt(getmsgToNetIn()));
        }


    }
    return restartingpi;
}


bool nearprocess::msgtoProccessEmpty() {
    mu.lock();
    bool temp = nearprocess::msgToProccess.empty();

    mu.unlock();

    return temp;
}


//function to msgToProccess
void addmsgtoProccess(std::string incoming) {

    Generalmsg msg = decrypt(incoming);
    std::destroy(incoming.begin(), incoming.end());

    mu.lock();
    nearprocess::msgToProccess.push(msg);
    mu.unlock();
}

Generalmsg nearprocess::getmsgToProccess() {
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
void nearprocess::addmsgtoPack(const Generalmsg &outgoing) {
    mu2.lock();
    msgToPack.push(outgoing);
    mu2.unlock();

}

std::string getmsgToPack() {
    mu2.lock();
    std::string pack = encrypt(nearprocess::msgToPack.top());
    nearprocess::msgToPack.pop();
    mu2.unlock();
    return pack;

}

//function to msgToSend
void addmsgtoSend(std::string outgoing) {
    mu3.lock();
    nearprocess::msgToSend.push(outgoing);
    mu3.unlock();
    std::destroy(outgoing.begin(), outgoing.end());
}

std::string getmsgToSend() {
    mu3.lock();
    std::string pack;
    if (!nearprocess::msgToSend.empty()) {
        pack = nearprocess::msgToSend.top();
        nearprocess::msgToSend.pop();
    }
    mu3.unlock();
    return pack;

}

//Functions to msgToUnpack
void addmsgtoUnpack(std::string incoming) {

    mu4.lock();

    nearprocess::msgToUnPack.push(incoming);
    mu4.unlock();
    std::destroy(incoming.begin(), incoming.end());
}

std::string getmsgToUnpack() {
    mu4.lock();
    std::string pack;
    if (!nearprocess::msgToUnPack.empty()) {

        pack = nearprocess::msgToUnPack.top();
        nearprocess::msgToUnPack.pop();
    }
    mu4.unlock();


    return pack;


}



//piCommand
int nearprocess::bashCmd(const std::string &cmd) {
    return system(cmd.c_str());
}


std::string encrypt(const Generalmsg &generalmsg) {
    return std::string(generalmsg.getID() + "[" + generalmsg.getRev() + "]:" + std::to_string(generalmsg.getSize()) +
                       generalmsg.getPayload() + '\0');
}

Generalmsg decrypt(std::string input) {
    Generalmsg msg;
    std::string type = input.substr(0, 4);
    unsigned long headerend = input.find(':');

    try {
        std::string payload = input.substr(headerend + 1);
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


        }
    }
    catch (const std::out_of_range) {
        msg = Generalmsg("ERRO", "REV0", input, 12);
    }

    return msg;
}
