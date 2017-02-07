// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "device/time_zone_monitor/time_zone_monitor.h"

#include <stddef.h>
#include <stdlib.h>

#include <vector>

#include "base/bind.h"
#include "base/files/file_path.h"
#include "base/files/file_path_watcher.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/sequenced_task_runner.h"
#include "base/stl_util.h"
#include "base/threading/thread_restrictions.h"
#include "base/threading/thread_task_runner_handle.h"
#include "build/build_config.h"

namespace device {

namespace {
class TimeZoneMonitorAndroidImpl;
}  // namespace

class TimeZoneMonitorAndroid : public TimeZoneMonitor {
 public:
  TimeZoneMonitorAndroid(
      scoped_refptr<base::SequencedTaskRunner> file_task_runner);
  ~TimeZoneMonitorAndroid() override;

  void NotifyClientsFromImpl() { NotifyClients(); }

 private:
  scoped_refptr<TimeZoneMonitorAndroidImpl> impl_;

  DISALLOW_COPY_AND_ASSIGN(TimeZoneMonitorAndroid);
};

namespace {

// FilePathWatcher needs to run on the FILE thread, but TimeZoneMonitor runs
// on the UI thread. TimeZoneMonitorAndroidImpl is the bridge between these
// threads.
class TimeZoneMonitorAndroidImpl
    : public base::RefCountedThreadSafe<TimeZoneMonitorAndroidImpl> {
 public:
  explicit TimeZoneMonitorAndroidImpl(
      TimeZoneMonitorAndroid* owner,
      scoped_refptr<base::SequencedTaskRunner> file_task_runner)
      : base::RefCountedThreadSafe<TimeZoneMonitorAndroidImpl>(),
        file_path_watchers_(),
        main_task_runner_(base::ThreadTaskRunnerHandle::Get()),
        file_task_runner_(file_task_runner),
        owner_(owner) {
    DCHECK(main_task_runner_->RunsTasksOnCurrentThread());
    file_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&TimeZoneMonitorAndroidImpl::StartWatchingOnFileThread, this));
  }

  void StopWatching() {
    DCHECK(main_task_runner_->RunsTasksOnCurrentThread());
    owner_ = NULL;
    file_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&TimeZoneMonitorAndroidImpl::StopWatchingOnFileThread, this));
  }

 private:
  friend class base::RefCountedThreadSafe<TimeZoneMonitorAndroidImpl>;

  ~TimeZoneMonitorAndroidImpl() {
    DCHECK(!owner_);
    base::STLDeleteElements(&file_path_watchers_);
  }

  void StartWatchingOnFileThread() {
    base::ThreadRestrictions::AssertIOAllowed();
    DCHECK(file_task_runner_->RunsTasksOnCurrentThread());

    // There is no true standard for where time zone information is actually
    // stored. glibc uses /etc/localtime, uClibc uses /etc/TZ, and some older
    // systems store the name of the time zone file within /usr/share/zoneinfo
    // in /etc/timezone. Different libraries and custom builds may mean that
    // still more paths are used. Just watch all three of these paths, because
    // false positives are harmless, assuming the false positive rate is
    // reasonable.
    const char* const kFilesToWatch[] = {
        "/etc/localtime", "/etc/timezone", "/etc/TZ",
    };

    for (size_t index = 0; index < arraysize(kFilesToWatch); ++index) {
      file_path_watchers_.push_back(new base::FilePathWatcher());
      file_path_watchers_.back()->Watch(
          base::FilePath(kFilesToWatch[index]), false,
          base::Bind(&TimeZoneMonitorAndroidImpl::OnTimeZoneFileChanged, this));
    }
  }

  void StopWatchingOnFileThread() {
    DCHECK(file_task_runner_->RunsTasksOnCurrentThread());
    base::STLDeleteElements(&file_path_watchers_);
  }

  void OnTimeZoneFileChanged(const base::FilePath& path, bool error) {
    DCHECK(file_task_runner_->RunsTasksOnCurrentThread());
    main_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&TimeZoneMonitorAndroidImpl::OnTimeZoneFileChangedOnUIThread,
                   this));
  }

  void OnTimeZoneFileChangedOnUIThread() {
    DCHECK(main_task_runner_->RunsTasksOnCurrentThread());
    if (owner_) {
      owner_->NotifyClientsFromImpl();
    }
  }

  std::vector<base::FilePathWatcher*> file_path_watchers_;

  scoped_refptr<base::SequencedTaskRunner> main_task_runner_;
  scoped_refptr<base::SequencedTaskRunner> file_task_runner_;
  TimeZoneMonitorAndroid* owner_;

  DISALLOW_COPY_AND_ASSIGN(TimeZoneMonitorAndroidImpl);
};

}  // namespace

TimeZoneMonitorAndroid::TimeZoneMonitorAndroid(
    scoped_refptr<base::SequencedTaskRunner> file_task_runner)
    : TimeZoneMonitor(), impl_() {
  // If the TZ environment variable is set, its value specifies the time zone
  // specification, and it's pointless to monitor any files in /etc for
  // changes because such changes would have no effect on the TZ environment
  // variable and thus the interpretation of the local time zone in the
  // or renderer processes.
  //
  // The system-specific format for the TZ environment variable beginning with
  // a colon is implemented by glibc as the path to a time zone data file, and
  // it would be possible to monitor this file for changes if a TZ variable of
  // this format was encountered, but this is not necessary: when loading a
  // time zone specification in this way, glibc does not reload the file when
  // it changes, so it's pointless to respond to a notification that it has
  // changed.
  if (!getenv("TZ")) {
    impl_ = new TimeZoneMonitorAndroidImpl(this, file_task_runner);
  }
}

TimeZoneMonitorAndroid::~TimeZoneMonitorAndroid() {
  if (impl_.get()) {
    impl_->StopWatching();
  }
}

// static
std::unique_ptr<TimeZoneMonitor> TimeZoneMonitor::Create(
    scoped_refptr<base::SequencedTaskRunner> file_task_runner) {
  return std::unique_ptr<TimeZoneMonitor>(
      new TimeZoneMonitorAndroid(file_task_runner));
}

}  // namespace device
