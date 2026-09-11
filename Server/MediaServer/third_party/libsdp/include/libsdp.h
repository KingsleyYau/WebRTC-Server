/**
 * Copyright (c) 2017 Parrot Drones SAS
 * Copyright (c) 2017 Aurelien Barre
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the copyright holders nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LIBSDP_H_
#define _LIBSDP_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* To be used for all public API */
#ifdef SDP_API_EXPORTS
#	ifdef _WIN32
#		define SDP_API __declspec(dllexport)
#	else /* !_WIN32 */
#		define SDP_API __attribute__((visibility("default")))
#	endif /* !_WIN32 */
#else /* !SDP_API_EXPORTS */
#	define SDP_API
#endif /* !SDP_API_EXPORTS */

#include <errno.h>
#include <inttypes.h>
#include <time.h>

#include <list.h>

#include <ulog.h>

enum sdp_media_type {
	SDP_MEDIA_TYPE_AUDIO = 0,
	SDP_MEDIA_TYPE_VIDEO,
	SDP_MEDIA_TYPE_TEXT,
	SDP_MEDIA_TYPE_APPLICATION,
	SDP_MEDIA_TYPE_MESSAGE,

	SDP_MEDIA_TYPE_MAX,
};


enum sdp_start_mode {
	SDP_START_MODE_UNSPECIFIED = 0,
	SDP_START_MODE_RECVONLY,
	SDP_START_MODE_SENDRECV,
	SDP_START_MODE_SENDONLY,
	SDP_START_MODE_INACTIVE,

	SDP_START_MODE_MAX,
};


enum sdp_rtcp_xr_rtt_report_mode {
	SDP_RTCP_XR_RTT_REPORT_NONE = 0,
	SDP_RTCP_XR_RTT_REPORT_ALL,
	SDP_RTCP_XR_RTT_REPORT_SENDER,

	SDP_RTCP_XR_RTT_REPORT_MAX,
};


struct sdp_attr {
	char *key;
	char *value;

	struct list_node node;
};


/**
 * Range attribute definitions (see RFC 2326 chapter C.1.5)
 */

enum sdp_time_format {
	SDP_TIME_FORMAT_UNKNOWN = 0,
	SDP_TIME_FORMAT_NPT,
	SDP_TIME_FORMAT_SMPTE,
	SDP_TIME_FORMAT_ABSOLUTE,
};

/* Normal Play Time (NPT), see RFC 2326 chapter 3.6 */
struct sdp_time_npt {
	int now;
	int infinity;
	time_t sec;
	uint32_t usec;
};

/* SMPTE Relative Timestamps, see RFC 2326 chapter 3.5 */
struct sdp_time_smpte {
	int infinity;
	time_t sec;
	unsigned int frames;
};

/* Absolute Time (UTC, ISO 8601), see RFC 2326 chapter 3.7 */
struct sdp_time_absolute {
	int infinity;
	time_t sec;
	uint32_t usec;
};

struct sdp_time {
	enum sdp_time_format format;
	union {
		struct sdp_time_npt npt;
		struct sdp_time_smpte smpte;
		struct sdp_time_absolute absolute;
	};
};

struct sdp_range {
	struct sdp_time start;
	struct sdp_time stop;
};


/* H.264 payload format parameters (see RFC 6184) */
struct sdp_h264_fmtp {
	int valid;
	unsigned int packetization_mode;
	unsigned int profile_idc;
	unsigned int profile_iop;
	unsigned int level_idc;
	uint8_t *sps;
	unsigned int sps_size;
	uint8_t *pps;
	unsigned int pps_size;
};


/* RFC 3611 and RFC 7005 RTCP extended reports */
struct sdp_rtcp_xr {
	int valid;
	int loss_rle_report;
	unsigned int loss_rle_report_max_size;
	int dup_rle_report;
	unsigned int dup_rle_report_max_size;
	int pkt_receipt_times_report;
	unsigned int pkt_receipt_times_report_max_size;
	enum sdp_rtcp_xr_rtt_report_mode rtt_report;
	unsigned int rtt_report_max_size;
	int stats_summary_report_loss;
	int stats_summary_report_dup;
	int stats_summary_report_jitter;
	int stats_summary_report_ttl;
	int stats_summary_report_hl;
	int voip_metrics_report;
	int djb_metrics_report;
};

struct sdp_payload {
	/* RTP/AVP type */
	unsigned int payload_type;

	/* RTP/AVP rtpmap attribute */
	char *encoding_name;
	char *encoding_params;
	unsigned int clock_rate;
	char *fmtp;
};

#define SDP_MAX_PAYLOAD_TYPE_COUNT 64
struct sdp_media {
	enum sdp_media_type type;
	char *media_title;
	char *connection_addr;
	int multicast;
	unsigned int dst_stream_port;
	unsigned int dst_control_port;

	char *payload_type_str;
	struct sdp_payload payload_type_array[SDP_MAX_PAYLOAD_TYPE_COUNT];
	unsigned int payload_type_array_count;

	char *control_url;
	enum sdp_start_mode start_mode;
	struct sdp_range range;

	/* H.264 payload format parameters */
	struct sdp_h264_fmtp h264_fmtp;

	/* RTCP extended reports */
	struct sdp_rtcp_xr rtcp_xr;

	unsigned int attr_count;
	struct list_node attrs;

	struct list_node node;
};


struct sdp_session {
	int deletion;
	uint64_t session_id;
	uint64_t session_version;
	char *server_addr;
	char *session_name;
	char *session_info;
	char *uri;
	char *email;
	char *phone;
	char *tool;
	char *type;
	char *charset;
	char *connection_addr;
	int multicast;
	char *control_url;
	enum sdp_start_mode start_mode;
	struct sdp_range range;

	/* RTCP extended reports */
	struct sdp_rtcp_xr rtcp_xr;

	unsigned int attr_count;
	struct list_node attrs;
	unsigned int media_count;
	struct list_node medias;
};


SDP_API struct sdp_session *sdp_session_new(void);


SDP_API int sdp_session_destroy(struct sdp_session *session);


SDP_API int sdp_session_copy(const struct sdp_session *src,
			     struct sdp_session *dst);


/**
 * Comparison function between 2 session descriptions.
 * This function compares the following elements of 2 sdp session objets:
 *    - session name
 *    - number of media
 *    - control url of each media
 *    - type of each media
 * NULL input session description is accepted
 * 2 NULL input sdp are considered identical
 * @param a: pointer to sdp_session structure to compare
 * @param b: pointer to sdp_session structure to compare
 * @return 0 if SDPs are identical, 1 if the SDPs differs or comparison was not
 * possible
 */
SDP_API int sdp_session_compare(const struct sdp_session *a,
				const struct sdp_session *b);


SDP_API struct sdp_media *sdp_media_new(void);


SDP_API int sdp_media_destroy(struct sdp_media *media);


SDP_API int sdp_media_copy(const struct sdp_media *src, struct sdp_media *dst);


SDP_API struct sdp_attr *sdp_attr_new(void);


SDP_API int sdp_attr_destroy(struct sdp_attr *attr);


SDP_API int sdp_attr_copy(const struct sdp_attr *src, struct sdp_attr *dst);


SDP_API int sdp_session_attr_add(struct sdp_session *session,
				 struct sdp_attr **ret_obj);


SDP_API int sdp_session_attr_add_existing(struct sdp_session *session,
					  struct sdp_attr *attr);


SDP_API int sdp_session_attr_remove(struct sdp_session *session,
				    struct sdp_attr *attr);


SDP_API int sdp_session_media_add(struct sdp_session *session,
				  struct sdp_media **ret_obj);


SDP_API int sdp_session_media_add_existing(struct sdp_session *session,
					   struct sdp_media *media);


SDP_API int sdp_session_media_remove(struct sdp_session *session,
				     struct sdp_media *media);


SDP_API int sdp_media_attr_add(struct sdp_media *media,
			       struct sdp_attr **ret_obj);


SDP_API int sdp_media_attr_add_existing(struct sdp_media *media,
					struct sdp_attr *attr);


SDP_API int sdp_media_attr_remove(struct sdp_media *media,
				  struct sdp_attr *attr);


SDP_API int sdp_description_read(const char *session_desc,
				 struct sdp_session **ret_obj);


SDP_API int sdp_description_write(const struct sdp_session *session,
				  char **ret_str);


SDP_API const char *sdp_media_type_str(enum sdp_media_type val);


SDP_API const char *sdp_start_mode_str(enum sdp_start_mode val);


SDP_API const char *
sdp_rtcp_xr_rtt_report_mode_str(enum sdp_rtcp_xr_rtt_report_mode val);


SDP_API const char *sdp_time_format_str(enum sdp_time_format val);

// Add by Max 2022/10/19
SDP_API void ulog_set_func(SDP_LOG_FUNC_IMP logImp);

static inline int sdp_time_us_to_npt(uint64_t time_us,
				     struct sdp_time_npt *time_npt)
{
	if (time_npt == NULL)
		return -EINVAL;

	time_npt->now = 0;
	time_npt->infinity = 0;
	time_npt->sec = time_us / 1000000;
	time_npt->usec = time_us - time_npt->sec * 1000000;

	return 0;
}


static inline int sdp_time_npt_to_us(const struct sdp_time_npt *time_npt,
				     uint64_t *time_us)
{
	if ((time_npt == NULL) || (time_us == NULL))
		return -EINVAL;
	if ((time_npt->now) || (time_npt->infinity))
		return -EINVAL;

	*time_us = time_npt->sec * 1000000 + time_npt->usec;

	return 0;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !_LIBSDP_H_ */
