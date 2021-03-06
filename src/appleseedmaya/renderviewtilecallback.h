
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2016-2017 Esteban Tovagliari, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef APPLESEED_MAYA_RENDERVIEW_TILECALLBACK_H
#define APPLESEED_MAYA_RENDERVIEW_TILECALLBACK_H

// Standard headers.
#include <cstddef>

// Maya headers.
#include <maya/MComputation.h>

// appleseed.foundation headers.
#include "foundation/math/aabb.h"

// appleseed.renderer headers.
#include "renderer/api/rendering.h"

// appleseed.maya headers.
#include "appleseedmaya/renderercontroller.h"
#include "appleseedmaya/utils.h"

// Forward declarations.
namespace foundation    { class Tile; }
namespace renderer      { class Frame; }


class RenderViewTileCallbackFactory
  : public renderer::ITileCallbackFactory
{
  public:

    RenderViewTileCallbackFactory(
        RendererController& rendererController,
        ComputationPtr      computation);

    virtual ~RenderViewTileCallbackFactory();

    virtual void release();

    virtual renderer::ITileCallback* create();

    void renderViewStart(const renderer::Frame& frame);

  private:
    RendererController& m_rendererController;
    ComputationPtr      m_computation;
    foundation::AABB2i  m_displayWindow;
    foundation::AABB2i  m_dataWindow;
};

#endif  // !APPLESEED_MAYA_RENDERVIEW_TILECALLBACK_H
