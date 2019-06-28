#include <gflags/gflags.h>

#include <folly/init/Init.h>
#include <wangle/service/Service.h>
#include <wangle/service/ExecutorFilter.h>
#include <wangle/service/ServerDispatcher.h>
#include <wangle/bootstrap/ServerBootstrap.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/codec/LengthFieldBasedFrameDecoder.h>
#include <wangle/codec/LengthFieldPrepender.h>
#include <wangle/channel/EventBaseHandler.h>
#include <folly/executors/CPUThreadPoolExecutor.h>

#include "ServerSerializeHandler.h"

using namespace folly;
using namespace wangle;

using SerializePipeline = wangle::Pipeline<IOBufQueue&, Result>;

DEFINE_int32(port, 8080, "test server port");

class RpcService : public Service<Query, Result> {
 public:
  Future<Result> operator()(Query request) override {
    printf("Query: %s, %s\n", request.traceid().c_str(), request.query().c_str());

    Result response;
    response.set_traceid(request.traceid());
    response.set_code(ResultCode::OK);
    response.set_result("result string");
    return response;
  }
};

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
    pipeline->addBack(ServerSerializeHandler());
    // We could use a serial dispatcher instead easily
    // pipeline->addBack(SerialServerDispatcher<Query>(&service_));
    // Or a Pipelined Dispatcher
    // pipeline->addBack(PipelinedServerDispatcher<Query>(&service_));
    pipeline->addBack(MultiplexServerDispatcher<Query, Result>(&service_));
    pipeline->finalize();

    return pipeline;
  }

 private:
  ExecutorFilter<Query, Result> service_{
      std::make_shared<CPUThreadPoolExecutor>(10),
      std::make_shared<RpcService>()};
};

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  ServerBootstrap<SerializePipeline> server;
  server.childPipeline(std::make_shared<RpcPipelineFactory>());
  server.bind(FLAGS_port);
  server.waitForStop();

  return 0;
}
