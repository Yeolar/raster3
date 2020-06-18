/*
 * Copyright 2019 Yeolar
 */

#pragma once

#include <wangle/channel/Handler.h>

#include "Message.pb.h"

namespace raster {

class ClientSerializeHandler : public wangle::Handler<
  std::unique_ptr<folly::IOBuf>, Result,
  Query, std::unique_ptr<folly::IOBuf>> {
 public:
  void read(Context* ctx, std::unique_ptr<folly::IOBuf> msg) override {
    Result received;
    received.ParseFromArray(msg->data(), msg->length());
    ctx->fireRead(received);
  }

  folly::Future<folly::Unit> write(Context* ctx, Query b) override {
    std::string out;
    b.SerializePartialToString(&out);
    return ctx->fireWrite(folly::IOBuf::copyBuffer(out));
  }
};

} // namespace raster
