/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "core/components_ng/render/polygon_painter.h"

#include "core/components_ng/pattern/shape/polygon_paint_property.h"
#include "core/components_ng/pattern/shape/shape_paint_property.h"
#include "core/components_ng/render/drawing.h"
#include "core/components_ng/render/drawing_prop_convertor.h"
#include "core/components_ng/render/shape_painter.h"

namespace OHOS::Ace::NG {
void PolygonPainter::DrawPolygon(RSCanvas& canvas, const PolygonPaintProperty& polygonPaintProperty, bool isClose)
{
    if (!polygonPaintProperty.HasPoints()) {
        return;
    }
    RSPen pen;
    RSBrush brush;
    ShapePainter::SetPan(pen, polygonPaintProperty);
    ShapePainter::SetBrush(brush, polygonPaintProperty);
    canvas.AttachPen(pen);
    canvas.AttachBrush(brush);
    RSPath path;
    std::vector<RSPoint> points;
    for (auto point : polygonPaintProperty.GetPointsValue()) {
        points.emplace_back(RSPoint(
            static_cast<RSScalar>(point.first.ConvertToPx()), static_cast<RSScalar>(point.second.ConvertToPx())));
    }
    if (points.empty()) {
        return;
    }
    path.AddPoly(points, static_cast<int>(points.size()), isClose);
    canvas.DrawPath(path);
}
} // namespace OHOS::Ace::NG