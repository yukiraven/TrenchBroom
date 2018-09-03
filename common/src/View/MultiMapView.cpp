/*
 Copyright (C) 2010-2017 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "MultiMapView.h"

#include "View/MapView.h"
#include "View/MapViewBase.h"
#include "Renderer/Camera.h"

#include <cassert>
#include <limits>

namespace TrenchBroom {
    namespace View {
        MultiMapView::MultiMapView(wxWindow* parent) :
        MapViewContainer(parent),
        m_maximizedView(nullptr) {}
        
        MultiMapView::~MultiMapView() {}

        void MultiMapView::addMapView(MapView* mapView) {
            ensure(mapView != nullptr, "mapView is nullptr");
            m_mapViews.push_back(mapView);
        }

        void MultiMapView::doFlashSelection() {
            for (MapView* mapView : m_mapViews)
                mapView->flashSelection();
        }

        bool MultiMapView::doGetIsCurrent() const {
            for (MapView* mapView : m_mapViews) {
                if (mapView->isCurrent())
                    return true;
            }
            return false;
        }

        void MultiMapView::doSetToolBoxDropTarget() {
            for (MapView* mapView : m_mapViews)
                mapView->setToolBoxDropTarget();
        }
        
        void MultiMapView::doClearDropTarget() {
            for (MapView* mapView : m_mapViews)
                mapView->clearDropTarget();
        }
        
        bool MultiMapView::doCanSelectTall() {
            if (currentMapView() == nullptr)
                return false;
            return currentMapView()->canSelectTall();
        }
        
        void MultiMapView::doSelectTall() {
            if (currentMapView() != nullptr)
                currentMapView()->selectTall();
        }

        void MultiMapView::doFocusCameraOnSelection(const bool animate) {
            for (MapView* mapView : m_mapViews)
                mapView->focusCameraOnSelection(animate);
        }
        
        void MultiMapView::doMoveCameraToPosition(const Vec3& position, const bool animate) {
            for (MapView* mapView : m_mapViews)
                mapView->moveCameraToPosition(position, animate);
        }
        
        void MultiMapView::doMoveCameraToCurrentTracePoint() {
            for (MapView* mapView : m_mapViews)
                mapView->moveCameraToCurrentTracePoint();
        }

        bool MultiMapView::doCanMaximizeCurrentView() const {
            return m_maximizedView != nullptr || currentMapView() != nullptr;
        }
        
        bool MultiMapView::doCurrentViewMaximized() const {
            return m_maximizedView != nullptr;
        }
        
        void MultiMapView::doToggleMaximizeCurrentView() {
            if (m_maximizedView != nullptr) {
                doRestoreViews();
                m_maximizedView = nullptr;
            } else {
                m_maximizedView = currentMapView();
                doMaximizeView(m_maximizedView);
            }
        }

        MapView* MultiMapView::doGetCurrentMapView() const {
            for (MapView* mapView : m_mapViews) {
                if (mapView->isCurrent())
                    return mapView;
            }
            return nullptr;
        }

        bool MultiMapView::doCancelMouseDrag() {
            bool result = false;
            for (MapView* mapView : m_mapViews)
                result |= mapView->cancelMouseDrag();
            return result;
        }

        BBox3 importantBoundsForMapViews(const std::vector<MapViewBase*>& views) {
            BBox3f bounds{Vec3f::fill(-std::numeric_limits<float>::infinity()),
                          Vec3f::fill(+std::numeric_limits<float>::infinity())};

            for (auto* view : views) {
                auto& camera = view->camera();

                Plane3f top, right, bottom, left;
                camera.frustumPlanes(top, right, bottom, left);

                // these are facing away from the viewport
                for (auto& plane : {top, bottom, left, right}) {
                    // skip non-axial planes
                    if (plane.normal.firstAxis() != plane.normal) {
                        std::cout << "skipping non-axial\n";
                        continue;
                    }

                    // clip `bounds` by `plane`
                    const size_t comp = plane.normal.firstComponent();
                    const auto axisSign = plane.normal.firstAxis()[comp];
                    if (axisSign < 0) {
                        if (std::isinf(bounds.min[comp])) {
                            bounds.min[comp] = axisSign * plane.distance;
                        } else {
                            bounds.min[comp] = std::min(bounds.min[comp], axisSign * plane.distance);
                        }
                    } else {
                        if (std::isinf(bounds.max[comp])) {
                            bounds.max[comp] = axisSign * plane.distance;
                        } else {
                            bounds.max[comp] = std::max(bounds.max[comp], axisSign * plane.distance);
                        }
                    }
                }
            }

            std::cout << "important bounds for " << views.size() << " views: " << bounds << "\n";
            return BBox3(Vec3(bounds.min), Vec3(bounds.max));
        }
    }
}
