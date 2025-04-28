#include "ipcclient.h"
#include "ipcserver.h"


IpcClient::IpcClient() : socket_(new QLocalSocket()) {}

IpcClient::~IpcClient()
{
  delete socket_;
}

bool IpcClient::connect()
{
  socket_->connectToServer(IpcServer::server_name);
  socket_->waitForConnected(50);
  return socket_->state() == QLocalSocket::ConnectedState;
}

void IpcClient::sendString(const std::string& data)
{
  QByteArray bytes;
  for(auto c : data)
    bytes.push_back(c);
  socket_->write(bytes);
  socket_->waitForBytesWritten(1000);
}
