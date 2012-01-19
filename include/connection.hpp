//-------------------------------------------------------------------------------------------------
#if 0

FIX8 is released under the New BSD License.

Copyright (c) 2010-12, David L. Dight <fix@fix8.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of
	 	conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list
	 	of conditions and the following disclaimer in the documentation and/or other
		materials provided with the distribution.
    * Neither the name of the author nor the names of its contributors may be used to
	 	endorse or promote products derived from this software without specific prior
		written permission.
    * Products derived from this software may not be called "Fix8", nor can "Fix8" appear
	   in their name without written permission from fix8.org

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#endif
//-------------------------------------------------------------------------------------------------
#ifndef _FIX8_CONNECTION_HPP_
#define _FIX8_CONNECTION_HPP_

#include <Poco/Net/StreamSocket.h>
#include <Poco/Timespan.h>
#include <Poco/Net/NetException.h>
#include <tbb/concurrent_queue.h>

//----------------------------------------------------------------------------------------
namespace FIX8 {

class Session;

//----------------------------------------------------------------------------------------
/// Half duplex asynch socket wrapper with thread
/*! \tparam T the object type to queue */
template <typename T>
class AsyncSocket
{
	Thread<AsyncSocket> _thread;

protected:
	Poco::Net::StreamSocket *_sock;
	tbb::concurrent_bounded_queue<T> _msg_queue;
	Session& _session;

public:
	/*! Ctor.
	    \param sock connected socket
	    \param session session */
	AsyncSocket(Poco::Net::StreamSocket *sock, Session& session)
		: _thread(ref(*this)), _sock(sock), _session(session) {}

	/// Dtor.
	virtual ~AsyncSocket() {}

	/*! Get the number of messages queued on this socket.
	    \return number of queued messages */
	size_t queued() const { return _msg_queue.size(); }

	/*! Pure virtual Function operator. Called by thread to process message on queue.
	    \return 0 on success */
	virtual int operator()() = 0;

	/// Start the processing thread.
	virtual void start() { _thread.Start(); }

	/// Stop the processing thread and quit.
	virtual void quit() { _thread.Kill(1); }

	/*! Get the underlying socket object.
	    \return the socket */
	Poco::Net::StreamSocket *socket() { return _sock; }

	/*! Wait till processing thead has finished.
	    \return 0 on success */
	int join() { return _thread.Join(); }
};

//----------------------------------------------------------------------------------------
/// Fix message reader
class FIXReader : public AsyncSocket<f8String>
{
	static RegExp _hdr;
	static const size_t _max_msg_len = 1024, _chksum_sz = 7;

	Thread<FIXReader> _callback_thread;

	/*! Process messages from inbound queue, calls session process method.
	    \return number of messages processed */
	int callback_processor();

	size_t _bg_sz; // 8=FIXx.x^A9=x

	/*! Read a Fix message. Throws InvalidBodyLength, IllegalMessage.
	    \param to string to place message in
	    \return true on success */
	bool read(f8String& to);

	/*! Read bytes from the socket layer, throws PeerResetConnection.
	    \param where buffer to place bytes in
	    \param sz number of bytes to read
	    \return number of bytes read */
	int sockRead(char *where, size_t sz);

protected:
	/*! Reader thread method. Reads messages and places them on the queue for processing.
	    \return 0 on success */
	int operator()();

public:
	/*! Ctor.
	    \param sock connected socket
	    \param session session */
	FIXReader(Poco::Net::StreamSocket *sock, Session& session)
		: AsyncSocket<f8String>(sock, session), _callback_thread(ref(*this), &FIXReader::callback_processor), _bg_sz()
	{
		set_preamble_sz();
	}

	/// Dtor.
	virtual ~FIXReader() {}

	/// Start the processing threads.
	virtual void start()
	{
		AsyncSocket<f8String>::start();
		_callback_thread.Start();
	}

	/// Stop the processing threads and quit.
	virtual void quit() { _callback_thread.Kill(1); AsyncSocket<f8String>::quit(); }

	/// Send a message to the processing method instructing it to quit.
	virtual void stop() { const f8String from; _msg_queue.try_push(from); }

	/// Calculate the length of the Fix message preamble, e.g. "8=FIX.4.4^A9=".
	void set_preamble_sz();
};

//----------------------------------------------------------------------------------------
/// Fix message writer
class FIXWriter : public AsyncSocket<Message *>
{
protected:
	/*! Writer thread method. Reads messages from the queue and sends them over the socket.
	    \return 0 on success */
	int operator()();

public:
	/*! Ctor.
	    \param sock connected socket
	    \param session session */
	FIXWriter(Poco::Net::StreamSocket *sock, Session& session) : AsyncSocket<Message *>(sock, session) {}

	/// Dtor.
	virtual ~FIXWriter() {}

	/*! Place Fix message on outbound message queue.
	    \param from message to send
	    \return true in success */
	bool write(Message *from) { return _msg_queue.try_push(from); }

	/*! Send message over socket.
	    \param msg message string to send
	    \return number of bytes sent */
	int send(const f8String& msg) { return _sock->sendBytes(msg.data(), msg.size()); }

	/// Send a message to the processing method instructing it to quit.
	virtual void stop() { _msg_queue.try_push(0); }
};

//----------------------------------------------------------------------------------------
/// Complete Fix connection (reader and writer).
class Connection
{
public:
	/// Roles: acceptor, initiator or unknown.
	enum Role { cn_acceptor, cn_initiator, cn_unknown };

protected:
	Poco::Net::StreamSocket *_sock;
	bool _connected;
	Session& _session;
	Role _role;
	unsigned _hb_interval, _hb_interval20pc;

	FIXReader _reader;
	FIXWriter _writer;

public:
	/*! Ctor. Initiator.
	    \param sock connected socket
	    \param session session */
	Connection(Poco::Net::StreamSocket *sock, Session &session)	// client
		: _sock(sock), _connected(), _session(session), _role(cn_initiator),
		_hb_interval(10), _reader(sock, session), _writer(sock, session) {}

	/*! Ctor. Acceptor.
	    \param sock connected socket
	    \param session session
	    \param hb_interval heartbeat interval */
	Connection(Poco::Net::StreamSocket *sock, Session &session, const unsigned hb_interval) // server
		: _sock(sock), _connected(true), _session(session), _role(cn_acceptor), _hb_interval(hb_interval),
		_hb_interval20pc(hb_interval + hb_interval / 5),
		  _reader(sock, session), _writer(sock, session) {}

	/// Dtor.
	virtual ~Connection() {}

	/*! Get the role for this connection.
	    \return the role */
	const Role get_role() const { return _role; }

	/// Start the reader and writer threads.
	void start();

	/// Stop the reader and writer threads.
	void stop();

	/*! Get the connection state.
	    \return true if connected */
	virtual bool connect() { return _connected; }

	/*! Write a message to the underlying socket.
	    \param from Message to write
	    \return true on success */
	virtual bool write(Message *from) { return _writer.write(from); }

	/*! Write a string message to the underlying socket.
	    \param from Message (string) to write
	    \return number of bytes written */
	int send(const f8String& from) { return _writer.send(from); }

	/*! Set the heartbeat interval for this connection.
	    \param hb_interval heartbeat interval */
	void set_hb_interval(const unsigned hb_interval)
		{ _hb_interval = hb_interval; _hb_interval20pc = hb_interval + hb_interval / 5; }

	/*! Get the heartbeat interval for this connection.
	    \return the heartbeat interval */
	unsigned get_hb_interval() const { return _hb_interval; }

	/*! Get the heartbeat interval + %20 for this connection.
	    \return the heartbeat interval + %20 */
	unsigned get_hb_interval20pc() const { return _hb_interval20pc; }

	/*! Wait till reader thead has finished.
	    \return 0 on success */
	int join() { return _reader.join(); }

	/*! Get the session associated with this connection.
	    \return the session */
	Session& get_session() { return _session; }
};

//-------------------------------------------------------------------------------------------------
/// Client (initiator) specialisation of Connection.
class ClientConnection : public Connection
{
	Poco::Net::SocketAddress _addr;

public:
	/*! Ctor. Initiator.
	    \param sock connected socket
	    \param addr sock address structure
	    \param session session */
	ClientConnection(Poco::Net::StreamSocket *sock, Poco::Net::SocketAddress& addr, Session &session)
		: Connection(sock, session), _addr(addr) {}

	/// Dtor.
	virtual ~ClientConnection() {}

	/*! Establish connection.
	    \return true on success */
	bool connect();
};

//-------------------------------------------------------------------------------------------------
/// Server (acceptor) specialisation of Connection.
class ServerConnection : public Connection
{

public:
	/*! Ctor. Initiator.
	    \param sock connected socket
	    \param session session
	    \param hb_interval heartbeat interval */
	ServerConnection(Poco::Net::StreamSocket *sock, Session &session, const unsigned hb_interval) :
		Connection(sock, session, hb_interval) {}

	/// Dtor.
	virtual ~ServerConnection() {}
};

//-------------------------------------------------------------------------------------------------

} // FIX8

#endif // _FIX8_CONNECTION_HPP_
