/*
 *  Copyright 2020 The mediaserver Project Authors. All rights reserved.
 *
 *  Created on: 2020/07/16
 *      Author: max
 *		Email: Kingsleyyau@gmail.com
 *
 *  Borrow from WebRTC Project
 */

#ifndef RTP_BASE_METRICS_H_
#define RTP_BASE_METRICS_H_

#include <stddef.h>

#include <map>
#include <memory>
#include <string>

#include <rtp/base/atomic_ops.h>
#include <rtp/base/checks.h>

// Macros for allowing WebRTC clients (e.g. Chrome) to gather and aggregate
// statistics.
//
// Histogram for counters.
// RTC_HISTOGRAM_COUNTS(name, sample, min, max, bucket_count);
//
// Histogram for enumerators.
// The boundary should be above the max enumerator sample.
// RTC_HISTOGRAM_ENUMERATION(name, sample, boundary);
//
//
// The macros use the methods HistogramFactoryGetCounts,
// HistogramFactoryGetEnumeration and HistogramAdd.
//
// By default WebRTC provides implementations of the aforementioned methods
// that can be found in system_wrappers/source/metrics.cc. If clients want to
// provide a custom version, they will have to:
//
// 1. Compile WebRTC defining the preprocessor macro
//    WEBRTC_EXCLUDE_METRICS_DEFAULT (if GN is used this can be achieved
//    by setting the GN arg rtc_exclude_metrics_default to true).
// 2. Provide implementations of:
//    Histogram* qpidnetwork::metrics::HistogramFactoryGetCounts(
//        const std::string& name, int sample, int min, int max,
//        int bucket_count);
//    Histogram* qpidnetwork::metrics::HistogramFactoryGetEnumeration(
//        const std::string& name, int sample, int boundary);
//    void qpidnetwork::metrics::HistogramAdd(
//        Histogram* histogram_pointer, const std::string& name, int sample);
//
// Example usage:
//
// RTC_HISTOGRAM_COUNTS("WebRTC.Video.NacksSent", nacks_sent, 1, 100000, 100);
//
// enum Types {
//   kTypeX,
//   kTypeY,
//   kBoundary,
// };
//
// RTC_HISTOGRAM_ENUMERATION("WebRTC.Types", kTypeX, kBoundary);
//
// NOTE: It is recommended to do the Chromium review for modifications to
// histograms.xml before new metrics are committed to WebRTC.

// Macros for adding samples to a named histogram.

// Histogram for counters (exponentially spaced buckets).
#define RTC_HISTOGRAM_COUNTS_100(name, sample) \
  RTC_HISTOGRAM_COUNTS(name, sample, 1, 100, 50)

#define RTC_HISTOGRAM_COUNTS_200(name, sample) \
  RTC_HISTOGRAM_COUNTS(name, sample, 1, 200, 50)

#define RTC_HISTOGRAM_COUNTS_500(name, sample) \
  RTC_HISTOGRAM_COUNTS(name, sample, 1, 500, 50)

#define RTC_HISTOGRAM_COUNTS_1000(name, sample) \
  RTC_HISTOGRAM_COUNTS(name, sample, 1, 1000, 50)

#define RTC_HISTOGRAM_COUNTS_10000(name, sample) \
  RTC_HISTOGRAM_COUNTS(name, sample, 1, 10000, 50)

#define RTC_HISTOGRAM_COUNTS_100000(name, sample) \
  RTC_HISTOGRAM_COUNTS(name, sample, 1, 100000, 50)

#define RTC_HISTOGRAM_COUNTS(name, sample, min, max, bucket_count)       \
  RTC_HISTOGRAM_COMMON_BLOCK(name, sample,                               \
                             qpidnetwork::metrics::HistogramFactoryGetCounts( \
                                 name, min, max, bucket_count))

#define RTC_HISTOGRAM_COUNTS_LINEAR(name, sample, min, max, bucket_count)      \
  RTC_HISTOGRAM_COMMON_BLOCK(name, sample,                                     \
                             qpidnetwork::metrics::HistogramFactoryGetCountsLinear( \
                                 name, min, max, bucket_count))

// Slow metrics: pointer to metric is acquired at each call and is not cached.
//
#define RTC_HISTOGRAM_COUNTS_SPARSE_100(name, sample) \
  RTC_HISTOGRAM_COUNTS_SPARSE(name, sample, 1, 100, 50)

#define RTC_HISTOGRAM_COUNTS_SPARSE_200(name, sample) \
  RTC_HISTOGRAM_COUNTS_SPARSE(name, sample, 1, 200, 50)

#define RTC_HISTOGRAM_COUNTS_SPARSE_500(name, sample) \
  RTC_HISTOGRAM_COUNTS_SPARSE(name, sample, 1, 500, 50)

#define RTC_HISTOGRAM_COUNTS_SPARSE_1000(name, sample) \
  RTC_HISTOGRAM_COUNTS_SPARSE(name, sample, 1, 1000, 50)

#define RTC_HISTOGRAM_COUNTS_SPARSE_10000(name, sample) \
  RTC_HISTOGRAM_COUNTS_SPARSE(name, sample, 1, 10000, 50)

#define RTC_HISTOGRAM_COUNTS_SPARSE_100000(name, sample) \
  RTC_HISTOGRAM_COUNTS_SPARSE(name, sample, 1, 100000, 50)

#define RTC_HISTOGRAM_COUNTS_SPARSE(name, sample, min, max, bucket_count)     \
  RTC_HISTOGRAM_COMMON_BLOCK_SLOW(name, sample,                               \
                                  qpidnetwork::metrics::HistogramFactoryGetCounts( \
                                      name, min, max, bucket_count))

// Histogram for percentage (evenly spaced buckets).
#define RTC_HISTOGRAM_PERCENTAGE_SPARSE(name, sample) \
  RTC_HISTOGRAM_ENUMERATION_SPARSE(name, sample, 101)

// Histogram for booleans.
#define RTC_HISTOGRAM_BOOLEAN_SPARSE(name, sample) \
  RTC_HISTOGRAM_ENUMERATION_SPARSE(name, sample, 2)

// Histogram for enumerators (evenly spaced buckets).
// |boundary| should be above the max enumerator sample.
//
// TODO(qingsi): Refactor the default implementation given by RtcHistogram,
// which is already sparse, and remove the boundary argument from the macro.
#define RTC_HISTOGRAM_ENUMERATION_SPARSE(name, sample, boundary) \
  RTC_HISTOGRAM_COMMON_BLOCK_SLOW(                               \
      name, sample,                                              \
      qpidnetwork::metrics::SparseHistogramFactoryGetEnumeration(name, boundary))

// Histogram for percentage (evenly spaced buckets).
#define RTC_HISTOGRAM_PERCENTAGE(name, sample) \
  RTC_HISTOGRAM_ENUMERATION(name, sample, 101)

// Histogram for booleans.
#define RTC_HISTOGRAM_BOOLEAN(name, sample) \
  RTC_HISTOGRAM_ENUMERATION(name, sample, 2)

// Histogram for enumerators (evenly spaced buckets).
// |boundary| should be above the max enumerator sample.
#define RTC_HISTOGRAM_ENUMERATION(name, sample, boundary) \
  RTC_HISTOGRAM_COMMON_BLOCK_SLOW(                        \
      name, sample,                                       \
      qpidnetwork::metrics::HistogramFactoryGetEnumeration(name, boundary))

// The name of the histogram should not vary.
// TODO(asapersson): Consider changing string to const char*.
#define RTC_HISTOGRAM_COMMON_BLOCK(constant_name, sample,                  \
                                   factory_get_invocation)                 \
  do {                                                                     \
    static qpidnetwork::metrics::Histogram* atomic_histogram_pointer = nullptr; \
    qpidnetwork::metrics::Histogram* histogram_pointer =                        \
        rtc::AtomicOps::AcquireLoadPtr(&atomic_histogram_pointer);         \
    if (!histogram_pointer) {                                              \
      histogram_pointer = factory_get_invocation;                          \
      qpidnetwork::metrics::Histogram* prev_pointer =                           \
          rtc::AtomicOps::CompareAndSwapPtr(                               \
              &atomic_histogram_pointer,                                   \
              static_cast<qpidnetwork::metrics::Histogram*>(nullptr),           \
              histogram_pointer);                                          \
      RTC_DCHECK(prev_pointer == nullptr ||                                \
                 prev_pointer == histogram_pointer);                       \
    }                                                                      \
    if (histogram_pointer) {                                               \
      qpidnetwork::metrics::HistogramAdd(histogram_pointer, sample);            \
    }                                                                      \
  } while (0)

// The histogram is constructed/found for each call.
// May be used for histograms with infrequent updates.`
#define RTC_HISTOGRAM_COMMON_BLOCK_SLOW(name, sample, factory_get_invocation) \
  do {                                                                        \
    qpidnetwork::metrics::Histogram* histogram_pointer = factory_get_invocation;   \
    if (histogram_pointer) {                                                  \
      qpidnetwork::metrics::HistogramAdd(histogram_pointer, sample);               \
    }                                                                         \
  } while (0)

// Helper macros.
// Macros for calling a histogram with varying name (e.g. when using a metric
// in different modes such as real-time vs screenshare). Fast, because pointer
// is cached. |index| should be different for different names. Allowed |index|
// values are 0, 1, and 2.
#define RTC_HISTOGRAMS_COUNTS_100(index, name, sample) \
  RTC_HISTOGRAMS_COMMON(index, name, sample,           \
                        RTC_HISTOGRAM_COUNTS(name, sample, 1, 100, 50))

#define RTC_HISTOGRAMS_COUNTS_200(index, name, sample) \
  RTC_HISTOGRAMS_COMMON(index, name, sample,           \
                        RTC_HISTOGRAM_COUNTS(name, sample, 1, 200, 50))

#define RTC_HISTOGRAMS_COUNTS_500(index, name, sample) \
  RTC_HISTOGRAMS_COMMON(index, name, sample,           \
                        RTC_HISTOGRAM_COUNTS(name, sample, 1, 500, 50))

#define RTC_HISTOGRAMS_COUNTS_1000(index, name, sample) \
  RTC_HISTOGRAMS_COMMON(index, name, sample,            \
                        RTC_HISTOGRAM_COUNTS(name, sample, 1, 1000, 50))

#define RTC_HISTOGRAMS_COUNTS_10000(index, name, sample) \
  RTC_HISTOGRAMS_COMMON(index, name, sample,             \
                        RTC_HISTOGRAM_COUNTS(name, sample, 1, 10000, 50))

#define RTC_HISTOGRAMS_COUNTS_100000(index, name, sample) \
  RTC_HISTOGRAMS_COMMON(index, name, sample,              \
                        RTC_HISTOGRAM_COUNTS(name, sample, 1, 100000, 50))

#define RTC_HISTOGRAMS_ENUMERATION(index, name, sample, boundary) \
  RTC_HISTOGRAMS_COMMON(index, name, sample,                      \
                        RTC_HISTOGRAM_ENUMERATION(name, sample, boundary))

#define RTC_HISTOGRAMS_PERCENTAGE(index, name, sample) \
  RTC_HISTOGRAMS_COMMON(index, name, sample,           \
                        RTC_HISTOGRAM_PERCENTAGE(name, sample))

#define RTC_HISTOGRAMS_COMMON(index, name, sample, macro_invocation) \
  do {                                                               \
    switch (index) {                                                 \
      case 0:                                                        \
        macro_invocation;                                            \
        break;                                                       \
      case 1:                                                        \
        macro_invocation;                                            \
        break;                                                       \
      case 2:                                                        \
        macro_invocation;                                            \
        break;                                                       \
      default:                                                       \
        RTC_NOTREACHED();                                            \
    }                                                                \
  } while (0)

namespace qpidnetwork {
namespace metrics {

// Time that should have elapsed for stats that are gathered once per call.
enum { kMinRunTimeInSeconds = 10 };

class Histogram;

// Functions for getting pointer to histogram (constructs or finds the named
// histogram).

// Get histogram for counters.
Histogram* HistogramFactoryGetCounts(const std::string& name,
                                     int min,
                                     int max,
                                     int bucket_count);

// Get histogram for counters with linear bucket spacing.
Histogram* HistogramFactoryGetCountsLinear(const std::string& name,
                                           int min,
                                           int max,
                                           int bucket_count);

// Get histogram for enumerators.
// |boundary| should be above the max enumerator sample.
Histogram* HistogramFactoryGetEnumeration(const std::string& name,
                                          int boundary);

// Get sparse histogram for enumerators.
// |boundary| should be above the max enumerator sample.
Histogram* SparseHistogramFactoryGetEnumeration(const std::string& name,
                                                int boundary);

// Function for adding a |sample| to a histogram.
void HistogramAdd(Histogram* histogram_pointer, int sample);

struct SampleInfo {
  SampleInfo(const std::string& name, int min, int max, size_t bucket_count);
  ~SampleInfo();

  const std::string name;
  const int min;
  const int max;
  const size_t bucket_count;
  std::map<int, int> samples;  // <value, # of events>
};

// Enables collection of samples.
// This method should be called before any other call into webrtc.
void Enable();

// Gets histograms and clears all samples.
void GetAndReset(
    std::map<std::string, std::unique_ptr<SampleInfo>>* histograms);

// Functions below are mainly for testing.

// Clears all samples.
void Reset();

// Returns the number of times the |sample| has been added to the histogram.
int NumEvents(const std::string& name, int sample);

// Returns the total number of added samples to the histogram.
int NumSamples(const std::string& name);

// Returns the minimum sample value (or -1 if the histogram has no samples).
int MinSample(const std::string& name);

// Returns a map with keys the samples with at least one event and values the
// number of events for that sample.
std::map<int, int> Samples(const std::string& name);

}  // namespace metrics
}  // namespace webrtc

#endif  // SYSTEM_WRAPPERS_INCLUDE_METRICS_H_
