
import QtQuick 2.14
import QtQuick.Controls 2.3


Item {

    width: childrenRect.width
    height: childrenRect.height

    property alias from: timeSliderId.from
    property alias to: timeSliderId.to

    Row {

        Text {
            text: qsTr("Time range:");
        }

        RangeSlider {
            id: timeSliderId

            from: 0
            to: new Date().getTime()
        }

        Text {
            property var from: timeSliderId.first.value
            property var to: timeSliderId.second.value

            text: Qt.formatDate(new Date(from), Qt.ISODate) + " - " + Qt.formatDate(new Date(to), Qt.ISODate)
        }
    }

}