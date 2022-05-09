//
// Created by robin on 10/21/21.
//
#include "t2li.h"
#include "NearProccess.h"
#include <mutex>
#include <thread>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <utility>
#include "packinterface.h"
#include "networkcom.h"
#include <unistd.h>

mutex mu;
mutex mu2;
mutex mu3;
mutex mu4;
mutex mu5;
bool restartingpi = false;
bool getRestart(){
    return restartingpi;
}
string user;
bool init() { //set baud rates and check file system layout
    msgToProccess = priority_queue<Generalmsg>();
    msgToPack = priority_queue<Generalmsg>();
    msgToUnPack = priority_queue<string>();
    msgToSend = priority_queue<string>();
    msgToCentral = priority_queue<Generalmsg>();
    msgFromCentral = priority_queue<Generalmsg>();

    user= "/home/"+ string(getenv("USER"));
    try{
        filesystem::current_path(user);
        filesystem::current_path(user+"/data");
        filesystem::current_path(user+"/data/log");
        filesystem::current_path(user+"/data/temp");
        filesystem::current_path(user);



    }
    catch (std::filesystem::filesystem_error const& ex) {
        cout<< "File system not as expected, please check\n";
        return false;
    }

    return true;
}

int main() {//this is called on pi boot
    if (init()) {

        thread processThread(npt);

        //Xbee thread
        thread xbeeThread(xbeeLoop);
        //server thread
        thread serverThread(startServer);

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
        std::cout<< msgToUnPack.size();
        auto msg = getmsgToUnpack(); // this is always returning an empty string



        addmsgtoProccess(msg);
        /*
        if (!msgtoProccessEmpty()) {
            Generalmsg msg = getmsgToProccess();
            string type = msg.gedID();
            printf("%s", "msg recieved");
            if (type == "T3LI") {
                addmsgtoPack(msg);// sent to storGE




            } else if (type.substr(0, 2) == "CMD") {


                if (msg.gedID() == "CMDN") {
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
                Generalmsg msg = msg;
                std::cout << encrypt(msg);
                //add a report to log
            }
        }*/
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
void addmsgtoProccess(string incoming) {

    Generalmsg msg = decrypt(std::move(incoming));
    mu.lock();
    msgToProccess.push(msg);
    mu.unlock();
}

Generalmsg NearProccess::getmsgToProccess() {
    Generalmsg msg;
    mu.lock();

    if(!msgToProccess.empty()) {
        msg = msgToProccess.top();
        msgToProccess.pop();
    }

    mu.unlock();
    return msg;
}

// functions to msgToPack
void NearProccess::addmsgtoPack(const Generalmsg& outgoing) {
    mu2.lock();
    msgToPack.push(outgoing);
    mu2.unlock();

};

string getmsgToPack() {
    mu2.lock();
    string pack = encrypt(msgToPack.top());
    msgToPack.pop();
    mu2.unlock();
    return pack;

}

//function to msgToSend
void addmsgtoSend(string outgoing) {
    mu3.lock();
    msgToSend.push(outgoing);
    mu3.unlock();

};

string getmsgToSend() {
    mu3.lock();
    string pack;
    if(!msgToSend.empty()) {
        string pack = msgToSend.top();
        msgToSend.pop();
    }
    mu3.unlock();
    return pack;

}

//Functions to msgToUnpack
void addmsgtoUnpack(string incoming) {
    //std::cout<< incoming;
    mu4.lock();

    msgToUnPack.push(incoming);
    mu4.unlock();

};

string getmsgToUnpack() {
    mu4.lock();
    string pack;
    if(!msgToUnPack.empty()) {

        string pack = msgToUnPack.top();
        msgToUnPack.pop();
    }
    mu4.unlock();
    std::cout<<pack;

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
int NearProccess::bashCmd(const string& cmd) {
    return system(cmd.c_str());
}


string encrypt(const Generalmsg& generalmsg) {
    return string(generalmsg.gedID() + "[" + generalmsg.getRev() + "]:" + to_string(generalmsg.getSize()) +
                  generalmsg.getPayload());
}

Generalmsg decrypt(basic_string<char> input) {
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
