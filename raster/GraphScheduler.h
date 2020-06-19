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

#include "raster/Context.h"
#include "raster/Message.pb.h"
#include "raster/taskflow/taskflow.hpp"

namespace raster {

typedef std::function<
  void(tf::Taskflow& taskflow,
       const Query& request,
       Result& response,
       Context& context)> ScheduleFn;

class ScheduleManager {
 public:
  static ScheduleManager* getInstance() {
    static ScheduleManager* sm = new ScheduleManager();
    return sm;
  }

  const ScheduleFn& get(const std::string& name) const {
    return map_.at(name);
  }

  void add(const std::string& name, ScheduleFn&& fn) {
    map_.emplace(name, std::move(fn));
  }

 private:
  ScheduleManager() = default;
  ~ScheduleManager() = default;

  std::map<std::string, ScheduleFn> map_;
};

struct ScheduleRegister {
  ScheduleRegister(const char* name, ScheduleFn fn) {
    ScheduleManager::getInstance()->add(name, std::move(fn));
  }
};

} // namespace raster
