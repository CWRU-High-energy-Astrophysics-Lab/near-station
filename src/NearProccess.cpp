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

mutex mu;

bool init() { //set baud rates and check file system layout
    msgToProccess = priority_queue<Generalmsg>();
    msgToPack = priority_queue<Generalmsg>();
    msgToUnPack = priority_queue<string>();
    msgToSend = priority_queue<string>();
    msgToCentral = priority_queue<Generalmsg>();
    msgFromCentral = priority_queue<Generalmsg>();


    try{
        filesystem::current_path("/home/$(USER)");
        filesystem::current_path("/home/$(USER)/data/log");

        filesystem::current_path("/home/$(USER)/rsato_su_emu");
        filesystem::current_path("/home/$(USER)/rsato_su_emu/bin");
        filesystem::current_path("/home/$(USER)/rsato_su_emu/src");
        system("make");

        filesystem::current_path("/home/$(USER)");
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
        //server thread
        thread serverThread(startServer);


        processThread.join();




//start theads



    }
    return 1;
}

bool npt() {
    NearProccess node = NearProccess();

    return node.start();


}

int NearProccess::start() {
    bool restartingpi = false;
    while (!restartingpi) {
        if (!msgtoProccessEmpty()) {
            Generalmsg msg = getmsgToProccess();
            string type = msg.gedID();
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
                //should not be here, report to log
            } else if (type == "LOGB") {
                addmsgtoPack(msg);
                // log request, send log file.
            } else {
                Generalmsg *msg = msg;
                //add a report to log
            }
        }
        while(ismsgFromNetIn()==true){
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
    msg = msgToProccess.top();
    msgToProccess.pop();
    mu.unlock();
    return msg;
}

// functions to msgToPack
void NearProccess::addmsgtoPack(const Generalmsg& outgoing) {
    mu.lock();
    msgToPack.push(outgoing);
    mu.unlock();

};

string getmsgToPack() {
    mu.lock();
    string pack = encrypt(msgToPack.top());
    msgToPack.pop();
    mu.unlock();
    return pack;

}

//function to msgToSend
void addmsgtoSend(string outgoing) {
    mu.lock();
    msgToSend.push(outgoing);
    mu.unlock();

};

string getmsgToSend() {
    mu.lock();
    string pack = msgToSend.top();
    msgToSend.pop();
    mu.unlock();
    return pack;

}

//Functions to msgToUnpack
void addmsgtoUnpack(string incoming) {
    mu.lock();

    msgToUnPack.push(incoming);
    mu.unlock();

};

string getmsgToUnpack() {
    mu.lock();
    string pack = msgToUnPack.top();
    msgToUnPack.pop();
    mu.unlock();
    return pack;


}


//Functions to msgToUnpack
void addmsgtoCentral(Generalmsg incoming) {
    mu.lock();
    msgToCentral.push(incoming);
    mu.unlock();

};

Generalmsg getmsgFromCentral() {
    mu.lock();
    Generalmsg pack = (msgFromCentral.top());
    msgToCentral.pop();
    mu.unlock();
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
