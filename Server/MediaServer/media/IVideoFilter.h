/*
 * IVideoFilter.h
 *
 *  Created on: 2017年8月7日
 *      Author: max
 */

#ifndef RTMPDUMP_IVIDEOFILTER_H_
#define RTMPDUMP_IVIDEOFILTER_H_

namespace qpidnetwork {
class VideoFrame;
class VideoFilter {
public:
	virtual ~VideoFilter(){};

	virtual bool FilterFrame(VideoFrame* srcFrame, VideoFrame* dstFrame) = 0;
};
} /* namespace qpidnetwork */

#endif /* RTMPDUMP_IVIDEOFILTER_H_ */
