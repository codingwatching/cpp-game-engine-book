//
// Created by captainchen on 2022/8/19.
//

#include "render_texture.h"
#include "render_device/gpu_resource_mapper.h"
#include "render_device/render_task_producer.h"

RenderTexture::RenderTexture(): width_(128), height_(128), frame_buffer_object_handle_(0),in_use_(false) {
}

RenderTexture::~RenderTexture() {
    if(frame_buffer_object_handle_>0){
        RenderTaskProducer::ProduceRenderTaskDeleteFBO(frame_buffer_object_handle_);
    }
}

void RenderTexture::Init(unsigned short width, unsigned short height) {
    width_=width;
    height_=height;
    //创建FBO任务
    frame_buffer_object_handle_ = GPUResourceMapper::GenerateFBOHandle();
    RenderTaskProducer::ProduceRenderTaskCreateFBO(frame_buffer_object_handle_,width_,height_);
}
