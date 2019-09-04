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

WSServer::WSServer()
:mServerMutex(KMutex::MutexType_Recursive) {
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
	miMaxConnection = maxConnection;

    try {
        // Set logging settings
    	mServer.set_access_channels(log::alevel::none);
//    	mServer.clear_access_channels(log::alevel::frame_payload);
    	mServer.set_error_channels(log::alevel::none);

        // Initialize Asio
    	mServer.init_asio();
    	mRunning = true;

    	mServer.set_reuse_addr(true);
    	mServer.set_listen_backlog(maxConnection);

        // Register our message handler
    	mServer.set_open_handler(bind(&WSServer::OnOpen, this, ::_1));
        mServer.set_close_handler(bind(&WSServer::OnClose, this, ::_1));
    	mServer.set_message_handler(bind(&WSServer::OnMessage, this, ::_1, ::_2));
//    	mServer.set_tls_init_handler(bind(&WSServer::OnTlsInit, 1, ::_1));

        // Listen on port
    	mServer.listen(lib::asio::ip::tcp::v4(), port);

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
		if( 0 == mIOThread.Start(mpIORunnable, "WSServer") ) {
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
	LogAync(
			LOG_MSG,
			"WSServer::Stop("
			")"
			);

	mServerMutex.lock();

	if ( mRunning ) {
		mRunning = false;
		if( mServer.is_listening() ) {
			mServer.stop_listening();
		}
		/**
		 * Must be call after mServer.init_asio()
		 */
		mServer.stop();
		mIOThread.Stop();
	}

	mServerMutex.unlock();

	LogAync(
			LOG_MSG,
			"WSServer::Stop( "
			"[OK] "
			")"
			);
}

bool WSServer::SendText(connection_hdl hdl, const string& str) {
	server::connection_ptr conn = mServer.get_con_from_hdl(hdl);
	LogAync(
			LOG_STAT,
			"WSServer::SendText( "
			"hdl : %p, "
			"ip : %s, "
			"str(%u) : %s "
			")",
			hdl.lock().get(),
			conn->get_remote_endpoint().c_str(),
			str.length(),
			str.c_str()
			);
	bool bFlag = false;

    try {
        mServer.send(hdl, str, frame::opcode::value::text);
        bFlag = true;
    } catch (websocketpp::exception const & e) {
    	LogAync(
    			LOG_MSG,
    			"WSServer::SendText( "
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
			LOG_STAT,
			"WSServer::Disconnect( "
			"hdl : %p, "
			"ip : %s "
			")",
			hdl.lock().get(),
			conn->get_remote_endpoint().c_str()
			);

	mServer.close(hdl, 0, "Disconnect");

	LogAync(
			LOG_STAT,
			"WSServer::Disconnect( "
			"[Finish], "
			"hdl : %p, "
			"ip : %s "
			")",
			hdl.lock().get(),
			conn->get_remote_endpoint().c_str()
			);
}

//context_ptr WSServer::OnTlsInit(int mode, connection_hdl hdl) {
//	server::connection_ptr conn = mServer.get_con_from_hdl(hdl);
//	namespace asio = lib::asio;
//
//    try {
//        if (mode == 2) {
//            // Modern disables TLSv1
//            ctx->set_options(asio::ssl::context::default_workarounds |
//                             asio::ssl::context::no_sslv2 |
//                             asio::ssl::context::no_sslv3 |
//                             asio::ssl::context::no_tlsv1 |
//                             asio::ssl::context::single_dh_use);
//        } else {
//            ctx->set_options(asio::ssl::context::default_workarounds |
//                             asio::ssl::context::no_sslv2 |
//                             asio::ssl::context::no_sslv3 |
//                             asio::ssl::context::single_dh_use);
//        }
//        ctx->use_certificate_chain_file("./ssl/server.pem");
//        ctx->use_private_key_file("./ssl/server.pem", asio::ssl::context::pem);
//
//        std::string ciphers;
//
//        if (mode == 2) {
//            ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";
//        } else {
//            ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:AES:CAMELLIA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
//        }
//
//        if (SSL_CTX_set_cipher_list(ctx->native_handle() , ciphers.c_str()) != 1) {
//        	LogAync(
//        			LOG_ERR_SYS,
//        			"WSServer::OnTlsInit( "
//        			"[Error setting cipher list], "
//        			"hdl : %p, "
//        			"ip : %s "
//        			")",
//        			hdl.lock().get(),
//        			conn->get_remote_endpoint().c_str()
//        			);
//        }
//    } catch (std::exception& e) {
//    	LogAync(
//    			LOG_MSG,
//    			"WSServer::OnTlsInit( "
//				"hdl : %p, "
//    			"[Exception], "
//				"ip : %s, "
//    			"e : %s "
//    			")",
//				hdl.lock().get(),
//				conn->get_remote_endpoint().c_str(),
//				e.what()
//    			);
//    }
//    return ctx;
//}

void WSServer::OnOpen(connection_hdl hdl) {
	server::connection_ptr conn = mServer.get_con_from_hdl(hdl);

	LogAync(
			LOG_STAT,
			"WSServer::OnOpen( "
			"hdl : %p, "
			"addr : %s "
			")",
			hdl.lock().get(),
			conn->get_remote_endpoint().c_str()
			);

	if ( mpWSServerCallback ) {
		mpWSServerCallback->OnWSOpen(this, hdl, conn->get_remote_endpoint());
	}
}

void WSServer::OnClose(connection_hdl hdl) {
	server::connection_ptr conn = mServer.get_con_from_hdl(hdl);

	LogAync(
			LOG_STAT,
			"WSServer::OnClose( "
			"hdl : %p "
			")",
			hdl.lock().get()
			);

	if ( mpWSServerCallback ) {
		mpWSServerCallback->OnWSClose(this, hdl, conn->get_remote_endpoint());
	}
}

void WSServer::OnMessage(websocketpp::connection_hdl hdl, message_ptr msg) {
	server::connection_ptr conn = mServer.get_con_from_hdl(hdl);

	LogAync(
			LOG_STAT,
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
