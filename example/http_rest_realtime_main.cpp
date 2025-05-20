
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/config.hpp>
#include <json/json.h>
#include <iostream>
#include <string>
#include <thread>
#include "basetoweb.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

void do_session(tcp::socket socket) {
    try {
        beast::flat_buffer buffer;
        http::request<http::string_body> req;
        http::read(socket, buffer, req);

        // ★ 1. CORS预检，所有OPTIONS直接返回200和CORS头
        if (req.method() == http::verb::options) {
            http::response<http::string_body> res;
            res.result(http::status::ok);
            res.set(http::field::access_control_allow_origin, "*");
            res.set(http::field::access_control_allow_methods, "POST, GET, OPTIONS");
            res.set(http::field::access_control_allow_headers, "Content-Type");
            res.set(http::field::content_type, "text/plain");
            res.body() = "OK";
            res.prepare_payload();
            http::write(socket, res);
            return;
        }

        http::response<http::string_body> res;
        Json::Value request_json;
        Json::CharReaderBuilder builder;
        std::string errs;

        std::istringstream s(req.body());
        if (!Json::parseFromStream(builder, s, &request_json, &errs)) {
            res.result(http::status::bad_request);
            res.body() = "{\"status\":\"error\",\"message\":\"Invalid JSON\"}";
            // ★ 必须加CORS头
            res.set(http::field::access_control_allow_origin, "*");
            res.set(http::field::access_control_allow_methods, "POST, GET, OPTIONS");
            res.set(http::field::access_control_allow_headers, "Content-Type");
            res.version(11);
            res.set(http::field::content_type, "application/json");
            res.prepare_payload();
            http::write(socket, res);
            return;
        } else {
            BaseToWeb db("/userdata/sqlite/lop1.db");
            Json::Value response_json;

            if (req.method() == http::verb::post && req.target() == "/api/data/realtime") {
                response_json = db.handleRealtime(request_json);
                res.result(http::status::ok);
            } else if (req.method() == http::verb::post && req.target() == "/api/data/query") {
                response_json = db.handleQuery(request_json);
                res.result(http::status::ok);
            } else if (req.method() == http::verb::post && req.target() == "/api/data/delete") {
                response_json = db.handleDelete(request_json);
                res.result(http::status::ok);
            } else {
                res.result(http::status::not_found);
                res.body() = "Unknown endpoint";
                // ★ 必须加CORS头
                res.set(http::field::access_control_allow_origin, "*");
                res.set(http::field::access_control_allow_methods, "POST, GET, OPTIONS");
                res.set(http::field::access_control_allow_headers, "Content-Type");
                res.version(11);
                res.set(http::field::content_type, "text/plain");
                res.prepare_payload();
                http::write(socket, res);
                return;
            }

            Json::StreamWriterBuilder writer;
            res.body() = Json::writeString(writer, response_json);
        }

        // ★★ 响应所有正式POST/GET请求也要带CORS头
        res.set(http::field::access_control_allow_origin, "*");
        res.set(http::field::access_control_allow_methods, "POST, GET, OPTIONS");
        res.set(http::field::access_control_allow_headers, "Content-Type");
        res.version(11);
        res.set(http::field::content_type, "application/json");
        res.prepare_payload();
        http::write(socket, res);
    } catch (const std::exception& e) {
        std::cerr << "Session error: " << e.what() << std::endl;
    }
}



int main() {
    try {
        net::io_context ioc;
        tcp::acceptor acceptor(ioc, tcp::endpoint(tcp::v4(), 8080));
        std::cout << "HTTP server started at http://0.0.0.0:8080" << std::endl;

        while (true) {
            tcp::socket socket(ioc);
            acceptor.accept(socket);
            std::thread(&do_session, std::move(socket)).detach();
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
