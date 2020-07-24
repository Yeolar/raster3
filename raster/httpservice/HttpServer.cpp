/*
 * Copyright 2020 Yeolar
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/Memory.h>
#include <folly/portability/GFlags.h>
#include <folly/portability/Unistd.h>
#include <proxygen/httpserver/HTTPServer.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>

#include <accelerator/FileUtil.h>
#include <accelerator/Logging.h>
#include <crystal/table/TableFactory.h>

#include "HttpHandler.h"
#include "HttpStats.h"

using namespace raster;

using folly::SocketAddress;
using Protocol = proxygen::HTTPServer::Protocol;

DEFINE_int32(http_port, 11000, "Port to listen on with HTTP protocol");
DEFINE_int32(spdy_port, 11001, "Port to listen on with SPDY protocol");
DEFINE_int32(h2_port, 11002, "Port to listen on with HTTP/2 protocol");
DEFINE_string(ip, "localhost", "IP/Hostname to bind to");
DEFINE_string(conf, "conf/raster.cson", "server conf");

class HttpHandlerFactory : public proxygen::RequestHandlerFactory {
 public:
  HttpHandlerFactory(crystal::TableFactory* factory)
      : factory_(factory) {}

  void onServerStart(folly::EventBase* /*evb*/) noexcept override {
    stats_.reset(new HttpStats);
  }

  void onServerStop() noexcept override {
    stats_.reset();
  }

  proxygen::RequestHandler* onRequest(proxygen::RequestHandler*,
                                      proxygen::HTTPMessage*)
      noexcept override {
    return new HttpHandler(factory_, stats_.get());
  }

 private:
  crystal::TableFactory* factory_;
  folly::ThreadLocalPtr<HttpStats> stats_;
};

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  std::string confStr;
  if (!acc::readFile(FLAGS_conf.c_str(), confStr)) {
    ACCLOG(ERROR) << "read conf '" << FLAGS_conf << "' failed";
    return -1;
  }
  acc::dynamic conf;
  try {
    conf = acc::parseCson(confStr);
  } catch (std::exception& e) {
    ACCLOG(ERROR) << "parse conf '" << FLAGS_conf << "' failed";
    return -1;
  }
  auto threads = conf.getDefault("threads", 0).asInt();
  auto crystal_conf = conf.getDefault("crystal_conf");
  auto crystal_data = conf.getDefault("crystal_data");

  crystal::TableFactory factory;
  if (!factory.load(crystal_conf.c_str(), crystal_data.c_str(), true)) {
    ACCLOG(ERROR) << "load data from '" << crystal_data
        << "' with conf '" << crystal_conf << "' failed";
    return -1;
  }

  std::vector<proxygen::HTTPServer::IPConfig> IPs = {
    {SocketAddress(FLAGS_ip, FLAGS_http_port, true), Protocol::HTTP},
    {SocketAddress(FLAGS_ip, FLAGS_spdy_port, true), Protocol::SPDY},
    {SocketAddress(FLAGS_ip, FLAGS_h2_port, true), Protocol::HTTP2},
  };

  if (threads <= 0) {
    threads = sysconf(_SC_NPROCESSORS_ONLN);
    CHECK(threads > 0);
  }

  proxygen::HTTPServerOptions options;
  options.threads = static_cast<size_t>(threads);
  options.idleTimeout = std::chrono::milliseconds(60000);
  options.shutdownOn = {SIGINT, SIGTERM};
  options.enableContentCompression = false;
  options.handlerFactories = proxygen::RequestHandlerChain()
      .addThen<HttpHandlerFactory>(&factory)
      .build();
  options.h2cEnabled = true;

  proxygen::HTTPServer server(std::move(options));
  server.bind(IPs);

  // Start HTTPServer mainloop in a separate thread
  std::thread t([&] () {
    server.start();
  });

  t.join();
  return 0;
}
