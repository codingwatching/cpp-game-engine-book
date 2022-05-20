﻿//
// Created by captain on 2021/10/14.
//

#ifndef UNTITLED_UI_IMAGE_H
#define UNTITLED_UI_IMAGE_H

#include "component/component.h"
#include "renderer/mesh_filter.h"

class Texture2D;
class UIImage : public Component {
public:
    UIImage();
    ~UIImage() override;

    Texture2D* texture2D(){return texture2D_;}
    void set_texture(Texture2D* texture2D){texture2D_=texture2D;}

public:
    void Update() override;
    /// 渲染之前
    void OnPreRender() override;

    void OnPostRender() override;
private:
    Texture2D* texture2D_= nullptr;//Texture

};


#endif //UNTITLED_UI_IMAGE_H
