/**
 * Shadow Daemon -- High-Interaction Web Honeypot
 *
 *   Copyright (C) 2014 Hendrik Buchwald <hb@zecure.org>
 *
 * This file is part of Shadow Daemon. Shadow Daemon is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONNECTION_H
#define CONNECTION_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "reply.h"
#include "request.h"
#include "request_parser.h"

namespace swd {
	/**
	 * @brief Boost tcp acceptor.
	 */
	typedef boost::asio::ip::tcp::acceptor acceptor;

	/**
	 * @brief Boost tcp socket.
	 */
	typedef boost::asio::ip::tcp::socket socket;

	/**
	 * @brief Boost tcp ssl socket.
	 */
	typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

	/**
	 * @brief Boost ssl context.
	 */
	typedef boost::asio::ssl::context context;

	/**
	 * @brief Represents a connection from a client.
	 */
	class connection :
	 public boost::enable_shared_from_this<swd::connection>,
	 private boost::noncopyable {
		public:
			/**
			 * @brief Construct a connection with the given io_service.
			 *
			 * @param io_service The io_service used to perform asynchronous
			 *  operations.
			 * @param context The (possible empty) ssl context
			 * @param ssl True if ssl is enabled
			 */
			explicit connection(boost::asio::io_service& io_service,
			 swd::context& context, bool ssl);

			/**
			 * @brief Get the socket associated with the connection.
			 *
			 * @return The socket
			 */
			swd::socket& socket();

			/**
			 * @brief Get the ssl socket associated with the connection.
			 *
			 * @return The ssl socket
			 */
			swd::ssl_socket::lowest_layer_type& ssl_socket();

			/**
			 * @brief Start the asynchronous operation for the connection.
			 */
			void start();

		private:
			/**
			 * @brief Start reading data from the socket.
			 */
			void start_read();

			/**
			 * @brief Call start_read, but allow an error code for the ssl handshake.
			 *
			 * @param e The error code of the ssl handshake
			 */
			void start_read(const boost::system::error_code& e);

			/**
			 * @brief Handle completion of a read operation.
			 *
			 * @param e The error code of the read operation
			 * @param bytes_transferred The number of transferred bytes
			 */
			void handle_read(const boost::system::error_code& e,
			 std::size_t bytes_transferred);

			/**
			 * @brief Handle completion of a write operation.
			 *
			 * @param e The error code of the write operation
			 */
			void handle_write(const boost::system::error_code& e);

			/**
			 * @brief Strand to ensure the connection's handlers are not called concurrently.
			 */
			boost::asio::io_service::strand strand_;

			/**
			 * @brief Socket for a connection.
			 */
			swd::socket socket_;

			/**
			 * @brief Socket for a ssl connection.
			 */
			swd::ssl_socket ssl_socket_;

			/**
			 * @brief Buffer for incoming data.
			 */
			boost::array<char, 8192> buffer_;

			/**
			 * @brief IP address of shadowd client/httpd server.
			 */
			boost::asio::ip::address remote_address_;

			/**
			 * @brief The incoming request.
			 */
			swd::request_ptr request_;

			/**
			 * @brief The reply to be sent back to the client.
			 */
			swd::reply_ptr reply_;

			/**
			 * @brief The parser for the incoming request.
			 *
			 * Since the request_parser is working with states we have to create it
			 * here to not lose the current state.
			 */
			swd::request_parser request_parser_;

			/**
			 * @brief The status of ssl usage
			 */
			bool ssl_;
	};

	/**
	 * @brief Connection pointer.
	 */
	typedef boost::shared_ptr<swd::connection> connection_ptr;
}

#endif /* CONNECTION_H */