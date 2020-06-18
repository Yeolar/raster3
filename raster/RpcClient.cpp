/*
 * Copyright 2019 Yeolar
 */

#include <gflags/gflags.h>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <folly/init/Init.h>
#include <wangle/service/Service.h>
#include <wangle/service/ExpiringFilter.h>
#include <wangle/service/ClientDispatcher.h>
#include <wangle/bootstrap/ClientBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/codec/LengthFieldBasedFrameDecoder.h>
#include <wangle/codec/LengthFieldPrepender.h>
#include <wangle/channel/EventBaseHandler.h>

#include "ClientSerializeHandler.h"

using namespace folly;
using namespace wangle;
using namespace raster;

using SerializePipeline = wangle::Pipeline<IOBufQueue&, Query>;

DEFINE_int32(port, 8080, "test server port");
DEFINE_string(host, "127.0.0.1", "test server address");

class RpcPipelineFactory : public PipelineFactory<SerializePipeline> {
 public:
  SerializePipeline::Ptr newPipeline(
      std::shared_ptr<AsyncTransportWrapper> sock) override {
    auto pipeline = SerializePipeline::create();
    pipeline->addBack(AsyncSocketHandler(sock));
    // ensure we can write from any thread
    pipeline->addBack(EventBaseHandler());
    pipeline->addBack(LengthFieldBasedFrameDecoder());
    pipeline->addBack(LengthFieldPrepender());
    pipeline->addBack(ClientSerializeHandler());
    pipeline->finalize();

    return pipeline;
  }
};

// Client multiplex dispatcher.  Uses Query.type as request ID
class QueryMultiplexClientDispatcher
    : public ClientDispatcherBase<SerializePipeline, Query, Result> {
 public:
  void read(Context*, Result in) override {
    auto search = requests_.find(in.traceid());
    CHECK(search != requests_.end());
    auto p = std::move(search->second);
    requests_.erase(in.traceid());
    p.setValue(in);
  }

  Future<Result> operator()(Query arg) override {
    auto& p = requests_[arg.traceid()];
    auto f = p.getFuture();
    p.setInterruptHandler([arg, this](const folly::exception_wrapper&) {
      this->requests_.erase(arg.traceid());
    });
    this->pipeline_->write(arg);

    return f;
  }

  // Print some nice messages for close

  Future<Unit> close() override {
    printf("Channel closed\n");
    return ClientDispatcherBase::close();
  }

  Future<Unit> close(Context* ctx) override {
    printf("Channel closed\n");
    return ClientDispatcherBase::close(ctx);
  }

 private:
  std::unordered_map<std::string, Promise<Result>> requests_;
};

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  /**
   * For specific protocols, all the following code would be wrapped
   * in a protocol-specific ServiceFactories.
   *
   * TODO: examples of ServiceFactoryFilters, for connection pooling, etc.
   */
  ClientBootstrap<SerializePipeline> client;
  client.group(std::make_shared<folly::IOThreadPoolExecutor>(1));
  client.pipelineFactory(std::make_shared<RpcPipelineFactory>());
  auto pipeline = client.connect(SocketAddress(FLAGS_host, FLAGS_port)).get();
  // A serial dispatcher would assert if we tried to send more than one
  // request at a time
  // SerialClientDispatcher<SerializePipeline, Query> service;
  // Or we could use a pipelined dispatcher, but responses would always come
  // back in order
  // PipelinedClientDispatcher<SerializePipeline, Query> service;
  auto dispatcher = std::make_shared<QueryMultiplexClientDispatcher>();
  dispatcher->setPipeline(pipeline);

  // Set an idle timeout of 5s using a filter.
  ExpiringFilter<Query, Result> service(dispatcher, std::chrono::seconds(5));

  try {
    boost::uuids::random_generator ugen;
    Query request;
    request.set_traceid(boost::uuids::to_string(ugen()));
    request.set_query("query string");
    service(request).then([request](Result response) {
      CHECK(request.traceid() == response.traceid());
      std::cout << response.result() << std::endl;
    });
  } catch (const std::exception& e) {
    std::cout << exceptionStr(e) << std::endl;
  }

  return 0;
}
