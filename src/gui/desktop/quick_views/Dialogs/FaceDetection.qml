
import QtQuick 2.15
import "../Components" as Components

Item {
    GridView {
        id: guessedFaces
        height: cellHeight + 30
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
    }

    GridView {
        id: unknownFaces
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: guessedFaces.bottom
        anchors.bottom: parent.bottom
        delegate: Item {
            x: 5
            height: 50
            Column {
                Rectangle {
                    width: 40
                    height: 40
                    color: colorCode
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    x: 5
                    text: name
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                spacing: 5
            }
        }
        model: ListModel {
            ListElement {
                name: "Grey"
                colorCode: "grey"
            }

            ListElement {
                name: "Red"
                colorCode: "red"
            }

            ListElement {
                name: "Blue"
                colorCode: "blue"
            }

            ListElement {
                name: "Green"
                colorCode: "green"
            }
        }
        cellWidth: 70
        cellHeight: 70
    }

    Component.ThumbnailSlider {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }


}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}D{i:2}
}
##^##*/
