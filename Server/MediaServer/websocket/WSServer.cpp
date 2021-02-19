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
			LOG_INFO,
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
    	mServer.set_error_channels(log::elevel::none);

        // Initialize Asio
    	mServer.init_asio();
    	mRunning = true;

    	mServer.set_reuse_addr(true);
    	mServer.set_listen_backlog(maxConnection);

        // Register our message handler
    	mServer.set_validate_handler(bind(&WSServer::OnValid, this, ::_1));
    	mServer.set_open_handler(bind(&WSServer::OnOpen, this, ::_1));
        mServer.set_close_handler(bind(&WSServer::OnClose, this, ::_1));
    	mServer.set_message_handler(bind(&WSServer::OnMessage, this, ::_1, ::_2));
//    	mServer.set_tls_init_handler(bind(&WSServer::OnTlsInit, 1, ::_1));

    	lib::asio::error_code ec;
    	lib::asio::ip::tcp::endpoint ep = mServer.get_local_endpoint(ec);

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
					LOG_ALERT,
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
				LOG_INFO,
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
				LOG_ALERT,
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
			LOG_INFO,
			"WSServer::Stop("
			")"
			);

	mServerMutex.lock();

	if ( mRunning ) {
		mRunning = false;
		if( mServer.is_listening() ) {
		    try {
		    	mServer.stop_listening();
		    } catch (websocketpp::exception const & e) {
		    	LogAync(
		    			LOG_INFO,
		    			"WSServer::Stop( "
		    			"[Exception], "
		    			"e : %s "
		    			")",
						e.what()
		    			);
		    }
		}
		/**
		 * Must be call after mServer.init_asio()
		 */
		mServer.stop();
		mIOThread.Stop();
	}

	mServerMutex.unlock();

	LogAync(
			LOG_INFO,
			"WSServer::Stop( "
			"[OK] "
			")"
			);
}

void WSServer::OnForkBefore() {
	mServer.get_io_service().notify_fork(boost::asio::io_service::fork_prepare);
}

void WSServer::OnForkParent() {
	mServer.get_io_service().notify_fork(boost::asio::io_service::fork_parent);
}

void WSServer::OnForkChild() {
	try {
		mServer.get_io_service().notify_fork(boost::asio::io_service::fork_child);
		if( mServer.is_listening() ) {
			mServer.stop_listening();
		}
	} catch (websocketpp::exception const & e) {
	}
}

bool WSServer::SendText(connection_hdl hdl, const string& str) {
	server::connection_ptr conn = mServer.get_con_from_hdl(hdl);
	LogAync(
			LOG_DEBUG,
			"WSServer::SendText( "
			"hdl : %p, "
			"addr : %s, "
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
    			LOG_INFO,
    			"WSServer::SendText( "
				"hdl : %p, "
    			"[Exception], "
				"addr : %s, "
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
			LOG_INFO,
			"WSServer::Disconnect( "
			"hdl : %p, "
			"addr : %s "
			")",
			hdl.lock().get(),
			conn->get_remote_endpoint().c_str()
			);

	try {
		mServer.close(hdl, websocketpp::close::status::normal, "Disconnect by server");
	} catch (websocketpp::exception const & e) {
	    	LogAync(
	    			LOG_INFO,
	    			"WSServer::Disconnect( "
					"hdl : %p, "
	    			"[Exception], "
					"addr : %s, "
	    			"e : %s "
	    			")",
					hdl.lock().get(),
					conn->get_remote_endpoint().c_str(),
					e.what()
	    			);
	}

	LogAync(
			LOG_DEBUG,
			"WSServer::Disconnect( "
			"[Finish], "
			"hdl : %p, "
			"addr : %s "
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
//        			LOG_ALERT,
//        			"WSServer::OnTlsInit( "
//        			"[Error setting cipher list], "
//        			"hdl : %p, "
//        			"addr : %s "
//        			")",
//        			hdl.lock().get(),
//        			conn->get_remote_endpoint().c_str()
//        			);
//        }
//    } catch (std::exception& e) {
//    	LogAync(
//    			LOG_INFO,
//    			"WSServer::OnTlsInit( "
//				"hdl : %p, "
//    			"[Exception], "
//				"addr : %s, "
//    			"e : %s "
//    			")",
//				hdl.lock().get(),
//				conn->get_remote_endpoint().c_str(),
//				e.what()
//    			);
//    }
//    return ctx;
//}

bool WSServer::OnValid(connection_hdl hdl) {
	server::connection_ptr conn = mServer.get_con_from_hdl(hdl);
	LogAync(
			LOG_DEBUG,
			"WSServer::OnValid( "
			"hdl : %p, "
			"addr : %s "
			")",
			hdl.lock().get(),
			conn->get_remote_endpoint().c_str()
			);

	return true;
}

void WSServer::OnOpen(connection_hdl hdl) {
	server::connection_ptr conn = mServer.get_con_from_hdl(hdl);
	string userAgent = conn->get_request_header("User-Agent");
	LogAync(
			LOG_DEBUG,
			"WSServer::OnOpen( "
			"hdl : %p, "
			"addr : %s, "
			"userAgent : %s "
			")",
			hdl.lock().get(),
			conn->get_remote_endpoint().c_str(),
			userAgent.c_str()
			);

	if ( mpWSServerCallback ) {
		mpWSServerCallback->OnWSOpen(this, hdl, conn->get_remote_endpoint(), userAgent);
	}
}

void WSServer::OnClose(connection_hdl hdl) {
	LogAync(
			LOG_DEBUG,
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
			LOG_DEBUG,
			"WSServer::OnMessage( "
			"hdl : %p, "
			"addr : %s, "
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
			LOG_INFO,
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
			LOG_INFO,
			"WSServer::IOHandleThread( [Exit] )"
			);
}

} /* namespace mediaserver */
