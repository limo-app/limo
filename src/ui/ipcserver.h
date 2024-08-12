/*!
 * \file ipcserver.h
 * \brief Header for the IpcServer class.
 */

#pragma once

#include <QLocalServer>


/*!
 * \brief Manages a QLocalServer used for communication with other Limo instances.
 */
class IpcServer : public QObject
{
  Q_OBJECT
public:
  /*! \brief Initializes the server. Does NOT start it. */
  IpcServer();
  /*! \brief Stops and deletes the server. */
  ~IpcServer();

  /*! \brief The name of the server. */
  static constexpr char server_name[] = "_Limo_Server_";

  /*!
   * \brief Starts the server.
   * \return True if the server is running.
   */
  bool setup();
  /*! \brief Stops the server. */
  void shutdown();

private:
  /*! \brief The server used for IPC. */
  QLocalServer* server_;

private slots:
  /*! \brief Initializes a connection with a QLocalSocket. */
  void setupConnection();
  /*! \brief Processes data received from a QLocalSocket. */
  void processData();

signals:
  /*!
   * \brief Sends the message received from an IpcClient.
   * \param message The message.
   */
  void receivedMessage(QString message);
};
