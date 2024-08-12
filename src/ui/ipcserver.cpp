#include "ipcserver.h"
#include <QDebug>
#include <QLocalSocket>


IpcServer::IpcServer() : server_(new QLocalServer())
{
  server_->setSocketOptions(QLocalServer::WorldAccessOption);
}

IpcServer::~IpcServer()
{
  server_->close();
  delete server_;
}

bool IpcServer::setup()
{
  bool started = server_->listen(server_name);
  connect(server_, &QLocalServer::newConnection, this, &IpcServer::setupConnection);

  return started;
}

void IpcServer::shutdown()
{
  server_->close();
}

void IpcServer::setupConnection()
{
  auto socket = server_->nextPendingConnection();
  connect(socket, &QLocalSocket::readyRead, this, &IpcServer::processData);
}

void IpcServer::processData()
{
  auto socket = static_cast<QLocalSocket*>(sender());
  auto bytes = socket->readAll();
  std::string data;
  for(const auto& b : std::as_const(bytes))
    data.append({ b });
  emit receivedMessage(data.c_str());
}
