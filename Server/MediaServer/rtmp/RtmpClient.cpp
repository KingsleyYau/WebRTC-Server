//
//  RtmpClient.cpp
//  RtmpClient
//
//  Created by Max on 2017/4/5.
//  Copyright © 2017年 net.qdating. All rights reserved.
//

#include "RtmpClient.h"

#include <common/CommonFunc.h>

#include <errno.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <srs/srs_librtmp.h>

namespace mediaserver {

#define DEFAULT_VIDEO_FRAME_SIZE 65535
#define DEFAULT_AUDIO_FRAME_SIZE DEFAULT_VIDEO_FRAME_SIZE

#define ADTS_HEADER_SIZE 7

class ConnectRunnable : public KRunnable {
  public:
    ConnectRunnable(RtmpClient *container) {
        mContainer = container;
    }
    virtual ~ConnectRunnable() {
        mContainer = NULL;
    }

  protected:
    void onRun() {
        mContainer->ConnectRunnableHandle();
    }

  private:
    RtmpClient *mContainer;
};

class CheckConnectRunnable : public KRunnable {
  public:
    CheckConnectRunnable(RtmpClient *container) {
        mContainer = container;
    }
    virtual ~CheckConnectRunnable() {
        mContainer = NULL;
    }

  protected:
    void onRun() {
        mContainer->CheckConnectRunnableHandle();
    }

  private:
    RtmpClient *mContainer;
};

void RtmpClient::GobalInit() {
	LogAync(
			LOG_ERR_SYS,
			"RtmpClient::GobalInit( Example for srs-librtmp )");
	LogAync(
			LOG_ERR_SYS,
			"RtmpClient::GobalInit( SRS(ossrs) client librtmp library )");
	LogAync(
			LOG_ERR_SYS,
			"RtmpClient::GobalInit( version: %d.%d.%d )", srs_version_major(), srs_version_minor(), srs_version_revision());
}

RtmpClient::RtmpClient()
    : mClientMutex(KMutex::MutexType_Recursive) {
    mpRtmpClientCallback = NULL;
    mbRunning = false;
    mUrl = "";
    mIO = NULL;

    mpRtmp = NULL;
    mpFlv = NULL;
    mRecordFlvFilePath = "";

    mpSps = NULL;
    mSpsSize = 0;

    mpPps = NULL;
    mPpsSize = 0;

    mNaluHeaderSize = 0;

    mWidth = 0;
    mHeight = 0;

    mEncodeVideoTimestamp = 0;
    mEncodeAudioTimestamp = 0;
    mSendVideoFrameTimestamp = 0;
    mSendAudioFrameTimestamp = 0;

    mIsPlay = true;
    mIsConnected = false;
    mConnectTimeout = 5000;

    mpConnectRunnable = new ConnectRunnable(this);
    mpCheckConnectRunnable = new CheckConnectRunnable(this);
}

RtmpClient::~RtmpClient() {
    Close();

    if (mpConnectRunnable) {
        delete mpConnectRunnable;
        mpConnectRunnable = NULL;
    }

    if (mpCheckConnectRunnable) {
        delete mpCheckConnectRunnable;
        mpCheckConnectRunnable = NULL;
    }

    if (mpSps) {
        delete[] mpSps;
        mpSps = NULL;
    }

    if (mpPps) {
        delete[] mpPps;
        mpPps = NULL;
    }
}

void RtmpClient::SetCallback(RtmpClientCallback *callback) {
    mpRtmpClientCallback = callback;
}

void RtmpClient::SetVideoParam(int width, int height) {
	LogAync(
			LOG_MSG,
			"RtmpClient::SetVideoParam( "
			"this : %p, "
			"width : %d, "
			"height : %d "
			")",
			this,
			width,
			height
			);

    mClientMutex.lock();
    mWidth = width;
    mHeight = height;
    mClientMutex.unlock();
}

bool RtmpClient::PlayStream(const string &url, const string &recordFilePath) {
    bool bFlag = true;

    LogAync(
			LOG_MSG,
    		"RtmpClient::PlayStream( "
    		"this : %p, "
    		"url : %s, "
    		"recordFilePath : %s "
    		")",
			this,
			url.c_str(),
			recordFilePath.c_str());

    mClientMutex.lock();
    if (!mbRunning) {
		mIsPlay = true;
		mpRtmp = srs_rtmp_create(url.c_str());
		mUrl = url;

		if (bFlag && mpRtmp) {
			// 创建FLV文件
			mRecordFlvFilePath = recordFilePath;
			if (mRecordFlvFilePath.length() > 0) {
				mpFlv = srs_flv_open_write(mRecordFlvFilePath.c_str());

				if (mpFlv) {
					LogAync(
							LOG_MSG,
							"RtmpClient::PlayStream( "
							"this : %p, "
							"[Create record file success], "
							"recordFilePath : %s "
							")",
							this,
							mRecordFlvFilePath.c_str()
							);
				} else {
					LogAync(
							LOG_MSG,
							"RtmpClient::PlayStream( "
							"this : %p, "
							"[Create record file fail], "
							"recordFilePath : %s "
							")",
							this,
							mRecordFlvFilePath.c_str()
							);
				}
			}

			mbRunning = true;
			mConnectThread.Start(mpConnectRunnable);
			mCheckConnectThread.Start(mpCheckConnectRunnable);
		}
    }

    mClientMutex.unlock();

    LogAync(
			LOG_MSG,
    		"RtmpClient::PlayStream( "
    		"this : %p, "
    		"[%s], "
    		"url : %s, "
    		"mpRtmp : %p "
    		")",
			this,
			bFlag ? "Success" : "Fail",
			url.c_str(),
			mpRtmp);

    return bFlag;
}

bool RtmpClient::PublishUrl(const string &url) {
    bool bFlag = false;

    LogAync(
    		LOG_MSG,
			"RtmpClient::PublishUrl( "
			"this : %p, "
			"url : %s "
			")",
			this,
			url.c_str()
			);

    mClientMutex.lock();
    if (mbRunning) {
		mIsPlay = false;
		mpRtmp = srs_rtmp_create(url.c_str());
		mUrl = url;

		if (mpRtmp) {
			mbRunning = true;
			mConnectThread.Start(mpConnectRunnable);
			mCheckConnectThread.Start(mpCheckConnectRunnable);
			bFlag = true;
		}
    }
    mClientMutex.unlock();

    LogAync(
    		LOG_MSG,
    		"RtmpClient::PublishUrl( "
    		"this : %p, "
    		"[%s], "
    		"url : %s, "
    		"mpRtmp : %p "
    		")",
			this,
			bFlag ? "Success" : "Fail",
			url.c_str(),
			mpRtmp);

    return bFlag;
}

void RtmpClient::Shutdown() {
	LogAync(
			LOG_MSG,
    		"RtmpClient::Shutdown( "
    		"this : %p, "
			"mpRtmp : %p "
			")",
			this,
			mpRtmp);

    mClientMutex.lock();
    if (mbRunning) {
        if (mpRtmp) {
            srs_rtmp_shutdown(mpRtmp);
        }
    }
    mClientMutex.unlock();

    mConnectedMutex.lock();
    mIsConnected = false;
    mConnectedMutex.unlock();

    LogAync(
    		LOG_MSG,
			"RtmpClient::Shutdown( "
			"this : %p, "
			"[Success] "
			")",
			this);
}

void RtmpClient::Close() {
	LogAync(
			LOG_MSG,
    		"RtmpClient::Close( "
    		"this : %p, "
			"mpRtmp : %p "
			")",
			this,
			mpRtmp);

    mClientMutex.lock();
    if (mbRunning) {
        mbRunning = false;

        mConnectThread.Stop();
        mCheckConnectThread.Stop();

        Destroy();
    }
    mClientMutex.unlock();

    mConnectedMutex.lock();
    mIsConnected = false;
    mConnectedMutex.unlock();

    LogAync(
    		LOG_MSG,
			"RtmpClient::Close( "
			"this : %p, "
			"[Success] "
			")",
			this);
}

bool RtmpClient::RecvPacket() {
	bool bFlag = true;

    char type;
    u_int32_t timestamp = 0;
    char *frame = NULL;
    int frame_size = 0;

    int ret = 0;
    if ( 0 == (ret = srs_rtmp_read_packet(mpRtmp, &type, &timestamp, &frame, &frame_size)) ) {
        if (type == SRS_RTMP_TYPE_AUDIO) {
            Flv2Audio(frame, frame_size, timestamp);

        } else if (type == SRS_RTMP_TYPE_VIDEO) {
            Flv2Video(frame, frame_size, timestamp);

        } else if (type == SRS_RTMP_TYPE_COMMAND) {
            RecvCmd(frame, frame_size, timestamp);
        }

        // we only write some types of messages to flv file.
        int is_flv_msg = ((type == SRS_RTMP_TYPE_AUDIO) ||
                          (type == SRS_RTMP_TYPE_VIDEO) ||
                          (type == SRS_RTMP_TYPE_SCRIPT));

        // for script data, ignore except onMetaData
        if (type == SRS_RTMP_TYPE_SCRIPT) {
            if (!srs_rtmp_is_onMetaData(type, frame, frame_size)) {
                is_flv_msg = 0;
            }
        }

        if (mpFlv) {
            if (is_flv_msg) {
                if (srs_flv_write_tag(mpFlv, type, timestamp, frame, frame_size) != 0) {
                }
            } else {
            }
        }

        if (frame) {
        	delete[] frame;
        	frame = NULL;
        }
    } else {
    	if (errno == EAGAIN || errno == EWOULDBLOCK) {
    	} else {
    		bFlag = false;
    	}
    }

//    if( bFlag && ret == 0 ) {
//		LogAync(
//				LOG_STAT,
//				"RtmpClient::RecvPacket( "
//				"this : %p, "
//				"type : %d, "
//				"timestamp : %u, "
//				"size : %d "
//				")",
//				this,
//				type,
//				timestamp,
//				frame_size
//				);
//    }

    return bFlag;
}

void RtmpClient::ConnectRunnableHandle() {
	LogAync(
			LOG_STAT,
    		"RtmpClient::ConnectRunnableHandle( "
    		"this : %p, "
    		"[Start] "
    		")",
			this);

    // 设置读写超时
    srs_rtmp_set_timeout(mpRtmp, 0, 0);
    bool bFlag = (0 == srs_rtmp_handshake(mpRtmp));
    if (bFlag) {
        bFlag = (0 == srs_rtmp_connect_app(mpRtmp));
        if (bFlag) {
            if (mIsPlay) {
                bFlag = (0 == srs_rtmp_play_stream(mpRtmp));
                if (!bFlag) {
                	LogAync(
							LOG_ERR_USER,
							"RtmpClient::ConnectRunnableHandle( "
							"this : %p, "
							"[srs_rtmp_play_stream fail] "
							")",
							this);
                }
            } else {
                bFlag = (0 == srs_rtmp_publish_stream(mpRtmp));
                if (!bFlag) {
                	LogAync(
							LOG_ERR_USER,
							"RtmpClient::ConnectRunnableHandle( "
							"this : %p, "
							"[srs_rtmp_publish_stream fail] "
							")",
							this);
                }
                // 发送支持Flash播放的视频分辨率
                srs_rtmp_set_data_frame(mpRtmp, mWidth, mHeight);
            }
        } else {
        	LogAync(
            		LOG_ERR_USER,
            		"RtmpClient::ConnectRunnableHandle( "
            		"this : %p, "
            		"[srs_rtmp_connect_app fail]"
            		")",
					this);
        }
    } else {
    	LogAync(
    			LOG_ERR_USER,
        		"RtmpClient::ConnectRunnableHandle( "
        		"this : %p, "
        		"[srs_rtmp_handshake fail] "
        		")",
				this);
    }

    // 标记为已经连接上服务器
    if (bFlag) {
        mConnectedMutex.lock();
        mIsConnected = true;
        mEncodeVideoTimestamp = 0;
        mEncodeAudioTimestamp = 0;
        mSendVideoFrameTimestamp = 0;
        mSendAudioFrameTimestamp = 0;
        mConnectedMutex.unlock();

        LogAync(
				LOG_MSG,
        		"RtmpClient::ConnectRunnableHandle( "
        		"this : %p, "
        		"[Connected] "
        		")",
				this);

        if (mpRtmpClientCallback) {
            mpRtmpClientCallback->OnConnect(this);
        }
    }

    LogAync(
    		LOG_STAT,
    		"RtmpClient::ConnectRunnableHandle( "
    		"this : %p, "
    		"[Exit] "
    		")",
			this);
}

bool RtmpClient::SendVideoFrame(char *frame, int frame_size, u_int32_t timestamp) {
    bool bFlag = false;
    int ret = 0;
    //    int nb_start_code = 0;

    char *sendFrame = frame;
    int sendSize = frame_size;

    mClientMutex.lock();
    mConnectedMutex.lock();
    if (mbRunning && mpRtmp && mIsConnected) {
        //        // 判断是否{0x00, 0x00, 0x00, 0x01}开头
        //        if( !srs_h264_startswith_annexb(frame, frame_size, &nb_start_code) ) {
        //            // Add NALU start code
        //            static char nalu[] = {0x00, 0x00, 0x00, 0x01};
        //            memcpy(mpTempVideoBuffer, nalu, sizeof(nalu));
        //            memcpy(mpTempVideoBuffer + sizeof(nalu), frame, frame_size);
        //
        //            sendFrame = mpTempVideoBuffer;
        //            sendSize = sizeof(nalu) + frame_size;
        //        }

        // 计算RTMP时间戳
        int sendTimestamp = 0;

        // 第一帧
        if (mEncodeVideoTimestamp == 0) {
            mEncodeVideoTimestamp = timestamp;
        }

        // 当前帧比上一帧时间戳大, 计算时间差
        if (timestamp > mEncodeVideoTimestamp) {
            sendTimestamp = timestamp - mEncodeVideoTimestamp;
        }

        // 生成RTMP相对时间戳
        mSendVideoFrameTimestamp += sendTimestamp;
        mEncodeVideoTimestamp = timestamp;

        // 因为没有B帧, 所以dts和pts一样就可以
//        FileLevelLog("RtmpClient", KLog::LOG_MSG, "RtmpClient::SendVideoFrame( this : %p, timestamp : %u, size : %d, frameType : 0x%x )", this, mSendVideoFrameTimestamp, sendSize, sendFrame[0]);
        ret = srs_h264_write_raw_frame_without_startcode(mpRtmp, sendFrame, sendSize, mSendVideoFrameTimestamp, mSendVideoFrameTimestamp);
        //        ret = srs_h264_write_raw_frames(mpRtmp, sendFrame, sendSize, mSendVideoFrameTimestamp, mSendVideoFrameTimestamp);
        if (ret != 0) {
            bFlag = true;

            if (srs_h264_is_dvbsp_error(ret)) {
                //                FileLog("RtmpClient", "RtmpClient::SendVideoFrame( ignore drop video error, code=%d )", ret);

            } else if (srs_h264_is_duplicated_sps_error(ret)) {
//                FileLevelLog("RtmpClient", KLog::LOG_MSG, "RtmpClient::SendVideoFrame( this : %p, Ignore duplicated sps, ret : %d )", this, ret);

            } else if (srs_h264_is_duplicated_pps_error(ret)) {
//                FileLevelLog("RtmpClient", KLog::LOG_MSG, "RtmpClient::SendVideoFrame( this : %p, Ignore duplicated pps, ret : %d )", this, ret);

            } else {
                bFlag = false;
            }

        } else {
            bFlag = true;
        }
    }
    mConnectedMutex.unlock();
    mClientMutex.unlock();

    if (!bFlag) {
        mConnectedMutex.lock();
        if (mIsConnected) {
        	LogAync(
        			LOG_MSG,
					"RtmpClient::SendVideoFrame( "
					"this : %p, "
					"Send h264 raw data failed, "
					"ret : %d, "
					"timestamp : %u "
					")",
					this,
					ret,
					mSendVideoFrameTimestamp
					);
            srs_rtmp_shutdown(mpRtmp);
        }
        mConnectedMutex.unlock();

    } else {
        // 5bits, 7.3.1 NAL unit syntax,
        // H.264-AVC-ISO_IEC_14496-10.pdf, page 44.
        //  7: SPS, 8: PPS, 5: I Frame, 1: P Frame, 9: AUD, 6: SEI
        //        u_int8_t nut = (char)frame[nb_start_code] & 0x1f;
        //        FileLog("RtmpClient", "sent packet: type=%s, time=%d, size=%d, b[%d]=%#x(%s)",
        //                        srs_human_flv_tag_type2string(SRS_RTMP_TYPE_VIDEO), mSendVideoFrameTimestamp, frame_size, nb_start_code, (char)frame[nb_start_code],
        //                        (nut == 7? "SPS":(nut == 8? "PPS":(nut == 5? "I":(nut == 1? "P":(nut == 9? "AUD":(nut == 6? "SEI":"Unknown")))))));
    }

    return bFlag;
}

void RtmpClient::AddVideoTimestamp(u_int32_t timestamp) {
    mConnectedMutex.lock();
    if (mIsConnected) {
        mSendVideoFrameTimestamp += timestamp;
    }
    mConnectedMutex.unlock();
}

bool RtmpClient::SendAudioFrame(
    AudioFrameFormat sound_format,
    AudioFrameSoundRate sound_rate,
    AudioFrameSoundSize sound_size,
    AudioFrameSoundType sound_type,
    char *frame,
    int frame_size,
    u_int32_t timestamp) {
    bool bFlag = false;
    int ret = 0;

    mClientMutex.lock();
    mConnectedMutex.lock();
    if (mbRunning && mpRtmp && mIsConnected) {
        // 计算RTMP时间戳
        int sendTimestamp = 0;

        // 第一帧
        if (mEncodeAudioTimestamp == 0) {
            mEncodeAudioTimestamp = timestamp;
        }

        // 当前帧比上一帧时间戳大, 计算时间差
        if (timestamp > mEncodeAudioTimestamp) {
            sendTimestamp = timestamp - mEncodeAudioTimestamp;
        }

        // 生成RTMP相对时间戳
        mSendAudioFrameTimestamp += sendTimestamp;
        mEncodeAudioTimestamp = timestamp;

        if ((ret = srs_audio_write_raw_frame(mpRtmp, sound_format, sound_rate, sound_size, sound_type, frame, frame_size, mSendAudioFrameTimestamp)) == 0) {
            bFlag = true;
        }
    }
    mConnectedMutex.unlock();
    mClientMutex.unlock();

    if (!bFlag) {
        LogAync(
        		LOG_MSG,
				"RtmpClient::SendAudioFrame( "
				"this : %p, "
				"Send audio raw data failed, "
				"ret=%d, "
				"timestamp : %u "
				")",
				this,
				ret,
				mSendAudioFrameTimestamp);

        mConnectedMutex.lock();
        if (mIsConnected) {
            srs_rtmp_shutdown(mpRtmp);
        }
        mConnectedMutex.unlock();
    }

    return bFlag;
}

void RtmpClient::AddAudioTimestamp(u_int32_t timestamp) {
    mConnectedMutex.lock();
    if (mIsConnected) {
        mSendAudioFrameTimestamp += timestamp;
    }
    mConnectedMutex.unlock();
}

int RtmpClient::GetFD() {
	int fd = -1;

	if (mpRtmp) {
		fd = srs_rtmp_getfd(mpRtmp);
	}

	return fd;
}

ev_io *RtmpClient::GetIO() {
	return mIO;
}

void RtmpClient::SetIO(ev_io *w) {
	mIO = w;
}

const string& RtmpClient::GetUrl() {
	return mUrl;
}

void RtmpClient::Destroy() {
	LogAync(
			LOG_MSG,
			"RtmpClient::Destroy( "
			"this : %p, "
			"mpRtmp : %p "
			")",
			this,
			mpRtmp
			);

    // 关闭RTMP句柄
    if (mpRtmp) {
        srs_rtmp_destroy(mpRtmp);
        mpRtmp = NULL;
    }

    // 关闭FLV文件
    if (mpFlv) {
        srs_flv_close(mpFlv);
        mpFlv = NULL;
    }

    mUrl = "";

    LogAync(
    		LOG_MSG,
    		"RtmpClient::Destroy( "
            "[Success], "
            "this : %p "
            ")",
            this
            );
}

bool RtmpClient::Flv2Audio(char *frame, int frame_size, u_int32_t timestamp) {
    bool bFlag = true;

    AudioFrameFormat sound_format = AFF_UNKNOWN;
    AudioFrameSoundRate sound_rate = AFSR_UNKNOWN;
    AudioFrameSoundSize sound_size = AFSS_UNKNOWN;
    AudioFrameSoundType sound_type = AFST_UNKNOWN;
    AudioFrameAACType aac_type = AAC_UNKNOWN;

    char *pStart = frame + 1;
    int playload_size = frame_size - 1;

    sound_format = (AudioFrameFormat)srs_utils_flv_audio_sound_format(frame, frame_size);
    sound_rate = (AudioFrameSoundRate)srs_utils_flv_audio_sound_rate(frame, frame_size);
    sound_size = (AudioFrameSoundSize)srs_utils_flv_audio_sound_size(frame, frame_size);
    sound_type = (AudioFrameSoundType)srs_utils_flv_audio_sound_type(frame, frame_size);

    if (sound_format == AFF_AAC) {
        pStart++;
        playload_size--;

        aac_type = (AudioFrameAACType)srs_utils_flv_audio_aac_packet_type(frame, frame_size);
        if (aac_type == AAC_SEQUENCE_HEADER) {
            // AAC类型包
            if (mpRtmpClientCallback) {
                mpRtmpClientCallback->OnChangeAudioFormat(this, sound_format, sound_rate, sound_size, sound_type);
            }

        } else if (aac_type == AAC_RAW) {
            // Callback for Audio Data
            if (mpRtmpClientCallback) {
                mpRtmpClientCallback->OnRecvAudioFrame(this, sound_format, sound_rate, sound_size, sound_type, pStart, playload_size, timestamp);
            }
        }
    }

    return bFlag;
}

bool RtmpClient::Flv2Video(char *frame, int frame_size, u_int32_t timestamp) {
    bool bFlag = false;

    if (frame_size > 1) {
        VideoFrameFormat video_format = (VideoFrameFormat)srs_utils_flv_video_codec_id(frame, frame_size);
        switch (video_format) {
            case VVF_AVC: {
                // H264
                bFlag = FlvVideo2H264(frame, frame_size, timestamp);

            } break;
            default:
                break;
        }
    }

    return bFlag;
}

bool RtmpClient::FlvVideo2H264(char *frame, int frame_size, u_int32_t timestamp) {
    bool bFlag = true;

    char *pStart = frame;
    char *pEnd = frame + frame_size;

    if (
        srs_utils_flv_video_frame_type(frame, frame_size) == 0x01 &&  // Key frame
        srs_utils_flv_video_codec_id(frame, frame_size) == 0x07 &&    // H264
        srs_utils_flv_video_avc_packet_type(frame, frame_size) == 0x0 // SPS/PPS
        ) {
        pStart = frame + 2;
        int cfgVer = pStart[3];
        if (cfgVer == 1) {
            int spsIndex = 0;
            int spsLen = 0;

            int ppsIndex = 0;
            int ppsLen = 0;

            mNaluHeaderSize = (pStart[7] & 0x03) + 1; // 4

            spsIndex = pStart[8] & 0x1f; // 1
            pStart += 9;

            for (int i = 0; i < spsIndex; i++) {
                spsLen = ntohs(*(unsigned short *)pStart);
                pStart += 2;

                if (spsLen > pEnd - pStart) {
                    bFlag = false;
                    break;
                }

                bool bSpsChange = true;
                if (mpSps != NULL) {
                    if (mSpsSize == spsLen) {
                        if (0 != memcmp(mpSps, pStart, spsLen)) {
                            bSpsChange = true;

                        } else {
                            bSpsChange = false;
                        }
                    }
                }

                if (bSpsChange) {
                    if (mpSps) {
                        delete[] mpSps;
                        mpSps = NULL;
                    }

                    mSpsSize = spsLen;
                    mpSps = new char[mSpsSize];
                    memcpy(mpSps, pStart, mSpsSize);
                }

                pStart += spsLen;
            }

            ppsIndex = pStart[0];
            pStart += 1;
            for (int i = 0; i < ppsIndex; i++) {
                ppsLen = ntohs(*(unsigned short *)pStart);
                pStart += 2;
                if (ppsLen > pEnd - pStart) {
                    bFlag = false;
                    break;
                }

                bool bPpsChange = true;
                if (mpPps != NULL) {
                    if (mPpsSize == ppsLen) {
                        if (0 != memcmp(mpPps, pStart, spsLen)) {
                            bPpsChange = true;

                        } else {
                            bPpsChange = false;
                        }
                    }
                }

                if (bPpsChange) {
                    if (mpPps) {
                        delete[] mpPps;
                        mpPps = NULL;
                    }

                    mPpsSize = ppsLen;
                    mpPps = new char[mPpsSize];
                    memcpy(mpPps, pStart, mPpsSize);
                }

                pStart += ppsLen;
            }

            if (mpRtmpClientCallback) {
                mpRtmpClientCallback->OnChangeVideoSpsPps(this, mpSps, mSpsSize, mpPps, mPpsSize, mNaluHeaderSize, timestamp);
            }

        } else {
            // UnSupport version
            bFlag = false;
        }

    } else if (
        (
            srs_utils_flv_video_frame_type(frame, frame_size) == 0x01 || // Key frame
            srs_utils_flv_video_frame_type(frame, frame_size) == 0x02    // Inter frame
            ) &&
        srs_utils_flv_video_codec_id(frame, frame_size) == 0x07 &&    // H264
        srs_utils_flv_video_avc_packet_type(frame, frame_size) == 0x1 // NALU
        ) {
        if (mpSps && mpPps && mNaluHeaderSize != 0) {
            // Skip FLV video tag
            pStart = frame + 5;

            // Callback for Video
            if (mpRtmpClientCallback) {
                VideoFrameType type = VFT_UNKNOWN;
                if (srs_utils_flv_video_frame_type(frame, frame_size) == 0x01) {
                    type = VFT_IDR;
                } else if (srs_utils_flv_video_frame_type(frame, frame_size) == 0x02) {
                    type = VFT_NOTIDR;
                }

                mpRtmpClientCallback->OnRecvVideoFrame(this, pStart, (int)(pEnd - pStart), timestamp, type);
            }
        }
    }

    return bFlag;
}

u_int32_t RtmpClient::GetCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (u_int32_t)(tv.tv_sec * 1000) + (u_int32_t)(tv.tv_usec / 1000);
}

void RtmpClient::CheckConnectRunnableHandle() {
    bool bBreak = false;
    long long startTime = (long long)getCurrentTime();

    LogAync(
			LOG_STAT,
			"RtmpClient::CheckConnectRunnableHandle( "
			"this : %p, "
			"[Start] "
			")",
			this);

    while (mbRunning) {
    	mConnectedMutex.lock();
        if (mIsConnected) {
            // 已经连接上服务器, 标记退出线程
            bBreak = true;
        } else {
            // 计算超时
            long long curTime = (long long)getCurrentTime();
            int diffTime = (int)(curTime - startTime);
            if (diffTime >= mConnectTimeout) {
                // 超时, 断开连接
            	LogAync(
            			LOG_STAT,
						"RtmpClient::CheckConnectRunnableHandle( "
						"this : %p, "
						"[Shutdown for connect timeout] "
						")",
						this);

                if (mpRtmp) {
                    srs_rtmp_shutdown(mpRtmp);
                }
                bBreak = true;
            }
        }
        mConnectedMutex.unlock();

        if (bBreak) {
            break;
        }

        Sleep(100);
    }

    LogAync(
    		LOG_STAT,
			"RtmpClient::CheckConnectRunnableHandle( "
			"this : %p, "
			"[Exit] "
			")",
			this);
}

void RtmpClient::RecvCmd(char *frame, int frame_size, u_int32_t timestamp) {
    int index = 0;
    int last = frame_size;
    int size = 0;
    
    srs_amf0_t cmd = srs_amf0_parse(frame, last, &size);
    index += size;
    last -= size;
    if (srs_amf0_is_string(cmd)) {
//        const char *cmdString = srs_amf0_to_string(cmd);
    }
    srs_amf0_free(cmd);
}
}
