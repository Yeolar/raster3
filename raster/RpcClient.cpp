/*
 * Copyright 2019 Yeolar
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

#include <gflags/gflags.h>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <folly/init/Init.h>
#include <wangle/bootstrap/ClientBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/channel/EventBaseHandler.h>
#include <wangle/codec/LengthFieldBasedFrameDecoder.h>
#include <wangle/codec/LengthFieldPrepender.h>
#include <wangle/service/ClientDispatcher.h>
#include <wangle/service/ExpiringFilter.h>
#include <wangle/service/Service.h>

#include "ClientSerializeHandler.h"

DEFINE_int32(port, 8080, "server port");
DEFINE_string(host, "127.0.0.1", "server address");

namespace raster {

using SerializePipeline = wangle::Pipeline<folly::IOBufQueue&, Query>;

class RpcPipelineFactory : public wangle::PipelineFactory<SerializePipeline> {
 public:
  SerializePipeline::Ptr newPipeline(
      std::shared_ptr<folly::AsyncTransportWrapper> sock) override {
    auto pipeline = SerializePipeline::create();
    pipeline->addBack(wangle::AsyncSocketHandler(sock));
    // ensure we can write from any thread
    pipeline->addBack(wangle::EventBaseHandler());
    pipeline->addBack(wangle::LengthFieldBasedFrameDecoder());
    pipeline->addBack(wangle::LengthFieldPrepender());
    pipeline->addBack(ClientSerializeHandler());
    pipeline->finalize();

    return pipeline;
  }
};

// Client multiplex dispatcher.  Uses Query.type as request ID
class QueryMultiplexClientDispatcher
    : public wangle::ClientDispatcherBase<SerializePipeline, Query, Result> {
 public:
  void read(Context*, Result in) override {
    auto search = requests_.find(in.traceid());
    CHECK(search != requests_.end());
    auto p = std::move(search->second);
    requests_.erase(in.traceid());
    p.setValue(in);
  }

  folly::Future<Result> operator()(Query arg) override {
    auto& p = requests_[arg.traceid()];
    auto f = p.getFuture();
    p.setInterruptHandler([arg, this](const folly::exception_wrapper&) {
      this->requests_.erase(arg.traceid());
    });
    this->pipeline_->write(arg);

    return f;
  }

  // Print some nice messages for close

  folly::Future<folly::Unit> close() override {
    printf("Channel closed\n");
    return ClientDispatcherBase::close();
  }

  folly::Future<folly::Unit> close(Context* ctx) override {
    printf("Channel closed\n");
    return ClientDispatcherBase::close(ctx);
  }

 private:
  std::unordered_map<std::string, folly::Promise<Result>> requests_;
};

} // namespace raster

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  /**
   * For specific protocols, all the following code would be wrapped
   * in a protocol-specific ServiceFactories.
   *
   * TODO: examples of ServiceFactoryFilters, for connection pooling, etc.
   */
  folly::SocketAddress sa(FLAGS_host, FLAGS_port);
  wangle::ClientBootstrap<raster::SerializePipeline> client;
  client.group(std::make_shared<folly::IOThreadPoolExecutor>(1));
  client.pipelineFactory(std::make_shared<raster::RpcPipelineFactory>());
  auto pipeline = client.connect(sa).get();
  // A serial dispatcher would assert if we tried to send more than one
  // request at a time
  // SerialClientDispatcher<SerializePipeline, Query> service;
  // Or we could use a pipelined dispatcher, but responses would always come
  // back in order
  // PipelinedClientDispatcher<SerializePipeline, Query> service;
  auto dispatcher = std::make_shared<raster::QueryMultiplexClientDispatcher>();
  dispatcher->setPipeline(pipeline);

  // Set an idle timeout of 5s using a filter.
  wangle::ExpiringFilter<raster::Query, raster::Result>
      service(dispatcher, std::chrono::seconds(5));

  try {
    boost::uuids::random_generator ugen;
    raster::Query request;
    request.set_traceid(boost::uuids::to_string(ugen()));
    request.set_query("query string");
    service(request).then([request](raster::Result response) {
      CHECK(request.traceid() == response.traceid());
      std::cout << response.result() << std::endl;
    });
  } catch (const std::exception& e) {
    std::cout << folly::exceptionStr(e) << std::endl;
  }
  sleep(1);

  return 0;
}
