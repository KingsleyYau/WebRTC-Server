/*
 * WSServer.cpp
 *
 *  Created on: 2019/07/23
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 */

#include "WSServer.h"

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

namespace mediaserver {
class WSIORunnable : public KRunnable {
public:
	WSIORunnable(WSServer *container) {
		mContainer = container;
	}
	virtual ~WSIORunnable() {
		mContainer = NULL;
	}
protected:
	void onRun() {
		mContainer->IOHandleThread();
	}
private:
	WSServer *mContainer;
};

//void on_open(connection_hdl hdl)  {
//    websocket_server::connection_ptr conn = m_server.get_con_from_hdl(hdl);
//    std::cout << conn->get_uri()->str()  << " : " << conn->get_uri()->get_resource() << " - "<< conn->get_remote_endpoint() << std::endl;
//
//    nice_agent_ptr agent = std::make_shared<nice_agent>(&m_server, hdl, true);
//    int nstreamid = agent->add_stream("audio", 1);
//    agent->start_gather(nstreamid);
//    connection_data data;
//    data.agent = agent;
//    data.msgid = 0;
//    m_connections[hdl] = data;
//}
//
//void on_close(connection_hdl hdl) {
//
//}
//
//// Define a callback to handle incoming messages
//void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg) {
//	LogAync(
//			LOG_WARNING,
//			"WSServer::on_message( "
//			"hdl : 0x%x, "
//			"opcode : 0x%x, "
//			"payload : %s "
//			")",
//			hdl.lock().get(),
//			msg->get_opcode(),
//			msg->get_payload().c_str()
//			);
//
//    if (msg->get_payload() == "stop-listening") {
//        s->stop_listening();
//        return;
//    }
//
//    try {
//        s->send(hdl, msg->get_payload(), msg->get_opcode());
//    } catch (websocketpp::exception const & e) {
//    	LogAync(
//    			LOG_WARNING,
//    			"WSServer::on_message( "
//    			"[Exception], "
//    			"e : %s "
//    			")",
//				e.what()
//    			);
//    }
//}

WSServer::WSServer() {
	// TODO Auto-generated constructor stub

	mpIORunnable = new WSIORunnable(this);
	mRunning = false;
	miMaxConnection = 0;

	mpWSServerCallback = NULL;
}

void WSServer::SetCallback(WSServerCallback *callback) {
	mpWSServerCallback = callback;
}

WSServer::~WSServer() {
	// TODO Auto-generated destructor stub
	if( mpIORunnable ) {
		delete mpIORunnable;
		mpIORunnable = NULL;
	}
}

bool WSServer::Start(int port, int maxConnection) {
	bool bFlag = false;

	LogAync(
			LOG_MSG,
			"WSServer::Start( "
			"port : %u, "
			"maxConnection : %d "
			")",
			port,
			maxConnection
			);

	mServerMutex.lock();
	if( mRunning ) {
		Stop();
	}
	mRunning = true;
	miMaxConnection = maxConnection;

    try {
        // Set logging settings
    	mServer.set_access_channels(log::alevel::none);
//    	mServer.clear_access_channels(log::alevel::frame_payload);
    	mServer.set_error_channels(log::alevel::none);

        // Initialize Asio
    	mServer.init_asio();
    	mServer.set_reuse_addr(true);

        // Register our message handler
    	mServer.set_open_handler(bind(&WSServer::OnOpen, this, ::_1));
        mServer.set_close_handler(bind(&WSServer::OnClose, this, ::_1));
    	mServer.set_message_handler(bind(&WSServer::OnMessage, this, ::_1, ::_2));

        // Listen on port
    	mServer.listen(port);

        // Start the server accept loop
    	mServer.start_accept();

    	bFlag = true;

    } catch (websocketpp::exception const & e) {
    	LogAync(
    			LOG_WARNING,
    			"WSServer::Start( "
    			"[Exception], "
    			"e : %s "
    			")",
				e.what()
    			);
    } catch (...) {
    	LogAync(
    			LOG_WARNING,
    			"WSServer::Start( "
    			"[Unknow Exception] "
    			")"
    			);
    }

	if( bFlag ) {
		// 启动IO监听线程
		if( 0 == mIOThread.Start(mpIORunnable) ) {
			LogAync(
					LOG_ERR_SYS,
					"WSServer::Start( "
					"[Create IO Thread Fail], "
					"port : %u, "
					"maxConnection : %d, "
					")",
					port,
					maxConnection
					);
			bFlag = false;
		}
	}

	if( bFlag ) {
		LogAync(
				LOG_MSG,
				"WSServer::Start( "
				"[OK], "
				"port : %d, "
				"maxConnection : %d "
				")",
				port,
				maxConnection
				);
	} else {
		LogAync(
				LOG_ERR_SYS,
				"WSServer::Start( "
				"[Fail], "
				"port : %d, "
				"maxConnection : %d "
				")",
				port,
				maxConnection
				);
		Stop();
	}

	mServerMutex.unlock();

	return bFlag;
}

void WSServer::Stop() {
	mServerMutex.lock();

	mRunning = false;
	mServer.stop_listening();
	mServer.stop();

	mServerMutex.unlock();
}

bool WSServer::SendText(connection_hdl hdl, const string& str) {
	server::connection_ptr conn = mServer.get_con_from_hdl(hdl);
	LogAync(
			LOG_MSG,
			"WSServer::Send( "
			"hdl : %p, "
			"ip : %s, "
			"str : %s "
			")",
			hdl.lock().get(),
			conn->get_remote_endpoint().c_str(),
			str.c_str()
			);
	bool bFlag = false;

    try {
        mServer.send(hdl, str, frame::opcode::value::text);
        bFlag = true;
    } catch (websocketpp::exception const & e) {
    	LogAync(
    			LOG_WARNING,
    			"WSServer::Send( "
				"hdl : %p, "
    			"[Exception], "
				"ip : %s, "
    			"e : %s, "
				"str : %s "
    			")",
				hdl.lock().get(),
				conn->get_remote_endpoint().c_str(),
				e.what(),
				str.c_str()
    			);
    }

	return bFlag;
}

void WSServer::Disconnect(connection_hdl hdl) {
	server::connection_ptr conn = mServer.get_con_from_hdl(hdl);
	LogAync(
			LOG_MSG,
			"WSServer::Disconnect( "
			"hdl : %p, "
			"ip : %s "
			")",
			hdl.lock().get(),
			conn->get_remote_endpoint().c_str()
			);

	mServer.close(hdl, 0, "Disconnect");

	LogAync(
			LOG_MSG,
			"WSServer::Disconnect( "
			"[Finish], "
			"hdl : %p, "
			"ip : %s "
			")",
			hdl.lock().get(),
			conn->get_remote_endpoint().c_str()
			);
}

void WSServer::OnOpen(connection_hdl hdl) {
	server::connection_ptr conn = mServer.get_con_from_hdl(hdl);

	LogAync(
			LOG_MSG,
			"WSServer::OnOpen( "
			"hdl : %p, "
			"ip : %s "
			")",
			hdl.lock().get(),
			conn->get_remote_endpoint().c_str()
			);

	if ( mpWSServerCallback ) {
		mpWSServerCallback->OnWSOpen(this, hdl);
	}
}

void WSServer::OnClose(connection_hdl hdl) {
	server::connection_ptr conn = mServer.get_con_from_hdl(hdl);

	LogAync(
			LOG_MSG,
			"WSServer::OnClose( "
			"hdl : %p "
			")",
			hdl.lock().get()
			);

	if ( mpWSServerCallback ) {
		mpWSServerCallback->OnWSClose(this, hdl);
	}
}

void WSServer::OnMessage(websocketpp::connection_hdl hdl, message_ptr msg) {
	server::connection_ptr conn = mServer.get_con_from_hdl(hdl);

	LogAync(
			LOG_MSG,
			"WSServer::OnMessage( "
			"hdl : %p, "
			"ip : %s, "
			"opcode : 0x%x, "
			"payload : %s "
			")",
			hdl.lock().get(),
			conn->get_remote_endpoint().c_str(),
			msg->get_opcode(),
			msg->get_payload().c_str()
			);

	if ( mpWSServerCallback ) {
		mpWSServerCallback->OnWSMessage(this, hdl, msg->get_payload());
	}
}

void WSServer::IOHandleThread() {
	LogAync(
			LOG_MSG,
			"WSServer::IOHandleThread( [Start] )"
			);

    try {
        // Start the ASIO io_service run loop
    	mServer.run();
    } catch (websocketpp::exception const & e) {
    	LogAync(
    			LOG_WARNING,
    			"WSServer::IOHandleThread( "
    			"[Exception], "
    			"e : %s "
    			")",
				e.what()
    			);
    } catch (...) {
    	LogAync(
    			LOG_WARNING,
    			"WSServer::IOHandleThread( "
    			"[Unknow Exception] "
    			")"
    			);
    }

	LogAync(
			LOG_MSG,
			"WSServer::IOHandleThread( [Exit] )"
			);
}

} /* namespace mediaserver */
