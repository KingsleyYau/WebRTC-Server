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

#include "sdp.h"

struct sdp_payload* sdp_media_type_check_valid(int payload_type, struct sdp_media *media) {
	struct sdp_payload *payload = NULL;
	for(int i = 0; i < media->payload_type_array_count; i++) {
		if( payload_type == media->payload_type_array[i].payload_type ) {
			payload = &(media->payload_type_array[i]);
			break;
		}
	}
	return payload;
}

const char *sdp_media_type_str(enum sdp_media_type val)
{
	switch (val) {
	case SDP_MEDIA_TYPE_AUDIO:
		return "audio";
	case SDP_MEDIA_TYPE_VIDEO:
		return "video";
	case SDP_MEDIA_TYPE_TEXT:
		return "text";
	case SDP_MEDIA_TYPE_APPLICATION:
		return "application";
	case SDP_MEDIA_TYPE_MESSAGE:
		return "message";
	default:
		return "unknown";
	}
}


static int sdp_media_type_from_str(const char *str, enum sdp_media_type *type)
{
	if (strcmp(str, "audio") == 0) {
		*type = SDP_MEDIA_TYPE_AUDIO;
		return 0;
	} else if (strcmp(str, "video") == 0) {
		*type = SDP_MEDIA_TYPE_VIDEO;
		return 0;
	} else if (strcmp(str, "text") == 0) {
		*type = SDP_MEDIA_TYPE_TEXT;
		return 0;
	} else if (strcmp(str, "application") == 0) {
		*type = SDP_MEDIA_TYPE_APPLICATION;
		return 0;
	} else if (strcmp(str, "message") == 0) {
		*type = SDP_MEDIA_TYPE_MESSAGE;
		return 0;
	}
	return -EINVAL;
}


const char *sdp_start_mode_str(enum sdp_start_mode val)
{
	switch (val) {
	default:
	case SDP_START_MODE_UNSPECIFIED:
		return "unspecified";
	case SDP_START_MODE_RECVONLY:
		return SDP_ATTR_RECVONLY;
	case SDP_START_MODE_SENDRECV:
		return SDP_ATTR_SENDRECV;
	case SDP_START_MODE_SENDONLY:
		return SDP_ATTR_SENDONLY;
	case SDP_START_MODE_INACTIVE:
		return SDP_ATTR_INACTIVE;
	}
}


const char *
sdp_rtcp_xr_rtt_report_mode_str(enum sdp_rtcp_xr_rtt_report_mode val)
{
	switch (val) {
	case SDP_RTCP_XR_RTT_REPORT_NONE:
		return SDP_RTCP_XR_RTT_REPORT_NONE_STR;
	case SDP_RTCP_XR_RTT_REPORT_ALL:
		return SDP_RTCP_XR_RTT_REPORT_ALL_STR;
	case SDP_RTCP_XR_RTT_REPORT_SENDER:
		return SDP_RTCP_XR_RTT_REPORT_SENDER_STR;
	default:
		return "unknown";
	}
}


const char *sdp_time_format_str(enum sdp_time_format val)
{
	switch (val) {
	case SDP_TIME_FORMAT_NPT:
		return "NPT";
	case SDP_TIME_FORMAT_SMPTE:
		return "SMPTE";
	case SDP_TIME_FORMAT_ABSOLUTE:
		return "ABSOLUTE";
	default:
	case SDP_TIME_FORMAT_UNKNOWN:
		return "UNKNOWN";
	}
}


struct sdp_session *sdp_session_new(void)
{
	struct sdp_session *session = calloc(1, sizeof(*session));
	ULOG_ERRNO_RETURN_VAL_IF(session == NULL, ENOMEM, NULL);
	list_init(&session->attrs);
	list_init(&session->medias);

	return session;
}


int sdp_session_destroy(struct sdp_session *session)
{
	struct sdp_attr *attr = NULL, *tmp_attr = NULL;
	struct sdp_media *media = NULL, *tmp_media = NULL;

	if (session == NULL)
		return 0;

	/* Remove all attibutes */
	list_walk_entry_forward_safe(&session->attrs, attr, tmp_attr, node)
	{
		sdp_session_attr_remove(session, attr);
	}

	/* Remove all media */
	list_walk_entry_forward_safe(&session->medias, media, tmp_media, node)
	{
		sdp_session_media_remove(session, media);
	}

	free(session->server_addr);
	free(session->session_name);
	free(session->session_info);
	free(session->uri);
	free(session->email);
	free(session->phone);
	free(session->tool);
	free(session->type);
	free(session->charset);
	free(session->connection_addr);
	free(session->control_url);
	free(session);

	return 0;
}


int sdp_session_copy(const struct sdp_session *src, struct sdp_session *dst)
{
	struct sdp_attr *attr, *_attr = NULL;
	struct sdp_media *media, *_media = NULL;
	int err;

	ULOG_ERRNO_RETURN_ERR_IF(src == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(dst == NULL, EINVAL);

	dst->session_id = src->session_id;
	dst->session_version = src->session_version;
	dst->server_addr = xstrdup(src->server_addr);
	dst->session_name = xstrdup(src->session_name);
	dst->session_info = xstrdup(src->session_info);
	dst->uri = xstrdup(src->uri);
	dst->email = xstrdup(src->email);
	dst->phone = xstrdup(src->phone);
	dst->tool = xstrdup(src->tool);
	dst->type = xstrdup(src->type);
	dst->charset = xstrdup(src->charset);
	dst->connection_addr = xstrdup(src->connection_addr);
	dst->multicast = src->multicast;
	dst->control_url = xstrdup(src->control_url);
	dst->start_mode = src->start_mode;
	dst->range = src->range;
	dst->rtcp_xr = src->rtcp_xr;

	list_walk_entry_forward(&src->attrs, _attr, node)
	{
		err = sdp_session_attr_add(dst, &attr);
		if (err < 0)
			return err;
		err = sdp_attr_copy(_attr, attr);
		if (err < 0)
			return err;
	}
	dst->attr_count = src->attr_count;

	list_walk_entry_forward(&src->medias, _media, node)
	{
		err = sdp_session_media_add(dst, &media);
		if (err < 0)
			return err;
		err = sdp_media_copy(_media, media);
		if (err < 0)
			return err;
	}
	dst->media_count = src->media_count;

	return 0;
}


int sdp_session_compare(const struct sdp_session *a,
			const struct sdp_session *b)
{
	int url_match = 0;
	struct sdp_media *a_media = NULL;
	struct sdp_media *b_media = NULL;

	if (a == NULL && b == NULL) {
		/* 2 NULL input SDP are considered identical */
		return 0;
	} else if (a == NULL || b == NULL) {
		/* If one of the input sdp is NULL, they differ */
		return 1;
	}

	if (a->media_count != b->media_count) {
		/* Different media nb makes sdp different */
		return 1;
	}

	/* Compare the session name */
	if (a->session_name && b->session_name) {
		if (strcmp(a->session_name, b->session_name)) {
			/* Different session name makes sdp different */
			return 1;
		}
	} else if ((a->session_name != NULL) != (b->session_name != NULL)) {
		/* Only one of the session name is NULL */
		return 1;
	}

	/* Compare each media url */
	list_walk_entry_forward(&a->medias, a_media, node)
	{
		url_match = 0;
		b_media = NULL;
		list_walk_entry_forward(&b->medias, b_media, node)
		{
			if (a_media->control_url == NULL &&
			    b_media->control_url == NULL) {
				url_match = 1;
				break;
			} else if (a_media->control_url == NULL ||
				   b_media->control_url == NULL) {
				/* Control url differs since one of them is
				 * NULL */
				continue;
			}
			if (!strcmp(a_media->control_url,
				    b_media->control_url)) {
				/* Control url are identical */
				url_match = 1;
				break;
			}
		}
		if (!url_match) {
			/* Media control url differs */
			return 1;
		} else if (a_media->type != b_media->type) {
			/* Media type differs */
			return 1;
		}
	}
	/* Every parameter we checked were equal, so are the SDPs */
	return 0;
}


struct sdp_media *sdp_media_new(void)
{
	struct sdp_media *media = calloc(1, sizeof(*media));
	ULOG_ERRNO_RETURN_VAL_IF(media == NULL, ENOMEM, NULL);
	list_node_unref(&media->node);
	list_init(&media->attrs);

	return media;
}


int sdp_media_destroy(struct sdp_media *media)
{
	struct sdp_attr *attr = NULL, *tmp_attr = NULL;

	if (media == NULL)
		return 0;

	ULOG_ERRNO_RETURN_ERR_IF(list_node_is_ref(&media->node), EBUSY);

	/* Remove all attributes */
	list_walk_entry_forward_safe(&media->attrs, attr, tmp_attr, node)
	{
		sdp_media_attr_remove(media, attr);
	}

	free(media->media_title);
	free(media->connection_addr);
	free(media->control_url);

//	free(media->encoding_name);
//	free(media->encoding_params);
	free(media->payload_type_str);
	for(int i = 0; i < media->payload_type_array_count; i++) {
		struct sdp_payload *payload = &(media->payload_type_array[i]);
		free(payload->encoding_name);
		free(payload->encoding_params);
		free(payload->fmtp);
	}

	free(media->h264_fmtp.sps);
	free(media->h264_fmtp.pps);
	free(media);

	return 0;
}


int sdp_media_copy(const struct sdp_media *src, struct sdp_media *dst)
{
	struct sdp_attr *attr, *_attr = NULL;
	int err;

	ULOG_ERRNO_RETURN_ERR_IF(src == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(dst == NULL, EINVAL);

	dst->type = src->type;
	dst->media_title = xstrdup(src->media_title);
	dst->connection_addr = xstrdup(src->connection_addr);
	dst->multicast = src->multicast;
	dst->dst_stream_port = src->dst_stream_port;
	dst->dst_control_port = src->dst_control_port;
//	dst->payload_type = src->payload_type;
	dst->payload_type_str = xstrdup(src->payload_type_str);
	dst->control_url = xstrdup(src->control_url);
	dst->start_mode = src->start_mode;
	dst->range = src->range;
//	dst->encoding_name = xstrdup(src->encoding_name);
//	dst->encoding_params = xstrdup(src->encoding_params);
//	dst->clock_rate = src->clock_rate;

	dst->payload_type_array_count = src->payload_type_array_count;
	for(int i = 0; i < dst->payload_type_array_count; i++) {
		struct sdp_payload *src_payload = &(src->payload_type_array[i]);
		struct sdp_payload *dst_payload = &(dst->payload_type_array[i]);

		dst_payload->encoding_name = xstrdup(src_payload->encoding_name);
		dst_payload->encoding_params = xstrdup(src_payload->encoding_params);
		dst_payload->clock_rate = src_payload->clock_rate;
		dst_payload->fmtp = xstrdup(src_payload->fmtp);
	}

	dst->h264_fmtp = src->h264_fmtp;
	dst->h264_fmtp.sps = NULL;
	dst->h264_fmtp.sps_size = 0;
	if (src->h264_fmtp.sps_size > 0) {
		dst->h264_fmtp.sps = calloc(1, src->h264_fmtp.sps_size);
		ULOG_ERRNO_RETURN_ERR_IF(dst->h264_fmtp.sps == NULL, ENOMEM);
		memcpy(dst->h264_fmtp.sps,
		       src->h264_fmtp.sps,
		       src->h264_fmtp.sps_size);
		dst->h264_fmtp.sps_size = src->h264_fmtp.sps_size;
	}
	dst->h264_fmtp.pps = NULL;
	dst->h264_fmtp.pps_size = 0;
	if (src->h264_fmtp.pps_size > 0) {
		dst->h264_fmtp.pps = calloc(1, src->h264_fmtp.pps_size);
		ULOG_ERRNO_RETURN_ERR_IF(dst->h264_fmtp.pps == NULL, ENOMEM);
		memcpy(dst->h264_fmtp.pps,
		       src->h264_fmtp.pps,
		       src->h264_fmtp.pps_size);
		dst->h264_fmtp.pps_size = src->h264_fmtp.pps_size;
	}
	dst->rtcp_xr = src->rtcp_xr;


	list_walk_entry_forward(&src->attrs, _attr, node)
	{
		err = sdp_media_attr_add(dst, &attr);
		if (err < 0)
			return err;
		err = sdp_attr_copy(_attr, attr);
		if (err < 0)
			return err;
	}
	dst->attr_count = src->attr_count;

	return 0;
}


struct sdp_attr *sdp_attr_new(void)
{
	struct sdp_attr *attr = calloc(1, sizeof(*attr));
	ULOG_ERRNO_RETURN_VAL_IF(attr == NULL, ENOMEM, NULL);
	list_node_unref(&attr->node);

	return attr;
}


int sdp_attr_destroy(struct sdp_attr *attr)
{
	if (attr == NULL)
		return 0;

	ULOG_ERRNO_RETURN_ERR_IF(list_node_is_ref(&attr->node), EBUSY);

	free(attr->key);
	free(attr->value);
	free(attr);

	return 0;
}


int sdp_attr_copy(const struct sdp_attr *src, struct sdp_attr *dst)
{
	ULOG_ERRNO_RETURN_ERR_IF(src == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(dst == NULL, EINVAL);

	dst->key = xstrdup(src->key);
	dst->value = xstrdup(src->value);

	return 0;
}


int sdp_session_attr_add(struct sdp_session *session, struct sdp_attr **ret_obj)
{
	struct sdp_attr *attr = NULL;

	ULOG_ERRNO_RETURN_ERR_IF(session == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(ret_obj == NULL, EINVAL);

	attr = sdp_attr_new();
	ULOG_ERRNO_RETURN_ERR_IF(attr == NULL, ENOMEM);

	/* Add to the list */
	list_add_after(list_last(&session->attrs), &attr->node);
	session->attr_count++;

	*ret_obj = attr;
	return 0;
}


int sdp_session_attr_add_existing(struct sdp_session *session,
				  struct sdp_attr *attr)
{
	ULOG_ERRNO_RETURN_ERR_IF(session == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(attr == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(list_node_is_ref(&attr->node), EBUSY);

	/* Add to the list */
	list_add_after(list_last(&session->attrs), &attr->node);
	session->attr_count++;

	return 0;
}


int sdp_session_attr_remove(struct sdp_session *session, struct sdp_attr *attr)
{
	int err, found = 0;
	struct sdp_attr *_attr = NULL;

	ULOG_ERRNO_RETURN_ERR_IF(session == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(attr == NULL, EINVAL);

	list_walk_entry_forward(&session->attrs, _attr, node)
	{
		if (_attr == attr) {
			found = 1;
			break;
		}
	}

	if (!found) {
		ULOGE("%s: failed to find the attribute in the list", __func__);
		return -ENOENT;
	}

	/* Remove from the list */
	list_del(&attr->node);
	session->attr_count--;

	err = sdp_attr_destroy(attr);
	if (err < 0)
		return err;

	return 0;
}


int sdp_session_media_add(struct sdp_session *session,
			  struct sdp_media **ret_obj)
{
	struct sdp_media *media = NULL;

	ULOG_ERRNO_RETURN_ERR_IF(session == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(ret_obj == NULL, EINVAL);

	media = sdp_media_new();
	ULOG_ERRNO_RETURN_ERR_IF(media == NULL, ENOMEM);

	/* Add to the list */
	list_add_after(list_last(&session->medias), &media->node);
	session->media_count++;

	*ret_obj = media;
	return 0;
}


int sdp_session_media_add_existing(struct sdp_session *session,
				   struct sdp_media *media)
{
	ULOG_ERRNO_RETURN_ERR_IF(session == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(media == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(list_node_is_ref(&media->node), EBUSY);

	/* Add to the list */
	list_add_after(list_last(&session->medias), &media->node);
	session->media_count++;

	return 0;
}


int sdp_session_media_remove(struct sdp_session *session,
			     struct sdp_media *media)
{
	int err, found = 0;
	struct sdp_media *_media = NULL;

	ULOG_ERRNO_RETURN_ERR_IF(session == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(media == NULL, EINVAL);

	list_walk_entry_forward(&session->medias, _media, node)
	{
		if (_media == media) {
			found = 1;
			break;
		}
	}

	if (!found) {
		ULOGE("%s: failed to find the media in the list", __func__);
		return -ENOENT;
	}

	/* Remove from the list */
	list_del(&media->node);
	session->media_count--;

	err = sdp_media_destroy(media);
	if (err < 0)
		return err;

	return 0;
}


int sdp_media_attr_add(struct sdp_media *media, struct sdp_attr **ret_obj)
{
	struct sdp_attr *attr = NULL;

	ULOG_ERRNO_RETURN_ERR_IF(media == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(ret_obj == NULL, EINVAL);

	attr = sdp_attr_new();
	ULOG_ERRNO_RETURN_ERR_IF(attr == NULL, ENOMEM);

	/* Add to the list */
	list_add_after(list_last(&media->attrs), &attr->node);
	media->attr_count++;

	*ret_obj = attr;
	return 0;
}


int sdp_media_attr_add_existing(struct sdp_media *media, struct sdp_attr *attr)
{
	ULOG_ERRNO_RETURN_ERR_IF(media == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(attr == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(list_node_is_ref(&attr->node), EBUSY);

	/* Add to the list */
	list_add_after(list_last(&media->attrs), &attr->node);
	media->attr_count++;

	return 0;
}


int sdp_media_attr_remove(struct sdp_media *media, struct sdp_attr *attr)
{
	int err, found = 0;
	struct sdp_attr *_attr = NULL;

	ULOG_ERRNO_RETURN_ERR_IF(media == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(attr == NULL, EINVAL);

	list_walk_entry_forward(&media->attrs, _attr, node)
	{
		if (_attr == attr) {
			found = 1;
			break;
		}
	}

	if (!found) {
		ULOGE("%s: failed to find the attribute in the list", __func__);
		return -ENOENT;
	}

	/* Remove from the list */
	list_del(&attr->node);
	media->attr_count--;

	err = sdp_attr_destroy(attr);
	if (err < 0)
		return err;

	return 0;
}


static int sdp_time_write(const struct sdp_time *time, struct sdp_string *sdp)
{
	int ret;

	switch (time->format) {
	case SDP_TIME_FORMAT_NPT:
		if (time->npt.now) {
			CHECK_FUNC(sdp_sprintf,
				   ret,
				   return ret,
				   sdp,
				   SDP_TIME_NPT_NOW);
		} else {
			ULOG_ERRNO_RETURN_ERR_IF(time->npt.infinity, EINVAL);
			unsigned int hrs, min;
			unsigned int sec =
				time->npt.sec + time->npt.usec / 1000000;
			hrs = sec / (60 * 60);
			min = sec / 60 - hrs * 60;
			sec = sec - min * 60 - hrs * 60 * 60;
			float usec = (float)time->npt.usec / 1000000. -
				     (time->npt.usec / 1000000);
			char fraction[6];
			snprintf(fraction, sizeof(fraction), "%.3f", usec);
			if ((min > 0) || (hrs > 0)) {
				CHECK_FUNC(sdp_sprintf,
					   ret,
					   return ret,
					   sdp,
					   "%u:%02u:%02u%s",
					   hrs,
					   min,
					   sec,
					   (usec != 0.) ? fraction + 1 : "");
			} else {
				CHECK_FUNC(sdp_sprintf,
					   ret,
					   return ret,
					   sdp,
					   "%u%s",
					   sec,
					   (usec != 0.) ? fraction + 1 : "");
			}
		}
		break;
	case SDP_TIME_FORMAT_SMPTE:
	case SDP_TIME_FORMAT_ABSOLUTE:
		/* TODO */
		ULOGE("unsupported time format: %d", time->format);
		return -ENOSYS;
	default:
		ULOGE("unknown time format: %d", time->format);
		return -EINVAL;
	}

	return 0;
}


static int sdp_time_read(struct sdp_time *time, char *value)
{
	char *s;

	switch (time->format) {
	case SDP_TIME_FORMAT_NPT:
		s = strchr(value, ':');
		if (s != NULL) {
			/* Hours, minutes, seconds */
			char *hrs_str = value;
			*s = '\0';
			char *min_str = s + 1;
			s = strchr(min_str, ':');
			ULOG_ERRNO_RETURN_ERR_IF(s == NULL, EINVAL);
			*s = '\0';
			char *sec_str = s + 1;
			unsigned int hrs = atoi(hrs_str);
			unsigned int min = atoi(min_str);
			float sec_f = atof(sec_str);
			time->npt.sec = (time_t)sec_f;
			time->npt.sec += min * 60 + hrs * 60 * 60;
			time->npt.usec = (uint32_t)(
				(sec_f - (float)((unsigned int)sec_f)) *
				1000000);
		} else {
			if (strcmp(value, SDP_TIME_NPT_NOW) == 0) {
				/* now */
				time->npt.now = 1;
			} else {
				/* seconds only */
				float sec = atof(value);
				time->npt.sec = (time_t)sec;
				time->npt.usec = (uint32_t)(
					(sec - (float)time->npt.sec) * 1000000);
			}
		}
		break;
	case SDP_TIME_FORMAT_SMPTE:
	case SDP_TIME_FORMAT_ABSOLUTE:
		/* TODO */
		ULOGE("unsupported time format: %d", time->format);
		return -ENOSYS;
	default:
		ULOGE("unknown time format: %d", time->format);
		return -EINVAL;
	}

	return 0;
}


static int sdp_range_attr_write(const struct sdp_range *range,
				struct sdp_string *sdp)
{
	int ret;

	CHECK_FUNC(sdp_sprintf,
		   ret,
		   return ret,
		   sdp,
		   "%c=%s:",
		   SDP_TYPE_ATTRIBUTE,
		   SDP_ATTR_RANGE);

	switch (range->start.format) {
	case SDP_TIME_FORMAT_NPT:
		/* Start and stop cannot be both infinity */
		ULOG_ERRNO_RETURN_ERR_IF(range->start.npt.infinity &&
						 range->stop.npt.infinity,
					 EINVAL);
		/* 'now' makes no sense in SDP */
		ULOG_ERRNO_RETURN_ERR_IF(range->start.npt.now, EINVAL);
		ULOG_ERRNO_RETURN_ERR_IF(range->stop.npt.now, EINVAL);
		CHECK_FUNC(sdp_sprintf, ret, return ret, sdp, SDP_TIME_NPT "=");
		if (!range->start.npt.infinity) {
			ret = sdp_time_write(&range->start, sdp);
			if (ret < 0)
				return ret;
		}
		CHECK_FUNC(sdp_sprintf, ret, return ret, sdp, "-");
		if (!range->stop.npt.infinity) {
			ret = sdp_time_write(&range->stop, sdp);
			if (ret < 0)
				return ret;
		}
		break;
	case SDP_TIME_FORMAT_SMPTE:
	case SDP_TIME_FORMAT_ABSOLUTE:
		/* TODO */
		ULOGE("unsupported time format: %d", range->start.format);
		return -ENOSYS;
	default:
		ULOGE("unknown time format: %d", range->start.format);
		return -EINVAL;
	}

	CHECK_FUNC(sdp_sprintf, ret, return ret, sdp, SDP_CRLF);

	return 0;
}


static int sdp_range_attr_read(struct sdp_range *range, char *value)
{
	int err;
	char *s;
	char *start_str = NULL, *stop_str = NULL;

	memset(range, 0, sizeof(*range));

	s = strchr(value, '=');
	ULOG_ERRNO_RETURN_ERR_IF(s == NULL, EINVAL);

	*s = '\0';
	start_str = s + 1;

	s = strchr(start_str, '-');
	ULOG_ERRNO_RETURN_ERR_IF(s == NULL, EINVAL);

	*s = '\0';
	stop_str = s + 1;

	if (strcmp(value, SDP_TIME_NPT) == 0) {
		/* Normal Play Time (NPT) */
		range->start.format = SDP_TIME_FORMAT_NPT;
		range->stop.format = SDP_TIME_FORMAT_NPT;
		if (strlen(start_str)) {
			err = sdp_time_read(&range->start, start_str);
			if (err < 0)
				return -err;
		} else {
			range->start.npt.infinity = 1;
		}
		if (strlen(stop_str)) {
			err = sdp_time_read(&range->stop, stop_str);
			if (err < 0)
				return -err;
		} else {
			range->stop.npt.infinity = 1;
		}

	} else if (strcmp(value, SDP_TIME_SMPTE) == 0) {
		/* SMPTE Relative Timestamps */
		range->start.format = SDP_TIME_FORMAT_SMPTE;
		range->stop.format = SDP_TIME_FORMAT_SMPTE;
		/* TODO*/
		ULOGE("unsupported time format: %s", value);
		return -ENOSYS;

	} else if (strcmp(value, SDP_TIME_ABSOLUTE) == 0) {
		/* Absolute Time (UTC, ISO 8601) */
		range->start.format = SDP_TIME_FORMAT_ABSOLUTE;
		range->stop.format = SDP_TIME_FORMAT_ABSOLUTE;
		/* TODO*/
		ULOGE("unsupported time format: %s", value);
		return -ENOSYS;

	} else {
		ULOGE("unknown time format: %s", value);
		return -EINVAL;
	}

	ULOG_ERRNO_RETURN_ERR_IF(range->start.format != range->stop.format,
				 EINVAL);
	/* Start and stop cannot be both infinity */
	ULOG_ERRNO_RETURN_ERR_IF(
		range->start.npt.infinity && range->stop.npt.infinity, EINVAL);
	/* 'now' makes no sense in SDP */
	ULOG_ERRNO_RETURN_ERR_IF(range->start.npt.now, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(range->stop.npt.now, EINVAL);

	return 0;
}


static int sdp_h264_fmtp_write(const struct sdp_h264_fmtp *fmtp,
			       unsigned int payload_type,
			       struct sdp_string *sdp)
{
	int ret = 0;
	char *sps_b64 = NULL;
	char *pps_b64 = NULL;

	CHECK_FUNC(sdp_sprintf,
		   ret,
		   return ret,
		   sdp,
		   "%c=%s:%d ",
		   SDP_TYPE_ATTRIBUTE,
		   SDP_ATTR_FMTP,
		   payload_type);

	/* packetization-mode */
	CHECK_FUNC(sdp_sprintf,
		   ret,
		   return ret,
		   sdp,
		   "%s=%d;",
		   SDP_FMTP_H264_PACKETIZATION,
		   fmtp->packetization_mode);

	/* profile-level-id */
	CHECK_FUNC(sdp_sprintf,
		   ret,
		   return ret,
		   sdp,
		   "%s=%02X%02X%02X;",
		   SDP_FMTP_H264_PROFILE_LEVEL,
		   fmtp->profile_idc,
		   fmtp->profile_iop,
		   fmtp->level_idc);

	/* sprop-parameter-sets */
	if ((fmtp->sps) && (fmtp->sps_size) && (fmtp->pps) &&
	    (fmtp->pps_size)) {
		ret = sdp_base64_encode(
			(void *)fmtp->sps, (size_t)fmtp->sps_size, &sps_b64);
		if (ret < 0)
			goto out;
		ret = sdp_base64_encode(
			(void *)fmtp->pps, (size_t)fmtp->pps_size, &pps_b64);
		if (ret < 0)
			goto out;
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto out,
			   sdp,
			   "%s=%s,%s;",
			   SDP_FMTP_H264_PARAM_SETS,
			   sps_b64,
			   pps_b64);
	}

	CHECK_FUNC(sdp_sprintf, ret, goto out, sdp, SDP_CRLF);

out:
	free(sps_b64);
	free(pps_b64);
	return ret;
}


/* NB: the data pointed by 'value' will be modified */
static int sdp_h264_fmtp_read(struct sdp_h264_fmtp *fmtp, char *value)
{
	int ret;
	char *temp1 = NULL;
	char *param = NULL;
	char *val = NULL;

	fmtp->valid = 0;
	param = strtok_r(value, ";", &temp1);
	while (param) {
		val = strchr(param, '=');
		if (val != NULL) {
			*val = '\0';
			val++;
		}
		if ((strcmp(param, SDP_FMTP_H264_PROFILE_LEVEL) == 0) &&
		    (val)) {
			/* profile-level-id */
			uint32_t profile_level_id = 0;
			sscanf(val, "%6X", &profile_level_id);
			fmtp->profile_idc = (profile_level_id >> 16) & 0xFF;
			fmtp->profile_iop = (profile_level_id >> 8) & 0xFF;
			fmtp->level_idc = profile_level_id & 0xFF;

		} else if ((strcmp(param, SDP_FMTP_H264_PACKETIZATION) == 0) &&
			   (val)) {
			/* packetization-mode */
			fmtp->packetization_mode = atoi(val);

		} else if ((strcmp(param, SDP_FMTP_H264_PARAM_SETS) == 0) &&
			   (val)) {
			/* sprop-parameter-sets */
			char *p3 = NULL;
			p3 = strchr(val, ',');
			if (p3 != NULL) {
				*p3 = '\0';
				char *sps_b64 = val;
				char *pps_b64 = p3 + 1;
				void *sps = NULL;
				size_t sps_size = 0;
				void *pps = NULL;
				size_t pps_size = 0;
				ret = sdp_base64_decode(
					sps_b64, &sps, &sps_size);
				if (ret < 0) {
					free(sps);
					free(pps);
					return ret;
				}
				ret = sdp_base64_decode(
					pps_b64, &pps, &pps_size);
				if (ret < 0) {
					free(sps);
					free(pps);
					return ret;
				}
				fmtp->sps = (uint8_t *)sps;
				fmtp->sps_size = (unsigned int)sps_size;
				fmtp->pps = (uint8_t *)pps;
				fmtp->pps_size = (unsigned int)pps_size;
			}
		}

		param = strtok_r(NULL, ";", &temp1);
	};

	fmtp->valid = 1;
	return 0;
}


static int sdp_rtcp_xr_attr_write(const struct sdp_rtcp_xr *xr,
				  struct sdp_string *sdp)
{
	int ret, is_first = 1;

	if ((!xr->loss_rle_report) && (!xr->dup_rle_report) &&
	    (!xr->pkt_receipt_times_report) &&
	    ((xr->rtt_report <= SDP_RTCP_XR_RTT_REPORT_NONE) ||
	     (xr->rtt_report >= SDP_RTCP_XR_RTT_REPORT_MAX)) &&
	    (!xr->stats_summary_report_loss) &&
	    (!xr->stats_summary_report_dup) &&
	    (!xr->stats_summary_report_jitter) &&
	    (!xr->stats_summary_report_ttl) && (!xr->stats_summary_report_hl) &&
	    (!xr->voip_metrics_report) && (!xr->djb_metrics_report)) {
		return 0;
	}

	CHECK_FUNC(sdp_sprintf,
		   ret,
		   return ret,
		   sdp,
		   "%c=%s:",
		   SDP_TYPE_ATTRIBUTE,
		   SDP_ATTR_RTCP_XR);

	/* pkt-loss-rle */
	if (xr->loss_rle_report) {
		/* No need to test is_first here since it has its init value */
		if (xr->loss_rle_report_max_size > 0) {
			CHECK_FUNC(sdp_sprintf,
				   ret,
				   return ret,
				   sdp,
				   "%s=%d",
				   SDP_ATTR_RTCP_XR_LOSS_RLE,
				   xr->loss_rle_report_max_size);
		} else {
			CHECK_FUNC(sdp_sprintf,
				   ret,
				   return ret,
				   sdp,
				   "%s",
				   SDP_ATTR_RTCP_XR_LOSS_RLE);
		}
		is_first = 0;
	}

	/* pkt-dup-rle */
	if (xr->dup_rle_report) {
		if (xr->dup_rle_report_max_size > 0) {
			CHECK_FUNC(sdp_sprintf,
				   ret,
				   return ret,
				   sdp,
				   "%s%s=%d",
				   (is_first) ? "" : " ",
				   SDP_ATTR_RTCP_XR_DUP_RLE,
				   xr->dup_rle_report_max_size);
		} else {
			CHECK_FUNC(sdp_sprintf,
				   ret,
				   return ret,
				   sdp,
				   "%s%s",
				   (is_first) ? "" : " ",
				   SDP_ATTR_RTCP_XR_DUP_RLE);
		}
		is_first = 0;
	}

	/* pkt-rcpt-times */
	if (xr->pkt_receipt_times_report) {
		if (xr->pkt_receipt_times_report_max_size > 0) {
			CHECK_FUNC(sdp_sprintf,
				   ret,
				   return ret,
				   sdp,
				   "%s%s=%d",
				   (is_first) ? "" : " ",
				   SDP_ATTR_RTCP_XR_RCPT_TIMES,
				   xr->pkt_receipt_times_report_max_size);
		} else {
			CHECK_FUNC(sdp_sprintf,
				   ret,
				   return ret,
				   sdp,
				   "%s%s",
				   (is_first) ? "" : " ",
				   SDP_ATTR_RTCP_XR_RCPT_TIMES);
		}
		is_first = 0;
	}

	/* rcvr-rtt */
	if ((xr->rtt_report > SDP_RTCP_XR_RTT_REPORT_NONE) &&
	    (xr->rtt_report < SDP_RTCP_XR_RTT_REPORT_MAX)) {
		if (xr->loss_rle_report_max_size > 0) {
			CHECK_FUNC(
				sdp_sprintf,
				ret,
				return ret,
				sdp,
				"%s%s=%s:%d",
				(is_first) ? "" : " ",
				SDP_ATTR_RTCP_XR_RCVR_RTT,
				sdp_rtcp_xr_rtt_report_mode_str(xr->rtt_report),
				xr->loss_rle_report_max_size);
		} else {
			CHECK_FUNC(sdp_sprintf,
				   ret,
				   return ret,
				   sdp,
				   "%s%s=%s",
				   (is_first) ? "" : " ",
				   SDP_ATTR_RTCP_XR_RCVR_RTT,
				   sdp_rtcp_xr_rtt_report_mode_str(
					   xr->rtt_report));
		}
		is_first = 0;
	}

	/* stat-summary */
	if ((xr->stats_summary_report_loss) || (xr->stats_summary_report_dup) ||
	    (xr->stats_summary_report_jitter) ||
	    (xr->stats_summary_report_ttl) || (xr->stats_summary_report_hl)) {
		int is_first2 = 1;
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   return ret,
			   sdp,
			   "%s%s=",
			   (is_first) ? "" : " ",
			   SDP_ATTR_RTCP_XR_STAT_SUMMARY);
		is_first = 0;
		if (xr->stats_summary_report_loss) {
			/* No need to test is_first2 here since it has its init
			 * value */
			CHECK_FUNC(sdp_sprintf,
				   ret,
				   return ret,
				   sdp,
				   "%s",
				   SDP_ATTR_RTCP_XR_STAT_LOSS);
			is_first2 = 0;
		}
		if (xr->stats_summary_report_dup) {
			CHECK_FUNC(sdp_sprintf,
				   ret,
				   return ret,
				   sdp,
				   "%s%s",
				   (is_first2) ? "" : ",",
				   SDP_ATTR_RTCP_XR_STAT_DUP);
			is_first2 = 0;
		}
		if (xr->stats_summary_report_jitter) {
			CHECK_FUNC(sdp_sprintf,
				   ret,
				   return ret,
				   sdp,
				   "%s%s",
				   (is_first2) ? "" : ",",
				   SDP_ATTR_RTCP_XR_STAT_JITT);
			is_first2 = 0;
		}
		if (xr->stats_summary_report_ttl) {
			CHECK_FUNC(sdp_sprintf,
				   ret,
				   return ret,
				   sdp,
				   "%s%s",
				   (is_first2) ? "" : ",",
				   SDP_ATTR_RTCP_XR_STAT_TTL);
			is_first2 = 0;
		}
		if (xr->stats_summary_report_hl) {
			CHECK_FUNC(sdp_sprintf,
				   ret,
				   return ret,
				   sdp,
				   "%s%s",
				   (is_first2) ? "" : ",",
				   SDP_ATTR_RTCP_XR_STAT_HL);
			is_first2 = 0;
		}
	}

	/* voip-metrics */
	if (xr->voip_metrics_report) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   return ret,
			   sdp,
			   "%s%s",
			   (is_first) ? "" : " ",
			   SDP_ATTR_RTCP_XR_VOIP_METRICS);
		is_first = 0;
	}

	/* de-jitter-buffer */
	if (xr->djb_metrics_report) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   return ret,
			   sdp,
			   "%s%s",
			   (is_first) ? "" : " ",
			   SDP_ATTR_RTCP_XR_DJB_METRICS);
		is_first = 0;
	}

	CHECK_FUNC(sdp_sprintf, ret, return ret, sdp, SDP_CRLF);

	return 0;
}


/* NB: the data pointed by 'value' will be modified */
static int sdp_rtcp_xr_attr_read(struct sdp_rtcp_xr *xr, char *value)
{
	char *temp1 = NULL;
	char *xr_format = NULL;
	char *val = NULL;

	xr->valid = 0;
	xr_format = strtok_r(value, " ", &temp1);
	while (xr_format) {
		val = strchr(xr_format, '=');
		if (val != NULL) {
			*val = '\0';
			val++;
		}
		if (strcmp(xr_format, SDP_ATTR_RTCP_XR_LOSS_RLE) == 0) {
			/* pkt-loss-rle */
			xr->loss_rle_report = 1;
			if (val != NULL)
				xr->loss_rle_report_max_size = atoi(val);

		} else if (strcmp(xr_format, SDP_ATTR_RTCP_XR_DUP_RLE) == 0) {
			/* pkt-dup-rle */
			xr->dup_rle_report = 1;
			if (val != NULL)
				xr->dup_rle_report_max_size = atoi(val);

		} else if (strcmp(xr_format, SDP_ATTR_RTCP_XR_RCPT_TIMES) ==
			   0) {
			/* pkt-rcpt-times */
			xr->pkt_receipt_times_report = 1;
			if (val != NULL)
				xr->pkt_receipt_times_report_max_size =
					atoi(val);

		} else if (strcmp(xr_format, SDP_ATTR_RTCP_XR_RCVR_RTT) == 0) {
			/* rcvr-rtt */
			char *sz =
				(val != NULL) ? strchr(xr_format, ':') : NULL;
			if (sz != NULL) {
				*sz = '\0';
				sz++;
				xr->rtt_report_max_size = atoi(sz);
			}
			if ((val != NULL) &&
			    (strcmp(val, SDP_RTCP_XR_RTT_REPORT_ALL_STR) == 0))
				xr->rtt_report = SDP_RTCP_XR_RTT_REPORT_ALL;
			else if ((val != NULL) &&
				 (strcmp(val,
					 SDP_RTCP_XR_RTT_REPORT_SENDER_STR) ==
				  0))
				xr->rtt_report = SDP_RTCP_XR_RTT_REPORT_SENDER;
			else
				xr->rtt_report = SDP_RTCP_XR_RTT_REPORT_NONE;

		} else if (strcmp(xr_format, SDP_ATTR_RTCP_XR_STAT_SUMMARY) ==
			   0) {
			/* stat-summary */
			if (val == NULL)
				continue;
			char *temp2 = NULL;
			char *stat_flag = NULL;
			stat_flag = strtok_r(val, ",", &temp2);
			while (stat_flag) {
				if (strcmp(stat_flag,
					   SDP_ATTR_RTCP_XR_STAT_LOSS) == 0)
					xr->stats_summary_report_loss = 1;
				else if (strcmp(stat_flag,
						SDP_ATTR_RTCP_XR_STAT_DUP) == 0)
					xr->stats_summary_report_dup = 1;
				else if (strcmp(stat_flag,
						SDP_ATTR_RTCP_XR_STAT_JITT) ==
					 0)
					xr->stats_summary_report_jitter = 1;
				else if (strcmp(stat_flag,
						SDP_ATTR_RTCP_XR_STAT_TTL) == 0)
					xr->stats_summary_report_ttl = 1;
				else if (strcmp(stat_flag,
						SDP_ATTR_RTCP_XR_STAT_HL) == 0)
					xr->stats_summary_report_hl = 1;
				stat_flag = strtok_r(NULL, ",", &temp2);
			}

		} else if (strcmp(xr_format, SDP_ATTR_RTCP_XR_VOIP_METRICS) ==
			   0) {
			/* voip-metrics */
			xr->voip_metrics_report = 1;

		} else if (strcmp(xr_format, SDP_ATTR_RTCP_XR_DJB_METRICS) ==
			   0) {
			/* de-jitter-buffer */
			xr->djb_metrics_report = 1;
		}

		xr_format = strtok_r(NULL, " ", &temp1);
	};

	xr->valid = 1;
	return 0;
}


/* NB: the data pointed by 'value' will be modified */
static int sdp_attr_read(struct sdp_session *session,
			 struct sdp_media *media,
			 char *value,
			 struct sdp_attr **out_attr)
{
	int err;
	char *temp2 = NULL;
	struct sdp_attr *attr;
	char *attr_key = strtok_r(value, ":", &temp2);
	char *attr_value = strtok_r(NULL, "", &temp2);
	*out_attr = NULL;

	if (attr_key == NULL) {
		ULOGE("no attribute key");
		return -EPROTO;
	}

	if ((strcmp(attr_key, SDP_ATTR_RTPAVP_RTPMAP) == 0) && (attr_value)) {
		/* a=rtpmap */
		if (media == NULL) {
			ULOGE("RTPMAP: attribute 'rtpmap' not on media level");
			return -EPROTO;
		}
		char *temp3 = NULL;
		char *payload_type = NULL;
		char *encoding_name = NULL;
		char *clock_rate = NULL;
		char *encoding_params = NULL;
		payload_type = strtok_r(attr_value, " ", &temp3);
		unsigned int payload_type_int =
			(payload_type) ? atoi(payload_type) : 0;
		encoding_name = strtok_r(NULL, "/", &temp3);
		clock_rate = strtok_r(NULL, "/", &temp3);
		unsigned int i_clock_rate = (clock_rate) ? atoi(clock_rate) : 0;
		encoding_params = strtok_r(NULL, "/", &temp3);

		struct sdp_payload *payload = sdp_media_type_check_valid(payload_type_int, media);
		if( !payload ) {
			ULOGE("RTPMAP: invalid payload type (%d in %s)",
			      payload_type_int,
			      media->payload_type_str);
			return -EPROTO;
		}

		if (encoding_name == NULL) {
			ULOGE("RTPMAP: encoding name is missing");
			return -EPROTO;
		}
//		/* Clock rate must be 90000 for H.264
//		 * (RFC6184 ch. 8.2.1) */
//		if ((strcmp(encoding_name, SDP_ENCODING_H264) == 0) &&
//		    (i_clock_rate != SDP_H264_CLOCKRATE)) {
//			ULOGE("unsupported clock rate %d", i_clock_rate);
//			return -EPROTO;
//		}
		ULOGD("RTPMAP: payload_type=%d"
		      " encoding_name=%s clock_rate=%d"
		      " encoding_params=%s",
		      payload_type_int,
		      encoding_name,
		      i_clock_rate,
		      encoding_params);

		payload->encoding_name = strdup(encoding_name);
		payload->encoding_params = xstrdup(encoding_params);
		payload->clock_rate = i_clock_rate;

	} else if ((strcmp(attr_key, SDP_ATTR_FMTP) == 0) && (attr_value)) {
		/* a=fmtp */
		if (media == NULL) {
			ULOGE("FMTP: attribute 'fmtp' not on media level");
			return -EPROTO;
		}
		char *temp3 = NULL;
		char *payload_type = NULL;
		char *fmtp = NULL;
		payload_type = strtok_r(attr_value, " ", &temp3);
		unsigned int payload_type_int =
			(payload_type) ? atoi(payload_type) : 0;

		struct sdp_payload *payload = sdp_media_type_check_valid(payload_type_int, media);
		if( !payload ) {
			ULOGE("FMTP: invalid payload type (%d in %s)",
			      payload_type_int,
			      media->payload_type_str);
			return -EPROTO;
		}

		ULOGD("FMTP: payload_type:%d encoding_name:%s encoding_params:%s",
				payload->payload_type,
				payload->encoding_name,
				payload->encoding_params);

		// Mark by Max
//		if ((payload->encoding_name) &&
//				(strcmp(payload->encoding_name, SDP_ENCODING_H264) == 0)) {
//			ULOGD("FMTP: H264 Found, %s", temp3);
//
//			fmtp = strtok_r(NULL, "", &temp3);
//			err = sdp_h264_fmtp_read(&media->h264_fmtp, fmtp);
//			if (err < 0)
//				return err;
//		}

		payload->fmtp = xstrdup(temp3);

	} else if (strcmp(attr_key, SDP_ATTR_TOOL) == 0) {
		/* a=tool */
		if (media)
			ULOGW("attribute 'tool' not on session level");
		else
			session->tool = xstrdup(attr_value);
	} else if (strcmp(attr_key, SDP_ATTR_TYPE) == 0) {
		/* a=type */
		if (media)
			ULOGW("attribute 'type' not on session level");
		else
			session->type = xstrdup(attr_value);
	} else if (strcmp(attr_key, SDP_ATTR_CHARSET) == 0) {
		/* a=charset */
		if (media)
			ULOGW("attribute 'charset' not on session level");
		else
			session->charset = xstrdup(attr_value);
	} else if (strcmp(attr_key, SDP_ATTR_CONTROL_URL) == 0) {
		/* a=control */
		if (media)
			media->control_url = xstrdup(attr_value);
		else
			session->control_url = xstrdup(attr_value);
	} else if ((strcmp(attr_key, SDP_ATTR_RANGE) == 0) && (attr_value)) {
		/* a=range */
		if (media)
			err = sdp_range_attr_read(&media->range, attr_value);
		else
			err = sdp_range_attr_read(&session->range, attr_value);
		if (err < 0)
			return err;

	} else if (strcmp(attr_key, SDP_ATTR_RECVONLY) == 0) {
		/* a=recvonly */
		if (media)
			media->start_mode = SDP_START_MODE_RECVONLY;
		else
			session->start_mode = SDP_START_MODE_RECVONLY;

	} else if (strcmp(attr_key, SDP_ATTR_SENDRECV) == 0) {
		/* a=sendrecv */
		if (media)
			media->start_mode = SDP_START_MODE_SENDRECV;
		else
			session->start_mode = SDP_START_MODE_SENDRECV;

	} else if (strcmp(attr_key, SDP_ATTR_SENDONLY) == 0) {
		/* a=sendonly */
		if (media)
			media->start_mode = SDP_START_MODE_SENDONLY;
		else
			session->start_mode = SDP_START_MODE_SENDONLY;

	} else if (strcmp(attr_key, SDP_ATTR_INACTIVE) == 0) {
		/* a=inactive */
		if (media)
			media->start_mode = SDP_START_MODE_INACTIVE;
		else
			session->start_mode = SDP_START_MODE_INACTIVE;

	} else if ((strcmp(attr_key, SDP_ATTR_RTCP_XR) == 0) && (attr_value)) {
		/* a=rtcp-xr */
		if (media) {
			err = sdp_rtcp_xr_attr_read(&media->rtcp_xr,
						    attr_value);
		} else {
			err = sdp_rtcp_xr_attr_read(&session->rtcp_xr,
						    attr_value);
		}
		if (err < 0)
			return err;

	} else if ((strcmp(attr_key, SDP_ATTR_RTCP_PORT) == 0) &&
		   (attr_value)) {
		/* a=rtcp */
		if (media == NULL) {
			ULOGE("attribute 'rtcp' not on media level");
			return -EPROTO;
		}
		int port = atoi(attr_value);
		if (port > 0) {
			media->dst_control_port = port;
			ULOGD("SDP: rtcp_dst_port=%d", port);
		}
	} else {
		/* No special case, create an sdp_attr for the key/value pair */
		attr = sdp_attr_new();
		if (attr == NULL) {
			ULOGE("new SDP attribute creation failed");
			return -ENOMEM;
		}
		attr->key = xstrdup(attr_key);
		attr->value = xstrdup(attr_value);
		*out_attr = attr;
	}

	return 0;
}


static int sdp_media_write(const struct sdp_media *media,
			   struct sdp_string *sdp,
			   int session_level_connection_addr)
{
	int ret;

	if (((!media->connection_addr) || (*media->connection_addr == '\0')) &&
	    (!session_level_connection_addr)) {
		ULOGE("invalid connection address");
		return -EINVAL;
	}
//	if (!media->payload_type_str) {
//		ULOGE("invalid payload type");
//		return -EINVAL;
//	}
	if (media->type >= SDP_MEDIA_TYPE_MAX) {
		ULOGE("invalid media type");
		return -EINVAL;
	}
//	if ((!media->encoding_name) || (*media->encoding_name == '\0')) {
//		ULOGE("invalid encoding name");
//		return -EINVAL;
//	}

	/* Media description (m=<media> <port> <proto> <fmt> ...) */
	CHECK_FUNC(sdp_sprintf,
		   ret,
		   return ret,
		   sdp,
		   "%c=%s %d " SDP_PROTO_RTPAVP " %d" SDP_CRLF,
		   SDP_TYPE_MEDIA,
		   sdp_media_type_str(media->type),
		   media->dst_stream_port,
		   "H264");

	/* Media title (i=<media title>) */
	if ((media->media_title) && (*media->media_title != '\0')) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   return ret,
			   sdp,
			   "%c=%s" SDP_CRLF,
			   SDP_TYPE_INFORMATION,
			   media->media_title);
	}

	/* Connection data (c=<nettype> <addrtype> <connection-address>) */
	if ((media->connection_addr) && (*media->connection_addr != '\0')) {
		int multicast = 0;
		int addr_first = atoi(media->connection_addr);
		if ((addr_first >= SDP_MULTICAST_ADDR_MIN) &&
		    (addr_first <= SDP_MULTICAST_ADDR_MAX))
			multicast = 1;
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   return ret,
			   sdp,
			   "%c=IN IP4 %s%s" SDP_CRLF,
			   SDP_TYPE_CONNECTION,
			   media->connection_addr,
			   (multicast) ? "/127" : "");
	}

	/* Start mode (a=<start_mode>) */
	if ((media->start_mode > SDP_START_MODE_UNSPECIFIED) &&
	    (media->start_mode < SDP_START_MODE_MAX)) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   return ret,
			   sdp,
			   "%c=%s" SDP_CRLF,
			   SDP_TYPE_ATTRIBUTE,
			   sdp_start_mode_str(media->start_mode));
	}

	/* Control URL for use with RTSP (a=control) */
	if ((media->control_url) && (*media->control_url != '\0')) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   return ret,
			   sdp,
			   "%c=%s:%s" SDP_CRLF,
			   SDP_TYPE_ATTRIBUTE,
			   SDP_ATTR_CONTROL_URL,
			   media->control_url);
	}

	/* Range (a=range) */
	if (media->range.start.format != SDP_TIME_FORMAT_UNKNOWN) {
		ret = sdp_range_attr_write(&media->range, sdp);
		if (ret < 0)
			return ret;
	}

	/* RTP/AVP rtpmap attribute (a=rtpmap) */
	CHECK_FUNC(
		sdp_sprintf,
		ret,
		return ret,
		sdp,
		"%c=%s:%d %s/%d%s%s" SDP_CRLF,
		SDP_TYPE_ATTRIBUTE,
		SDP_ATTR_RTPAVP_RTPMAP,
//		media->payload_type,
		102,
//		media->encoding_name,
		"H264",
//		media->clock_rate,
		90000,
//		((media->encoding_params) && (*media->encoding_params != '\0'))
//			? "/"
//			: "",
		"",
//		((media->encoding_params) && (*media->encoding_params != '\0'))
//			? media->encoding_params
//			: ""
		""
		);

	/* H.264 payload format parameters (a=fmtp) */
	if ((strcmp("H264", SDP_ENCODING_H264) == 0) &&
	    (media->h264_fmtp.valid)) {
		ret = sdp_h264_fmtp_write(
			&media->h264_fmtp,
//			media->payload_type,
			102,
			sdp
			);
		if (ret < 0)
			return ret;
	}

	/* RTCP destination port (if not RTP port + 1) (a=rtcp) */
	if ((media->dst_stream_port != 0) &&
	    (media->dst_control_port != media->dst_stream_port + 1)) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   return ret,
			   sdp,
			   "%c=%s:%d" SDP_CRLF,
			   SDP_TYPE_ATTRIBUTE,
			   SDP_ATTR_RTCP_PORT,
			   media->dst_control_port);
	}

	/* RTCP extended reports attribute (a=rtcp-xr) */
	if (media->rtcp_xr.valid) {
		ret = sdp_rtcp_xr_attr_write(&media->rtcp_xr, sdp);
		if (ret < 0)
			return ret;
	}

	/* Other attributes (a=<attribute>:<value> or a=<attribute>) */
	struct sdp_attr *attr = NULL;
	list_walk_entry_forward(&media->attrs, attr, node)
	{
		if ((attr->key) && (*attr->key != '\0')) {
			if ((attr->value) && (*attr->value != '\0')) {
				CHECK_FUNC(sdp_sprintf,
					   ret,
					   return ret,
					   sdp,
					   "%c=%s:%s" SDP_CRLF,
					   SDP_TYPE_ATTRIBUTE,
					   attr->key,
					   attr->value);
			} else {
				CHECK_FUNC(sdp_sprintf,
					   ret,
					   return ret,
					   sdp,
					   "%c=%s" SDP_CRLF,
					   SDP_TYPE_ATTRIBUTE,
					   attr->key);
			}
		}
	}

	return 0;
}


/* NB: the data pointed by 'value' will be modified */
static int sdp_media_read(struct sdp_media *media, char *value)
{
	char *temp2 = NULL;
	char *smedia = strtok_r(value, " ", &temp2);
	char *port = strtok_r(NULL, " ", &temp2);
	char *proto = strtok_r(NULL, " ", &temp2);

	if( temp2 ) {
		media->payload_type_str = strdup(temp2);
	}

	if (smedia) {
		int res = sdp_media_type_from_str(smedia, &media->type);
		if (res < 0) {
			ULOGE("unsupported media type '%s'", smedia);
			return -EPROTO;
		}
	} else {
		ULOGE("null media type");
		return -EPROTO;
	}
	int port_int = (port) ? atoi(port) : 0;
	if (port_int) {
		media->dst_stream_port = port_int;
		media->dst_control_port = port_int + 1;
	} else {
		media->dst_control_port = 0;
	}
	if ((proto == NULL) || (strcmp(proto, SDP_PROTO_RTPAVP) != 0)) {
		ULOGE("unsupported protocol '%s'", (proto) ? proto : "");
		return -EPROTO;
	}

	char *fmt = strtok_r(NULL, " ", &temp2);
	int payload_type = 0;
	int i = 0;
	while( fmt && i < SDP_MAX_PAYLOAD_TYPE_COUNT) {
		/* Payload type must be dynamic
		 * (RFC3551 ch. 6) */
		payload_type = (fmt) ? atoi(fmt) : 0;
		if ((payload_type < SDP_DYNAMIC_PAYLOAD_TYPE_MIN) ||
		    (payload_type > SDP_DYNAMIC_PAYLOAD_TYPE_MAX)) {
			ULOGE("unsupported payload type (%d)", payload_type);
			return -EPROTO;
		}

		struct sdp_payload *payload = &(media->payload_type_array[i++]);
		payload->payload_type = payload_type;

		fmt = strtok_r(NULL, " ", &temp2);
	}
	media->payload_type_array_count = i;

	ULOGD("SDP: media=%s port=%d proto=%s payload_type_str=%s",
	      smedia,
	      port_int,
	      proto,
		  media->payload_type_str
		  );

	return 0;
}


int sdp_description_write(const struct sdp_session *session, char **ret_str)
{
	int ret;
	int session_level_connection_addr = 0;
	struct sdp_string sdp;
	struct sdp_attr *attr = NULL;
	struct sdp_media *media = NULL;

	ULOG_ERRNO_RETURN_ERR_IF(session == NULL, EINVAL);

	if ((!session->server_addr) || (*session->server_addr == '\0')) {
		ULOGE("invalid server address");
		return -EINVAL;
	}

	sdp.str = malloc(SDP_DEFAULT_LEN);
	ULOG_ERRNO_RETURN_ERR_IF(sdp.str == NULL, ENOMEM);
	sdp.len = 0;
	sdp.max_len = SDP_DEFAULT_LEN;

	if (session->deletion) {
		/* Origin (o=<username> <sess-id> <sess-version>
		 * <nettype> <addrtype> <unicast-address>) */
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto error,
			   &sdp,
			   "%c=- %" PRIu64 " %" PRIu64 " IN IP4 %s" SDP_CRLF,
			   SDP_TYPE_ORIGIN,
			   session->session_id,
			   session->session_version,
			   session->server_addr);

		*ret_str = sdp.str;
		return 0;
	}

	/* Protocol version (v=0) */
	CHECK_FUNC(sdp_sprintf,
		   ret,
		   goto error,
		   &sdp,
		   "%c=%d" SDP_CRLF,
		   SDP_TYPE_VERSION,
		   SDP_VERSION);

	/* Origin (o=<username> <sess-id> <sess-version>
	 * <nettype> <addrtype> <unicast-address>) */
	CHECK_FUNC(sdp_sprintf,
		   ret,
		   goto error,
		   &sdp,
		   "%c=- %" PRIu64 " %" PRIu64 " IN IP4 %s" SDP_CRLF,
		   SDP_TYPE_ORIGIN,
		   session->session_id,
		   session->session_version,
		   session->server_addr);

	/* Session name (s=<session name>) */
	if ((session->session_name) && (*session->session_name != '\0')) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto error,
			   &sdp,
			   "%c=%s" SDP_CRLF,
			   SDP_TYPE_SESSION_NAME,
			   session->session_name);
	} else {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto error,
			   &sdp,
			   "%c= " SDP_CRLF,
			   SDP_TYPE_SESSION_NAME);
	}

	/* Session information (i=<session description>) */
	if ((session->session_info) && (*session->session_info != '\0')) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto error,
			   &sdp,
			   "%c=%s" SDP_CRLF,
			   SDP_TYPE_INFORMATION,
			   session->session_info);
	}

	/* URI (u=<uri>) */
	if ((session->uri) && (*session->uri != '\0')) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto error,
			   &sdp,
			   "%c=%s" SDP_CRLF,
			   SDP_TYPE_URI,
			   session->uri);
	}

	/* Email address (e=<email-address>) */
	if ((session->email) && (*session->email != '\0')) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto error,
			   &sdp,
			   "%c=%s" SDP_CRLF,
			   SDP_TYPE_EMAIL,
			   session->email);
	}

	/* Phone number (p=<phone-number>) */
	if ((session->phone) && (*session->phone != '\0')) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto error,
			   &sdp,
			   "%c=%s" SDP_CRLF,
			   SDP_TYPE_PHONE,
			   session->phone);
	}

	/* Connection data (c=<nettype> <addrtype> <connection-address>) */
	if ((session->connection_addr) && (*session->connection_addr != '\0')) {
		session_level_connection_addr = 1;
		int multicast = 0;
		int addr_first = atoi(session->connection_addr);
		if ((addr_first >= SDP_MULTICAST_ADDR_MIN) &&
		    (addr_first <= SDP_MULTICAST_ADDR_MAX))
			multicast = 1;
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto error,
			   &sdp,
			   "%c=IN IP4 %s%s" SDP_CRLF,
			   SDP_TYPE_CONNECTION,
			   session->connection_addr,
			   (multicast) ? "/127" : "");
	}

	/* Timing (t=<start-time> <stop-time>) */
	/*TODO*/
	CHECK_FUNC(sdp_sprintf,
		   ret,
		   goto error,
		   &sdp,
		   "%c=0 0" SDP_CRLF,
		   SDP_TYPE_TIME);

	/* Tool (a=tool) */
	if ((session->tool) && (*session->tool != '\0')) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto error,
			   &sdp,
			   "%c=%s:%s" SDP_CRLF,
			   SDP_TYPE_ATTRIBUTE,
			   SDP_ATTR_TOOL,
			   session->tool);
	}

	/* Start mode (a=<start_mode>) */
	if ((session->start_mode > SDP_START_MODE_UNSPECIFIED) &&
	    (session->start_mode <= SDP_START_MODE_INACTIVE)) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto error,
			   &sdp,
			   "%c=%s" SDP_CRLF,
			   SDP_TYPE_ATTRIBUTE,
			   sdp_start_mode_str(session->start_mode));
	}

	/* Session type (a=type) */
	if ((session->type) && (*session->type != '\0')) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto error,
			   &sdp,
			   "%c=%s:%s" SDP_CRLF,
			   SDP_TYPE_ATTRIBUTE,
			   SDP_ATTR_TYPE,
			   session->type);
	}

	/* Charset (a=charset) */
	if ((session->charset) && (*session->charset != '\0')) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto error,
			   &sdp,
			   "%c=%s:%s" SDP_CRLF,
			   SDP_TYPE_ATTRIBUTE,
			   SDP_ATTR_CHARSET,
			   session->charset);
	}

	/* Control URL for use with RTSP (a=control) */
	if ((session->control_url) && (*session->control_url != '\0')) {
		CHECK_FUNC(sdp_sprintf,
			   ret,
			   goto error,
			   &sdp,
			   "%c=%s:%s" SDP_CRLF,
			   SDP_TYPE_ATTRIBUTE,
			   SDP_ATTR_CONTROL_URL,
			   session->control_url);
	}

	/* Range (a=range) */
	if (session->range.start.format != SDP_TIME_FORMAT_UNKNOWN) {
		ret = sdp_range_attr_write(&session->range, &sdp);
		if (ret < 0)
			goto error;
	}

	/* RTCP extended reports attribute (a=rtcp-xr) */
	if (session->rtcp_xr.valid) {
		ret = sdp_rtcp_xr_attr_write(&session->rtcp_xr, &sdp);
		if (ret < 0)
			goto error;
	}

	/* Other attributes (a=<attribute>:<value> or a=<attribute>) */
	list_walk_entry_forward(&session->attrs, attr, node)
	{
		if ((attr->key) && (*attr->key != '\0')) {
			if ((attr->value) && (*attr->value != '\0')) {
				CHECK_FUNC(sdp_sprintf,
					   ret,
					   goto error,
					   &sdp,
					   "%c=%s:%s" SDP_CRLF,
					   SDP_TYPE_ATTRIBUTE,
					   attr->key,
					   attr->value);
			} else {
				CHECK_FUNC(sdp_sprintf,
					   ret,
					   goto error,
					   &sdp,
					   "%c=%s" SDP_CRLF,
					   SDP_TYPE_ATTRIBUTE,
					   attr->key);
			}
		}
	}

	/* Media (m=...) */
	list_walk_entry_forward(&session->medias, media, node)
	{
		ret = sdp_media_write(
			media, &sdp, session_level_connection_addr);
		if (ret < 0)
			goto error;
	}

	*ret_str = sdp.str;
	return 0;

error:
	free(sdp.str);
	return ret;
}


int sdp_description_read(const char *session_desc, struct sdp_session **ret_obj)
{
	int ret;
	char *sdp;
	struct sdp_media *media = NULL;
	char *p, type, *value, *temp;
	uint32_t mandatory_fields = 0;
	struct sdp_session *session = NULL;

	ULOG_ERRNO_RETURN_ERR_IF(session_desc == NULL, EINVAL);
	ULOG_ERRNO_RETURN_ERR_IF(ret_obj == NULL, EINVAL);

	session = sdp_session_new();
	ULOG_ERRNO_RETURN_ERR_IF(session == NULL, ENOMEM);

	sdp = strdup(session_desc);
	if (sdp == NULL) {
		ret = -ENOMEM;
		ULOG_ERRNO("strdup", -ret);
		goto error;
	}

	p = strtok_r(sdp, SDP_CRLF, &temp);
	while (p) {
		/* Each line should be more than 2 chars long and in the form
		 * "<type>=<value>" with <type> being a single char */
		if ((strlen(p) <= 2) || (p[1] != '=')) {
			p = strtok_r(NULL, SDP_CRLF, &temp);
			continue;
		}

		/* <type>=<value>, value is always at offset 2 */
		type = *p;
		value = p + 2;

		switch (type) {
		case SDP_TYPE_VERSION: {
			mandatory_fields |= SDP_MANDATORY_TYPE_MASK_VERSION;
			/* Protocol version (v=0) */
			int version = -1;
			if (sscanf(value, "%d", &version) == SDP_VERSION)
				ULOGD("SDP: version=%d", version);
			if (version != 0) {
				/* SDP version must be 0 (RFC4566) */
				ULOGE("unsupported SDP version (%d)", version);
				ret = -EPROTO;
				goto error;
			}
			break;
		}

		case SDP_TYPE_ORIGIN: {
			mandatory_fields |= SDP_MANDATORY_TYPE_MASK_ORIGIN;
			/* Origin (o=<username> <sess-id> <sess-version>
			 * <nettype> <addrtype> <unicast-address>) */
			char *temp2 = NULL;
			char *username = strtok_r(value, " ", &temp2);
			char *sess_id = strtok_r(NULL, " ", &temp2);
			char *sess_version = strtok_r(NULL, " ", &temp2);
			char *nettype = strtok_r(NULL, " ", &temp2);
			if ((!nettype) || (strcmp(nettype, "IN") != 0)) {
				/* Network type must be 'IN'
				 * (RFC4566 ch. 5.2) */
				ULOGE("unsupported network type '%s'",
				      (nettype) ? nettype : "");
				ret = -EPROTO;
				goto error;
			}
			char *addrtype = strtok_r(NULL, " ", &temp2);
			if ((!addrtype) || (strcmp(addrtype, "IP4") != 0)) {
				/* Only IPv4 is supported */
				ULOGE("unsupported address type '%s'",
				      (addrtype) ? addrtype : "");
				ret = -EPROTO;
				goto error;
			}
			char *unicast_address = strtok_r(NULL, " ", &temp2);
			session->server_addr = xstrdup(unicast_address);
			session->session_id = (sess_id) ? atoll(sess_id) : 0;
			session->session_version =
				(sess_version) ? atoll(sess_version) : 0;
			ULOGD("SDP: username=%s sess_id=%" PRIu64
			      " sess_version=%" PRIu64
			      " nettype=%s"
			      " addrtype=%s unicast_address=%s",
			      username,
			      session->session_id,
			      session->session_version,
			      nettype,
			      addrtype,
			      unicast_address);
			break;
		}

		case SDP_TYPE_SESSION_NAME: {
			mandatory_fields |=
				SDP_MANDATORY_TYPE_MASK_SESSION_NAME;
			/* Session name (s=<session name>) */
			session->session_name = strdup(value);
			ULOGD("SDP: session name=%s", session->session_name);
			break;
		}

		case SDP_TYPE_INFORMATION: {
			/* Session information (i=<session description>)
			 * or media title (i=<media title>) */
			if (media) {
				media->media_title = strdup(value);
				ULOGD("SDP: media title=%s",
				      media->media_title);
			} else {
				session->session_info = strdup(value);
				ULOGD("SDP: session info=%s",
				      session->session_info);
			}
			break;
		}

		case SDP_TYPE_URI: {
			/* URI (u=<uri>) */
			session->uri = strdup(value);
			ULOGD("SDP: uri=%s", session->uri);
			break;
		}

		case SDP_TYPE_EMAIL: {
			/* Email address (e=<email-address>) */
			session->email = strdup(value);
			ULOGD("SDP: email=%s", session->email);
			break;
		}

		case SDP_TYPE_PHONE: {
			/* Phone number (p=<phone-number>) */
			session->phone = strdup(value);
			ULOGD("SDP: phone=%s", session->phone);
			break;
		}

		case SDP_TYPE_CONNECTION: {
			mandatory_fields |= SDP_MANDATORY_TYPE_MASK_CONNECTION;
			/* Connection data (c=<nettype> <addrtype>
			 * <connection-address>) */
			char *temp2 = NULL;
			char *nettype = strtok_r(value, " ", &temp2);
			if ((!nettype) || (strcmp(nettype, "IN") != 0)) {
				/* Network type must be 'IN'
				 * (RFC4566 ch. 5.7) */
				ULOGE("unsupported network type '%s'",
				      (nettype) ? nettype : "");
				ret = -EPROTO;
				goto error;
			}
			char *addrtype = strtok_r(NULL, " ", &temp2);
			if ((!addrtype) || (strcmp(addrtype, "IP4") != 0)) {
				/* Only IPv4 is supported */
				ULOGE("unsupported address type '%s'",
				      (addrtype) ? addrtype : "");
				ret = -EPROTO;
				goto error;
			}
			char *connection_address = strtok_r(NULL, " ", &temp2);
			if (!connection_address)
				continue;
			int addr_first = atoi(connection_address);
			int multicast =
				((addr_first >= SDP_MULTICAST_ADDR_MIN) &&
				 (addr_first <= SDP_MULTICAST_ADDR_MAX))
					? 1
					: 0;
			if (multicast) {
				char *p2 = strchr(connection_address, '/');
				if (p2 != NULL)
					*p2 = '\0';
			}
			if (media) {
				media->connection_addr =
					strdup(connection_address);
				media->multicast = multicast;
				ULOGD("SDP: media nettype=%s addrtype=%s"
				      " connection_address=%s",
				      nettype,
				      addrtype,
				      connection_address);
			} else {
				session->connection_addr =
					strdup(connection_address);
				session->multicast = multicast;
				ULOGD("SDP: nettype=%s addrtype=%s"
				      " connection_address=%s",
				      nettype,
				      addrtype,
				      connection_address);
			}
			break;
		}

		case SDP_TYPE_TIME: {
			mandatory_fields |= SDP_MANDATORY_TYPE_MASK_TIME;
			/* Time (t=<start-time> <stop-time>) */
			char *temp2 = NULL;
			char *start_time = NULL;
			start_time = strtok_r(value, " ", &temp2);
			uint64_t start_time_int =
				(start_time) ? atoll(start_time) : 0;
			char *stop_time = NULL;
			stop_time = strtok_r(NULL, " ", &temp2);
			uint64_t stop_time_int =
				(stop_time) ? atoll(stop_time) : 0;
			ULOGD("SDP: start_time=%" PRIu64 "stop_time=%" PRIu64,
			      start_time_int,
			      stop_time_int);
			/* TODO */
			break;
		}

		case SDP_TYPE_MEDIA: {
			/* Media (m=...) */
			ret = sdp_session_media_add(session, &media);
			if (ret < 0)
				goto error;
			ret = sdp_media_read(media, value);
			if (ret < 0)
				goto error;
			break;
		}

		case SDP_TYPE_ATTRIBUTE: {
			/* Attributes (a=...) */
			struct sdp_attr *attr = NULL;
			ret = sdp_attr_read(session, media, value, &attr);
			if (ret < 0)
				goto error;

			if (!attr)
				break;

			if (media) {
				ret = sdp_media_attr_add_existing(media, attr);
				ULOGD("media_attr_add attr:%p, attr->key:%s, attr->value:%s, ret:%d", attr, attr->key, attr->value, ret);
				if (ret < 0)
					goto error;
			} else {
				ret = sdp_session_attr_add_existing(session, attr);
				if (ret < 0)
					goto error;
			}
			break;
		}

		default:
			break;
		}
		p = strtok_r(NULL, SDP_CRLF, &temp);
	}

	/* Copy session-level parameters to media-level if undefined */
	list_walk_entry_forward(&session->medias, media, node)
	{
		if ((!media->connection_addr) && (session->connection_addr)) {
			media->connection_addr =
				strdup(session->connection_addr);
			media->multicast = session->multicast;
		}
		if (media->start_mode == SDP_START_MODE_UNSPECIFIED)
			media->start_mode = session->start_mode;
		if ((!media->rtcp_xr.valid) && (session->rtcp_xr.valid))
			media->rtcp_xr = session->rtcp_xr;
	}

	if (mandatory_fields == SDP_MANDATORY_TYPE_MASK_ORIGIN) {
		/* If only origin is present, this is a deletion SDP */
		ULOGD("SDP is of type deletion");
		session->deletion = 1;
	} else if ((SDP_MANDATORY_TYPE_MASK_ALL & mandatory_fields) !=
		   SDP_MANDATORY_TYPE_MASK_ALL) {
		/* Check that mandatory fields are present */
		if ((SDP_MANDATORY_TYPE_MASK_VERSION & mandatory_fields) !=
		    SDP_MANDATORY_TYPE_MASK_VERSION)
			ULOGE("missing mandatory field version (v=)");
		if ((SDP_MANDATORY_TYPE_MASK_ORIGIN & mandatory_fields) !=
		    SDP_MANDATORY_TYPE_MASK_ORIGIN)
			ULOGE("missing mandatory field origin (o=)");
		if ((SDP_MANDATORY_TYPE_MASK_SESSION_NAME & mandatory_fields) !=
		    SDP_MANDATORY_TYPE_MASK_SESSION_NAME)
			ULOGE("missing mandatory field session name (s=)");
		if ((SDP_MANDATORY_TYPE_MASK_CONNECTION & mandatory_fields) !=
		    SDP_MANDATORY_TYPE_MASK_CONNECTION)
			ULOGE("missing mandatory field connection (c=)");
		if ((SDP_MANDATORY_TYPE_MASK_TIME & mandatory_fields) !=
		    SDP_MANDATORY_TYPE_MASK_TIME)
			ULOGE("missing mandatory field time (t=)");
		ret = -EPROTO;
		goto error;
	}

	free(sdp);
	*ret_obj = session;

	return 0;

error:
	free(sdp);
	sdp_session_destroy(session);

	return ret;
}
