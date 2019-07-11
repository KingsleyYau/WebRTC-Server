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

#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ULOG_TAG sdp_test
#include <ulog.h>
ULOG_DECLARE_TAG(sdp_test);

#include <libsdp.h>


static void print_h264_fmtp(struct sdp_h264_fmtp *fmtp, const char *prefix)
{
	if (!fmtp)
		return;

	printf("%s-- H.264 format parameters\n", prefix);
	printf("%s   -- packetization mode: %d\n",
	       prefix,
	       fmtp->packetization_mode);
	printf("%s   -- profile_idc: %d\n", prefix, fmtp->profile_idc);
	printf("%s   -- profile-iop: 0x%02X\n", prefix, fmtp->profile_iop);
	printf("%s   -- level_idc: %d\n", prefix, fmtp->level_idc);
	if (fmtp->sps) {
		char sps[300];
		unsigned int i, len;
		for (i = 0, len = 0; i < fmtp->sps_size; i++) {
			len += snprintf(sps + len,
					sizeof(sps) - len,
					"%02X ",
					fmtp->sps[i]);
		}
		printf("%s   -- SPS (size %d): %s\n",
		       prefix,
		       fmtp->sps_size,
		       sps);
	}
	if (fmtp->pps) {
		char pps[300];
		unsigned int i, len;
		for (i = 0, len = 0; i < fmtp->pps_size; i++) {
			len += snprintf(pps + len,
					sizeof(pps) - len,
					"%02X ",
					fmtp->pps[i]);
		}
		printf("%s   -- PPS (size %d): %s\n",
		       prefix,
		       fmtp->pps_size,
		       pps);
	}
}


static void print_rtcp_xr_info(struct sdp_rtcp_xr *xr, const char *prefix)
{
	if (!xr)
		return;

	printf("%s-- RTCP XR\n", prefix);
	printf("%s   -- loss RLE report: %d (%d)\n",
	       prefix,
	       xr->loss_rle_report,
	       xr->loss_rle_report_max_size);
	printf("%s   -- duplicate RLE report: %d (%d)\n",
	       prefix,
	       xr->dup_rle_report,
	       xr->dup_rle_report_max_size);
	printf("%s   -- packet receipt times report: %d (%d)\n",
	       prefix,
	       xr->pkt_receipt_times_report,
	       xr->pkt_receipt_times_report_max_size);
	printf("%s   -- receiver reference time report: %d (%d)\n",
	       prefix,
	       xr->rtt_report,
	       xr->rtt_report_max_size);
	printf("%s   -- statistics summary report (loss): %d\n",
	       prefix,
	       xr->stats_summary_report_loss);
	printf("%s   -- statistics summary report (dup): %d\n",
	       prefix,
	       xr->stats_summary_report_dup);
	printf("%s   -- statistics summary report (jitter): %d\n",
	       prefix,
	       xr->stats_summary_report_jitter);
	printf("%s   -- statistics summary report (ttl): %d\n",
	       prefix,
	       xr->stats_summary_report_ttl);
	printf("%s   -- statistics summary report (hl): %d\n",
	       prefix,
	       xr->stats_summary_report_hl);
	printf("%s   -- VOIP metrics report: %d\n",
	       prefix,
	       xr->voip_metrics_report);
	printf("%s   -- de-jitter buffer metrics report: %d\n",
	       prefix,
	       xr->djb_metrics_report);
}


static void print_media_info(struct sdp_media *media)
{
	if (!media)
		return;

	printf("-- Media\n");
	printf("   -- type: %s\n", sdp_media_type_str(media->type));
	printf("   -- media title: %s\n", media->media_title);
	printf("   -- connection address: %s%s\n",
	       media->connection_addr,
	       (media->multicast) ? " (multicast)" : "");
	printf("   -- control URL: %s\n", media->control_url);
	if ((media->range.start.format == SDP_TIME_FORMAT_NPT) &&
	    (!media->range.start.npt.now) &&
	    (!media->range.start.npt.infinity) &&
	    (!media->range.stop.npt.now) && (!media->range.stop.npt.infinity)) {
		printf("   -- range: start=%ld.%d stop=%ld.%d\n",
		       (long)media->range.start.npt.sec,
		       media->range.start.npt.usec / 1000,
		       (long)media->range.stop.npt.sec,
		       media->range.stop.npt.usec / 1000);
	}
	printf("   -- start mode: %s\n", sdp_start_mode_str(media->start_mode));
	printf("   -- stream port: %d\n", media->dst_stream_port);
	printf("   -- control port: %d\n", media->dst_control_port);
	printf("   -- payload type: %d\n", media->payload_type);
	printf("   -- encoding name: %s\n", media->encoding_name);
	printf("   -- encoding params: %s\n", media->encoding_params);
	printf("   -- clock rate: %d\n", media->clock_rate);
	if (media->h264_fmtp.valid)
		print_h264_fmtp(&media->h264_fmtp, "   ");
	if (media->rtcp_xr.valid)
		print_rtcp_xr_info(&media->rtcp_xr, "   ");
	struct sdp_attr *attr = NULL;
	list_walk_entry_forward(&media->attrs, attr, node)
	{
		printf("   -- attribute %s%s%s\n",
		       attr->key,
		       (attr->value) ? ": " : "",
		       (attr->value) ? attr->value : "");
	}
}


static void print_session_info(struct sdp_session *session)
{
	if (!session)
		return;

	printf("Session\n");
	printf("-- session ID: %" PRIu64 "\n", session->session_id);
	printf("-- session version: %" PRIu64 "\n", session->session_version);
	printf("-- server address: %s\n", session->server_addr);
	printf("-- session name: %s\n", session->session_name);
	printf("-- session info: %s\n", session->session_info);
	printf("-- URI: %s\n", session->uri);
	printf("-- email: %s\n", session->email);
	printf("-- phone: %s\n", session->phone);
	printf("-- tool: %s\n", session->tool);
	printf("-- type: %s\n", session->type);
	printf("-- charset: %s\n", session->charset);
	printf("-- connection address: %s%s\n",
	       session->connection_addr,
	       (session->multicast) ? " (multicast)" : "");
	printf("-- control URL: %s\n", session->control_url);
	printf("-- start mode: %s\n", sdp_start_mode_str(session->start_mode));
	if ((session->range.start.format == SDP_TIME_FORMAT_NPT) &&
	    (!session->range.start.npt.now) &&
	    (!session->range.start.npt.infinity) &&
	    (!session->range.stop.npt.now) &&
	    (!session->range.stop.npt.infinity)) {
		printf("-- range: start=%ld.%d stop=%ld.%d\n",
		       (long)session->range.start.npt.sec,
		       session->range.start.npt.usec / 1000,
		       (long)session->range.stop.npt.sec,
		       session->range.stop.npt.usec / 1000);
	}
	if (session->rtcp_xr.valid)
		print_rtcp_xr_info(&session->rtcp_xr, "");
	struct sdp_attr *attr = NULL;
	list_walk_entry_forward(&session->attrs, attr, node)
	{
		printf("-- attribute %s%s%s\n",
		       attr->key,
		       (attr->value) ? ": " : "",
		       (attr->value) ? attr->value : "");
	}
	struct sdp_media *media = NULL;
	list_walk_entry_forward(&session->medias, media, node)
		print_media_info(media);
}


static void welcome(char *prog_name)
{
	printf("\n%s - Session Description Protocol library test program\n"
	       "Copyright (c) 2017 Parrot Drones SAS\n"
	       "Copyright (c) 2017 Aurelien Barre\n\n",
	       prog_name);
}


static void usage(char *prog_name)
{
	printf("Usage:\n"
	       "\n"
	       "%s -h  ||  %s --help\n"
	       "  Print this message\n"
	       "\n"
	       "%s\n"
	       "  Generate a SDP document, parse it and print the SDP info\n"
	       "\n"
	       "%s <file>\n"
	       "  Read a SDP file and print the SDP info\n"
	       "\n"
	       "%s <file1> <file2>\n"
	       "  Read 2 SDP files and compare SDP contents\n",
	       prog_name,
	       prog_name,
	       prog_name,
	       prog_name,
	       prog_name);
}


int main(int argc, char **argv)
{
	int status = EXIT_SUCCESS, err;
	FILE *f = NULL;
	struct sdp_session *session = NULL, *session2 = NULL, *session3 = NULL;
	struct sdp_media *media1 = NULL, *media2 = NULL;
	char *sdp = NULL, *sdp3 = NULL;
	uint8_t sps[] = {0x67, 0x64, 0x00, 0x28, 0xAC, 0xD9, 0x80, 0x78,
			 0x06, 0x5B, 0x01, 0x10, 0x00, 0x00, 0x3E, 0x90,
			 0x00, 0x0B, 0xB8, 0x08, 0xF1, 0x83, 0x19, 0xA0};
	uint8_t pps[] = {0x68, 0xE9, 0x78, 0xF3, 0xC8, 0xF0};

	welcome(argv[0]);

	if ((argc == 2) && ((strcmp(argv[1], "-h") == 0) ||
			    (strcmp(argv[1], "--help") == 0))) {
		usage(argv[0]);
		exit(EXIT_SUCCESS);
	}

	if (argc < 2) {
		session = sdp_session_new();
		if (session == NULL) {
			ULOG_ERRNO("sdp_session_new", ENOMEM);
			status = EXIT_FAILURE;
			goto cleanup;
		}
		session->session_id = 123456789;
		session->session_version = 1;
		session->server_addr = strdup("192.168.43.1");
		session->session_name = strdup("Bebop2");
		session->control_url = strdup("rtsp://192.168.43.1/video");
		session->start_mode = SDP_START_MODE_RECVONLY;
		session->tool = strdup(argv[0]);
		session->type = strdup("broadcast");
		session->rtcp_xr.valid = 1;
		session->rtcp_xr.loss_rle_report = 1;
		session->rtcp_xr.djb_metrics_report = 1;
		session->range.start.format = SDP_TIME_FORMAT_NPT;
		session->range.start.npt.sec = 0;
		session->range.start.npt.usec = 0;
		session->range.stop.format = SDP_TIME_FORMAT_NPT;
		session->range.stop.npt.sec = 12 * 60 + 34;
		session->range.stop.npt.usec = 567000;

		err = sdp_session_media_add(session, &media1);
		if (err < 0) {
			ULOG_ERRNO("sdp_session_media_add", -err);
			status = EXIT_FAILURE;
			goto cleanup;
		}
		media1->type = SDP_MEDIA_TYPE_VIDEO;
		media1->media_title = strdup("Front camera");
		media1->connection_addr = strdup("239.255.42.1");
		media1->dst_stream_port = 55004;
		media1->dst_control_port = 55005;
		media1->control_url = strdup("stream=0");
		media1->payload_type = 96;
		media1->encoding_name = strdup("H264");
		media1->clock_rate = 90000;
		media1->h264_fmtp.valid = 1;
		media1->h264_fmtp.packetization_mode = 1;
		media1->h264_fmtp.profile_idc = 66;
		media1->h264_fmtp.profile_iop = 0;
		media1->h264_fmtp.level_idc = 41;
		media1->h264_fmtp.sps = malloc(sizeof(sps));
		if (media1->h264_fmtp.sps) {
			memcpy(media1->h264_fmtp.sps, sps, sizeof(sps));
			media1->h264_fmtp.sps_size = sizeof(sps);
		}
		media1->h264_fmtp.pps = malloc(sizeof(pps));
		if (media1->h264_fmtp.pps) {
			memcpy(media1->h264_fmtp.pps, pps, sizeof(pps));
			media1->h264_fmtp.pps_size = sizeof(pps);
		}

		err = sdp_session_media_add(session, &media2);
		if (err < 0) {
			ULOG_ERRNO("sdp_session_media_add", -err);
			status = EXIT_FAILURE;
			goto cleanup;
		}
		media2->type = SDP_MEDIA_TYPE_VIDEO;
		media2->media_title = strdup("Vertical camera");
		media2->connection_addr = strdup("239.255.42.1");
		media2->dst_stream_port = 55006;
		media2->dst_control_port = 55007;
		media2->control_url = strdup("stream=1");
		media2->payload_type = 96;
		media2->encoding_name = strdup("H264");
		media2->clock_rate = 90000;
		media2->h264_fmtp.valid = 1;
		media2->h264_fmtp.packetization_mode = 1;
		media2->h264_fmtp.profile_idc = 66;
		media2->h264_fmtp.profile_iop = 0;
		media2->h264_fmtp.level_idc = 41;
		media2->h264_fmtp.sps = malloc(sizeof(sps));
		if (media2->h264_fmtp.sps) {
			memcpy(media2->h264_fmtp.sps, sps, sizeof(sps));
			media2->h264_fmtp.sps_size = sizeof(sps);
		}
		media2->h264_fmtp.pps = malloc(sizeof(pps));
		if (media2->h264_fmtp.pps) {
			memcpy(media2->h264_fmtp.pps, pps, sizeof(pps));
			media2->h264_fmtp.pps_size = sizeof(pps);
		}

		err = sdp_description_write(session, &sdp);
		if (err < 0) {
			ULOG_ERRNO("sdp_description_write", -err);
			status = EXIT_FAILURE;
			goto cleanup;
		}

		printf("\n%s\n", sdp);
	} else if (argc == 2) {
		f = fopen(argv[1], "r");
		if (!f) {
			err = -errno;
			ULOG_ERRNO("fopen('%s')", -err, argv[1]);
			status = EXIT_FAILURE;
			goto cleanup;
		}
		fseek(f, 0, SEEK_END);
		long file_size = ftell(f);
		if (file_size < 0) {
			err = -errno;
			ULOG_ERRNO("ftell", -err);
			status = EXIT_FAILURE;
			goto cleanup;
		}
		fseek(f, 0, SEEK_SET);
		sdp = calloc(file_size + 1, 1);
		if (!sdp) {
			ULOG_ERRNO("calloc", ENOMEM);
			status = EXIT_FAILURE;
			goto cleanup;
		}
		int err = fread(sdp, file_size, 1, f);
		if (err != 1) {
			err = -errno;
			ULOG_ERRNO("fread", -err);
			status = EXIT_FAILURE;
			goto cleanup;
		}
	} else if (argc == 3) {
		/* get 1st sdp file */
		f = fopen(argv[1], "r");
		if (!f) {
			err = -errno;
			ULOG_ERRNO("fopen('%s')", -err, argv[1]);
			status = EXIT_FAILURE;
			goto cleanup;
		}
		fseek(f, 0, SEEK_END);
		long file_size = ftell(f);
		if (file_size < 0) {
			err = -errno;
			ULOG_ERRNO("ftell", -err);
			status = EXIT_FAILURE;
			goto cleanup;
		}
		fseek(f, 0, SEEK_SET);
		sdp = calloc(file_size + 1, 1);
		if (!sdp) {
			ULOG_ERRNO("calloc", ENOMEM);
			status = EXIT_FAILURE;
			goto cleanup;
		}
		int err = fread(sdp, file_size, 1, f);
		if (err != 1) {
			err = -errno;
			ULOG_ERRNO("fread", -err);
			status = EXIT_FAILURE;
			goto cleanup;
		}

		/* get 2nd sdp file */
		f = fopen(argv[2], "r");
		if (!f) {
			err = -errno;
			ULOG_ERRNO("fopen('%s')", -err, argv[2]);
			status = EXIT_FAILURE;
			goto cleanup;
		}
		fseek(f, 0, SEEK_END);
		file_size = ftell(f);
		if (file_size < 0) {
			err = -errno;
			ULOG_ERRNO("ftell", -err);
			status = EXIT_FAILURE;
			goto cleanup;
		}
		fseek(f, 0, SEEK_SET);
		sdp3 = calloc(file_size + 1, 1);
		if (!sdp3) {
			ULOG_ERRNO("calloc", ENOMEM);
			status = EXIT_FAILURE;
			goto cleanup;
		}
		err = fread(sdp3, file_size, 1, f);
		if (err != 1) {
			err = -errno;
			ULOG_ERRNO("fread", -err);
			status = EXIT_FAILURE;
			goto cleanup;
		}
	} else {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	err = sdp_description_read(sdp, &session2);
	if (err < 0) {
		ULOG_ERRNO("sdp_description_read", -err);
		status = EXIT_FAILURE;
		goto cleanup;
	}

	if (sdp3) {
		err = sdp_description_read(sdp3, &session3);
		if (err < 0) {
			ULOG_ERRNO("sdp_description_read", -err);
			status = EXIT_FAILURE;
			goto cleanup;
		}
		if (sdp_session_compare(session2, session3) == 0)
			printf("The 2 SDP documents are identical\n");
		else
			printf("The 2 SDP documents are different\n");

		goto cleanup;
	}

	printf("\n");
	print_session_info(session2);

cleanup:
	free(sdp);
	if (session)
		sdp_session_destroy(session);
	if (session2)
		sdp_session_destroy(session2);
	if (f)
		fclose(f);
	if (sdp3)
		free(sdp3);
	if (session3)
		sdp_session_destroy(session3);

	printf("%s\n", (status == EXIT_SUCCESS) ? "Done!" : "Failed!");
	exit(status);
}
