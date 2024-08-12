/*!
 * \file ipcclient.h
 * \brief Header for the IpcClient class.
 */

#pragma once

#include <QLocalSocket>
#include <QObject>


/*!
 * \brief Manages a QLocalSocket used for communication with other Limo instances.
 */
class IpcClient : public QObject
{
  Q_OBJECT
public:
  /*! \brief Initializes the socket. */
  IpcClient();
  /*! \brief Deletes the socket. */
  ~IpcClient();

  /*!
   * \brief Connects the socket to the IpcServer.
   * \return True if the connection was successful.
   */
  bool connect();
  /*!
   * \brief Sends the given string to the IpcServer.
   * \param data String to send.
   */
  void sendString(const std::string& data);

private:
  /*! \brief The socket used for IPC */
  QLocalSocket* socket_;
};
