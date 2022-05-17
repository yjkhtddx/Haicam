#include "gtest/gtest.h"
#include "haicam/TCPServer.hpp"
#include "haicam/TCPClient.hpp"

using namespace haicam;
using namespace std::placeholders;

int haicam_TCPTest_callback_times = 0;

void haicam_TCPTest_server_onSentCallback(TCPServer* server, TCPConnectionPtr conn)
{
    haicam_TCPTest_callback_times ++;

    server->shutdown();
}

void haicam_TCPTest_server_onNewConnection(TCPServer* server, TCPConnectionPtr conn)
{
    haicam_TCPTest_callback_times ++;
}

void haicam_TCPTest_server_onDataCallback(TCPServer* server, TCPConnectionPtr conn, ByteBufferPtr data)
{
    std::string str = data->toString();
    ASSERT_EQ(str, "hello");
    conn->sendData(ByteBuffer::create("world"));

    haicam_TCPTest_callback_times ++;
}

void haicam_TCPTest_client_onConnectedCallback(TCPClient* client, TCPConnectionPtr conn)
{
    conn->sendData(ByteBuffer::create("hello"));

    haicam_TCPTest_callback_times ++;
}

void haicam_TCPTest_client_onDataCallback(TCPClient* client, TCPConnectionPtr conn, ByteBufferPtr data)
{
    std::string str = data->toString();
    ASSERT_EQ(str, "world");

    haicam_TCPTest_callback_times ++;

    client->close();

}

TEST(haicam_TCPTest, tcp_test) {
    Context *context = Context::getInstance();

    TCPServerPtr server = TCPServer::create(context, "127.0.0.1", 8888);
    server->onNewConnectionCallback = std::bind(haicam_TCPTest_server_onNewConnection, server.get(), _1);
    server->onDataCallback = std::bind(haicam_TCPTest_server_onDataCallback, server.get(), _1, _2);
    server->onSentCallback = std::bind(haicam_TCPTest_server_onSentCallback, server.get(), _1);
    server->listen();

    TCPClientPtr client = TCPClient::create(context, "127.0.0.1", 8888);
    client->onConnectedCallback = std::bind(haicam_TCPTest_client_onConnectedCallback, client.get(), _1);
    client->onDataCallback = std::bind(haicam_TCPTest_client_onDataCallback, client.get(), _1, _2);
    client->connect();

    context->run();
    delete context;

    ASSERT_EQ(haicam_TCPTest_callback_times, 5);
}
