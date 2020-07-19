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

#pragma once

#include <accelerator/Macro.h>

#include "raster/rpcservice/Context.h"
#include "raster/rpcservice/Message.pb.h"
#include "raster/taskflow/taskflow.hpp"

namespace raster {

class Scheduler {
 public:
  virtual ~Scheduler() {}
  virtual void operator()(tf::Taskflow&, const Context&) const = 0;
};

class SchedulerRegistry {
 public:
  static SchedulerRegistry* getInstance();

  const Scheduler* get(const std::string& name) const;
  void add(const std::string& name, Scheduler* sched);

 private:
  SchedulerRegistry() = default;
  ~SchedulerRegistry() = default;

  std::map<std::string, std::unique_ptr<Scheduler>> map_;
};

struct SchedulerRegistryReceiver {
  SchedulerRegistryReceiver(const char* name, Scheduler* sched) {
    SchedulerRegistry::getInstance()->add(name, sched);
  }
};

#define RASTER_REG_SCHED(cls)                     \
  static struct raster::SchedulerRegistryReceiver \
    ACC_ANONYMOUS_VARIABLE(cls)(#cls, new cls())

//////////////////////////////////////////////////////////////////////

inline SchedulerRegistry* SchedulerRegistry::getInstance() {
  static SchedulerRegistry* sm = new SchedulerRegistry();
  return sm;
}

inline const Scheduler* SchedulerRegistry::get(const std::string& name) const {
  auto it = map_.find(name);
  return it != map_.end() ? it->second.get() : nullptr;
}

inline void SchedulerRegistry::add(const std::string& name, Scheduler* sched) {
  map_.emplace(name, std::unique_ptr<Scheduler>(sched));
}

} // namespace raster
