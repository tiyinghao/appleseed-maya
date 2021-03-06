
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

// Interface header.
#include "appleseedmaya/exporters/lightexporter.h"

// Standard headers.
#include <sstream>

// Maya headers.
#include <maya/MFnDagNode.h>

// appleseed.foundation headers.
#include "foundation/math/scalar.h"

// appleseed.renderer headers.
#include "renderer/api/scene.h"

// appleseed.maya headers.
#include "appleseedmaya/attributeutils.h"
#include "appleseedmaya/exporters/exporterfactory.h"

namespace asf = foundation;
namespace asr = renderer;

void LightExporter::registerExporter()
{
    NodeExporterFactory::registerDagNodeExporter("directionalLight", &LightExporter::create);
    NodeExporterFactory::registerDagNodeExporter("pointLight", &LightExporter::create);
    NodeExporterFactory::registerDagNodeExporter("spotLight", &LightExporter::create);
}

DagNodeExporter *LightExporter::create(
    const MDagPath&                 path,
    asr::Project&                   project,
    AppleseedSession::SessionMode   sessionMode)
{
    return new LightExporter(path, project, sessionMode);
}

LightExporter::LightExporter(
    const MDagPath&                 path,
    asr::Project&                   project,
    AppleseedSession::SessionMode   sessionMode)
  : DagNodeExporter(path, project, sessionMode)
{
}

LightExporter::~LightExporter()
{
    if (sessionMode() == AppleseedSession::ProgressiveRenderSession)
    {
        mainAssembly().colors().remove(m_lightColor.get());
        mainAssembly().lights().remove(m_light.get());
    }
}

bool LightExporter::supportsMotionBlur() const
{
    return false;
}

void LightExporter::createEntities(const AppleseedSession::Options& options)
{
    asr::LightFactoryRegistrar lightFactories;
    const asr::ILightFactory *lightFactory = 0;
    asr::ParamArray lightParams;

    MFnDependencyNode depNodeFn(node());

    MStatus status;
    MPlug plug = depNodeFn.findPlug("intensity", &status);
    if (plug.isConnected())
    {
        // todo: add warning here...
    }

    float intensity = 1.0;
    AttributeUtils::get(plug, intensity);

    plug = depNodeFn.findPlug("color", &status);
    if (plug.isConnected())
    {
        // todo: add warning here...
    }

    // Create a color entity.
    MString colorName = appleseedName() + MString("_intensity_color");
    {
        MColor color(1.0f, 1.0f, 1.0f);
        AttributeUtils::get(plug, color);
        asr::ColorValueArray values(3, &color.r);

        asr::ParamArray params;
        params.insert("color_space", "linear_rgb");
        m_lightColor.reset(
            asr::ColorEntityFactory::create(colorName.asChar(), params, values));
    }

    if (depNodeFn.typeName() == "directionalLight")
    {
        lightFactory = lightFactories.lookup("directional_light");
        lightParams.insert("irradiance", colorName.asChar());
        lightParams.insert("irradiance_multiplier", intensity);
    }
    else if (depNodeFn.typeName() == "pointLight")
    {
        lightFactory = lightFactories.lookup("point_light");
        lightParams.insert("intensity", colorName.asChar());
        lightParams.insert("intensity_multiplier", intensity);
    }
    else if (depNodeFn.typeName() == "spotLight")
    {
        lightFactory = lightFactories.lookup("spot_light");
        lightParams.insert("intensity", colorName.asChar());
        lightParams.insert("intensity_multiplier", intensity);

        MAngle coneAngle(20.0f, MAngle::kDegrees);
        AttributeUtils::get(node(), "coneAngle", coneAngle);
        lightParams.insert("inner_angle", coneAngle.asDegrees());

        MAngle penumbraAngle(5.0f, MAngle::kDegrees);
        AttributeUtils::get(node(), "penumbraAngle", penumbraAngle);
        const float outerAngle = coneAngle.asDegrees() + 2.0 * penumbraAngle.asDegrees();
        lightParams.insert("outer_angle", outerAngle);
    }
    else
    {
        RENDERER_LOG_WARNING(
            "Unsupported light type %s found. Skipping",
            depNodeFn.typeName().asChar());
        return;
    }

    m_light = lightFactory->create(appleseedName().asChar(), lightParams);
    asf::Matrix4d m = convert(dagPath().inclusiveMatrix());
    asf::Matrix4d invM = convert(dagPath().inclusiveMatrixInverse());
    asf::Transformd xform(m, invM);
    m_light->set_transform(xform);
}

void LightExporter::flushEntities()
{
    if (m_light.get())
    {
        mainAssembly().colors().insert(m_lightColor.release());
        mainAssembly().lights().insert(m_light.release());
    }
}
