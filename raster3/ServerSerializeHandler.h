/*
 * Copyright 2019 Yeolar
 */

#pragma once

#include <wangle/channel/Handler.h>

#include "Message.pb.h"

namespace cdata {

class ServerSerializeHandler : public wangle::Handler<
  std::unique_ptr<folly::IOBuf>, Query,
  Result, std::unique_ptr<folly::IOBuf>> {
 public:
  void read(Context* ctx, std::unique_ptr<folly::IOBuf> msg) override {
    Query received;
    received.ParseFromArray(msg->data(), msg->length());
    ctx->fireRead(received);
  }

  folly::Future<folly::Unit> write(Context* ctx, Result b) override {
    std::string out;
    b.SerializePartialToString(&out);
    return ctx->fireWrite(folly::IOBuf::copyBuffer(out));
  }
};

} // namespace cdata
