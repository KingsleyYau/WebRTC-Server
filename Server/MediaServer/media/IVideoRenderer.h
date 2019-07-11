//
//  IVideoRenderer.h
//  RtmpClient
//
//  Created by Max on 2017/6/14.
//  Copyright © 2017年 net.qdating. All rights reserved.
//

#ifndef IVideoRenderer_h
#define IVideoRenderer_h

namespace mediaserver {
class VideoRenderer {
public:
    virtual ~VideoRenderer(){};
    virtual void RenderVideoFrame(void* frame) = 0;
};
} /* namespace mediaserver */
#endif /* IVideoPlayer_h */
