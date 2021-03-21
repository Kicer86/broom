
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQml.Models 2.15
import photo_broom.qml 1.0
import "../Components" as Components

Item {

    SystemPalette { id: currentPalette; colorGroup: SystemPalette.Active }

    // Model working directly on database
    PhotosDataGuesser {
        id: dataSource
        database: PhotoBroomProject.database
    }

    ListView {
        id: listView

        clip: true
        anchors.fill: parent

        spacing: 2
        highlightMoveDuration: 100
        highlightMoveVelocity: -1
        model: dataSource

        delegate: Item {
            required property var photoPath
            required property int index

            // readonly property string guessedInformation: photoDataComplete(photoData)

            width: listView.width
            height: 60

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    listView.currentIndex = index
                }
            }

            Row {
                CheckBox {
                    // checkState: guessedInformation == ""? Qt.Unchecked: Qt.Checked
                }

                Components.PhotoThumbnail {

                    width: 60
                    height: 60

                    source: photoPath
                }

                Column {
                    Text {
                        text: photoPath
                    }

                    Text {
                        //text: guessedInformation
                    }
                }
            }
        }

        highlight: Rectangle {
            color: currentPalette.highlight
            radius: 5
        }

        ScrollBar.vertical: ScrollBar {}
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
