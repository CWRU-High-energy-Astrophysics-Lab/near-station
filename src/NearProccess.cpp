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
        filesystem::current_path("/home/$(USER)/data");
        filesystem::current_path("/home/$(USER)/data/t2");
        filesystem::current_path("/home/$(USER)/data/t3");
        filesystem::current_path("/home/pi/data/log");

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
                T3msg *newmsg = dynamic_cast<T3msg *>(newmsg);

                t3Time(newmsg->getPayload()); // calls cdas to send t3 for time


                //execute t3 collection
                send_t3();

            } else if (type.substr(0, 2) == "CMD") {
                Cmdmsg *msg = dynamic_cast<Cmdmsg *>(msg);
                if (msg->gedID() == "CMDB") {// is os9 command
                    os9cmd(msg->getPayload());
                } else if (msg->gedID() == "CMDR") {
                    restartingpi = true;
                } else {
                    bashCmd(msg->getPayload());
                }
            } else if (type == "HIST") {
                MsgHistory *msg = dynamic_cast<MsgHistory *>(msg);
                //history request, send history
            } else if (type == "T2LI") {
                T2msg *msg = dynamic_cast<T2msg *>(msg);
                //should not be here, report to log
            } else if (type == "LOGA") {
                Logmsg *msg = dynamic_cast<Logmsg *>(msg);
                // log request, send log file.
            } else {
                Generalmsg *msg = msg;
                //add a report to log
            }
        }
        // get t2 file
        bashCmd("cp T2_list.out T2_toSend.out");
        bashCmd("rm T2_list.out");

        string line;
        ifstream t2File("T2_toSend.out");
        string payload;
        if (t2File.is_open()) {
            while (getline(t2File, line)) {
                if (line.starts_with("---")) {
                    T2msg msg(payload);
                    addmsgtoPack(msg);
                     payload.clear();
                } else {
                    payload.append(line);
                }
            }
            t2File.close();
        }


    }
    return restartingpi;
}

bool send_t3(){
    int size =0;
    return true;
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









//comands to cdas
string NearProccess::startCDAS() const {
    string cmd = "./cdas_su_emu " + std::string(EKITPORT) + " " + std::string(EKITPORT) + " 3000";
    bashCmd(cmd);
    return (cmd);
}

string NearProccess::startDataCollection() const {
    string cmd = "./cl s " + string(EKITPORT);
    bashCmd(cmd);
    return (cmd);
}

string NearProccess::rebootStation() const {
    string cmd = "./cl r " + string(EKITPORT);
    bashCmd(cmd);
    return (cmd);
}

string NearProccess::rebootBrodcast() const {
    string cmd = "./cl R " + string(EKITPORT);
    bashCmd(cmd);
    return (cmd);
}

string NearProccess::t3Random() const {
    string cmd = "./cl t " + string(EKITPORT);
    bashCmd(cmd);
    return (cmd);
}

string NearProccess::t3Time(const basic_string<char>& time) const {
    string cmd = "./cl T " + time + " " + string(EKITPORT);
    bashCmd(cmd);
    return (cmd);
}

string NearProccess::os9cmd(const string& input) const {
    string cmd = "./cl c " + input + " " + string(EKITPORT);
    bashCmd(cmd);
    return (cmd);
}

string NearProccess::stopCDAS() const {
    string cmd = "./cl S " + string(EKITPORT);
    bashCmd(cmd);
    return (cmd);
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
