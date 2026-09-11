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

#ifndef _SDP_H_
#define _SDP_H_

#include <errno.h>
#include <libsdp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ULOG_TAG sdp
#include <ulog.h>

#define SDP_DEFAULT_LEN 1024

#define SDP_NTP_TO_UNIX_OFFSET 2208988800ULL

#define SDP_CRLF "\r\n"

#define SDP_TYPE_VERSION 'v'
#define SDP_TYPE_ORIGIN 'o'
#define SDP_TYPE_SESSION_NAME 's'
#define SDP_TYPE_INFORMATION 'i'
#define SDP_TYPE_URI 'u'
#define SDP_TYPE_EMAIL 'e'
#define SDP_TYPE_PHONE 'p'
#define SDP_TYPE_CONNECTION 'c'
#define SDP_TYPE_BANDWIDTH 'b'
#define SDP_TYPE_TIME 't'
#define SDP_TYPE_REPEAT_TIME 'r'
#define SDP_TYPE_TIME_ZONE 'z'
#define SDP_TYPE_ENCRYPTION_KEY 'k'
#define SDP_TYPE_ATTRIBUTE 'a'
#define SDP_TYPE_MEDIA 'm'

#define SDP_MANDATORY_TYPE_MASK_VERSION (1 << 0)
#define SDP_MANDATORY_TYPE_MASK_ORIGIN (1 << 1)
#define SDP_MANDATORY_TYPE_MASK_SESSION_NAME (1 << 2)
#define SDP_MANDATORY_TYPE_MASK_CONNECTION (1 << 3)
#define SDP_MANDATORY_TYPE_MASK_TIME (1 << 4)
#define SDP_MANDATORY_TYPE_MASK_ALL                                            \
	(SDP_MANDATORY_TYPE_MASK_VERSION | SDP_MANDATORY_TYPE_MASK_ORIGIN |    \
	 SDP_MANDATORY_TYPE_MASK_SESSION_NAME |                                \
	 SDP_MANDATORY_TYPE_MASK_CONNECTION | SDP_MANDATORY_TYPE_MASK_TIME)

#define SDP_VERSION 0

//#define SDP_PROTO_RTPAVP "RTP/AVP"
#define SDP_PROTO_RTPAVP "UDP/TLS/RTP/SAVPF"

//#define SDP_DYNAMIC_PAYLOAD_TYPE_MIN 96
#define SDP_DYNAMIC_PAYLOAD_TYPE_MIN 0
#define SDP_DYNAMIC_PAYLOAD_TYPE_MAX 127

#define SDP_ATTR_TOOL "tool"
#define SDP_ATTR_RECVONLY "recvonly"
#define SDP_ATTR_SENDRECV "sendrecv"
#define SDP_ATTR_SENDONLY "sendonly"
#define SDP_ATTR_INACTIVE "inactive"
#define SDP_ATTR_TYPE "type"
#define SDP_ATTR_CHARSET "charset"
#define SDP_ATTR_RTPAVP_RTPMAP "rtpmap"
#define SDP_ATTR_FMTP "fmtp"
#define SDP_ATTR_CONTROL_URL "control"
#define SDP_ATTR_RANGE "range"
#define SDP_ATTR_RTCP_PORT "rtcp"
#define SDP_ATTR_RTCP_XR "rtcp-xr"
#define SDP_ATTR_RTCP_XR_LOSS_RLE "pkt-loss-rle"
#define SDP_ATTR_RTCP_XR_DUP_RLE "pkt-dup-rle"
#define SDP_ATTR_RTCP_XR_RCPT_TIMES "pkt-rcpt-times"
#define SDP_ATTR_RTCP_XR_RCVR_RTT "rcvr-rtt"
#define SDP_ATTR_RTCP_XR_STAT_SUMMARY "stat-summary"
#define SDP_ATTR_RTCP_XR_STAT_LOSS "loss"
#define SDP_ATTR_RTCP_XR_STAT_DUP "dup"
#define SDP_ATTR_RTCP_XR_STAT_JITT "jitt"
#define SDP_ATTR_RTCP_XR_STAT_TTL "TTL"
#define SDP_ATTR_RTCP_XR_STAT_HL "HL"
#define SDP_ATTR_RTCP_XR_VOIP_METRICS "voip-metrics"
#define SDP_ATTR_RTCP_XR_DJB_METRICS "de-jitter-buffer"

#define SDP_RTCP_XR_RTT_REPORT_NONE_STR "none"
#define SDP_RTCP_XR_RTT_REPORT_ALL_STR "all"
#define SDP_RTCP_XR_RTT_REPORT_SENDER_STR "sender"

#define SDP_ENCODING_H264 "H264"
#define SDP_H264_CLOCKRATE 90000

#define SDP_FMTP_H264_PACKETIZATION "packetization-mode"
#define SDP_FMTP_H264_PROFILE_LEVEL "profile-level-id"
#define SDP_FMTP_H264_PARAM_SETS "sprop-parameter-sets"

#define SDP_MULTICAST_ADDR_MIN 224
#define SDP_MULTICAST_ADDR_MAX 239

#define SDP_TIME_NPT "npt"
#define SDP_TIME_NPT_NOW "now"
#define SDP_TIME_SMPTE "smpte"
#define SDP_TIME_SMPTE_30_DROP "smpte-30-drop"
#define SDP_TIME_SMPTE_25 "smpte-25"
#define SDP_TIME_ABSOLUTE "clock"
#define SDP_TIME_FORMAT_ISO8601 "%Y%m%dT%H%M%SZ"


#define CHECK_FUNC(_func, _ret, _on_err, ...)                                  \
	do {                                                                   \
		_ret = _func(__VA_ARGS__);                                     \
		if (_ret < 0) {                                                \
			ULOG_ERRNO(#_func, -_ret);                             \
			_on_err;                                               \
		}                                                              \
	} while (0)


struct sdp_string {
	char *str;
	size_t len;
	size_t max_len;
};


static inline int __attribute__((__format__(__printf__, 2, 0)))
sdp_vsprintf(struct sdp_string *str, const char *fmt, va_list args)
{
	if (str->len >= str->max_len)
		return -ENOBUFS;
	int len = vsnprintf(
		str->str + str->len, str->max_len - str->len, fmt, args);
	if (len < 0)
		return len;
	if (len >= (signed)(str->max_len - str->len)) {
		size_t new_len = (str->len + len + 1023) & ~1023;
		void *tmp = realloc(str->str, new_len);
		if (!tmp)
			return -ENOMEM;
		str->str = tmp;
		str->max_len = new_len;
		return -EAGAIN;
	}
	str->len += len;
	return 0;
}


static inline int __attribute__((__format__(__printf__, 2, 3)))
sdp_sprintf(struct sdp_string *str, const char *fmt, ...)
{
	int ret;
	va_list args;
	va_start(args, fmt);
	ret = sdp_vsprintf(str, fmt, args);
	va_end(args);
	if (ret == -EAGAIN) {
		va_start(args, fmt);
		ret = sdp_vsprintf(str, fmt, args);
		va_end(args);
	}
	return ret;
}


static inline char *xstrdup(const char *s)
{
	return s == NULL ? NULL : strdup(s);
}


int sdp_base64_encode(const void *data, size_t size, char **out);

int sdp_base64_decode(const char *str, void **out, size_t *out_size);


#endif /* !_SDP_H_ */
