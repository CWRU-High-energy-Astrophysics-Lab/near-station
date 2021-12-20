//
// blocking_tcp_echo_client.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2015 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

enum { max_length = 1024 };

int send(std::string hostname, std::string port, std::string msg) {
    try {


        boost::asio::io_service io_service;

        tcp::resolver resolver(io_service);
        tcp::resolver::query query(tcp::v4(), hostname, port);
        tcp::resolver::iterator iterator = resolver.resolve(query);

        tcp::socket s(io_service);
        boost::asio::connect(s, iterator);

        using namespace std; // For strlen.




        size_t request_length = strlen(msg.c_str());
        boost::asio::write(s, boost::asio::buffer(msg.c_str(), request_length));

        char reply[max_length];
        size_t reply_length = boost::asio::read(s,
                                                boost::asio::buffer(reply, request_length));
        std::cout << "Reply is: ";
        std::cout.write(reply, reply_length);
        std::cout << "\n";


    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}