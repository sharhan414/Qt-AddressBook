import QtQuick 2.0
import StyleModule 1.0
import QtGraphicalEffects 1.0

Rectangle {
    id: root
    property alias image: _internalImage

    property real boundingRadius
    property bool recolorIcon: true

    width: boundingRadius
    height: boundingRadius

    color: "transparent"
    radius: width / 2
    border.width: 2
    border.color: Style.primaryColor

    Image {
        id: _internalImage
        anchors.fill: root
        anchors.margins: Style.mediumOffset
        sourceSize.height: height
        sourceSize.width: width
    }

    ColorOverlay {
        visible: root.recolorIcon
        anchors.fill: _internalImage
        source: _internalImage
        color: Style.themeInvertedColor
        antialiasing: true
    }
}
