/* This file is part of the KDE project
 * Copyright (C) 2014 Dan Leinir Turthra Jensen <admin@leinir.dk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

import QtQuick 1.1

Item {
    id: base;
    height: toolOverlayContainer.height;

    function toolIDToName(toolID) {
        var names = {
            "KritaShape/KisToolBrush" : "paint",
            "KritaFill/KisToolFill" : "fill",
            "KritaFill/KisToolGradient" : "gradient",
            "KritaTransform/KisToolMove" : "move",
            "KisToolTransform" : "transform",
            "KisToolCrop" : "crop"
        };
        return names[toolID];
    }

    Loader {
        id: toolOverlayContainer;
        y: height > 0 ? 0 : Constants.DefaultMargin * 2;
        Behavior on y { PropertyAnimation { duration: 150; } }
        width: parent.width;
        height: item.childrenRect.height;
        source: "tooloverlays/none.qml";
        onStatusChanged: {
            if(status === Loader.Error) {
                source = "tooloverlays/none.qml";
            }
        }
    }
    Connections {
        target: toolManager;
        onCurrentToolChanged: toolOverlayContainer.source = "tooloverlays/" + toolIDToName(toolManager.currentTool.toolId()) + ".qml"
    }
}
