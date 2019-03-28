/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Paul Wichern
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "gtest/gtest.h"

#include "nbbt/Server.h"
#include "nbbt/Client.h"

#include <thread>

struct MyServer : public nbbt::Server
{
    void onConnected(Client* client) override
    {
        (void)client;
    }

    void onDisconnected(Client* client) override
    {
        (void)client;
    }

    void onReadyRead(Client* client) override
    {
        std::string msg;
        if (client->rbuffer.get_string(msg, true)) {
            EXPECT_EQ(msg, std::string("Hello, World!"));
            stop = true;
        }
    }

    bool stop = false;
};

struct MyClient : public nbbt::Client
{
    void onDisconnected() override
    {

    }

    void onReadyRead() override
    {

    }
};

void server_thread(MyServer* server) {
    while (!server->stop && server->run(500));
}

void client_thread() {
    MyClient client;
    while (!client.connect("localhost", 55555));
    EXPECT_EQ(client.wbuffer.send(reinterpret_cast<unsigned char const*>("Hello, World!"), 14), 1);
    return;
}

TEST(Server, SingleClient)
{
    MyServer server;
    EXPECT_TRUE(server.init(55555, AF_INET, 32));
    std::thread tserver = std::thread(&server_thread, &server);
    std::thread tclient = std::thread(&client_thread);
    tserver.join();
    tclient.join();
}
